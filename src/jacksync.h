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

#include <QObject>
#include <jack/jack.h>
#include <jack/transport.h>

#include "main.h"

/*!
 * The JackSync class is a QObject providing access to the transport status
 * of the Jack Audio Connection Kit (JACK) transport system. It provides
 * functions to register and initialise a jack client and to read the
 * current frame position of a transport master.
 *
 * @brief QObject class providing access to jack transport status.
 */
class JackSync : public QObject
{
    Q_OBJECT

  private:
    static int sync_callback(jack_transport_state_t state,
                            jack_position_t *pos, void *arg);
    static void jack_shutdown(void *arg);
    int jack_sync(jack_transport_state_t state);
    void update_ports();

    jack_port_t * in_port;
    int out_port_count;
    jack_port_t * out_ports[MAX_PORTS];

    bool jackRunning;
    int transportState;
    double j_frame_time;
    jack_client_t *jack_handle;
    jack_position_t current_pos;


  public:
    JackSync();
    ~JackSync();

  signals:
    void j_tr_state(bool on);
    void j_shutdown();

  public:
    bool isRunning() { return jackRunning; }
    int initJack(int out_port_count);
    int activateJack();
    int deactivateJack();

    void setJackRunning(bool on);
    bool is_stopped();
    jack_position_t get_pos();
};


#endif
