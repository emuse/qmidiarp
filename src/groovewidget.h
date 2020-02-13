/**
 * @file groovewidget.h
 * @brief Member definitions for the GrooveWidget QWidget class.
 *
 *
 *      Copyright 2009 - 2019 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef GROOVEWIDGET_H
#define GROOVEWIDGET_H

#include <QBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include "slider.h"
#include "midicontrol.h"

/*!
 * @brief Creates a QWidget with three sliders controlling the arpeggiator groove.
 *
 * The GrooveWidget is instantiated by MainWindow on program start. It is
 * embedded in a DockWindow and shown/hidden by a MainWindow menu entry and
 * tool button.
 * Each Slider controls a groove setting transmitted to Engine at every change.
 *
 */
class GrooveWidget : public QWidget

{
  Q_OBJECT

    bool needsGUIUpdate;
    int tickVal, velocityVal, lengthVal;

  public:
    Slider *grooveVelocity, *grooveTick, *grooveLength;

  public:
    GrooveWidget();
    MidiControl *midiControl;

/*!
* @brief This function reads all parameters of this module from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief This function writes all parameters of this module to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);

/*!
 * @brief ENUM for Internal MIDI Control IDs supported 
 * by the GrooveWidget widget 
 */         
    enum GROOVEWIDGET_CTRL_IDS {
        GROOVE_TICK = 1,
        GROOVE_VELOCITY = 2,
        GROOVE_LENGTH = 3,
    };

#ifdef APPBUILD
/*!
* @brief allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It also advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml reference to QXmlStreamReader containing the open XML stream
*/
    void skipXmlElement(QXmlStreamReader& xml);
#endif

  signals:
    void newGrooveVelocity(int);
    void newGrooveTick(int);
    void newGrooveLength(int);

  public slots:
    void updateGrooveVelocity(int);
    void updateGrooveTick(int);
    void updateGrooveLength(int);
    void handleController(int ccnumber, int channel, int value);
    void updateDisplay();
};

#endif
