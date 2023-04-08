/*!
 * @file screen.cpp
 * @brief Implementation of the Screen base class
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
 *
 */

#include "screen.h"


Screen::Screen(QWidget* parent) : QWidget (parent)
{
    mouseX = 0;
    mouseY = 0;
    currentIndex = 0;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    isMuted = false;
    needsRedraw = false;
    recordMode = false;
    w = QWidget::width();
    h = QWidget::height();
    mouseW = 0;
}

void Screen::updateDraw()
{
    if (!needsRedraw) return;
    needsRedraw = false;
    update();
}

void Screen::setMuted(bool on)
{
    isMuted = on;
    needsRedraw = true;
}

void Screen::mouseMoveEvent(QMouseEvent *event)
{
    emitMouseEvent(event, 0);
}

void Screen::mousePressEvent(QMouseEvent *event)
{
    emitMouseEvent(event, 1);
}

void Screen::mouseReleaseEvent(QMouseEvent *event)
{
    emitMouseEvent(event, 2);
}

void Screen::wheelEvent(QWheelEvent *event)
{
    mouseW = event->angleDelta().y();
    emit mouseWheel(mouseW / 120);
    event->accept();
}

void Screen::setRecordMode(bool on)
{
    recordMode = on;
}

void Screen::newGrooveValues(int tick, int vel, int length)
{
    grooveTick = tick;
    grooveVelocity = vel;
    grooveLength = length;
    needsRedraw = true;
}

QSize Screen::sizeHint() const
{
    return QSize(SCR_MIN_W, SCR_MIN_H);
}

QSizePolicy Screen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}
