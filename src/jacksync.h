/*!
 * @file jacksync.h
 * @brief Headers for the JackSync QObject class.
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
#ifndef JACKSYNC_H
#define JACKSYNC_H

#include <QQueue>
#include <QThread>
#include <jack/jack.h>
#include <jack/transport.h>
#include <jack/midiport.h>

#include "main.h"
#include "driverbase.h"

/*!
 * The JackSync class is a QObject providing access to the transport status
 * of the Jack Audio Connection Kit (JACK) transport system. It provides
 * functions to register and initialise a jack client and to read the
 * current frame position of a transport master.
 *
 * @brief QObject class providing access to jack transport status.
 */
class JackSync : public DriverBase
{
    Q_OBJECT

  private:
    static int process_callback(jack_nframes_t nframes, void *arg);
    static void jack_shutdown(void *arg);
    void update_ports();

    jack_port_t * in_port;
    jack_port_t * out_ports[MAX_PORTS];

    bool jackRunning;
    int transportState;
    uint lastSchedTick;
    uint jackOffsetTick;
    uint64_t curJFrame;
    QQueue<uint> echoTickQueue;
    QQueue<MidiEvent> evQueue;
    QQueue<uint> evTickQueue;
    QQueue<uint> evPortQueue;
    jack_client_t *jack_handle;
    jack_position_t currentPos;
    void handleEchoes();


  public:
    JackSync(int p_portCount,
            void * callback_context,
            void (* p_tr_state_cb)(bool j_tr_state, void * context),
            bool (* midi_event_received_callback)(void * context, MidiEvent ev),
            void (* tick_callback)(void * context, bool echo_from_trig));
    ~JackSync();

    void (* trStateCb)(bool j_tr_state, void * context);
    void * cbContext;

  signals:
    void j_shutdown();

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
    int getClientId() {return 0; }
};


#endif
