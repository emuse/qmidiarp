#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qhbox.h>
#include <qvbox.h>
#include "slider.h"

Slider::Slider(int minValue, int maxValue, int pageStep, int value,
               Orientation orientation, QWidget * parent, const char * name) : QVBox(parent, name) {

  QString qs;

  QHBox *labelBox = new QHBox(this);
  qs.sprintf("%4d", minValue);
  minLabel = new QLabel(qs, labelBox);
  new QWidget(labelBox);
  new QWidget(labelBox);
  new QWidget(labelBox);
  qs.sprintf("%4d", value);
  valueLabel = new QLabel(qs, labelBox);
  new QWidget(labelBox);
  new QWidget(labelBox);
  new QWidget(labelBox);
  qs.sprintf("%4d", maxValue);
  maxLabel = new QLabel(qs, labelBox);
  slider = new QSlider(minValue, maxValue, pageStep, value, orientation, parent, name);
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateLabel(int)));
}

Slider::~Slider() {
  
}

void Slider::setValue(int val) {

  slider->setValue(val);
}

int Slider::value() {

  return(slider->value());
}

void Slider::updateLabel(int val) {

  QString qs;

  qs.sprintf("%4d", val);
  valueLabel->setText(qs);
  emit(valueChanged(val));
}

