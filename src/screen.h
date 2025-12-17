/*!
 * @file screen.h
 * @brief Header for the Screen class
 *
 *
 *      Copyright 2009 - 2025 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef SCREEN_H
#define SCREEN_H

#include <QtGlobal>
#include <QPainter>
#include <QPaintDevice>
#include <QPen>
#include <QPixmap>
#include <QWidget>
#include <QString>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QSizePolicy>
#include <QSize>

#define SCR_MIN_W   250
#define SCR_MIN_H   120
#define SCR_VMARG    10
#define SCR_HMARG    20


/*! @brief Drawing base widget for data visualization using QPainter
 *
 * Screen the base class for module data visualization. The painter callback
 * has to be implemented in child classes for the actual draw operation. The
 * display is updated by calling Screen::updateData() with the
 * data as argument (to be implemented in child class) followed by updateDraw().
 * Screen emits mouse events combining the Qt mousePressed()
 * and mouseMoved() events. The mouse position is transferred as a
 * double from 0 ... 1.0 representing the relative mouse position on the
 * entire Screen display area.
 */
class Screen : public QWidget
{
  Q_OBJECT

  protected:

  public:
    Screen(QWidget* parent=0);
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    virtual void emitMouseEvent(QMouseEvent *event, int pressed) = 0;
    int grooveTick, grooveVelocity, grooveLength;
    int mouseX, mouseY, mouseW;
    int w, h;
    int currentIndex;
    bool recordMode;
    bool isMuted;
    bool needsRedraw;

  signals:
    void mouseEvent(double, double, int, int pressed);
    void mouseWheel(int);

  public slots:
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void setRecordMode(bool on);
    virtual void wheelEvent(QWheelEvent* event);
    virtual void newGrooveValues(int tick, int vel, int length);
    virtual void setMuted(bool on);
    virtual void updateCursor(int c) { (void)c; };
    virtual void updateDraw();
};

#endif
