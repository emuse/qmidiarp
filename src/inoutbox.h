/*!
 * @file inoutbox.h
 * @brief Member definitions for the InOutBox GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef INOUTBOX_H
#define INOUTBOX_H

#include <QString>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>

#include "managebox.h"

/*! @brief GUI class creating an input/output parameter box
 *
*/
class InOutBox: public QWidget
{
  Q_OBJECT
  
	public:
#ifdef APPBUILD
	InOutBox( int portCount, bool compactStyle,
		bool inOutVisible, const QString& type);
    ManageBox *manageBox;
#else
	InOutBox(bool compactStyle,
		bool inOutVisible, const QString& type);
#endif
    bool modified;      /**< @brief Is set to True if unsaved parameter modifications exist */
    QLabel *rangeInLabel, *indexInLabel;
    QGroupBox *inputFilterBox;
    QComboBox *chIn;                        // Channel of input events
    QComboBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QCheckBox *enableRestartByKbd;
    QCheckBox *enableTrigByKbd;
    QCheckBox *enableTrigLegato;
    QCheckBox *enableNoteIn;
    QCheckBox *enableVelIn;
    QCheckBox *enableNoteOff;
    QSpinBox  *ccnumberInBox;
    QSpinBox  *ccnumberBox;
    
	QWidget *inOutBoxWidget;

    virtual void setChIn(int value);
    virtual void setIndexIn(int index, int value);
    virtual void setRangeIn(int index, int value);
    virtual bool isModified();
    virtual void setModified(bool);
    virtual void checkIfInputFilterSet();
/*!
* @brief Setter for the InOutBox::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/

    virtual void setChannelOut(int value);
/*!
* @brief Setter for the InOutBox::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
#ifdef APPBUILD
    virtual void setPortOut(int value);
#endif

public slots:    
    virtual void setInputFilterVisible(bool on);

};
#endif
