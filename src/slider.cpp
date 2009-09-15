#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QSpinBox> 
#include <QBoxLayout>

#include "slider.h"


Slider::Slider(int minValue, int maxValue, int pageStep, int value,
        Qt::Orientation orientation, QString label, QWidget * parent) : QWidget(parent) {

    QHBoxLayout *sliderLayout = new QHBoxLayout;
    slider = new QSlider(orientation, parent);
    slider->setTickInterval(pageStep);
    slider->setTickPosition(QSlider::TicksLeft);
    slider->setRange(minValue, maxValue);
    slider->setSingleStep(pageStep);
    slider->setValue(value);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBox(int)));

    sliderSpin = new QSpinBox(this);
    sliderSpin->setRange(minValue, maxValue);
    sliderSpin->setValue(value);
	sliderSpin->setKeyboardTracking(false);
    connect(sliderSpin, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

	sliderLabel = new QLabel(this);
	sliderLabel->setText(label);
	sliderLabel->setFixedWidth(100);
	sliderLabel->setBuddy(sliderSpin);
    sliderLayout->setSpacing(0);
    sliderLayout->setMargin(0);
	sliderLayout->addWidget(sliderLabel);
    sliderLayout->addWidget(slider);
	sliderLayout->addWidget(sliderSpin);
	
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

void Slider::updateSpinBox(int val) {

    emit(valueChanged(val));
	sliderSpin->setValue(val);
}

