/*!
 * @file seqdriver.cpp
 * @brief Implementation of the SeqDriver class
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 */
#include <cstdio>
#include <QString>
#include <alsa/asoundlib.h>

#include "seqdriver.h"
#include "config.h"


SeqDriver::SeqDriver(
    JackDriver *p_jackSync,
    int p_portCount,
    void * callback_context,
    bool (* midi_event_received_callback)(void * context, MidiEvent ev),
    void (* tick_callback)(void * context, bool echo_from_trig))
    : DriverBase(callback_context, midi_event_received_callback, tick_callback, 60e9)
    , jackSync(p_jackSync)
{
    int err;
    char buf[16];
    int l1;

    /* Register ALSA client */
    err = snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) {
        qWarning("Error opening ALSA sequencer (%s).", snd_strerror(err));
        exit(1);
    }

    snd_seq_set_client_name(seq_handle, PACKAGE);
    clientid = snd_seq_client_id(seq_handle);

    /* Register ALSA input port */
    portid_in = snd_seq_create_simple_port(seq_handle, "in",
                    SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                    SND_SEQ_PORT_TYPE_APPLICATION);
    if (portid_in < 0) {
        qWarning("Error creating sequencer port (%s).",
                snd_strerror(portid_in));
        exit(1);
    }

    /* Setup ALSA sequencer queue */
    snd_seq_set_client_pool_output(seq_handle, SEQPOOL);
    queue_id = snd_seq_alloc_queue(seq_handle);

    /* Register ALSA output ports */
    portCount = p_portCount;
    for (l1 = 0; l1 < portCount; l1++) {
        snprintf(buf, sizeof(buf), "out %d", l1 + 1);
        portid_out[l1] = snd_seq_create_simple_port(seq_handle, buf,
                SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                SND_SEQ_PORT_TYPE_APPLICATION);
        if (portid_out[l1] < 0) {
            qWarning("Error creating sequencer port (%s).",
                    snd_strerror(portid_out[l1]));
            exit(1);
        }
    }
    // prepare mem for note removal at queue stop here to avoid malloc
    // during call via jack transport
    snd_seq_remove_events_malloc(&remove_ev);

    portUnmatched = 0;
    forwardUnmatched = false;

    lastSchedTick = 0;
    m_current_tick = 0;

    queueStatus = false;
    startQueue = false;
    useJackSync = false;
    useMidiClock = false;
    midiTick = 0;
    lastRatioTick = 0;
    tempoChangeTick = 0;
    midiTempoRefreshTick = 0;
    tempoChangeFrame = 0;
    requestedTempo = 120;
    internalTempo = 120;
    initTempo();
    clockRatio = 60e9/TPQN/tempo;
    tempoChangeTime = 0;

    threadAbort = false;
    start(Priority(6));
}

SeqDriver::~SeqDriver(){

    if (useJackSync) setUseJackTransport(false);

    threadAbort = true;
    wait();
    snd_seq_remove_events_free(remove_ev);

}

void SeqDriver::run()
{
    snd_seq_event_t *evIn;
    bool unmatched = true;
    MidiEvent inEv;
    int pollr = 0;

    int nfds;
    struct pollfd *pfds;

    nfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfds = (struct pollfd *) alloca(nfds * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfds, nfds, POLLIN);

    while (((long)poll >= 0) && (!threadAbort)) {

        pollr = poll(pfds, nfds, 200);
        while (pollr > 0) {

            snd_seq_event_input(seq_handle, &evIn);

            inEv.type = evIn->type;
            inEv.data = evIn->data.note.note;
            inEv.channel = 0;
            inEv.value = 0;

            if ((inEv.type == EV_CLOCK)&& useMidiClock) {
                calcCurrentTick(getCurrentTime());
                midiTick++;
            }
            if (((inEv.type == EV_ECHO) || startQueue) && queueStatus) {
                startQueue = false;
                unmatched = false;
                calcCurrentTick(aTimeToDelta(&evIn->time.time));
                tick_callback((inEv.data));
            }
            else {
                unmatched = true;
                inEv.channel = evIn->data.control.channel;

                if ((inEv.type == EV_NOTEON) || (inEv.type == EV_NOTEOFF)) {
                    inEv.data = evIn->data.note.note;
                    if (inEv.type == EV_NOTEON) {
                        inEv.value = evIn->data.note.velocity;
                    }
                    else {
                        inEv.value = 0;
                        inEv.type = EV_NOTEON;
                    }
                    calcCurrentTick(aTimeToDelta(&evIn->time.time));
                }
                else inEv.value = evIn->data.control.value;

                if (inEv.type == EV_CONTROLLER) {
                    inEv.data = evIn->data.control.param;
                    calcCurrentTick(aTimeToDelta(&evIn->time.time));
                }

                unmatched = midi_event_received(inEv);

                if (forwardUnmatched && unmatched) {
                    snd_seq_ev_set_subs(evIn);
                    snd_seq_ev_set_direct(evIn);
                    snd_seq_ev_set_source(evIn, portid_out[portUnmatched]);
                    snd_seq_event_output_direct(seq_handle, evIn);
                }
            }
            if (!queueStatus) m_current_tick = 0; //some events still come in after queue stop
            pollr = snd_seq_event_input_pending(seq_handle, 0);
        }
    }
}

void SeqDriver::calcCurrentTick(double tmpTime) {

    if (useMidiClock) {
        m_current_tick = midiTick * TPQN / MIDICLK_TPQN;
        if (midiTick > lastRatioTick + 4 || !midiTick) {
            calcClockRatio(tmpTime);
            lastRatioTick = midiTick;
            internalTempo = (60e9/TPQN/clockRatio);
        }
        if (midiTick > midiTempoRefreshTick + 48) {
            tempoChangeTick = m_current_tick;
            tempoChangeTime = tmpTime;
            midiTempoRefreshTick = midiTick;
            jackSync->tempoCb(internalTempo, jackSync->cbContext);
        }
    }
    else if (useJackSync) {
        jPos = jackSync->getCurrentPos();
        if (jPos.beats_per_minute > 0) requestedTempo = jPos.beats_per_minute;

        m_current_tick = (long)(jPos.frame - tempoChangeFrame) * TPQN  * tempo
                / jPos.frame_rate / 60.
                + tempoChangeTick;
        calcClockRatio(tmpTime);
    }
    else {
        m_current_tick = deltaToTick(tmpTime);
        clockRatio = 60e9/TPQN/tempo;
    }
}

void SeqDriver::initTempo()
{
    if (useJackSync) {
        jPos = jackSync->getCurrentPos();
        if (jPos.beats_per_minute > 0)
            tempo = jPos.beats_per_minute;
        else
            tempo = internalTempo;

        tempoChangeFrame = (uint64_t)jPos.frame;
    }
    else {
        tempo = internalTempo;
    }

    clockRatio = 60e9/TPQN/tempo;

    if (useMidiClock) {
        midiTick = 0;
        lastRatioTick = 0;
        midiTempoRefreshTick = 0;
    }
}

void SeqDriver::sendMidiEvent(MidiEvent outEv, int n_tick, unsigned outport, unsigned length)
{
    //~ qWarning("sendMidiEvent([%d, %d, %d, %d], %u, %u) at tick %d", ev.type, ev.channel, ev.data, ev.value, outport, duration, n_tick);
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);

    ev.type = outEv.type;
    if (ev.type == EV_NOTEON) {
        snd_seq_ev_set_note(&ev, 0, outEv.data, outEv.value, length);
    }
    else if (ev.type == EV_CONTROLLER) {
        ev.data.control.param = outEv.data;
        ev.data.control.value = outEv.value;
    }

    ev.data.control.channel = outEv.channel;
    snd_seq_ev_schedule_real(&ev, queue_id, 0, deltaToATime(tickToDelta(n_tick)));
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_source(&ev, portid_out[outport]);
    snd_seq_event_output_direct(seq_handle, &ev);
}

bool SeqDriver::requestEchoAt(int echo_tick, bool echo_from_trig)
{
    if ((echo_tick == lastSchedTick) && (echo_tick)) return false;

    lastSchedTick = echo_tick;
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    ev.type = SND_SEQ_EVENT_ECHO;
    ev.data.note.note = echo_from_trig;
    snd_seq_ev_schedule_real(&ev, queue_id,  0, deltaToATime(tickToDelta(echo_tick)));
    snd_seq_ev_set_dest(&ev, clientid, portid_in);
    snd_seq_event_output_direct(seq_handle, &ev);
    return true;
}

void SeqDriver::requestTempo(double bpm)
{
    double tmpTime = getCurrentTime();
    calcCurrentTick(tmpTime);
    tempoChangeTick = m_current_tick;
    tempoChangeTime = tmpTime;
    internalTempo = bpm;
    initTempo();
    requestedTempo = bpm;
    requestEchoAt(lastSchedTick + 1);
}

void SeqDriver::setTempo(double bpm)
{
    tempoChangeTick = m_current_tick;
    tempoChangeTime = getCurrentTime();
    internalTempo = bpm;
    initTempo();
}

double SeqDriver::getCurrentTime()
{
    snd_seq_queue_status_t *status;

    snd_seq_queue_status_malloc(&status);
    snd_seq_get_queue_status(seq_handle, queue_id, status);

    const snd_seq_real_time_t* current_time =
        snd_seq_queue_status_get_real_time(status);
    snd_seq_real_time_t tmpTime = *current_time;
    snd_seq_queue_status_free(status);

    return aTimeToDelta(&tmpTime);
}

void SeqDriver::setTransportStatus(bool run)
{
    if (run) {
        tempoChangeTick = 0;
        tempoChangeTime = 0;
        queueStatus = true;
        startQueue = true;

        initTempo();

        snd_seq_start_queue(seq_handle, queue_id, NULL);
        snd_seq_drain_output(seq_handle);

        requestEchoAt(0);

        printf("Alsa Queue started \n");
    }
    else {
        queueStatus = false;
        snd_seq_remove_events_set_queue(remove_ev, queue_id);
        snd_seq_remove_events_set_condition(remove_ev,
                SND_SEQ_REMOVE_OUTPUT | SND_SEQ_REMOVE_IGNORE_OFF);
        snd_seq_remove_events(seq_handle, remove_ev);

        snd_seq_stop_queue(seq_handle, queue_id, NULL);

        m_current_tick = 0;

        printf("Alsa Queue stopped \n");
    }
}

void SeqDriver::setUseMidiClock(bool on)
{
    clockRatio = 60e9/TPQN/tempo;
    useMidiClock = on;
    if (!on) jackSync->tempoCb(internalTempo, jackSync->cbContext);
}

double SeqDriver::tickToDelta(int tick)
{
    if (tick > tempoChangeTick)
        return (double)clockRatio * (tick-tempoChangeTick) + tempoChangeTime;
    else
        return 0;
}

int SeqDriver::deltaToTick(double curtime)
{
    if (tempoChangeTime < curtime)
        return (int)((curtime-tempoChangeTime) / clockRatio)+tempoChangeTick;
    else
        return tempoChangeTick;
}

double SeqDriver::aTimeToDelta(snd_seq_real_time_t* atime)
{
    return (double)atime->tv_sec * 1e9 + (double)atime->tv_nsec;
}

const snd_seq_real_time_t* SeqDriver::deltaToATime(double curtime)
{
    atime.tv_sec = (int)(curtime * 1e-9);
    atime.tv_nsec = (long)(curtime - (double)atime.tv_sec * 1e9);
    return &atime;
}

void SeqDriver::calcClockRatio(double realtime)
{
    double old_clock_ratio = clockRatio;

    if (m_current_tick > 0) {
        clockRatio = (realtime - tempoChangeTime)/(m_current_tick - tempoChangeTick);
    }
    if ((clockRatio == 0) || (clockRatio > 60e9 / tempo)) {
        clockRatio = old_clock_ratio;
    }
    clockRatio+=old_clock_ratio;
    clockRatio*=.5;
}

int SeqDriver::getClientId()
{
    return clientid;
}

void SeqDriver::setUseJackTransport(bool on)
{
    bool failed = true;

    if (on) {
        failed = callJack(0);
        if (!failed) {
            jackSync->useJackSync = true;
            useJackSync = true;
        }
    }
    else {
        jackSync->callJack(-1);
        useJackSync = false;
    }
}

bool SeqDriver::callJack(int portcount)
{
    return jackSync->callJack(portcount);
}
