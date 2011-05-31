/**
 * @file logwidget.cpp
 * @brief Implements the LogWidget QWidget class.
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
 */

#include <QString>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QStringList>
#include <QGroupBox>

#include "logwidget.h"


LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    logActive = false;
    logMidiActive = false;

    logText = new QTextEdit(this);
    logText->setTextColor(QColor(0,0,255));
    logText->setCurrentFont(QFont("Courier", 8));
    logText->setReadOnly(true);
    enableLog = new QCheckBox(this);
    enableLog->setText(tr("&Enable Log"));
    QObject::connect(enableLog, SIGNAL(toggled(bool)), this,
            SLOT(enableLogToggle(bool)));
    enableLog->setChecked(logActive);

    logMidiClock = new QCheckBox(this);
    logMidiClock->setText(tr("Log &MIDI Clock"));
    QObject::connect(logMidiClock, SIGNAL(toggled(bool)), this,
            SLOT(logMidiToggle(bool)));
    logMidiClock->setChecked(logMidiActive);

    QPushButton *clearButton = new QPushButton(tr("&Clear"), this);
    QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));
    QHBoxLayout *buttonBoxLayout = new QHBoxLayout;
    buttonBoxLayout->addWidget(enableLog);
    buttonBoxLayout->addWidget(logMidiClock);
    buttonBoxLayout->addStretch(10);
    buttonBoxLayout->addWidget(clearButton);

    QVBoxLayout *logBoxLayout = new QVBoxLayout;
    logBoxLayout->addWidget(logText);
    logBoxLayout->addLayout(buttonBoxLayout);
    setLayout(logBoxLayout);
}

LogWidget::~LogWidget()
{
}

void LogWidget::appendEvent(MidiEvent ev, int tick) {

    QString qs, qs2;

    if (!logActive) {
        return;
    }
    switch (ev.type) {
        case EV_NOTEON:
            qs.sprintf("Ch %2d, Note On %3d, Vel %3d, tick %d",
                    ev.channel + 1,
                    ev.data, ev.value, tick);
            break;
        case EV_NOTEOFF:
            qs.sprintf("Ch %2d, Note Off %3d, tick %d", ev.channel+1,
                    ev.data, tick);
            break;
        case EV_CONTROLLER:
            logText->setTextColor(QColor(100,160,0));
            qs.sprintf("Ch %2d, Ctrl %3d, Val %3d, tick %d", ev.channel+1,
                    ev.data, ev.value, tick);
            break;
        case EV_PITCHBEND:
            logText->setTextColor(QColor(100,0,255));
            qs.sprintf("Ch %2d, Pitch %5d, tick %d", ev.channel+1,
                    ev.value, tick);
            break;
        case EV_PGMCHANGE:
            logText->setTextColor(QColor(0,100,100));
            qs.sprintf("Ch %2d, PrgChg %5d, tick %d", ev.channel+1,
                    ev.value, tick);
            break;
        case EV_CLOCK:
            if (logMidiActive) {
                logText->setTextColor(QColor(150,150,150));
                qs = tr("MIDI Clock, tick");
            }
            break;
        case EV_START:
            logText->setTextColor(QColor(0,192,0));
            qs = tr("MIDI Start (Transport)");
            break;
        case EV_CONTINUE:
            logText->setTextColor(QColor(0,128,0));
            qs = tr("MIDI Continue (Transport)");
            break;
        case EV_STOP:
            logText->setTextColor(QColor(128,96,0));
            qs = tr("MIDI Stop (Transport)");
            break;
        default:
            logText->setTextColor(QColor(0,0,0));
            qs = tr("Unknown event type");
            break;
    }
    if ((ev.type != EV_CLOCK) || logMidiActive)
        logText->append(QTime::currentTime().toString(
                    "hh:mm:ss.zzz") + "  " + qs);
    logText->setTextColor(QColor(0, 0, 255));
}

void LogWidget::enableLogToggle(bool on)
{
    logActive = on;
}

void LogWidget::logMidiToggle(bool on)
{
    logMidiActive = on;
}

void LogWidget::clear()
{
    logText->clear();
}

void LogWidget::appendText(const QString& qs)
{
    logText->append(qs);
}

