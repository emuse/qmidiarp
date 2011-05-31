/**
 * @file groovewidget.h
 * @brief Member definitions for the GrooveWidget QWidget class.
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

#ifndef GROOVEWIDGET_H
#define GROOVEWIDGET_H

#include <QString>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include "slider.h"

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

  public:
    Slider *grooveVelocity, *grooveTick, *grooveLength;

  public:
    GrooveWidget(QWidget* parent=0);
    ~GrooveWidget();

  signals:
    void newGrooveVelocity(int);
    void newGrooveTick(int);
    void newGrooveLength(int);

  public slots:
    void updateGrooveVelocity(int);
    void updateGrooveTick(int);
    void updateGrooveLength(int);
};

#endif
