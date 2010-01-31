//      jacksync.cpp
//      
//      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include "jacksync.h"
#include "config.h"


JackSync::JackSync() : QThread()
{
    transportState = JackTransportStopped;
    j_frame_time = 0;
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
    wait();
}

int JackSync::initJack()
{
    if ((jack_handle = jack_client_open(PACKAGE, JackNullOption, NULL)) == 0) {
        qCritical("jack server not running?");
        return 1;
    }

    jack_on_shutdown(jack_handle, jack_shutdown, (void *)this);
    
    jack_set_sync_callback(jack_handle, sync_callback, (void *)this);

    qWarning("jack sync callback registered");
    
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

int JackSync::sync_callback(jack_transport_state_t state, 
                            jack_position_t *pos, void *arg)
{
    JackSync *rd;

    rd = (JackSync *)arg;

    return(rd->jack_sync(state));
}

int JackSync::jack_sync(jack_transport_state_t state)
{
    
    switch (state){
        case JackTransportStopped:
            qWarning( "[JackTransportStopped]" );
        break;
        
        case JackTransportRolling:  
            qWarning( "[JackTransportRolling]" );
        break;
        
        case JackTransportStarting:
            emit j_tr_state(true);
            qWarning( "[JackTransportStarting]" );
        break;
        
        case JackTransportLooping:
            qWarning( "[JackTransportLooping]" );
        break;
        default:
        break;
    }
    if (transportState != state)
        transportState = state;

    bool ready = true;
    return(ready);
}

jack_position_t JackSync::get_pos()
{
    start();
    wait();
    return(current_pos);
}

void JackSync::run()
{
    jack_transport_state_t state = jack_transport_query(jack_handle, &current_pos);
    if (transportState != state)
        transportState = state;
}

jack_transport_state_t JackSync::get_state()
{
    return jack_transport_query(jack_handle, &current_pos);
}

void JackSync::setJackRunning(bool on)
{
    jackRunning = on;
}
