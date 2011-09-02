/*!
 * @file groovewidget.cpp
 * @brief Implementation of the GrooveWidget QWidget class
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QString>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>

#include "groovewidget.h"
#include "slider.h"


GrooveWidget::GrooveWidget(QWidget *parent) : QWidget(parent)
{
    QStringList midiCCNames;
    midiCCNames << "Tick" << "Velocity" << "Length" << "unknown";
    midiControl = new MidiControl(midiCCNames);
    midiControl->ID = -1;
    midiControl->parentDockID = -1;

    QVBoxLayout *GrooveWidgetLayout = new QVBoxLayout;

    grooveTick = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Shift"), this);
    connect(grooveTick, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveTick(int)));
    midiControl->addMidiLearnMenu(grooveTick, 0);

    grooveVelocity = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Velocity"), this);
    connect(grooveVelocity, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveVelocity(int)));
    midiControl->addMidiLearnMenu(grooveVelocity, 1);

    grooveLength = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Length"), this);
    connect(grooveLength, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveLength(int)));
    midiControl->addMidiLearnMenu(grooveLength, 2);

    GrooveWidgetLayout->addWidget(grooveTick);
    GrooveWidgetLayout->addWidget(grooveVelocity);
    GrooveWidgetLayout->addWidget(grooveLength);
    GrooveWidgetLayout->addStretch();
    setLayout(GrooveWidgetLayout);
}

GrooveWidget::~GrooveWidget()
{
}

void GrooveWidget::updateGrooveVelocity(int val)
{
    emit(newGrooveVelocity(val));
}

void GrooveWidget::updateGrooveTick(int val)
{
    emit(newGrooveTick(val));
}

void GrooveWidget::updateGrooveLength(int val)
{
    emit(newGrooveLength(val));
}
void GrooveWidget::handleController(int ccnumber, int channel, int value)
{
    int min, max, sval;
    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        min = cclist.at(l2).min;
        max = cclist.at(l2).max;
        sval = min + ((double)value * (max - min) / 127);
        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            switch (cclist.at(l2).ID) {
                case 0:
                        grooveTick->setValue(sval);
                break;

                case 1:
                        grooveVelocity->setValue(sval);
                break;

                case 2:
                        grooveLength->setValue(sval);
                break;

                default:
                break;
            }
        }
    }
}
