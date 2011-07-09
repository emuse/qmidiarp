/*!
 * @file jacksync.cpp
 * @brief Implements the JackSync QObject class.
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

#include "jacksync.h"
#include "config.h"
#include <stdio.h>
#include "driverbase.h"         // temp include to check driverbase.h for synthax errors


JackSync::JackSync(void (* p_tr_state_cb)(bool j_tr_state, void * context),
                    void * p_cb_context)
{
    transportState = JackTransportStopped;
    out_port_count = 0;
    cbContext = p_cb_context;
    trStateCb = p_tr_state_cb;
 }

JackSync::~JackSync()
{
    if (jackRunning) {
        deactivateJack();
    }
    if (jack_handle != 0) {
        jack_client_close(jack_handle);
        jack_handle = 0;
    }
}

int JackSync::initJack(int out_port_count)
{
    char buf[16];

    if ((jack_handle = jack_client_open(PACKAGE, JackNullOption, NULL)) == 0) {
        qCritical("jack server not running?");
        return 1;
    }

    jack_on_shutdown(jack_handle, jack_shutdown, (void *)this);

    jack_set_process_callback(jack_handle, process_callback, (void *)this);

    qWarning("jack process callback registered");

    // register JACK MIDI input port
    if ((in_port = jack_port_register(jack_handle, "in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0)) == 0) {
        qCritical("Failed to register JACK MIDI input port.");
        return 1;
    }

    // register JACK MIDI output ports
    while (out_port_count-- > 0)
    {
      snprintf(buf, sizeof(buf), "out %d", JackSync::out_port_count + 1);
      if ((out_ports[JackSync::out_port_count] = jack_port_register(jack_handle, buf, JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0)) == 0)
      {
        qCritical("Failed to register JACK MIDI output port.");
        return 1;
      }

      JackSync::out_port_count++;
    }

    return(0);
}

int JackSync::activateJack()
{
    if (jack_activate(jack_handle)) {
        qWarning("cannot activate client");
        jackRunning = false;
        return(1);
    }

    jackRunning = true;
    return(0);
}

int JackSync::deactivateJack()
{
    if (jackRunning) {
        if (jack_deactivate(jack_handle)) {
            qWarning("cannot deactivate client");
            return(1);
        }
        jackRunning = false;
        qWarning("jack client deactivated");
    }
    return(0);
}

void JackSync::jack_shutdown(void *arg)
{
    JackSync *rd = (JackSync *) arg;
    rd->setJackRunning(false);

    qWarning("JACK shut down. JACK sync Disabled.");
    emit rd->j_shutdown();
}

int JackSync::process_callback(jack_nframes_t nframes, void *arg)
{
    ((JackSync *)arg)->jackTrCheckState();
    return(0);
}

void JackSync::jackTrCheckState()
{
    int state = getState();

    if (transportState == state) return;

    transportState = state;
    switch (state){
        case JackTransportStopped:
            trStateCb(false, cbContext);
            qWarning( "[JackTransportStopped]" );
        break;

        case JackTransportRolling:
            trStateCb(true, cbContext);
            qWarning( "[JackTransportRolling]" );
        break;

        case JackTransportStarting:
            qWarning( "[JackTransportStarting]" );
        break;

        case JackTransportLooping:
            qWarning( "[JackTransportLooping]" );
        break;
        default:
        break;
    }
}

bool JackSync::is_stopped()
{
    return jack_transport_query(jack_handle, &current_pos) == JackTransportStopped;
}

void JackSync::setJackRunning(bool on)
{
    jackRunning = on;
}

jack_position_t JackSync::getCurrentPos()
{
    return currentPos;
}
