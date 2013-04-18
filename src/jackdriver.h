/*!
 * @file jackdriver.h
 * @brief Headers for the JackDriver QObject class.
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
#ifndef JACKSYNC_H
#define JACKSYNC_H

#include <QVector>
#include <QThread>
#include "config.h"
#include <jack/jack.h>
#include <jack/transport.h>
#include <jack/midiport.h>

#ifdef JACK_SESSION
#include <jack/session.h>
#endif

#include "main.h"
#include "driverbase.h"

/*!
 * The JackDriver class is a QObject providing access to the MIDI interface
 * of the Jack Audio Connection Kit (JACK) transport system. It provides
 * functions to register and initialise a jack client and to read the
 * current frame position of a transport master. It establishes input and
 * output ports if requested and implements a sequencer queue based on
 * QQueue objects.
 *
 * @brief QObject class implementing a JACK MIDI and transport interface.
 */
class JackDriver : public DriverBase
{
    Q_OBJECT

  private:
    static int process_callback(jack_nframes_t nframes, void *arg);
    static void jack_shutdown(void *arg);
#ifdef JACK_SESSION
    static void session_callback(jack_session_event_t *ev, void *arg);
#endif
    void update_ports();

    jack_port_t * in_port;
    jack_port_t * out_ports[MAX_PORTS];

    bool jackRunning;
    uint transportState;
    uint lastSchedTick;
    uint tempoChangeTick;
    uint64_t curJFrame;
    uint64_t tempoChangeJPosFrame;
    QVector<uint> echoTickQueue;
    QVector<MidiEvent> evQueue;
    QVector<uint> evTickQueue;
    QVector<uint> evPortQueue;
    uint bufPtr;
    uint echoPtr;
    jack_client_t *jack_handle;
    jack_position_t currentPos;
    void handleEchoes(int nframes);

#ifdef JACK_SESSION
  public:
    jack_session_event_t *jsEv;
    bool jack_session_event();
#endif


  public:
    JackDriver(int p_portCount,
            const char *client_name,
            void * callback_context,
            void (* p_tr_state_cb)(bool j_tr_state, void * context),
            bool (* midi_event_received_callback)(void * context, MidiEvent ev),
            void (* tick_callback)(void * context, bool echo_from_trig),
            void (* p_tempo_callback)(double bpm, void * context));
    ~JackDriver();

    void (* trStateCb)(bool j_tr_state, void * context);
    void (* tempoCb)(double bpm, void * context);
    void * cbContext;

  signals:
    void j_shutdown();
    void jsEvent(int type);

  public:
    jack_nframes_t jSampleRate;
    bool isRunning() { return jackRunning; }
    int initJack(int out_port_count);
    int activateJack();
    int deactivateJack();

    void setJackRunning(bool on);

    void sendMidiEvent(MidiEvent ev, int n_tick, unsigned int outport, unsigned int duration = 0);
    jack_transport_state_t getState();
    void jackTrCheckState();
    jack_position_t getCurrentPos();
    bool requestEchoAt(int echoTick, bool echo_from_trig = 0);
    void setTransportStatus(bool run);
    void setTempo(double bpm);
    int getClientId() {return 0; }
    bool callJack(int portcount);
};


#endif
