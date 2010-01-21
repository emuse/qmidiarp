#ifndef JACKSYNC_H
#define JACKSYNC_H

#include <QWidget>
#include <jack/jack.h>
#include <jack/transport.h>

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
    jack_position_t current_pos;
    
    
  public:
    JackSync();
    ~JackSync();
    
  signals:
    void j_tr_state(bool on);
    void j_shutdown();

  public:
    bool isRunning() { return jackRunning; }
    int initJack();
    int activateJack();
    int deactivateJack();
     
    void setJackRunning(bool on);
    jack_transport_state_t get_state();
    jack_position_t get_pos();
};


#endif
