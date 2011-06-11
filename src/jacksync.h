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
    bool jackRunning;
    int transportState;
    double j_frame_time;
    jack_client_t *jack_handle;
    jack_position_t currentPos;
    jack_position_t lastPos;


  public:
    JackSync(void (* p_tr_state_cb)(bool j_tr_state, void * context),
            void * p_cb_context);
    ~JackSync();

    void (* trStateCb)(bool j_tr_state, void * context);
    void * cbContext;

  signals:
    void j_shutdown();

  public:
    bool isRunning() { return jackRunning; }
    int initJack();
    int activateJack();
    int deactivateJack();

    void setJackRunning(bool on);
    void setCurrentPos(jack_position_t *pos);
    void setLastPos(jack_position_t *pos);

    jack_transport_state_t get_state();
    jack_position_t get_pos();
};


#endif
