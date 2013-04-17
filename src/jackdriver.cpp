/*!
 * @file jackdriver.cpp
 * @brief Implements the JackDriver QObject class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2013 <qmidiarp-devel@lists.sourceforge.net>
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

#include "jackdriver.h"
#include <stdio.h>


JackDriver::JackDriver(
    int p_portCount,
    void * callback_context,
    void (* p_tr_state_cb)(bool j_tr_state, void * context),
    bool (* midi_event_received_callback)(void * context, MidiEvent ev),
    void (* tick_callback)(void * context, bool echo_from_trig),
    void (* p_tempo_callback)(double bpm, void * context))
    : DriverBase(callback_context, midi_event_received_callback, tick_callback, 60e9)
{
    portCount = p_portCount;
    cbContext = callback_context;
    trStateCb = p_tr_state_cb;
    tempoCb = p_tempo_callback;
    jackRunning = false;
    portUnmatched = 0;
    forwardUnmatched = false;
    useJackSync = false;
    echoTickQueue.resize(512);
    evQueue.resize(512);
    evTickQueue.resize(512);
    evPortQueue.resize(512);
    bufPtr = 0;
    echoPtr = 0;
    internalTempo = 120;
    requestedTempo = 120;
    tempoChangeTick = 0;
    tempoChangeJPosFrame = 0;

/* Initialize and activate Jack with out_port_count ports if we use
 *  JACK driver backend, i.e. portCount > 0 */
    if (portCount) {
        setTransportStatus(false);
    }
    else {
        transportState = JackTransportStopped;
    }
}
bool JackDriver::callJack(int port_count)
{
    if (port_count == -1 && jackRunning) {
        deactivateJack();
        if (jack_handle != 0) {
            jack_client_close(jack_handle);
            jack_handle = 0;
        }
    }
    else if (port_count != -1) {
        if (initJack(port_count)) {
            emit j_shutdown();
            return true;
        }
        else if (activateJack()) {
            emit j_shutdown();
            return true;
        }
        transportState = getState();
        jSampleRate = jack_get_sample_rate(jack_handle);
    }
    return false;
}

JackDriver::~JackDriver()
{
    if (jackRunning) {
        deactivateJack();
    }
    if (jack_handle != 0) {
        jack_client_close(jack_handle);
        jack_handle = 0;
    }
}

int JackDriver::initJack(int out_port_count)
{
    char buf[16];

#ifdef JACK_SESSION
       if (global_jack_session_uuid.isEmpty() || !out_port_count) {
           if ((jack_handle = jack_client_open(PACKAGE, JackNullOption, NULL)) == 0) {
            qCritical("jack server not running?");
               return 1;
           }
       }
       else if ((jack_handle = jack_client_open(PACKAGE, JackSessionID, NULL, global_jack_session_uuid.data())) == 0) {
        qCritical("jack server not running?");
           return 1;
       }
#else
    if ((jack_handle = jack_client_open(PACKAGE, JackNullOption, NULL)) == 0) {
        qCritical("jack server not running?");
        return 1;
    }
#endif

    jack_on_shutdown(jack_handle, jack_shutdown, (void *)this);

    jack_set_process_callback(jack_handle, process_callback, (void *)this);

    qWarning("jack process callback registered");

    if (!out_port_count) return(0);

    // register JACK MIDI input port
    if ((in_port = jack_port_register(jack_handle, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0)) == 0) {
        qCritical("Failed to register JACK MIDI input port.");
        return 1;
    }

    // register JACK MIDI output ports
    for (int l1 = 0; l1 < out_port_count; l1++)
    {
      snprintf(buf, sizeof(buf), "out %d", l1 + 1);
      if ((out_ports[l1] = jack_port_register(jack_handle, buf, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0)) == 0)
      {
        qCritical("Failed to register JACK MIDI output port.");
        return 1;
      }
    }

#ifdef JACK_SESSION
    jack_set_session_callback(jack_handle, session_callback, (void *)this);
    qWarning("Session callback registered");
#endif

    return(0);
}

int JackDriver::activateJack()
{
    if (jack_activate(jack_handle)) {
        qWarning("cannot activate client");
        jackRunning = false;
        return(1);
    }

    jackRunning = true;
    return(0);
}

int JackDriver::deactivateJack()
{
    if (!jackRunning) return(0);

    if (jack_deactivate(jack_handle)) {
        qWarning("cannot deactivate client");
        return(1);
    }
    jackRunning = false;
    qWarning("jack client deactivated");
    return(0);
}

void JackDriver::jack_shutdown(void *arg)
{
    JackDriver *rd = (JackDriver *) arg;
    rd->setJackRunning(false);

    qWarning("JACK shut down. JACK sync Disabled.");
    emit rd->j_shutdown();
}

int JackDriver::process_callback(jack_nframes_t nframes, void *arg)
{
    uint i;
    uint l1, l2;

    JackDriver *rd = (JackDriver *) arg;
    uint out_port_count = rd->portCount;
    rd->jackTrCheckState();

    if (!out_port_count) return (0);

    rd->handleEchoes(nframes);

    int cur_tempo = rd->tempo;
    uint64_t cur_j_frame = rd->curJFrame;
    bool forward_unmatched = rd->forwardUnmatched;
    int port_unmatched = rd->portUnmatched;
    uint tempochangetick = rd->tempoChangeTick;
    uint nexttick = 0;
    uint tmptick = 0;
    uint idx = 0;
    int evport;
    uint64_t ev_jframe, ev_sample;
    uint ev_inframe;
    MidiEvent inEv;
    inEv.type = 0;
    inEv.data = 0;
    inEv.channel = 0;
    inEv.value = 0;
    MidiEvent outEv;
    outEv.channel = 0;

    unsigned char* buffer;
    jack_midi_event_t in_event;
    jack_nframes_t event_index = 0;
    jack_nframes_t j_sample_rate = rd->jSampleRate;
    void *in_buf = jack_port_get_buffer(rd->in_port, nframes);
    void *out_buf[out_port_count];
    for (l1 = 0; l1 < out_port_count; l1++) {
        out_buf[l1] = jack_port_get_buffer(rd->out_ports[l1], nframes);
    }
    for (l1 = 0; l1 < out_port_count; l1++) {
        jack_midi_clear_buffer(out_buf[l1]);
    }

    jack_nframes_t event_count = jack_midi_get_event_count(in_buf);
    jack_midi_event_get(&in_event, in_buf, 0);

    for(i = 0; i < nframes; i++) {

        /* MIDI Output queue first **/
        if (rd->bufPtr) { /* If we have events, find earliest event tick **/
            idx = 0;
            nexttick = rd->evTickQueue.first();
            for (l1 = 0; l1 < rd->bufPtr; l1++) {
                tmptick = rd->evTickQueue.at(l1);
                if (nexttick > tmptick) {
                    idx = l1;
                    nexttick = tmptick;
                }
            }
            ev_sample = (uint64_t)j_sample_rate * 60 * (nexttick - tempochangetick) / TPQN / cur_tempo;
            ev_jframe = ev_sample / nframes;
            ev_inframe = ev_sample % nframes;
            if ((ev_jframe <= cur_j_frame) && (ev_inframe <= i)) {
                //qWarning("nexttick %d, ev_frame %d, ev_inframe %d, cur_jframe %d, buf_idx %d", nexttick, ev_jframe, ev_inframe, cur_j_frame, idx);
                outEv = rd->evQueue.at(idx);
                evport = rd->evPortQueue.at(idx);
                for (uint l4 = idx ; l4 < (rd->bufPtr - 1);l4++) {
                    rd->evQueue.replace(l4, rd->evQueue.at(l4 + 1));
                    rd->evPortQueue.replace(l4, rd->evPortQueue.at(l4 + 1));
                    rd->evTickQueue.replace(l4, rd->evTickQueue.at(l4 + 1));
                }
                rd->bufPtr--;
                int k = 0;
                do {
                    if ((ev_jframe) < cur_j_frame) ev_inframe = 0;
                    buffer = jack_midi_event_reserve(out_buf[evport], ev_inframe + k, 3);
                    k++;
                } while (buffer == NULL);

                buffer[2] = outEv.value;        /* velocity / value **/
                buffer[1] = outEv.data;         /* note / controller **/
                if (outEv.type == EV_NOTEON) buffer[0] = 0x90;
                if (outEv.type == EV_CONTROLLER) buffer[0] = 0xb0;
                buffer[0] += outEv.channel;
            }
        }
        /* MIDI Input handling **/
        while ((in_event.time == i) && (event_index < event_count)) {

            if( ((*(in_event.buffer) & 0xf0)) == 0x90 ) {
                inEv.type = EV_NOTEON;
                inEv.value = *(in_event.buffer + 2);
            }
            else if( ((*(in_event.buffer)) & 0xf0) == 0x80 ) {
                inEv.type = EV_NOTEON;
                inEv.value = 0;
            }
            else if( ((*(in_event.buffer)) & 0xf0) == 0xa0 ) {
                inEv.type = EV_KEYPRESS;
                inEv.value = *(in_event.buffer + 2);
            }
            else if( ((*(in_event.buffer)) & 0xf0) == 0xb0 ) {
                inEv.type = EV_CONTROLLER;
                inEv.value = *(in_event.buffer + 2);
            }
            else if( ((*(in_event.buffer)) & 0xf0) == 0xc0 ) {
                inEv.type = EV_PGMCHANGE;
                inEv.value = *(in_event.buffer + 1);
            }
            else if( ((*(in_event.buffer)) & 0xf0) == 0xd0 ) {
                inEv.type = EV_CHANPRESS;
                inEv.value = *(in_event.buffer + 1);
            }
            else if( ((*(in_event.buffer)) & 0xf0) == 0xe0 ) {
                inEv.type = EV_PITCHBEND;
                inEv.value = *(in_event.buffer + 2) * 128;
                inEv.value += *(in_event.buffer + 1);
                inEv.value -= 8192;
            }
            else inEv.type = EV_NONE;

            inEv.data = *(in_event.buffer + 1);
            inEv.channel = (*(in_event.buffer)) & 0x0f;
            bool unmatched = rd->midi_event_received(inEv);

            if (unmatched && forward_unmatched) {
                buffer = jack_midi_event_reserve(out_buf[port_unmatched], i, in_event.size);
                if (buffer) {
                    for (l2 = 0; l2 < in_event.size; l2++) {
                        buffer[l2] = *(in_event.buffer + l2);
                    }
                }
            }

            event_index++;
            if(event_index < event_count)
                jack_midi_event_get(&in_event, in_buf, event_index);
        }
    }
    rd->curJFrame++;
    return(0);
}

#ifdef JACK_SESSION
void JackDriver::session_callback(jack_session_event_t *event, void *arg )
{
    JackDriver *rd = (JackDriver *) arg;
    rd->jsEv = event;
    rd->jack_session_event();
}

bool JackDriver::jack_session_event()
{
    jsFilename = jsEv->session_dir;
    jsFilename += JSFILENAME;

    QString cmd = PACKAGE " ${SESSION_DIR}" JSFILENAME " --jack_session_uuid ";
    cmd += jsEv->client_uuid;
    cmd += " --portCount "+QString::number(portCount);
    emit jsEvent(0);

    jsEv->command_line = strdup(cmd.toAscii());

    jack_session_reply(jack_handle, jsEv);

    if(jsEv->type == JackSessionSaveAndQuit) emit jsEvent(1);

    jack_session_event_free(jsEv);

    return false;
}
#endif


void JackDriver::jackTrCheckState()
{
    if (!useJackSync) return;

    uint state = getState();

    if (!portCount && (currentPos.beats_per_minute != tempo)) {
        tempoCb(currentPos.beats_per_minute, cbContext);
    }

    if (transportState == state) return;
    transportState = state;
    switch (state){
        case JackTransportStopped:
            trStateCb(false, cbContext);
            printf( "[JackTransportStopped]\n" );
        break;

        case JackTransportRolling:
            trStateCb(true, cbContext);
            printf( "[JackTransportRolling]\n" );
        break;

        case JackTransportStarting:
            printf( "[JackTransportStarting]\n" );
        break;

        case JackTransportLooping:
            printf( "[JackTransportLooping]\n" );
        break;
        default:
        break;
    }
}

jack_transport_state_t JackDriver::getState()
{
    return jack_transport_query(jack_handle, &currentPos);
}

void JackDriver::setJackRunning(bool on)
{
    jackRunning = on;
}

jack_position_t JackDriver::getCurrentPos()
{
    return currentPos;
}

void JackDriver::sendMidiEvent(MidiEvent ev, int n_tick, unsigned outport, unsigned duration)
{
  //qWarning("sendMidiEvent([%d, %d, %d, %d], %u, %u) at tick %d", ev.type, ev.channel, ev.data, ev.value, outport, duration, n_tick);

    evQueue.replace(bufPtr,ev);
    evTickQueue.replace(bufPtr,n_tick);
    evPortQueue.replace(bufPtr,outport);
    bufPtr++;

    if ((ev.type == EV_NOTEON) && (ev.value)) {
        ev.value = 0;
        evQueue.replace(bufPtr,ev);
        evTickQueue.replace(bufPtr,n_tick + duration / 4);
        evPortQueue.replace(bufPtr,outport);
        bufPtr++;
    }
}

bool JackDriver::requestEchoAt(int echo_tick, bool echo_from_trig)
{
    if ((echo_tick == (int)lastSchedTick) && (echo_tick)) return false;
    echoTickQueue.replace(echoPtr, echo_tick);
    echoPtr++;
    lastSchedTick = echo_tick;
    if (echo_from_trig) tick_callback(true);

    return true;

}

void JackDriver::handleEchoes(int nframes)
{
    uint l1;
    int nexttick, tmptick, idx;

    if (useJackSync) {
        if (currentPos.beats_per_minute > 0.01) {
        m_current_tick = ((uint64_t)currentPos.frame - tempoChangeJPosFrame)
            * TPQN * tempo / (currentPos.frame_rate * 60) + tempoChangeTick;
            if (currentPos.beats_per_minute != tempo) {
                tempoChangeJPosFrame = currentPos.frame;
                setTempo(currentPos.beats_per_minute);
                // inform engine via callback about the tempo change
                tempoCb(tempo, cbContext);
                requestedTempo = currentPos.beats_per_minute;
            }
        }
    }
    else {
        m_current_tick =  (uint64_t)curJFrame * TPQN * tempo * nframes
            / (jSampleRate * 60) + tempoChangeTick;
        if (requestedTempo != tempo) setTempo(requestedTempo);
    }

    if (!queueStatus) {
        curJFrame++;
        return;
    }
    if (!echoPtr) return;

    idx = 0;
    nexttick = echoTickQueue.first();

    for (l1 = 0; l1 < echoPtr; l1++) {
        tmptick = echoTickQueue.at(l1);
        if (nexttick > tmptick) {
            idx = l1;
            nexttick = tmptick;
        }
    }
    if (m_current_tick >= echoTickQueue.at(idx)) {
        for (uint l4 = idx ; l4 < (echoPtr - 1); l4++) {
            echoTickQueue.replace(l4, echoTickQueue.at(l4 + 1));
        }
        echoPtr--;
        tick_callback(false);
    }
}

void JackDriver::setTempo(double bpm)
{
    tempoChangeTick = m_current_tick;
    curJFrame = 0;
    tempo = bpm;
    internalTempo = bpm;
}

void JackDriver::setTransportStatus(bool on)
{
    jack_position_t jpos = getCurrentPos();
    if (useJackSync) {
        if (jpos.beats_per_minute > 0.01)
            tempo = jpos.beats_per_minute;
        else
            tempo = internalTempo;

        tempoChangeJPosFrame = jpos.frame;
    }
    else {
        tempo = internalTempo;
    }

    m_current_tick = 0;

    if (on) {
        curJFrame = 0;
        tempoChangeTick = 0;
        lastSchedTick = 0;
        echoPtr = 0;
        bufPtr = 0;
        requestEchoAt(0);
        printf("Internal Transport started\n");
    }
    else {
        printf("Internal Transport stopped\n");
    }

    queueStatus = on;
}
