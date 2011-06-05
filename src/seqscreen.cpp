/*!
 * @file seqscreen.cpp
 * @brief Implementation of the SeqScreen class
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

#include <QPainter>
#include <QPaintDevice>
#include <QPen>
#include <QPixmap>

#include "seqscreen.h"
#include "engine.h"


SeqScreen::SeqScreen(QWidget* parent) : QWidget (parent)
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    mouseX = 0;
    mouseY = 0;
    baseOctave = 3;
    nOctaves= 4;
    recordMode = false;
    currentRecStep = false;
    currentIndex = 0;
    isMuted = false;
}

SeqScreen::~SeqScreen()
{
}

// Paint event handler.
void SeqScreen::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int l1, l2;
    double nsteps = 0.0;
    int beat = 4;
    int npoints = 0;
    int tmpval = 0;
    int ypos, xpos, xscale, yscale;
    w = QWidget::width();
    h = QWidget::height();
    int ofs;
    int x, x1;
    int minOctave = baseOctave;
    int maxOctave = nOctaves + minOctave;
    int notestreak_thick = 16 / nOctaves;
    int beatRes = 1.0;
    int beatDiv = 0;
    l2 = 0;

    //Grid setup
    if (p_data.isEmpty()) return;
    nsteps = p_data.at(p_data.count() - 1).tick / TPQN;
    beatRes = (p_data.count() - 1) / nsteps;
    beatDiv = (beatRes * nsteps > 64) ? 64 / nsteps : beatRes;
    npoints = beatRes * nsteps;
    xscale = (w - 2 * SEQSCR_HMARG) / nsteps;
    yscale = h - 2 * SEQSCR_VMARG;

    //Blue Filled Frame
    if (isMuted)
        p.fillRect(0, 0, w, h, QColor(70, 70, 70));
    else
        p.fillRect(0, 0, w, h, QColor(10, 10, 50));
    p.setViewport(0, 0, w, h);
    p.setWindow(0, 0, w, h);
    p.setPen(QColor(20, 20, 160));
    p.drawRect(0, 0, w - 1, h - 1);

    //Draw current record step
    if (recordMode)
    p.fillRect(currentRecStep * xscale * nsteps / npoints + SEQSCR_HMARG
                , SEQSCR_VMARG
                , xscale * nsteps / npoints
                , h - 2*SEQSCR_VMARG, QColor(5, 40, 100));

    //Beat separators
    for (l1 = 0; l1 < nsteps + 1; l1++) {

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
        p.drawLine(SEQSCR_HMARG + x, SEQSCR_VMARG,
                SEQSCR_HMARG + x, h-SEQSCR_VMARG);

        if (l1 < nsteps) {
            p.setPen(QColor(100, 150, 180));

            //Beat numbers
            p.drawText(ofs + x, SEQSCR_VMARG, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(20, 60, 120));
            x1 = x;
            for (l2 = 1; l2 < beatDiv; l2++) {
                x1 = x + l2 * xscale / beatDiv;
                if (x1 < xscale * nsteps)
                    p.drawLine(SEQSCR_HMARG + x1,
                            SEQSCR_VMARG, SEQSCR_HMARG + x1,
                            h - SEQSCR_VMARG);
            }
        }
    }

    //Horizontal separators and numbers
    int l3 = 0;
    for (l1 = 0; l1 <= nOctaves * 12; l1++) {
        l3 = l1%12;

        ypos = yscale * l1 / nOctaves / 12 + SEQSCR_VMARG;

        if (!l3) {
            p.setPen(QColor(30, 60, 180));
            p.drawText(w - SEQSCR_HMARG / 2 - 4,
                    ypos + SEQSCR_VMARG - 5 - yscale / nOctaves / 2,
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

    //Draw function

    pen.setWidth(notestreak_thick);
    p.setPen(pen);
    for (l1 = 0; l1 < npoints; l1++) {

        x = l1 * xscale * nsteps / npoints;
        tmpval = p_data.at(l1).value;
        if ((tmpval >= 12 * baseOctave) && (tmpval < 12 * maxOctave)) {
            ypos = yscale - yscale
                * (p_data.at(l1).value - 12 * baseOctave) / nOctaves / 12
                + SEQSCR_VMARG - pen.width() / 2;
            xpos = SEQSCR_HMARG + x + pen.width() / 2;
            if (p_data.at(l1).muted) {
                pen.setColor(QColor(5, 40, 100));
                p.setPen(pen);
            }
            else {
                pen.setColor(QColor(50, 130, 180));
                p.setPen(pen);
            }
            p.drawLine(xpos, ypos,
                            xpos + (xscale / beatRes) - pen.width(), ypos);
        }
    }
    ypos = int((mouseY - SEQSCR_VMARG + 3)/4) * 4 + SEQSCR_VMARG - 2;
    pen.setWidth(2);
    pen.setColor(QColor(50, 160, 220));
    p.setPen(pen);
    p.drawLine(SEQSCR_HMARG / 2, ypos,
                        SEQSCR_HMARG *2 / 3, ypos);

    // Cursor
    pen.setWidth(notestreak_thick);
    pen.setColor(QColor(50, 180, 220));
    p.setPen(pen);
    x = currentIndex * xscale * (int)nsteps / npoints;
    xpos = SEQSCR_HMARG + x + pen.width() / 2;
    p.drawLine(xpos, h - 2,
                    xpos + (xscale / beatRes) - pen.width(), h - 2);

}

void SeqScreen::updateScreen(const QVector<Sample>& data)
{
    p_data = data;
    update();
}

void SeqScreen::updateScreen(int p_index)
{
    currentIndex = p_index;
    update();
}

void SeqScreen::setMuted(bool on)
{
    isMuted = on;
    update();
}

void SeqScreen::mouseMoveEvent(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    if ((mouseX < SEQSCR_HMARG)|| (mouseX >= w - SEQSCR_HMARG))
        return;
    if ((mouseY <= SEQSCR_VMARG)|| (mouseY > h - SEQSCR_VMARG))
        return;
    emit mouseMoved(((double)mouseX - SEQSCR_HMARG) /
                            (w - 2 * SEQSCR_HMARG),
                1. - ((double)mouseY - SEQSCR_VMARG) /
                (h - 2 * SEQSCR_VMARG), event->buttons());

}

void SeqScreen::mousePressEvent(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    if ((mouseX < SEQSCR_HMARG)|| (mouseX >= w - SEQSCR_HMARG))
        return;
    if ((mouseY <= SEQSCR_VMARG)|| (mouseY > h - SEQSCR_VMARG))
        return;
    emit mousePressed(((double)mouseX - SEQSCR_HMARG) /
                            (w - 2 * SEQSCR_HMARG),
                1. - ((double)mouseY - SEQSCR_VMARG) /
                (h - 2 * SEQSCR_VMARG), event->buttons());
}

void SeqScreen::setRecordMode(bool on)
{
    recordMode = on;
}

void SeqScreen::setCurrentRecStep(int recStep)
{
    currentRecStep = recStep;
}

QSize SeqScreen::sizeHint() const
{
    return QSize(SEQSCR_MIN_W, SEQSCR_MIN_H);
}

QSizePolicy SeqScreen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}
