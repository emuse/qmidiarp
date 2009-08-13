#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include "passwidget.h"


PassWidget::PassWidget(int p_portcount, QWidget *parent) : QWidget(parent) {
QVBoxLayout *passWidgetLayout = new QVBoxLayout;

  QHBoxLayout *buttonBoxLayout = new QHBoxLayout;
  QLabel *discardLabel = new QLabel("Discard unmatched events ", this);
  discardCheck = new QCheckBox(this);
  discardCheck->setChecked(false);
  QObject::connect(discardCheck, SIGNAL(toggled(bool)), this, SLOT(updateDiscard(bool)));
  buttonBoxLayout->addWidget(discardLabel);
  buttonBoxLayout->addStretch(1);
  buttonBoxLayout->addWidget(discardCheck);

  QHBoxLayout *portBoxLayout = new QHBoxLayout;
  QLabel *portLabel = new QLabel("Send unmatched events to port ", this);
  portUnmatchedSpin = new QSpinBox(this);
  portUnmatchedSpin->setRange(0, p_portcount -1);
  QObject::connect(portUnmatchedSpin, SIGNAL(valueChanged(int)), this, SLOT(updatePortUnmatched(int)));
  portBoxLayout->addWidget(portLabel);
  portBoxLayout->addStretch(1);
  portBoxLayout->addWidget(portUnmatchedSpin);
  
  QHBoxLayout *mtpbBoxLayout = new QHBoxLayout;
  QLabel *mtpbLabel = new QLabel("MIDI Clock rate(tpb) ", this);
  mtpbSpin = new QSpinBox(this);
  QObject::connect(mtpbSpin, SIGNAL(valueChanged(int)), this, SLOT(updateMIDItpb_pw(int)));
  mtpbSpin->setRange(24,384);
  mtpbSpin->setValue(96);
  mtpbSpin->setSingleStep(24);
  mtpbSpin->setDisabled(true);
  mtpbBoxLayout->addWidget(mtpbLabel);
  mtpbBoxLayout->addStretch(1);
  mtpbBoxLayout->addWidget(mtpbSpin);
  
  QHBoxLayout *mbuttonBoxLayout = new QHBoxLayout;
  QLabel *mbuttonLabel = new QLabel("Use incoming MIDI Clock", this);
  mbuttonCheck = new QCheckBox(this);
  QObject::connect(mbuttonCheck, SIGNAL(toggled(bool)), this, SLOT(updateClockSetting(bool)));
  mbuttonCheck->setChecked(false);
  mbuttonBoxLayout->addWidget(mbuttonLabel);
  mbuttonBoxLayout->addStretch(1);
  mbuttonBoxLayout->addWidget(mbuttonCheck);

  passWidgetLayout->setMargin(5);
  passWidgetLayout->setSpacing(10);
  passWidgetLayout->addLayout(buttonBoxLayout);
  passWidgetLayout->addLayout(portBoxLayout);
  passWidgetLayout->addLayout(mbuttonBoxLayout);
  passWidgetLayout->addLayout(mtpbBoxLayout);
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

        
