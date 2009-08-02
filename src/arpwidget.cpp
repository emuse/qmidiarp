#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qstrlist.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qtextedit.h>
#include "midiarp.h"
#include "arpwidget.h"
#include "slider.h"

ArpWidget::ArpWidget(MidiArp *p_midiArp, int portCount, QWidget *parent, const char *name) : QVBox(parent, name) {

  setMargin(5);
  setSpacing(10);
  QVGroupBox *inBox = new QVGroupBox("Event Input", this);
  midiArp = p_midiArp;

  QHBox *spinInBox = new QHBox(inBox);
  QLabel *chInLabel = new QLabel("Channel:", spinInBox);
  chIn = new QSpinBox(0, 15, 1, spinInBox);
  QObject::connect(chIn, SIGNAL(valueChanged(int)), this, SLOT(updateChIn(int)));
        
  QHBox *spinIndexBox = new QHBox(inBox);      
  QLabel *indexInLabel = new QLabel("Note:", spinIndexBox);
  indexIn[0] = new QSpinBox(0, 127, 1, spinIndexBox);
  indexIn[1] = new QSpinBox(0, 127, 1, spinIndexBox);
  indexIn[1]->setValue(127);
  QObject::connect(indexIn[0], SIGNAL(valueChanged(int)), this, SLOT(updateIndexIn(int)));
  QObject::connect(indexIn[1], SIGNAL(valueChanged(int)), this, SLOT(updateIndexIn(int)));

  QHBox *spinRangeBox = new QHBox(inBox);
  QLabel *rangeInLabel = new QLabel("Velocity:", spinRangeBox);
  rangeIn[0] = new QSpinBox(0, 127, 1, spinRangeBox);
  rangeIn[1] = new QSpinBox(0, 127, 1, spinRangeBox);
  rangeIn[1]->setValue(127);
  QObject::connect(rangeIn[0], SIGNAL(valueChanged(int)), this, SLOT(updateRangeIn(int)));
  QObject::connect(rangeIn[1], SIGNAL(valueChanged(int)), this, SLOT(updateRangeIn(int)));

  QVGroupBox *patternBox = new QVGroupBox("Pattern", this);
  patternText = new QTextEdit(patternBox); 
  patternText->setAutoFormatting(QTextEdit::AutoNone);
  patternText->setWordWrap(QTextEdit::NoWrap);
  QObject::connect(patternText, SIGNAL(textChanged()), this, SLOT(updateText()));
  QObject::connect(this, SIGNAL(newPattern(QString)), midiArp, SLOT(updatePattern(QString)));
  QHBox *repeatBox = new QHBox(patternBox);
  QStrList *repeatPatternNames = new QStrList(true);
  repeatPatternNames->append("No");
  repeatPatternNames->append("Up");
  repeatPatternNames->append("Down");
  QLabel *repeatLabel = new QLabel("Repeat pattern through chord: ", repeatBox);
  repeatPatternThroughChord = new QComboBox(repeatBox);
  repeatPatternThroughChord->insertStrList(repeatPatternNames);
  QObject::connect(repeatPatternThroughChord, SIGNAL(highlighted(int)), this, SLOT(updateRepeatPattern(int)));
  repeatPatternThroughChord->setCurrentItem(1);
  QVGroupBox *randomBox = new QVGroupBox("Random", this);
  QHBox *tickBox = new QHBox(randomBox);
  new QWidget(tickBox);
  QLabel *tickLabel = new QLabel("Random Note Displacement", tickBox);
  new QWidget(tickBox);
  randomTick = new Slider(0, 100, 0, 0, QSlider::Horizontal, randomBox);
  QObject::connect(randomTick, SIGNAL(valueChanged(int)), midiArp, SLOT(updateRandomTickAmp(int)));
  QHBox *velocityBox = new QHBox(randomBox);
  new QWidget(velocityBox);
  QLabel *velocityLabel = new QLabel("Random Velocity", velocityBox);
  new QWidget(velocityBox);
  randomVelocity = new Slider(0, 100, 0, 0, QSlider::Horizontal, randomBox);
  QObject::connect(randomVelocity, SIGNAL(valueChanged(int)), midiArp, SLOT(updateRandomVelocityAmp(int)));
  QHBox *lengthBox = new QHBox(randomBox);
  new QWidget(lengthBox);
  QLabel *lengthLabel = new QLabel("Random Length", lengthBox);
  new QWidget(lengthBox);
  randomLength = new Slider(0, 100, 0, 0, QSlider::Horizontal, randomBox);
  QObject::connect(randomLength, SIGNAL(valueChanged(int)), midiArp, SLOT(updateRandomLengthAmp(int)));
 
  QHBox *portBox = new QHBox(this);
  new QWidget(portBox); 
  QLabel *channelLabel = new QLabel("Output to channel: ", portBox);
  channelOut = new QSpinBox(0, 15, 1, portBox);
  QObject::connect(channelOut, SIGNAL(valueChanged(int)), this, SLOT(updateChannelOut(int)));

  new QWidget(portBox); 
  QLabel *portLabel = new QLabel("Output to ALSA port: ", portBox);
  portOut = new QSpinBox(0, portCount - 1, 1, portBox);
  QObject::connect(portOut, SIGNAL(valueChanged(int)), this, SLOT(updatePortOut(int)));
}

ArpWidget::~ArpWidget() {
  
}

MidiArp *ArpWidget::getMidiArp() {

  return (midiArp);
}

void ArpWidget::updateChIn(int value) {

  midiArp->chIn = value;
}

void ArpWidget::updateIndexIn(int value) {

  if (indexIn[0] == sender()) {
    midiArp->indexIn[0] = value; 
  } else {
    midiArp->indexIn[1] = value;
  }  
}

void ArpWidget::updateRangeIn(int value) { 

  if (rangeIn[0] == sender()) {
    midiArp->rangeIn[0] = value; 
  } else {
    midiArp->rangeIn[1] = value;
  }  
}

void ArpWidget::updatePortOut(int value) { 

  midiArp->portOut = value;
}

void ArpWidget::updateChannelOut(int value) { 

  midiArp->channelOut = value;
}

void ArpWidget::writeArp(QFile *f) {

  QTextStream arpText(f);
  
  arpText << midiArp->chIn << " " << midiArp->repeatPatternThroughChord << "\n";
  arpText << midiArp->indexIn[0] << " " << midiArp->indexIn[1] << "\n";
  arpText << midiArp->rangeIn[0] << " " << midiArp->rangeIn[1] << "\n";
  arpText << midiArp->channelOut << " " << midiArp->portOut << "\n";
  arpText << midiArp->randomTickAmp << " " << midiArp->randomVelocityAmp << " "  << midiArp->randomLengthAmp << "\n";
  arpText << midiArp->pattern << "\n";
  arpText << "EOP\n"; // End Of Pattern
}                                      

void ArpWidget::readArp(QFile *f) {

  QString qs, qs2;
  QTextStream arpText(f);
  QRegExp sep(" ");
  
  qs = arpText.readLine();
  qs2 = qs.section(sep, 0, 0); 
  chIn->setValue(qs2.toInt());
  qs2 = qs.section(sep, 1, 1);
  repeatPatternThroughChord->setCurrentItem(qs2.toInt());
  qs = arpText.readLine();
  qs2 = qs.section(sep, 0, 0); 
  indexIn[0]->setValue(qs2.toInt());
  qs2 = qs.section(sep, 1, 1); 
  indexIn[1]->setValue(qs2.toInt());
  qs = arpText.readLine();
  qs2 = qs.section(sep, 0, 0); 
  rangeIn[0]->setValue(qs2.toInt());
  qs2 = qs.section(sep, 1, 1); 
  rangeIn[1]->setValue(qs2.toInt());
  qs = arpText.readLine();
  qs2 = qs.section(sep, 0, 0); 
  channelOut->setValue(qs2.toInt());
  qs2 = qs.section(sep, 1, 1); 
  portOut->setValue(qs2.toInt());

  qs = arpText.readLine();
  qs2 = qs.section(sep, 0, 0); 
  randomTick->setValue(qs2.toInt());
  qs2 = qs.section(sep, 1, 1); 
  randomVelocity->setValue(qs2.toInt());
  qs2 = qs.section(sep, 2, 2); 
  randomLength->setValue(qs2.toInt());

  qs = arpText.readLine();
  while (!arpText.eof()) {
    qs2 = arpText.readLine();
    if (qs2.contains("EOP", true)) {
      break;
    }
    qs += "\n" + qs2;
  }
  patternText->setText(qs);                    
}                                      

void ArpWidget::setChIn(int value) {

  chIn->setValue(value);
}

void ArpWidget::setIndexIn(int index, int value) {

  indexIn[index]->setValue(value); 
}

void ArpWidget::setRangeIn(int index, int value) { 

  rangeIn[index]->setValue(value);
}

void ArpWidget::setPortOut(int value) { 

  portOut->setValue(value);
}

void ArpWidget::setChannelOut(int value) { 

  channelOut->setValue(value);
}

void ArpWidget::updateText() { 

  emit(newPattern(patternText->text()));
}

void ArpWidget::updateRepeatPattern(int val) { 

  midiArp->repeatPatternThroughChord = val;
}
