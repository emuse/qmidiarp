/*!
 * @file slider.cpp
 * @brief Widget class combining slider and spinbox and label.
 *
 * The spinbox and slider are coupled so that the slider value is
 * visualized. The stepwidth when controller by keyboard cursor keys
 * can be set as well as the display orientation.
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 */
#include <QBoxLayout>
#include <QLabel>

#include "slider.h"


Slider::Slider(int minValue, int maxValue, int pageStep, int tickStep,
        int value, Qt::Orientation orientation, const QString& label,
        QWidget * parent): QWidget(parent)
{
    slider = new QSlider(orientation, parent);
    slider->setTickInterval(tickStep);
    slider->setTickPosition(QSlider::TicksLeft);
    slider->setRange(minValue, maxValue);
    slider->setSingleStep(pageStep);
    slider->setValue(value);
    if (orientation == Qt::Vertical) {
        slider->setMinimumHeight(150);
    }
    else {
        slider->setMinimumWidth(150);
    }
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateSpinBox(int)));
    connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(fillSpinBox(int)));

    sliderSpin = new QSpinBox(this);
    sliderSpin->setRange(minValue, maxValue);
    sliderSpin->setValue(value);
    sliderSpin->setKeyboardTracking(false);
    connect(sliderSpin, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
    connect(sliderSpin, SIGNAL(editingFinished()), this, SLOT(emitAsMoved()));

    QLabel* sliderLabel = new QLabel(this);
    sliderLabel->setText(label);
    sliderLabel->setBuddy(sliderSpin);
    sliderLabel->setMinimumWidth(5*sliderLabel->fontMetrics().maxWidth());
    QBoxLayout *sliderLayout = new QBoxLayout(QBoxLayout::LeftToRight,this);
    sliderLayout->setMargin(0);
    sliderLayout->addWidget(sliderLabel);
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
    setMinimumWidth(155 + sliderLabel->width() + sliderLabel->fontMetrics().maxWidth() * 3);
    valueChangedSignalSuppressed = false;
    setLayout(sliderLayout);
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
    if (!valueChangedSignalSuppressed)
        emit(valueChanged(val));
    sliderSpin->setValue(val);
}

void Slider::fillSpinBox(int val)
{
    emit(sliderMoved(val));
    sliderSpin->setValue(val);
}

void Slider::emitAsMoved()
{
    emit(sliderMoved(sliderSpin->value()));
}

