/**
 * @file logwidget.h
 * @brief Member definitions for the LogWidget QWidget class.
 *
 *
 *      Copyright 2009 - 2023 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QBoxLayout>
#include <QDateTime>
#include <QString>
#include <QLabel>
#include <QCheckBox>
#include <QTextEdit>

#include "midievent.h"

/*!
 * @brief Creates a QWidget displaying a log of received MIDI events from SeqDriver.
 *
 * The LogWidget is instantiated by MainWindow on program start. It is
 * embedded in a DockWindow and shown/hidden by a MainWindow menu entry and
 * tool button.
 * The Widget holds a QTextEdit panel, which is filled with information on
 * each MIDI Event passed by signalling to the LogWidget::appendEvent() slot.
 * The events are currently emitted by the SeqDriver backend.
 */
class LogWidget : public QWidget

{
  Q_OBJECT

  private:
    QVBoxLayout vBox;
    QTextEdit *logText;
    bool logActive;
    bool logMidiActive;


  public:
    LogWidget(QWidget* parent=0);
    ~LogWidget();
    QCheckBox *enableLog;
    QCheckBox *logMidiClock;

  signals:
    void sendLogEvents(bool on);

  public slots:
    void logMidiToggle(bool on);
    void enableLogToggle(bool on);
    void appendEvent(MidiEvent ev, int tick);
    void appendText(const QString&);
    void clear();
};

#endif
