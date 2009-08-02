#ifndef SEQDRIVER_H
#define SEQDRIVER_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qsocketnotifier.h>
#include <alsa/asoundlib.h>
#include "midiarp.h"
#include "main.h"

class SeqDriver : public QWidget  {
    
  Q_OBJECT

  private:
    int portCount;
    QPtrList<MidiArp> *midiArpList; 
    QSocketNotifier *seqNotifier;
    snd_seq_t *seq_handle;
    int clientid;
    int portid_out[MAX_PORTS];
    int portid_in;
    int queue_id;
    bool startQueue;
    snd_seq_tick_time_t tick;

  public:
    bool discardUnmatched, runQueueIfArp, runArp;
    int portUnmatched;
    int tempo;
    int grooveTick, grooveVelocity, grooveLength;        
    
  private:
    void initSeqNotifier();  

  public:
    SeqDriver(QPtrList<MidiArp> *p_midiArpList, QWidget* parent=0, const char *name=0);
    ~SeqDriver();
    void registerPorts(int num);
    int getPortCount();
    void initArpQueue();
    snd_seq_tick_time_t get_tick();
    void setQueueStatus(bool run);
    
  signals:
    void midiEvent(snd_seq_event_t *ev);

  public slots:
    void procEvents(int fd);  
    void setDiscardUnmatched(bool on);
    void setPortUnmatched(int id);
    void setQueueTempo(int bpm);
    void runQueue(bool);
    void setGrooveTick(int);
    void setGrooveVelocity(int);
    void setGrooveLength(int);
    void sendGroove();
};
                              
#endif
