/*!
 * @file indicator.cpp
 * @brief Implementation of the Indicator class
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
 *
 */

#include <QPainter>
#include <QPaintDevice>
#include <QPen>
#include <QPixmap>

#include "indicator.h"


Indicator::Indicator(int size, QChar modType)
{
    p_angle = 0;
    p_size = size;
    switch (modType.toLatin1()) {
        case 'A':
            fillColor = QColor(60, 150, 30);
        break;

        case 'L':
            fillColor = QColor(150, 60, 30);
        break;

        case 'S':
            fillColor = QColor(30, 60, 150);
        break;

        default: fillColor = QColor(200, 160, 0);
    }
    needsRedraw = false;
    isMuted = false;
}

// Paint event handler.
void Indicator::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(0, 0, 0, (int)!isMuted * 200 + 55));
    if (isMuted) p.setBrush(fillColor.lighter(220));
    else p.setBrush(fillColor);
    p.setPen(pen);
    p.setRenderHint(QPainter::Antialiasing, true);
    QRectF r(5.0, 5.0, p_size, p_size);
    p.drawPie(r, 90 * 16, p_angle);
}

void Indicator::updatePercent(int p)
{
    p_angle = (100 - p) * 16 * 360 / 100;
    needsRedraw = true;
}

void Indicator::updateDraw()
{
    if (!needsRedraw) return;
    needsRedraw = false;
    update();
}

void Indicator::setMuted(bool on)
{
    isMuted = on;
    needsRedraw = true;
}

QSize Indicator::sizeHint() const
{
    return QSize(p_size, p_size);
}

QSizePolicy Indicator::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}

