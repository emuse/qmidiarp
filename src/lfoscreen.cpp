/*!
 * @file lfoscreen.cpp
 * @brief Implementation of the LfoScreen class
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2015 <qmidiarp-devel@lists.sourceforge.net>
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

#include "lfoscreen.h"


LfoScreen::LfoScreen(QWidget* parent) : QWidget (parent)
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    mouseX = 0;
    mouseY = 0;
    xMax = LFOSCR_HMARG;
    currentIndex = 0;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    isMuted = false;
    needsRedraw = false;
}

LfoScreen::~LfoScreen()
{
}

// Paint event handler.
void LfoScreen::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int l1, l2;
    int nsteps = 0.0;
    int beat = 4;
    int npoints = 0;
    int ypos, xpos, xscale, yscale;
    w = QWidget::width();
    h = QWidget::height();
    int notestreak_thick = 2;
    int ofs;
    int x, x1;
    int beatRes = 1;
    int beatDiv = 0;
    int grooveTmp = 0;
    l2 = 0;

    //Beryll Filled Frame
    if (isMuted)
        p.fillRect(0, 0, w, h, QColor(70, 70, 70));
    else
        p.fillRect(0, 0, w, h, QColor(50, 10, 10));
    p.setViewport(0, 0, w, h);
    p.setWindow(0, 0, w, h);
    p.setPen(QColor(160, 20, 20));

    //Grid
    if (p_data.isEmpty()) return;
    npoints = p_data.count() - 1;
    nsteps = p_data.at(npoints).tick / TPQN;
    if (!nsteps) nsteps = 1;
    beatRes = npoints / nsteps;
    beatDiv = (npoints > 64) ? 64 / nsteps : beatRes;
    xscale = (w - 2 * LFOSCR_HMARG);
    yscale = h - 2 * LFOSCR_VMARG;

    //Beat separators
    for (l1 = 0; l1 < nsteps + 1; l1++) {

        if (l1 < 10) {
            ofs = w / nsteps * .5 - 4 + LFOSCR_HMARG;
        } else {
            ofs = w / nsteps * .5 - 6 + LFOSCR_HMARG;
        }
        if ((bool)(l1%beat)) {
            p.setPen(QColor(180, 100, 60));
        } else {
            p.setPen(QColor(180, 100, 100));
        }
        x = l1 * xscale / nsteps;
        p.drawLine(LFOSCR_HMARG + x, LFOSCR_VMARG,
                LFOSCR_HMARG + x, h-LFOSCR_VMARG);

        if (l1 < nsteps) {
            p.setPen(QColor(180, 150, 100));

            //Beat numbers
            p.drawText(ofs + x, LFOSCR_VMARG, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(120, 60, 20));
            x1 = x;
            for (l2 = 1; l2 < beatDiv; l2++) {
                x1 = x + l2 * xscale / nsteps / beatDiv;
                if (x1 < xscale)
                    p.drawLine(LFOSCR_HMARG + x1,
                            LFOSCR_VMARG, LFOSCR_HMARG + x1,
                            h - LFOSCR_VMARG);
            }
        }
        xMax = LFOSCR_HMARG + x;
    }

    //Draw function

    pen.setWidth(notestreak_thick);
    p.setPen(pen);
    grooveTmp = (beatRes < 32) ? grooveTick : 0;
    l1 = 0;
    while (l1 < npoints) {

        x = (l1 + .01 * (double)grooveTmp * (l1 % 2)) * xscale / npoints;
        ypos = yscale - yscale * p_data.at(l1).value / 128
                        + LFOSCR_VMARG;
        xpos = LFOSCR_HMARG + x + pen.width() / 2;
        if (p_data.at(l1).muted) {
            pen.setColor(QColor(100, 40, 5));
        }
        else {
            pen.setColor(QColor(180, 130, 50));
        }
        p.setPen(pen);
        p.drawLine(xpos, ypos,
                        xpos + (xscale / nsteps / beatRes)
                        - (pen.width()/(2+npoints/(TPQN*8))), ypos);
        l1++;
        l1+=npoints/(TPQN*4);
    }

    //Horizontal separators and numbers
    p.setPen(QColor(180, 120, 40));
    for (l1 = 0; l1 < 3; l1++) {
        ypos = yscale * l1 / 2 + LFOSCR_VMARG;
        p.drawLine(LFOSCR_HMARG, ypos, xMax, ypos);
        p.drawText(1, yscale * (l1) + LFOSCR_VMARG + 4,
                QString::number(128 * (1 - l1)));
    }

}

void LfoScreen::updateData(const QVector<Sample>& data)
{
    p_data = data;
    needsRedraw = true;
}

void LfoScreen::updateDraw()
{
    if (!needsRedraw) return;
    needsRedraw = false;
    update();
}

void LfoScreen::setMuted(bool on)
{
    isMuted = on;
    needsRedraw = true;
}

void LfoScreen::mouseMoveEvent(QMouseEvent *event)
{
    emitMouseEvent(event, 0);
}

void LfoScreen::mousePressEvent(QMouseEvent *event)
{
    emitMouseEvent(event, 1);
}

void LfoScreen::mouseReleaseEvent(QMouseEvent *event)
{
    emitMouseEvent(event, 2);
}

void LfoScreen::emitMouseEvent(QMouseEvent *event, int pressed)
{
    mouseX = event->x();
    mouseY = event->y();
    bool cl = false;

    mouseX = clip(mouseX, LFOSCR_HMARG, xMax, &cl);
    mouseY = clip(mouseY, LFOSCR_VMARG + 1, h - LFOSCR_VMARG, &cl);

    emit mouseEvent(((double)mouseX - LFOSCR_HMARG) /
                            ((double)xMax - LFOSCR_HMARG + .2),
                1. - ((double)mouseY - LFOSCR_VMARG) /
                (h - 2 * LFOSCR_VMARG), event->buttons(), pressed);
}

void LfoScreen::wheelEvent(QWheelEvent *event)
{
    mouseW = event->delta();
    emit mouseWheel(mouseW / 120);
    event->accept();
}

int LfoScreen::clip(int value, int min, int max, bool *outOfRange)
{
    int tmp = value;

    *outOfRange = false;
    if (tmp > max) {
        tmp = max;
        *outOfRange = true;
    } else if (tmp < min) {
        tmp = min;
        *outOfRange = true;
    }
    return(tmp);
}

void LfoScreen::setRecordMode(bool on)
{
    recordMode = on;
}

void LfoScreen::newGrooveValues(int tick, int vel, int length)
{
    grooveTick = tick;
    grooveVelocity = vel;
    grooveLength = length;
    needsRedraw = true;
}

QSize LfoScreen::sizeHint() const
{
    return QSize(LFOSCR_MIN_W, LFOSCR_MIN_H);
}

QSizePolicy LfoScreen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}
