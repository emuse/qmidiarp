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
#include "passwidget.h"


PassWidget::PassWidget(int p_portcount, QWidget *parent) : QWidget(parent) {
QVBoxLayout *passWidgetLayout = new QVBoxLayout;

  QWidget *buttonBox = new QWidget(this);
  QHBoxLayout *buttonBoxLayout = new QHBoxLayout;
  QLabel *discardLabel = new QLabel("Discard unmatched events ", buttonBox);
  discardCheck = new QCheckBox(buttonBox);
  discardCheck->setChecked(false);
  QObject::connect(discardCheck, SIGNAL(toggled(bool)), this, SLOT(updateDiscard(bool)));
  buttonBoxLayout->addWidget(discardLabel);
  buttonBoxLayout->addWidget(discardCheck);
  buttonBox->setLayout(buttonBoxLayout);
  //new QWidget(buttonBox);
  QWidget *portBox = new QWidget(this);
  QHBoxLayout *portBoxLayout = new QHBoxLayout;
  QLabel *portLabel = new QLabel("Send unmatched events to port ", portBox);
  portUnmatchedSpin = new QSpinBox(portBox);
  portUnmatchedSpin->setRange(0, p_portcount -1);
  QObject::connect(portUnmatchedSpin, SIGNAL(valueChanged(int)), this, SLOT(updatePortUnmatched(int)));
  portBoxLayout->addWidget(portLabel);
  portBoxLayout->addWidget(portUnmatchedSpin);
  portBox->setLayout(portBoxLayout);
  //new QWidget(portBox);
   
  
  QWidget *mtpbBox = new QWidget(this);
  QHBoxLayout *mtpbBoxLayout = new QHBoxLayout;
  QLabel *mtpbLabel = new QLabel("MIDI Clock rate(tpb) ", mtpbBox);
  mtpbSpin = new QSpinBox(mtpbBox);
  QObject::connect(mtpbSpin, SIGNAL(valueChanged(int)), this, SLOT(updateMIDItpb_pw(int)));
  mtpbSpin->setRange(24,384);
  mtpbSpin->setValue(96);
  mtpbSpin->setSingleStep(24);
  mtpbSpin->setDisabled(true);
  mtpbBoxLayout->addWidget(mtpbLabel);
  mtpbBoxLayout->addWidget(mtpbSpin);
  mtpbBox->setLayout(mtpbBoxLayout);
  
  QWidget *mbuttonBox = new QWidget(this);
  QHBoxLayout *mbuttonBoxLayout = new QHBoxLayout;
  QLabel *mbuttonLabel = new QLabel("Use incoming MIDI Clock", mbuttonBox);
  mbuttonCheck = new QCheckBox(mbuttonBox);
  QObject::connect(mbuttonCheck, SIGNAL(toggled(bool)), this, SLOT(updateClockSetting(bool)));
  mbuttonCheck->setChecked(false);
  mbuttonBoxLayout->addWidget(mbuttonLabel);
  mbuttonBoxLayout->addWidget(mbuttonCheck);
  mbuttonBox->setLayout(mbuttonBoxLayout);
   

  //new QWidget(this);
  passWidgetLayout->setMargin(5);
  passWidgetLayout->setSpacing(10);
  passWidgetLayout->addWidget(buttonBox);
  passWidgetLayout->addWidget(portBox);
  passWidgetLayout->addWidget(mbuttonBox);
  passWidgetLayout->addWidget(mtpbBox);
  setMaximumHeight(200);

setLayout(passWidgetLayout);
}

PassWidget::~PassWidget() {
  
}

void PassWidget::updateDiscard(bool on) {

  emit discardToggled(on);
  portUnmatchedSpin->setDisabled(on);
}
void PassWidget::updateClockSetting(bool on) {
mtpbSpin->setEnabled(on);
emit midiClockToggle(on);
}


void PassWidget::updatePortUnmatched(int id) {

  emit newPortUnmatched(id);
}

void PassWidget::setDiscard(bool on) {

  discardCheck->setChecked(on);
}

void PassWidget::setPortUnmatched(int id) {

  portUnmatchedSpin->setValue(id);
}

void PassWidget::updateMIDItpb_pw(int MIDItpb) {
emit newMIDItpb(MIDItpb);
  
}

        
