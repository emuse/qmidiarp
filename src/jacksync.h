/*!
 * @file jacksync.h
 * @brief Header for the JackSync class
 */
#ifndef JACKSYNC_H
#define JACKSYNC_H

#include <QObject>
#include <jack/jack.h>
#include <jack/transport.h>

#include "main.h"

/*!
 * @brief JACK backend class
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
    bool is_stoped();
    jack_position_t get_pos();
};


#endif
