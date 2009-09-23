#ifndef ARPDATA_H
#define ARPDATA_H

#include <QWidget>
#include <QList>
#include <alsa/asoundlib.h>
#include "seqdriver.h"
#include "midiarp.h"
#include "arpwidget.h"
#include "midilfo.h"
#include "lfowidget.h"

class ArpData : public QWidget  {
    
  Q_OBJECT

  private:
    QList<MidiArp *> midiArpList;
    QList<ArpWidget *> arpWidgetList;
    QList<MidiLfo *> midiLfoList;
    QList<LfoWidget *> lfoWidgetList;
    int portCount;

  public:
    SeqDriver *seqDriver;

  public:
    ArpData(QWidget* parent=0);
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
	
    void addMidiLfo(MidiLfo *midiLfo);
    void addLfoWidget(LfoWidget *lfoWidget);
    void removeMidiLfo(MidiLfo *midiLfo);
    void removeLfoWidget(LfoWidget *lfoWidget);
    int midiLfoCount();
    int lfoWidgetCount();
    MidiLfo *midiLfo(int index);
    LfoWidget *lfoWidget(int index);
    
  public slots:
    void runQueue(bool);
    
};
                              
#endif
