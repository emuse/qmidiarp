/*!
 * @file lfoscreen.cpp
 * @brief Implementation of the LfoScreen class
 *
 *
 *      Copyright 2009 - 2024 <qmidiarp-devel@lists.sourceforge.net>
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

#include "lfoscreen.h"

LfoScreen::LfoScreen(QWidget* parent) : Screen (parent)
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    xMax = LFOSCR_HMARG;
}

// Paint event handler.
void LfoScreen::paintEvent(QPaintEvent*)
{
    if (p_data.isEmpty()) return;

    QPainter p(this);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int beat = 4;
    int xscale, yscale;
    w = QWidget::width();
    h = QWidget::height();
    int notestreak_thick = 2;
    int x, x1;

    //Beryll Filled Frame
    if (isMuted)
        p.fillRect(0, 0, w, h, QColor(70, 70, 70));
    else
        p.fillRect(0, 0, w, h, QColor(50, 10, 10));
    p.setViewport(0, 0, w, h);
    p.setWindow(0, 0, w, h);
    p.setPen(QColor(160, 20, 20));

    //Grid
    int npoints = p_data.count() - 1;
    int nsteps = (int)( (double)p_data.at(p_data.count() - 1).tick / TPQN + .5);
    if (!nsteps) nsteps = 1;
    int beatRes = npoints / nsteps;
    int beatDiv = (npoints > 64) ? 64 / nsteps : beatRes;
    xscale = (w - 2 * LFOSCR_HMARG);
    yscale = h - 2 * LFOSCR_VMARG;

    //Beat separators
    for (int l1 = 0; l1 < nsteps + 1; l1++) {
        
        int ofs = 0;
        
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
            //Beat numbers
            p.setPen(QColor(180, 150, 100));
            p.drawText(ofs + x, LFOSCR_VMARG, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(120, 60, 20));
            for (int l2 = 1; l2 < beatDiv; l2++) {
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
    int grooveTmp = (beatRes < 32) ? grooveTick : 0;
    int l1 = 0;
    while (l1 < npoints) {

        x = (l1 + .01 * (double)grooveTmp * (l1 % 2)) * xscale / npoints;
        int ypos = yscale - yscale * p_data.at(l1).value / 128
                        + LFOSCR_VMARG;
        int xpos = LFOSCR_HMARG + x + pen.width() / 2;
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
    for (int l1 = 0; l1 < 3; l1++) {
        int ypos = yscale * l1 / 2 + LFOSCR_VMARG;
        p.drawLine(LFOSCR_HMARG, ypos, xMax, ypos);
        p.drawText(1, yscale * (l1) + LFOSCR_VMARG + 4,
                QString::number(128 * (1 - l1)));
    }

}

void LfoScreen::emitMouseEvent(QMouseEvent *event, int pressed)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mouseX = event->x();
    mouseY = event->y();
#else
    mouseX = event->position().x();
    mouseY = event->position().y();
#endif
    bool cl = false;

    mouseX = clip(mouseX, LFOSCR_HMARG, xMax, &cl);
    mouseY = clip(mouseY, LFOSCR_VMARG + 1, h - LFOSCR_VMARG, &cl);

    emit mouseEvent(((double)mouseX - LFOSCR_HMARG) /
                            ((double)xMax - LFOSCR_HMARG + .2),
                1. - ((double)mouseY - LFOSCR_VMARG) /
                (h - 2 * LFOSCR_VMARG), event->buttons(), pressed);
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

void LfoScreen::updateData(const QVector<Sample>& data)
{
    p_data = data;
    needsRedraw = true;
}
