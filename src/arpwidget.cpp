#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qgroupbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include "midiarp.h"
#include "arpwidget.h"
#include "slider.h"

ArpWidget::ArpWidget(MidiArp *p_midiArp, int portCount, QWidget *parent) : QWidget(parent) {
QVBoxLayout *arpWidgetLayout = new QVBoxLayout;

 
  QGroupBox *inBox = new QGroupBox("Input Note Filter", this);
  QVBoxLayout *inBoxLayout = new QVBoxLayout;
  midiArp = p_midiArp;

  QWidget *spinInBox = new QWidget(inBox);
  QHBoxLayout *spinInBoxLayout = new QHBoxLayout;
  
	QLabel *chInLabel = new QLabel("Channel:", spinInBox);
	chIn = new QSpinBox(spinInBox);
	chIn->setRange(0,15);
	QObject::connect(chIn, SIGNAL(valueChanged(int)), this, SLOT(updateChIn(int)));
	spinInBoxLayout->addWidget(chInLabel);
	spinInBoxLayout->addWidget(chIn);
	spinInBoxLayout->setMargin(1);
	spinInBoxLayout->setSpacing(1);
	spinInBox->setLayout(spinInBoxLayout);
	inBoxLayout->addWidget(spinInBox);
  
        
  QWidget *spinIndexBox = new QWidget(inBox);      
  QHBoxLayout *spinIndexBoxLayout = new QHBoxLayout;
  
	QLabel *indexInLabel = new QLabel("Note:", spinIndexBox);
	indexIn[0] = new QSpinBox(spinIndexBox);
	indexIn[1] = new QSpinBox(spinIndexBox);
	indexIn[0]->setRange(0,127);
	indexIn[1]->setRange(0,127);
	indexIn[1]->setValue(127);
	QObject::connect(indexIn[0], SIGNAL(valueChanged(int)), this, SLOT(updateIndexIn(int)));
	QObject::connect(indexIn[1], SIGNAL(valueChanged(int)), this, SLOT(updateIndexIn(int)));
	spinIndexBoxLayout->addWidget(indexInLabel);
	spinIndexBoxLayout->addWidget(indexIn[0]);
	spinIndexBoxLayout->addWidget(indexIn[1]);
	spinIndexBoxLayout->setMargin(1);
	spinIndexBoxLayout->setSpacing(1);
	spinIndexBox->setLayout(spinIndexBoxLayout);
	inBoxLayout->addWidget(spinIndexBox);
  
  inBoxLayout->setMargin(1);
  inBoxLayout->setSpacing(1);
  
  QWidget *spinRangeBox = new QWidget(inBox);      
  QHBoxLayout *spinRangeBoxLayout = new QHBoxLayout;
	QLabel *rangeInLabel = new QLabel("Velocity:", spinRangeBox);
	rangeIn[0] = new QSpinBox(spinRangeBox);
	rangeIn[1] = new QSpinBox(spinRangeBox);
	rangeIn[0]->setRange(0,127);
	rangeIn[1]->setRange(0,127);
	rangeIn[1]->setValue(127);
	spinRangeBoxLayout->addWidget(rangeInLabel);
	spinRangeBoxLayout->addWidget(rangeIn[0]);
	spinRangeBoxLayout->addWidget(rangeIn[1]);
	spinRangeBoxLayout->setMargin(1);
	spinRangeBoxLayout->setSpacing(1);
	spinRangeBox->setLayout(spinRangeBoxLayout);
  
  QObject::connect(rangeIn[0], SIGNAL(valueChanged(int)), this, SLOT(updateRangeIn(int)));
  QObject::connect(rangeIn[1], SIGNAL(valueChanged(int)), this, SLOT(updateRangeIn(int)));
  inBoxLayout->addWidget(spinRangeBox);
  inBox->setLayout(inBoxLayout); 
  QGroupBox *patternBox = new QGroupBox("Pattern", this);
  QVBoxLayout *patternBoxLayout = new QVBoxLayout;
  
  patternText = new QTextEdit(patternBox); 
  patternText->setAutoFormatting(QTextEdit::AutoNone);
  patternText->setLineWrapMode(QTextEdit::NoWrap);
  QObject::connect(patternText, SIGNAL(textChanged()), this, SLOT(updateText()));
  QObject::connect(this, SIGNAL(newPattern(QString)), midiArp, SLOT(updatePattern(QString)));
  patternBox->setToolTip("( ) chord Mode on/off\n + - octave up/down\n < . > tempo up/down\n d h note length up/down\n / velocity up/down");
           
  patternBoxLayout->addWidget(patternText);
  
  
  QWidget *repeatBox = new QWidget(patternBox);
  QHBoxLayout *repeatBoxLayout = new QHBoxLayout;
  QStringList repeatPatternNames; // = new QStringList(true);
  repeatPatternNames << "No" << "Up" << "Down";
  QLabel *repeatLabel = new QLabel("Repeat pattern through chord: ", repeatBox);
  repeatPatternThroughChord = new QComboBox(repeatBox);
  repeatPatternThroughChord->insertItems(0, repeatPatternNames);
  QObject::connect(repeatPatternThroughChord, SIGNAL(highlighted(int)), this, SLOT(updateRepeatPattern(int)));
  repeatPatternThroughChord->setCurrentIndex(1);
  repeatBoxLayout->addWidget(repeatLabel);
  repeatBoxLayout->addWidget(repeatPatternThroughChord);
  repeatBox->setLayout(repeatBoxLayout);
  QGroupBox *randomBox = new QGroupBox("Random", this);
  QVBoxLayout *randomBoxLayout = new QVBoxLayout;
  
  patternBoxLayout->addWidget(repeatBox);
  patternBoxLayout->setMargin(1);
  patternBoxLayout->setSpacing(1);
  patternBox->setLayout(patternBoxLayout); 
  QWidget *tickBox = new QWidget(randomBox);
  QHBoxLayout *tickBoxLayout = new QHBoxLayout;
  QLabel *tickLabel = new QLabel("Random Shift", tickBox);
  tickLabel->setFixedWidth(130);
  randomTick = new Slider(0, 100, 0, 0, Qt::Horizontal, randomBox);
  QObject::connect(randomTick, SIGNAL(valueChanged(int)), midiArp, SLOT(updateRandomTickAmp(int)));
  tickBoxLayout->addWidget(tickLabel);
  tickBoxLayout->addWidget(randomTick);
  tickBoxLayout->setMargin(1);
  tickBoxLayout->setSpacing(1);
  tickBox->setLayout(tickBoxLayout);

  
   
  QWidget *velocityBox = new QWidget(randomBox);
  QHBoxLayout *velocityBoxLayout = new QHBoxLayout;
  QLabel *velocityLabel = new QLabel("Random Velocity", velocityBox);
  velocityLabel->setFixedWidth(130);
  velocityBoxLayout->addWidget(velocityLabel);
  velocityBoxLayout->setMargin(1);
  velocityBoxLayout->setSpacing(1);
  velocityBox->setLayout(velocityBoxLayout);
  randomVelocity = new Slider(0, 100, 0, 0, Qt::Horizontal, randomBox);
  QObject::connect(randomVelocity, SIGNAL(valueChanged(int)), midiArp, SLOT(updateRandomVelocityAmp(int)));
  velocityBoxLayout->addWidget(velocityLabel);
  velocityBox->setLayout(velocityBoxLayout);
  velocityBoxLayout->addWidget(randomVelocity);
 
  QWidget *lengthBox = new QWidget(randomBox);
  QHBoxLayout *lengthBoxLayout = new QHBoxLayout;
  QLabel *lengthLabel = new QLabel("Random Length", lengthBox);
  lengthLabel->setFixedWidth(130);
  randomLength = new Slider(0, 100, 0, 0, Qt::Horizontal, randomBox);
  QObject::connect(randomLength, SIGNAL(valueChanged(int)), midiArp, SLOT(updateRandomVelocityAmp(int)));
  
  lengthBoxLayout->addWidget(lengthLabel);
  lengthBoxLayout->addWidget(randomLength);
  lengthBoxLayout->setMargin(1);
  lengthBoxLayout->setSpacing(1);
  lengthBox->setLayout(lengthBoxLayout);
  

  randomBoxLayout->setMargin(5);
  randomBoxLayout->setSpacing(1);
  randomBoxLayout->addWidget(tickBox);
  randomBoxLayout->addWidget(velocityBox);
  randomBoxLayout->addWidget(lengthBox);
  
  randomBox->setLayout(randomBoxLayout);  
   
  QWidget *portBox = new QWidget(this);
  QHBoxLayout *portBoxLayout = new QHBoxLayout;
  new QWidget(portBox); 
  QLabel *channelLabel = new QLabel("Output to channel: ", portBox);
  channelOut = new QSpinBox(portBox);
  channelOut->setRange(0,15);
  QObject::connect(channelOut, SIGNAL(valueChanged(int)), this, SLOT(updateChannelOut(int)));

  new QWidget(portBox); 
  QLabel *portLabel = new QLabel("Output to ALSA port: ", portBox);
  portOut = new QSpinBox(portBox);
  portOut->setRange(0,portCount - 1);
  QObject::connect(portOut, SIGNAL(valueChanged(int)), this, SLOT(updatePortOut(int)));
  
  portBoxLayout->addWidget(channelLabel);
  portBoxLayout->addWidget(channelOut);
  portBoxLayout->addWidget(portLabel);
  portBoxLayout->addWidget(portOut);

  portBox->setLayout(portBoxLayout);
 
  arpWidgetLayout->addWidget(inBox);
  arpWidgetLayout->addWidget(patternBox);
  arpWidgetLayout->addWidget(randomBox);
  arpWidgetLayout->addWidget(portBox);
  arpWidgetLayout->setMargin(2);
  arpWidgetLayout->setSpacing(5);
  setLayout(arpWidgetLayout);
  
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

void ArpWidget::writeArp(QTextStream& arpText) {

  arpText << midiArp->chIn << " " << midiArp->repeatPatternThroughChord << "\n";
  arpText << midiArp->indexIn[0] << " " << midiArp->indexIn[1] << "\n";
  arpText << midiArp->rangeIn[0] << " " << midiArp->rangeIn[1] << "\n";
  arpText << midiArp->channelOut << " " << midiArp->portOut << "\n";
  arpText << midiArp->randomTickAmp << " " << midiArp->randomVelocityAmp << " "  << midiArp->randomLengthAmp << "\n";
  arpText << midiArp->pattern << "\n";
  arpText << "EOP\n"; // End Of Pattern
}                                      

void ArpWidget::readArp(QTextStream& arpText) {

  QString qs, qs2;
  QRegExp sep(" ");
  QString stxt = "EOP";
  qs = arpText.readLine();
  qs2 = qs.section(sep, 0, 0); 
  chIn->setValue(qs2.toInt());
  qs2 = qs.section(sep, 1, 1);
  repeatPatternThroughChord->setCurrentIndex(qs2.toInt());
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
  while (!arpText.atEnd()) {
    qs2 = arpText.readLine();

    if (qs2.contains(qPrintable(stxt), Qt::CaseSensitivity(TRUE))) {
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

  emit(newPattern(patternText->toPlainText()));
}

void ArpWidget::updateRepeatPattern(int val) { 

  midiArp->repeatPatternThroughChord = val;
}
