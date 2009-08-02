#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qboxlayout.h>
#include "slider.h"

Slider::Slider(int minValue, int maxValue, int pageStep, int value,
               Qt::Orientation orientation, QWidget * parent) : QWidget(parent) {

    QVBoxLayout *sliderLayout = new QVBoxLayout;
	QString qs;

  QWidget *labelBox = new QWidget(this);
  QHBoxLayout *labelBoxLayout = new QHBoxLayout;
  qs.sprintf("%4d", minValue);
  minLabel = new QLabel(qs, labelBox);
  qs.sprintf("%4d", value);
  valueLabel = new QLabel(qs, labelBox);
  qs.sprintf("%4d", maxValue);
  maxLabel = new QLabel(qs, labelBox);
  QWidget *sliderBox = new QWidget(this);
  QHBoxLayout *sliderBoxLayout = new QHBoxLayout;
  slider = new QSlider(orientation, parent);
  QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateLabel(int)));
  sliderBoxLayout->addWidget(slider);
  sliderBoxLayout->setSpacing(0);
  sliderBoxLayout->setMargin(0);
  sliderBox->setLayout(sliderBoxLayout);
  slider->setRange(minValue, maxValue);
  slider->setSingleStep(pageStep);
  slider->setValue(value);
  labelBoxLayout->addWidget(minLabel);
  labelBoxLayout->addWidget(valueLabel);
  labelBoxLayout->addWidget(maxLabel);
  labelBoxLayout->setSpacing(50);
  labelBoxLayout->setMargin(0);
  labelBox->setLayout(labelBoxLayout);
  sliderLayout->setSpacing(0);
  sliderLayout->setMargin(0);
  sliderLayout->addWidget(slider);
  sliderLayout->addWidget(labelBox);
  setLayout(sliderLayout);
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

