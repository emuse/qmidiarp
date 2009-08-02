#ifndef ARPDATA_H
#define ARPDATA_H

#include <qwidget.h>
#include <qptrlist.h>
#include <alsa/asoundlib.h>
#include "seqdriver.h"
#include "midiarp.h"
#include "arpwidget.h"

class ArpData : public QWidget  {
    
  Q_OBJECT

  private:
    QPtrList<MidiArp> midiArpList;
    QPtrList<ArpWidget> arpWidgetList;
    int portCount;

  public:
    SeqDriver *seqDriver;

  public:
    ArpData(QWidget* parent=0, const char *name=0);
    ~ArpData();
    void registerPorts(int num);
    int getPortCount();
    void addMidiArp(MidiArp *midiArp);
    void addArpWidget(ArpWidget *arpWidget);
    void removeMidiArp(MidiArp *midiArp);
    void removeArpWidget(ArpWidget *arpWidget);
    int midiArpCount();
    int arpWidgetCount();
    MidiArp *midiArp(int index);
    ArpWidget *arpWidget(int index);
    
  public slots:
    void runQueue(bool);
    
};
                              
#endif
