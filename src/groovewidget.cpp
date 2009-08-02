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
#include "groovewidget.h"
#include "slider.h"

GrooveWidget::GrooveWidget(QWidget *parent, const char *name) : QVBox(parent, name) {

  setMargin(5);
  setSpacing(10);
  QHBox *tickBox = new QHBox(this);
  new QWidget(tickBox);
  QLabel *tickLabel = new QLabel("Groove Note Displacement", tickBox);
  new QWidget(tickBox);
  grooveTick = new Slider(-100, 100, 0, 0, QSlider::Horizontal, this);
  QObject::connect(grooveTick, SIGNAL(valueChanged(int)), this, SLOT(updateGrooveTick(int)));
  QHBox *velocityBox = new QHBox(this);
  new QWidget(velocityBox);
  QLabel *velocityLabel = new QLabel("Groove Velocity", velocityBox);
  new QWidget(velocityBox);
  grooveVelocity = new Slider(-100, 100, 0, 0, QSlider::Horizontal, this);
  QObject::connect(grooveVelocity, SIGNAL(valueChanged(int)), this, SLOT(updateGrooveVelocity(int)));
  QHBox *lengthBox = new QHBox(this);
  new QWidget(lengthBox);
  QLabel *lengthLabel = new QLabel("Groove Length", lengthBox);
  new QWidget(lengthBox);
  grooveLength = new Slider(-100, 100, 0, 0, QSlider::Horizontal, this);
  QObject::connect(grooveLength, SIGNAL(valueChanged(int)), this, SLOT(updateGrooveLength(int)));
  new QWidget(this);
}

GrooveWidget::~GrooveWidget() {
  
}

void GrooveWidget::updateGrooveVelocity(int val) {

  emit(newGrooveVelocity(val));
}

void GrooveWidget::updateGrooveTick(int val) {

  emit(newGrooveTick(val));
}

void GrooveWidget::updateGrooveLength(int val) {

  emit(newGrooveLength(val));
}

        