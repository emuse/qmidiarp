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
  QWidget *tempoBox = new QWidget(this);
  QHBoxLayout *tempoBoxLayout = new QHBoxLayout;
  QLabel *tempoLabel = new QLabel("Tempo (bpm) ", tempoBox);
  tempoSpin = new QSpinBox(tempoBox);
  tempoSpin->setRange(10,300);
  tempoSpin->setValue(100);
  QObject::connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(updateTempo(int)));
 
  tempoBoxLayout->addWidget(tempoLabel);
  tempoBoxLayout->addWidget(tempoSpin);
  tempoBox->setLayout(tempoBoxLayout);
  //new QWidget(tempoBox);
  QWidget *runBox = new QWidget(this);
  QHBoxLayout *runBoxLayout = new QHBoxLayout;
  QLabel *runQueueLabel = new QLabel("Run Arpeggiator Queue ", runBox);
  runQueueCheck = new QCheckBox(runBox);
  runQueueCheck->setChecked(true);
  QObject::connect(runQueueCheck, SIGNAL(toggled(bool)), this, SLOT(updateRunQueue(bool)));
  runBoxLayout->addWidget(runQueueLabel);
  runBoxLayout->addWidget(runQueueCheck);
  runBox->setLayout(runBoxLayout);
  //new QWidget(runBox);

  new QWidget(this);
  passWidgetLayout->setMargin(5);
  passWidgetLayout->setSpacing(10);
  passWidgetLayout->addWidget(buttonBox);
  passWidgetLayout->addWidget(portBox);
  passWidgetLayout->addWidget(tempoBox);
  passWidgetLayout->addWidget(runBox);
setLayout(passWidgetLayout);
}

PassWidget::~PassWidget() {
  
}

void PassWidget::updateDiscard(bool on) {

  emit discardToggled(on);
  portUnmatchedSpin->setDisabled(on);
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

void PassWidget::updateTempo(int p_tempo) {

  emit(newTempo(p_tempo));
}

void PassWidget::updateRunQueue(bool on) {

  emit(runQueue(on));
}

        
