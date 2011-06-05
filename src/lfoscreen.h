/*!
 * @file lfoscreen.h
 * @brief Header for the LfoScreen class
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
#ifndef LFOSCREEN_H
#define LFOSCREEN_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QSizePolicy>
#include <QSize>

#include "midilfo.h"

#define LFOSCR_MIN_W   250
#define LFOSCR_MIN_H   120
#define LFOSCR_VMARG    10
#define LFOSCR_HMARG    20


/*! @brief Drawing widget for visualization of waveforms using QPainter
 *
 * LfoScreen is created and embedded by SeqWidget. The painter callback
 * produces a streak map of a sequence as a piano roll display. The
 * display is updated by calling LfoScreen::updateScreen() with the
 * Sample vector as argument. A cursor is placed at the corresponding
 * vector index by calling LfoScreen::updateScreen() with the integer
 * current index as an overloaded member.
 * LfoScreen emits mouse events corresponding to the Qt mousePressed()
 * and mouseMoved() events. The mouse position is transferred as a
 * double from 0 ... 1.0 representing the relative mouse position on the
 * entire LfoScreen display area.
 */
class LfoScreen : public QWidget
{
  Q_OBJECT

  private:
    //QTimer *timer;
    QVector<Sample> p_data, data;
    int mouseX, mouseY, mouseW;
    int w, h;
    int xMax;
    int currentIndex;
    int clip(int value, int min, int max, bool *outOfRange);
    bool recordMode;
    bool isMuted;

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    LfoScreen(QWidget* parent=0);
    ~LfoScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  signals:
    void mousePressed(double, double, int);
    void mouseMoved(double, double, int);
    void mouseWheel(int);

  public slots:
    void updateScreen(const QVector<Sample>& data);
    void updateScreen(int p_index);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void setRecord(bool on);
    void wheelEvent(QWheelEvent* event);
    void setMuted(bool on);
};

#endif
