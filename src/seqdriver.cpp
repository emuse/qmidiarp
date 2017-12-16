/*!
 * @file seqdriver.cpp
 * @brief Implementation of the SeqDriver class
 *
 *
 *      Copyright 2009 - 2017 <qmidiarp-devel@lists.sourceforge.net>
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
#include "config.h"

#ifdef HAVE_ALSA

#include <cstdio>
#include <QString>
#include <alsa/asoundlib.h>

#include "seqdriver.h"

SeqDriver::SeqDriver(
    JackDriver *p_jackSync,
    int p_portCount,
    void * callback_context,
    bool (* midi_event_received_callback)(void * context, MidiEvent ev),
    void (* tick_callback)(void * context, bool echo_from_trig))
    : DriverBase(p_portCount, callback_context, midi_event_received_callback, tick_callback, 60e9)
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

    startQueue = false;
    midiTick = 0;
    lastRatioTick = 0;
    tempoChangeTick = 0;
    midiTempoRefreshTick = 0;
    trStartingTick = 0;
    trLoopingTick = 0;
    tempoChangeFrame = 0;
    initTempo();
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
    double tmpTime = 0;
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

            tmpTime = getCurrentTime();

            snd_seq_event_input(seq_handle, &evIn);

            inEv.type = evIn->type;
            inEv.data = evIn->data.note.note;
            inEv.channel = 0;
            inEv.value = 0;

            if ((inEv.type == EV_CLOCK)&& useMidiClock) {
                m_current_tick = midiTick * TPQN / MIDICLK_TPQN;
                if (!(midiTick % 8) || !midiTick) {
                    calcMidiClockTempo(tmpTime);
                    internalTempo = tempo;
                }
                if ((midiTick % 48) == 4 ) {
                    tempoChangeTick = m_current_tick;
                    tempoChangeTime = tmpTime;
                    jackSync->tempoCb(internalTempo, jackSync->cbContext);
                }
                midiTick++;
            }
            if (((inEv.type == EV_ECHO) || startQueue) && queueStatus) {
                startQueue = false;
                calcCurrentTick(tmpTime);
                tick_callback((inEv.data));
            }
            else {
                inEv.channel = evIn->data.control.channel;

                if ((inEv.type == EV_NOTEON) || (inEv.type == EV_NOTEOFF)) {
                    inEv.data = evIn->data.note.note;
                    inEv.value = evIn->data.note.velocity;
                    calcCurrentTick(tmpTime);
                }
                else inEv.value = evIn->data.control.value;

                if (inEv.type == EV_CONTROLLER) {
                    inEv.data = evIn->data.control.param;
                    calcCurrentTick(tmpTime);
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

    if (useJackSync) {
        jPos = jackSync->getCurrentPos();
        if (jPos.beats_per_minute > 0.01) requestedTempo = jPos.beats_per_minute;

        m_current_tick = (uint64_t)(jPos.frame - tempoChangeFrame) * TPQN  * tempo
                / jPos.frame_rate / 60.
                + tempoChangeTick;

        tmpTime = tickToDelta(m_current_tick);
        snd_seq_event_t ev;
        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_queue_pos_real(&ev, queue_id, deltaToATime(tmpTime));
        snd_seq_ev_set_direct(&ev);
        snd_seq_event_output_direct(seq_handle, &ev);
    }
    else {
        m_current_tick = deltaToTick(tmpTime);
    }
}

void SeqDriver::initTempo()
{
    if (useJackSync) {
        jPos = jackSync->getCurrentPos();
        if (jPos.beats_per_minute > 0.01)
            tempo = jPos.beats_per_minute;
        else
            tempo = internalTempo;

        tempoChangeFrame = (uint64_t)jPos.frame;
    }
    else {
        tempo = internalTempo;
    }

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
        snd_seq_ev_set_note(&ev, 0, outEv.data, outEv.value, length * 80 / tempo);
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
        queueStatus = true;
        startQueue = true;

        initTempo();
        tempoChangeTick = 0;
        tempoChangeTime = 0;
        tempoChangeFrame = 0;
        if (useJackSync)
            trStartingTick = jackSync->trStartingTick;
        else
            trStartingTick = 0;
        snd_seq_start_queue(seq_handle, queue_id, NULL);
        snd_seq_drain_output(seq_handle);
        calcCurrentTick(0);
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
    useMidiClock = on;
    if (!on) jackSync->tempoCb(internalTempo, jackSync->cbContext);
}

double SeqDriver::tickToDelta(int tick)
{
    if (tick > tempoChangeTick)
        return (double)(60e9/TPQN/tempo) * (tick-tempoChangeTick) + tempoChangeTime;
    else
        return 0;
}

int SeqDriver::deltaToTick(double curtime)
{
    if (tempoChangeTime < curtime)
        return (int)((curtime-tempoChangeTime) / (60e9/TPQN/tempo))+tempoChangeTick;
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
    atime.tv_nsec = (uint64_t)(curtime - (double)atime.tv_sec * 1e9);
    return &atime;
}

void SeqDriver::calcMidiClockTempo(double realtime)
{
    double old_tempo = tempo;

    if (m_current_tick > 0) {
        tempo =   60e9
                * (double)(m_current_tick - tempoChangeTick)
                / (realtime - tempoChangeTime)
                / TPQN;
    }
    if ((tempo == 0) || (tempo > 1000.)) {
        tempo = old_tempo;
    }
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

bool SeqDriver::callJack(int portcount, const QString & clientname)
{
    return jackSync->callJack(portcount, clientname);
}
#endif
