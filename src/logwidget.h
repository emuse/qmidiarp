/**
 * @file logwidget.h
 * @brief Member definitions for the LogWidget QWidget class.
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

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QBoxLayout>
#include <QString>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDateTime>

/*!
 * @brief Creates a QWidget with three sliders controlling the arpeggiator groove.
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

  public slots:
    void logMidiToggle(bool on);
    void enableLogToggle(bool on);
    void appendEvent(int type, int data, int channel, int value);
    void appendText(const QString&);
    void clear();
};

#endif
