#include <QLabel>
#include <QBoxLayout>

#include "slider.h"


Slider::Slider(int minValue, int maxValue, int pageStep, int tickStep,
        int value, Qt::Orientation orientation, QString label,
        QWidget * parent): QWidget(parent)
{
    slider = new QSlider(orientation, parent);
    slider->setTickInterval(tickStep);
    slider->setTickPosition(QSlider::TicksLeft);
    slider->setRange(minValue, maxValue);
    slider->setSingleStep(pageStep);
    slider->setValue(value);
    if (orientation == Qt::Vertical)
        slider->setMinimumHeight(150);
    else
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

    QBoxLayout *sliderLayout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    sliderLayout->setMargin(0);
    sliderLayout->addWidget(sliderLabel);
    sliderLayout->addStretch();
    sliderLayout->addWidget(slider);
    sliderLayout->addSpacing(2);
    sliderLayout->addWidget(sliderSpin);
    if (orientation == Qt::Vertical) {
        sliderLayout->setDirection(QBoxLayout::TopToBottom);
        sliderLayout->setAlignment(Qt::AlignHCenter);
    }
    else {
        sliderLayout->setDirection(QBoxLayout::LeftToRight);
        sliderLayout->setAlignment(Qt::AlignTop);
    }
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

