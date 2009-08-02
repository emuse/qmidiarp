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
#include "passwidget.h"

PassWidget::PassWidget(int p_portcount, QWidget *parent, const char *name) : QVBox(parent, name) {

  setMargin(5);
  setSpacing(10);
  QHBox *buttonBox = new QHBox(this);
  QLabel *discardLabel = new QLabel("Discard unmatched events ", buttonBox);
  discardCheck = new QCheckBox(buttonBox);
  discardCheck->setChecked(false);
  QObject::connect(discardCheck, SIGNAL(toggled(bool)), this, SLOT(updateDiscard(bool)));
  new QWidget(buttonBox);
  QHBox *portBox = new QHBox(this);
  QLabel *portLabel = new QLabel("Send unmatched events to port ", portBox);
  portUnmatchedSpin = new QSpinBox(0, p_portcount - 1, 1, portBox);
  QObject::connect(portUnmatchedSpin, SIGNAL(valueChanged(int)), this, SLOT(updatePortUnmatched(int)));
  new QWidget(portBox);
  QHBox *tempoBox = new QHBox(this);
  QLabel *tempoLabel = new QLabel("Tempo (bpm) ", tempoBox);
  tempoSpin = new QSpinBox(10, 300, 1, tempoBox);
  tempoSpin->setValue(100);
  QObject::connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(updateTempo(int)));
  new QWidget(tempoBox);
  QHBox *runBox = new QHBox(this);
  QLabel *runQueueLabel = new QLabel("Run Arpeggiator Queue ", runBox);
  runQueueCheck = new QCheckBox(runBox);
  runQueueCheck->setChecked(true);
  QObject::connect(runQueueCheck, SIGNAL(toggled(bool)), this, SLOT(updateRunQueue(bool)));
  new QWidget(runBox);

  new QWidget(this);
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

        