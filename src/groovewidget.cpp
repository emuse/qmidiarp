/*!
 * @file groovewidget.cpp
 * @brief Implementation of the GrooveWidget QWidget class
 *
 *
 *      Copyright 2009 - 2024 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QGroupBox>

#include "groovewidget.h"


GrooveWidget::GrooveWidget()
{
    midiControl = new MidiControl(this);
    midiControl->ID = -1;

    QVBoxLayout *GrooveWidgetLayout = new QVBoxLayout;

    grooveTick = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Shift"), this);
    connect(grooveTick, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveTick(int)));
    midiControl->addMidiLearnMenu("Tick", grooveTick, GROOVE_TICK);

    grooveVelocity = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Velocity"), this);
    connect(grooveVelocity, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveVelocity(int)));
    midiControl->addMidiLearnMenu("Velocity", grooveVelocity, GROOVE_VELOCITY);

    grooveLength = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Length"), this);
    connect(grooveLength, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveLength(int)));
    midiControl->addMidiLearnMenu("Length", grooveLength, 2);

    GrooveWidgetLayout->addWidget(grooveTick);
    GrooveWidgetLayout->addWidget(grooveVelocity);
    GrooveWidgetLayout->addWidget(grooveLength);
    GrooveWidgetLayout->addStretch();
    setLayout(GrooveWidgetLayout);
    tickVal = 0;
    velocityVal = 0;
    lengthVal = 0;
    needsGUIUpdate = false;
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
    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        int min = cclist.at(l2).min;
        int max = cclist.at(l2).max;
        int sval = min + ((double)value * (max - min) / 127);
        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            switch (cclist.at(l2).ID) {
                case GROOVE_TICK:
                        tickVal = sval;
                break;

                case GROOVE_VELOCITY:
                        velocityVal = sval;
                break;

                case GROOVE_LENGTH:
                        lengthVal = sval;
                break;

                default:
                break;
            }
            needsGUIUpdate = true;
        }
    }
}
void GrooveWidget::readData(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
        if (xml.name() == "tick")
            grooveTick->setValue(xml.readElementText().toInt());
        else if (xml.name() == "velocity")
            grooveVelocity->setValue(xml.readElementText().toInt());
        else if (xml.name() == "length")
            grooveLength->setValue(xml.readElementText().toInt());
        else if (xml.isStartElement() && (xml.name() == "midiControllers")) {
            midiControl->readData(xml);
        }
        else skipXmlElement(xml);
    }
}

void GrooveWidget::writeData(QXmlStreamWriter& xml)
{
    xml.writeStartElement("groove");
        xml.writeTextElement("tick",
            QString::number(grooveTick->value()));
        xml.writeTextElement("velocity",
            QString::number(grooveVelocity->value()));
        xml.writeTextElement("length",
            QString::number(grooveLength->value()));
        midiControl->writeData(xml);
    xml.writeEndElement();
}

void GrooveWidget::skipXmlElement(QXmlStreamReader& xml)
{
    if (xml.isStartElement()) {
        qWarning("Unknown Element in XML File: %s",qPrintable(xml.name().toString()));
        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.isEndElement())
                break;

            if (xml.isStartElement()) {
                skipXmlElement(xml);
            }
        }
    }
}

void GrooveWidget::updateDisplay()
{
    midiControl->update();
    if (!needsGUIUpdate) return;

    grooveTick->setValue(tickVal);
    grooveVelocity->setValue(velocityVal);
    grooveLength->setValue(lengthVal);

}
