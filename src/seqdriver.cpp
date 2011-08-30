/*!
 * @file seqdriver.cpp
 * @brief Implementation of the SeqDriver class
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
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

    /** Register ALSA client */
    err = snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) {
        qWarning("Error opening ALSA sequencer (%s).", snd_strerror(err));
        exit(1);
    }

    snd_seq_set_client_name(seq_handle, PACKAGE);
    clientid = snd_seq_client_id(seq_handle);

    /** Register ALSA input port */
    portid_in = snd_seq_create_simple_port(seq_handle, "in",
                    SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                    SND_SEQ_PORT_TYPE_APPLICATION);
    if (portid_in < 0) {
        qWarning("Error creating sequencer port (%s).",
                snd_strerror(portid_in));
        exit(1);
    }

    /** Setup ALSA sequencer queue */
    snd_seq_set_client_pool_output(seq_handle, SEQPOOL);
    queue_id = snd_seq_alloc_queue(seq_handle);

    /** Register ALSA output ports */
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

    portUnmatched = 0;
    forwardUnmatched = false;

    lastSchedTick = 0;
    m_current_tick = 0;
    jackOffsetTick = 0;

    queueStatus = false;
    startQueue = false;
    useJackSync = false;
    useMidiClock = false;

    internalTempo = 120;
    initTempo();
    clockRatio = 60e9/TPQN/tempo;

    threadAbort = false;
    start(Priority(6));
}

SeqDriver::~SeqDriver(){

    if (useJackSync) setUseJackTransport(false);

    threadAbort = true;
    wait();

}

void SeqDriver::run()
{
    snd_seq_event_t *evIn;
    bool unmatched = true;
    bool fallback = false;
    MidiEvent inEv;
    int pollr = 0;

    int nfds;
    struct pollfd *pfds;

    nfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfds = (struct pollfd *) alloca(nfds * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfds, nfds, POLLIN);

    while ((poll >= 0) && (!threadAbort)) {

        pollr = poll(pfds, nfds, 200);
        while (pollr > 0) {

            snd_seq_event_input(seq_handle, &evIn);

            inEv.type = evIn->type;
            inEv.data = evIn->data.note.note;
            inEv.channel = 0;
            inEv.value = 0;

            if ((inEv.type == EV_CLOCK)&& useMidiClock) {
                midiTick++;
                m_current_tick = midiTick*TPQN/MIDICLK_TPQN;
/*                if ((m_current_tick > nextMinLfoTick) && (midiLfoList->count())) {
                    fallback = true;
                }
                if ((m_current_tick > nextMinSeqTick) && (midiSeqList->count())) {
                    fallback = true;
                } */
            }
            if (((inEv.type == EV_ECHO) || startQueue || fallback) && queueStatus) {
                startQueue = false;
                fallback = false;
                unmatched = false;
                realTime = evIn->time.time;
                if (useMidiClock) {
                    m_current_tick = midiTick*TPQN/MIDICLK_TPQN;
                    calcClockRatio();
                }
                else if (useJackSync) {
                    jPos = jackSync->getCurrentPos();
                    if (jPos.beats_per_minute > 0)
                        tempo = jPos.beats_per_minute;

                    m_current_tick = (long)jPos.frame * TPQN
                            / jPos.frame_rate * tempo / 60.
                            - jackOffsetTick;
                    calcClockRatio();
                }
                else {
                    clockRatio = 60e9/TPQN/tempo;
                    m_current_tick = deltaToTick(aTimeToDelta(&realTime));
                }

                // Restart ALSA queue if initial jack position is such that ticks would be negative
                // this cannot actually happen because m_current_tick us unsigned...
                //if (m_current_tick < 0) setQueueStatus(true);

                tick_callback((inEv.data));
            }
            else {
                unmatched = true;
                inEv.channel = evIn->data.control.channel;

                if ((inEv.type == EV_NOTEON) || (inEv.type == EV_NOTEOFF)) {
                    realTime = evIn->time.time;
                    inEv.data = evIn->data.note.note;
                    if (inEv.type == EV_NOTEON) {
                        inEv.value = evIn->data.note.velocity;
                    }
                    else {
                        inEv.value = 0;
                        inEv.type = EV_NOTEON;
                    }
                }
                else inEv.value = evIn->data.control.value;

                if (inEv.type == EV_CONTROLLER) {
                    inEv.data = evIn->data.control.param;
                }

                if (useMidiClock){
                    if (inEv.type == EV_START) {
                        setTransportStatus(true);
                    }
                    if (inEv.type == EV_STOP) {
                        setTransportStatus(false);
                    }
                    unmatched = true;
                }

                getTime();
                m_current_tick = deltaToTick(aTimeToDelta(&tmpTime));

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

void SeqDriver::initTempo()
{
    if (useJackSync) {
        jPos = jackSync->getCurrentPos();
        if (jPos.beats_per_minute > 0)
            tempo = jPos.beats_per_minute;
        else
            tempo = internalTempo;

        jackOffsetTick = (uint64_t)jPos.frame * TPQN
                * tempo / (jPos.frame_rate * 60);
        clockRatio = 60e9/TPQN/tempo;
    }
    else {
        tempo = internalTempo;
        clockRatio = 60e9/TPQN/tempo;
    }
    if (useMidiClock) {
        midiTick = 0;
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

void SeqDriver::setTempo(int bpm)
{
    tempo = bpm;
    internalTempo = bpm;
    clockRatio = 60e9/TPQN/tempo;
}

void SeqDriver::getTime()
{
    snd_seq_queue_status_t *status;

    snd_seq_queue_status_malloc(&status);
    snd_seq_get_queue_status(seq_handle, queue_id, status);

    const snd_seq_real_time_t* current_time =
        snd_seq_queue_status_get_real_time(status);
    tmpTime = *current_time;
    snd_seq_queue_status_free(status);
}

void SeqDriver::setTransportStatus(bool run)
{
    if (run) {
        queueStatus = true;
        startQueue = true;

        initTempo();

        snd_seq_start_queue(seq_handle, queue_id, NULL);
        snd_seq_drain_output(seq_handle);

        requestEchoAt(0);

        qWarning("Alsa Queue started");
    }
    else {
        queueStatus = false;
        snd_seq_remove_events_t *remove_ev;

        snd_seq_remove_events_malloc(&remove_ev);
        snd_seq_remove_events_set_queue(remove_ev, queue_id);
        snd_seq_remove_events_set_condition(remove_ev,
                SND_SEQ_REMOVE_OUTPUT | SND_SEQ_REMOVE_IGNORE_OFF);
        snd_seq_remove_events(seq_handle, remove_ev);
        snd_seq_remove_events_free(remove_ev);

        snd_seq_stop_queue(seq_handle, queue_id, NULL);

        m_current_tick = 0;

        qWarning("Alsa Queue stopped");
    }
}

void SeqDriver::setUseMidiClock(bool on)
{
    clockRatio = 60e9/TPQN/tempo;
    setTransportStatus(false);
    useMidiClock = on;
}

double SeqDriver::tickToDelta(int tick)
{
    return (double)clockRatio * tick;
}

int SeqDriver::deltaToTick(double curtime)
{
    return (int)(curtime / clockRatio + .5);
}

double SeqDriver::aTimeToDelta(snd_seq_real_time_t* atime)
{
    return (double)atime->tv_sec * 1e9 + (double)atime->tv_nsec;
}

const snd_seq_real_time_t* SeqDriver::deltaToATime(double curtime)
{
    delta.tv_sec = (int)(curtime * 1e-9);
    delta.tv_nsec = (long)(curtime - (double)delta.tv_sec * 1e9);
    return &delta;
}

void SeqDriver::calcClockRatio()
{
    double old_clock_ratio = clockRatio;

    if (m_current_tick > 0) {
        clockRatio = aTimeToDelta(&realTime)/m_current_tick;
    }
    if ((clockRatio == 0) || (clockRatio > 60e9 / tempo)) {
        clockRatio = old_clock_ratio;
    }
}

int SeqDriver::getClientId()
{
    return clientid;
}

void SeqDriver::setUseJackTransport(bool on)
{
    if (on) {
        jackSync->callJack(0);
    }
    else {
        jackSync->callJack(-1);
    }
    useJackSync = on;
}
