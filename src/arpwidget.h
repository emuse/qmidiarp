#ifndef ARPWIDGET_H
#define ARPWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qfile.h>
#include <qtextedit.h>
#include "midiarp.h"
#include "slider.h"

class ArpWidget : public QVBox

{
  Q_OBJECT

  private:
    QSpinBox *chIn;                        // Channel of input events
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QSpinBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QComboBox *repeatPatternThroughChord;
    MidiArp *midiArp;
    QTextEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
    
  public:
    QString arpName;

  public:
    ArpWidget(MidiArp *p_midiArp, int portCount, QWidget* parent=0, const char *name=0);
    ~ArpWidget();
    MidiArp *getMidiArp();
    void readArp(QFile *f);
    void writeArp(QFile *f);
    void setChIn(int value);
    void setIndexIn(int index, int value);
    void setChannelOut(int value);
    void setPortOut(int value);
    void setRangeIn(int index, int value);
    
  signals:
    void newPattern(QString);  
    
  public slots:
    void updateChIn(int value);
    void updateIndexIn(int value);
    void updateRangeIn(int value);
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateText();
    void updateRepeatPattern(int);
};
  
#endif
