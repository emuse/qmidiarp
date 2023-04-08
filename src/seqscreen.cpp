/*!
 * @file seqscreen.cpp
 * @brief Implementation of the SeqScreen class
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

#include "seqscreen.h"

SeqScreen::SeqScreen()
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    baseOctave = 3;
    nOctaves= 4;
    currentRecStep = 0;
    loopMarker = 0;
    currentIndex = 0;
    mouseY = 0;
}

// Paint event handler.
void SeqScreen::paintEvent(QPaintEvent*)
{
    if (p_data.isEmpty()) return;

    QPainter p(this);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int beat = 4;
    int tmpval = 0;
    int ypos, xpos, yscale;
    double xscale;
    w = QWidget::width();
    h = QWidget::height();
    int ofs;
    int x, x1;
    int minOctave = baseOctave;
    int maxOctave = nOctaves + minOctave;
    int notestreak_thick = 16 / nOctaves;

    //Grid setup
    int nsteps = (int)( (double)p_data.at(p_data.count() - 1).tick / TPQN + .5);
    int beatRes = (p_data.count() - 1) / nsteps;
    int beatDiv = (beatRes * nsteps > 64) ? 64 / nsteps : beatRes;
    int npoints = beatRes * nsteps;
    xscale = (double)TPQN * (w - 2 * SEQSCR_HMARG) / p_data.at(p_data.count() - 1).tick;
    yscale = h - SEQSCR_VMARG_BOT - SEQSCR_VMARG_TOP;

    //Blue Filled Frame
    if (isMuted)
        p.fillRect(0, 0, w, h, QColor(70, 70, 70));
    else
        p.fillRect(0, 0, w, h, QColor(10, 10, 50));
    
    //Loop Marker Area
    p.fillRect(SEQSCR_HMARG, h - SEQSCR_VMARG_BOT, w - 2*SEQSCR_HMARG, h, QColor(20, 20, 90));
    p.setPen(QColor(90, 250, 120));
    p.drawText(SEQSCR_HMARG / 2 - 2, h - SEQSCR_VMARG_BOT / 2 + 2, "L");
    
    p.setViewport(0, 0, w, h);
    p.setWindow(0, 0, w, h);
    p.setPen(QColor(20, 20, 160));

    //Draw current record step
    if (recordMode)
    p.fillRect(currentRecStep * xscale * nsteps / npoints + SEQSCR_HMARG
                , SEQSCR_VMARG_TOP
                , xscale * nsteps / npoints
                , yscale, QColor(5, 40, 100));

    //Beat separators
    for (int l1 = 0; l1 < nsteps + 1; l1++) {

        if (l1 < 10) {
            ofs = w / nsteps * .5 - 4 + SEQSCR_HMARG;
        } else {
            ofs = w / nsteps * .5 - 6 + SEQSCR_HMARG;
        }
        if ((bool)(l1%beat)) {
            p.setPen(QColor(60, 100, 180));
        } else {
            p.setPen(QColor(100, 100, 180));
        }
        x = l1 * xscale;
        p.drawLine(SEQSCR_HMARG + x, SEQSCR_VMARG_TOP,
                SEQSCR_HMARG + x, h);

        if (l1 < nsteps) {
            //Beat numbers
            p.setPen(QColor(100, 150, 180));
            p.drawText(ofs + x, SEQSCR_VMARG_TOP, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(20, 60, 120));
            for (int l2 = 1; l2 < beatDiv; l2++) {
                x1 = x + l2 * xscale / beatDiv;
                if (x1 < xscale * nsteps)
                    p.drawLine(SEQSCR_HMARG + x1,
                            SEQSCR_VMARG_BOT, SEQSCR_HMARG + x1, h);
            }
        }
    }

    //Horizontal separators and numbers
    for (int l1 = 0; l1 <= nOctaves * 12; l1++) {
        int l3 = l1%12;

        ypos = yscale * l1 / nOctaves / 12 + SEQSCR_VMARG_TOP;

        if (!l3) {
            p.setPen(QColor(30, 60, 180));
            p.drawText(w - SEQSCR_HMARG / 2 - 4,
                    ypos + SEQSCR_VMARG_TOP - 5 - yscale / nOctaves / 2,
                    QString::number(maxOctave - l1 / 12));
        }
        else
            p.setPen(QColor(10, 20, 100));

        p.drawLine(0, ypos, w - SEQSCR_HMARG, ypos);
        if ((l3 == 2) || (l3 == 4) || (l3 == 6) || (l3 == 9) || (l3 == 11)) {
            pen.setColor(QColor(20, 60, 180));
            pen.setWidth(notestreak_thick);
            p.setPen(pen);
            p.drawLine(0, ypos - notestreak_thick / 2, SEQSCR_HMARG / 2,
            ypos- notestreak_thick / 2);
            pen.setWidth(1);
            p.setPen(pen);
        }
    }
    p.setPen(QColor(30, 60, 180));
    p.drawLine(0, h - 2, w - SEQSCR_HMARG, h - 2);

    //Draw function

    pen.setWidth(notestreak_thick);
    p.setPen(pen);
    for (int l1 = 0; l1 < npoints; l1++) {
        x = (l1 + .01 * (double)grooveTick * (l1 % 2)) * nsteps * xscale / npoints;
        tmpval = p_data.at(l1).data;
        if ((tmpval >= 12 * baseOctave) && (tmpval < 12 * maxOctave)) {
            ypos = yscale - yscale
                * (p_data.at(l1).data - 12 * baseOctave) / nOctaves / 12
                + SEQSCR_VMARG_TOP - pen.width() / 2;
            xpos = SEQSCR_HMARG + x + pen.width() / 2;
            if (p_data.at(l1).muted) {
                pen.setColor(QColor(5, 40, 100));
            }
            else {
                pen.setColor(QColor(50, 130, 180));
            }
            p.setPen(pen);
            p.drawLine(xpos, ypos,
                            xpos + (xscale / beatRes) - pen.width(), ypos);
        }
    }
    
    // Helper tickline on keyboard
    ypos = yscale - yscale * (int)((1. - ((double)mouseY - SEQSCR_VMARG_TOP)
            / yscale) * nOctaves * 12) / nOctaves / 12
            + SEQSCR_VMARG_TOP - 1 - pen.width() / 2;

    pen.setWidth(2);
    pen.setColor(QColor(50, 160, 220));
    p.setPen(pen);
    p.drawLine(SEQSCR_HMARG / 2, ypos, SEQSCR_HMARG *2 / 3, ypos);

    // Loop Marker
    if (loopMarker) {
        QPolygon trg;
        pen.setWidth(2);
        pen.setColor(QColor(80, 250, 120));
        p.setPen(pen);
        x = abs(loopMarker) * xscale * nsteps / npoints;
        xpos = SEQSCR_HMARG + x + pen.width() / 2;
        ypos = h - SEQSCR_VMARG_BOT;
        tmpval = SEQSCR_VMARG_BOT / 2;
        trg << QPoint(xpos, ypos + 2);
        if (loopMarker > 0)
            trg << QPoint(xpos - tmpval + 2, ypos + tmpval);
        else
            trg << QPoint(xpos + tmpval - 2, ypos + tmpval);
        trg << QPoint(xpos, h - 2);
        p.drawPolygon(trg, Qt::WindingFill);
    }
}

void SeqScreen::emitMouseEvent(QMouseEvent *event, int pressed)
{
    mouseX = event->x();
    mouseY = event->y();

    emit mouseEvent(((double)mouseX - SEQSCR_HMARG) /
                            (w - 2 * SEQSCR_HMARG),
                1. - ((double)mouseY - SEQSCR_VMARG_TOP) /
                (h - SEQSCR_VMARG_TOP- SEQSCR_VMARG_BOT), event->buttons(), pressed);
}

void SeqScreen::updateData(const QVector<Sample>& data)
{
    p_data = data;
    needsRedraw = true;
}

void SeqScreen::setCurrentRecStep(int recStep)
{
    currentRecStep = recStep;
    needsRedraw = true;
}

void SeqScreen::setLoopMarker(int pos)
{
    loopMarker = pos;
    needsRedraw = true;
}

void SeqScreen::updateDispVert(int mode)
{
    switch (mode) {
        case 0:
            nOctaves = 4;
            baseOctave = 3;
        break;
        case 1:
            nOctaves = 2;
            baseOctave = 5;
        break;
        case 2:
            nOctaves = 2;
            baseOctave = 4;
        break;
        case 3:
            nOctaves = 2;
            baseOctave = 3;
        break;
        default:
            nOctaves = 4;
            baseOctave = 3;
    }
    update();
}
