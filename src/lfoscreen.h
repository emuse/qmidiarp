/*!
 * @file lfoscreen.h
 * @brief Header for the LfoScreen class
 *
 *
 *      Copyright 2009 - 2017 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef LFOSCREEN_H
#define LFOSCREEN_H

#include "screen.h"
#include "midiworker.h"

#define LFOSCR_MIN_W   250
#define LFOSCR_MIN_H   120
#define LFOSCR_VMARG    10
#define LFOSCR_HMARG    20


/*! @brief Drawing widget for visualization of waveforms using QPainter
 *
 * LfoScreen is created and embedded by LfoWidget. The painter callback
 * produces a streak map of a sequence as a piano roll display. The
 * display is updated by calling LfoScreen::updateData() with the
 * Sample vector as argument followed by updateDraw().
 * LfoScreen emits mouse events combining the Qt mousePressed()
 * and mouseMoved() events. The mouse position is transferred as a
 * double from 0 ... 1.0 representing the relative mouse position on the
 * entire LfoScreen display area.
 */
class LfoScreen : public Screen
{
  Q_OBJECT

  private:
    QVector<Sample> p_data, data;
    int xMax;
    void emitMouseEvent(QMouseEvent *event, int pressed);
    int clip(int value, int min, int max, bool *outOfRange);

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    LfoScreen(QWidget* parent=0);

  public slots:
    void updateData(const QVector<Sample>& data);
};

#endif
