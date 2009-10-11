#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QSpinBox> 
#include <QBoxLayout>

#include "slider.h"


Slider::Slider(int minValue, int maxValue, int pageStep, int tickStep,
        int value, Qt::Orientation orientation, QString label,
        QWidget * parent): QWidget(parent)
{
    QHBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->setMargin(0);

    slider = new QSlider(orientation, parent);
    slider->setTickInterval(tickStep);
    slider->setTickPosition(QSlider::TicksLeft);
    slider->setRange(minValue, maxValue);
    slider->setSingleStep(pageStep);
    slider->setValue(value);
    slider->setMinimumWidth(150);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBox(int)));

    sliderSpin = new QSpinBox(this);
    sliderSpin->setRange(minValue, maxValue);
    sliderSpin->setValue(value);
    sliderSpin->setKeyboardTracking(false);
    connect(sliderSpin, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

    QLabel* sliderLabel = new QLabel(this);
    sliderLabel->setText(label);
    sliderLabel->setBuddy(sliderSpin);

    sliderLayout->addWidget(sliderLabel);
    sliderLayout->addStretch();
    sliderLayout->addWidget(slider);
    sliderLayout->addSpacing(2);
    sliderLayout->addWidget(sliderSpin);

    setLayout(sliderLayout);
}

Slider::~Slider()
{
}

void Slider::setValue(int val)
{
    slider->setValue(val);
}

int Slider::value()
{
    return(slider->value());
}

void Slider::updateSpinBox(int val)
{
    emit(valueChanged(val));
    sliderSpin->setValue(val);
}

