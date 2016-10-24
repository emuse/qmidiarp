/*!
 * @file cursor.cpp
 * @brief Implementation of the Cursor class
 *
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
 *
 */

#include <QPainter>
#include <QPaintDevice>
#include <QPen>
#include <QPixmap>

#include "cursor.h"


Cursor::Cursor(QChar modtype)
{
    currentIndex = 0;
    modType = modtype;
    nPoints = 16;
    nSteps = 4;
    setMinimumHeight(CSR_MIN_H);
    needsRedraw = false;
}

Cursor::~Cursor()
{
}

// Paint event handler.
void Cursor::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QColor bg, fg;
    QPen pen;

    int xpos, xscale;
    int x;
    w = QWidget::width();
    h = QWidget::height();
    int notestreak_thick = 2;

    if (modType == 'L') {
        bg = QColor(50, 10, 10);
        fg = QColor(200, 180, 70);
    }
    else if (modType == 'S') {
        bg = QColor(10, 10, 50);
        fg = QColor(50, 180, 220);
    }

    p.fillRect(0, 0, w, h, bg);

    xscale = (w - 2 * CSR_HMARG);

    // Cursor
    pen.setWidth(notestreak_thick * 2);
    pen.setColor(fg);
    p.setPen(pen);
    x = currentIndex * xscale / nPoints;
    xpos = CSR_HMARG + x + pen.width() / 2;
    p.drawLine(xpos, h - 2,
                    xpos + (xscale / nPoints) - pen.width(), h - 2);

}

void Cursor::updateNumbers(int res, int size)
{
    nPoints = res * size;
    nSteps = size;
    needsRedraw = true;
}

void Cursor::updatePosition(int p_index)
{
    currentIndex = p_index;
    needsRedraw = true;
}

void Cursor::updateDraw()
{
    if (!needsRedraw) return;
    needsRedraw = false;
    update();
}

QSize Cursor::sizeHint() const
{
    return QSize(CSR_MIN_W, CSR_MIN_H);
}

QSizePolicy Cursor::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::Fixed);
}
