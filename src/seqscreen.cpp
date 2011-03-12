/*
 *      seqscreen.cpp
 *
 *      This file is part of QMidiArp.
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
 */

#include <QBrush>
#include <QPainter>
#include <QPaintDevice>
#include <QPen>
#include <QPixmap>
#include <QPolygon>
#include <QSize>
#include <QSizePolicy>

#include "seqscreen.h"
#include "arpdata.h"


SeqScreen::SeqScreen(QWidget* parent) : QWidget (parent)
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    mouseX = 0;
    mouseY = 0;
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
    QPolygon points(7);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int l1, l2;
    double nsteps = 0.0;
    int beat = 4;
    int npoints = 0;
    int ypos, xpos, xscale, yscale;
    int octYoffset;
    w = QWidget::width();
    h = QWidget::height();
    int notestreak_thick = 4;
    int ofs;
    int x, x1;
    int octave = 0;
    int maxOctave = 4;
    int minOctave = 0;
    int beatRes = 1.0;
    int beatDiv = 0;
    int noctaves= 4;
    l2 = 0;
    QChar c;

    //Grid setup
    if (p_data.isEmpty()) return;
    nsteps = p_data.at(p_data.count() - 1).tick / TICKS_PER_QUARTER;
    beatRes = (p_data.count() - 1) / nsteps;
    beatDiv = (beatRes * nsteps > 64) ? 64 / nsteps : beatRes;
    npoints = beatRes * nsteps;
    xscale = (w - 2 * SEQSCREEN_HMARGIN) / nsteps;
    yscale = h - 2 * SEQSCREEN_VMARGIN;

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
    p.fillRect(currentRecStep * xscale * nsteps / npoints + SEQSCREEN_HMARGIN
                , SEQSCREEN_VMARGIN
                , xscale * nsteps / npoints
                , h - 2*SEQSCREEN_VMARGIN, QColor(5, 40, 100));

    //Beat separators
    for (l1 = 0; l1 < nsteps + 1; l1++) {

        if (l1 < 10) {
            ofs = w / nsteps * .5 - 4 + SEQSCREEN_HMARGIN;
        } else {
            ofs = w / nsteps * .5 - 6 + SEQSCREEN_HMARGIN;
        }
        if ((bool)(l1%beat)) {
            p.setPen(QColor(60, 100, 180));
        } else {
            p.setPen(QColor(100, 100, 180));
        }
        x = l1 * xscale;
        p.drawLine(SEQSCREEN_HMARGIN + x, SEQSCREEN_VMARGIN,
                SEQSCREEN_HMARGIN + x, h-SEQSCREEN_VMARGIN);

        if (l1 < nsteps) {
            p.setPen(QColor(100, 150, 180));

            //Beat numbers
            p.drawText(ofs + x, SEQSCREEN_VMARGIN, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(20, 60, 120));
            x1 = x;
            for (l2 = 1; l2 < beatDiv; l2++) {
                x1 = x + l2 * xscale / beatDiv;
                if (x1 < xscale * nsteps)
                    p.drawLine(SEQSCREEN_HMARGIN + x1,
                            SEQSCREEN_VMARGIN, SEQSCREEN_HMARGIN + x1,
                            h - SEQSCREEN_VMARGIN);
            }
        }
    }

    //Horizontal separators and numbers
    noctaves = maxOctave - minOctave;
    int l3 = 0;
    for (l1 = 0; l1 <= noctaves * 12; l1++) {
        l3 = l1%12;

    if (!l3)
        p.setPen(QColor(20, 60, 180));
    else
        p.setPen(QColor(10, 20, 100));

        ypos = yscale * l1 / noctaves / 12 + SEQSCREEN_VMARGIN;
        p.drawLine(0, ypos, w - SEQSCREEN_HMARGIN, ypos);
        if ((l3 == 2) || (l3 == 4) || (l3 == 6) || (l3 == 9) || (l3 == 11)) {
            pen.setColor(QColor(20, 60, 180));
            pen.setWidth(notestreak_thick);
            p.setPen(pen);
            p.drawLine(0, ypos - notestreak_thick / 2, SEQSCREEN_HMARGIN / 2,
            ypos- notestreak_thick / 2);
            pen.setWidth(1);
            p.setPen(pen);
        }
    }

    //Draw function

    octave = 0;

    pen.setWidth(notestreak_thick);
    p.setPen(pen);
    for (l1 = 0; l1 < npoints; l1++) {

        octYoffset = 0;
        x = l1 * xscale * nsteps / npoints;
        ypos = yscale - yscale * (p_data.at(l1).value - 36) / noctaves / 12
                        + SEQSCREEN_VMARGIN - pen.width() / 2;
        xpos = SEQSCREEN_HMARGIN + x + pen.width() / 2;
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
    ypos = int((mouseY - SEQSCREEN_VMARGIN + 3)/4) * 4 + SEQSCREEN_VMARGIN - 2;
    pen.setWidth(2);
    pen.setColor(QColor(50, 160, 220));
    p.setPen(pen);
    p.drawLine(SEQSCREEN_HMARGIN / 2, ypos,
                        SEQSCREEN_HMARGIN *2 / 3, ypos);

    // Cursor
    pen.setWidth(notestreak_thick * 2);
    pen.setColor(QColor(50, 180, 220));
    p.setPen(pen);
    x = currentIndex * xscale * (int)nsteps / npoints;
    xpos = SEQSCREEN_HMARGIN + x + pen.width() / 2;
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

QSize SeqScreen::sizeHint() const
{
    return QSize(SEQSCREEN_MINIMUM_WIDTH, SEQSCREEN_MINIMUM_HEIGHT);
}

QSizePolicy SeqScreen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}

void SeqScreen::mouseMoveEvent(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    if ((mouseX < SEQSCREEN_HMARGIN)|| (mouseX >= w - SEQSCREEN_HMARGIN))
        return;
    if ((mouseY <= SEQSCREEN_VMARGIN)|| (mouseY > h - SEQSCREEN_VMARGIN))
        return;
    emit mouseMoved(((double)mouseX - SEQSCREEN_HMARGIN) /
                            (w - 2 * SEQSCREEN_HMARGIN),
                1. - ((double)mouseY - SEQSCREEN_VMARGIN) /
                (h - 2 * SEQSCREEN_VMARGIN), event->buttons());

}

void SeqScreen::mousePressEvent(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    if ((mouseX < SEQSCREEN_HMARGIN)|| (mouseX >= w - SEQSCREEN_HMARGIN))
        return;
    if ((mouseY <= SEQSCREEN_VMARGIN)|| (mouseY > h - SEQSCREEN_VMARGIN))
        return;
    emit mousePressed(((double)mouseX - SEQSCREEN_HMARGIN) /
                            (w - 2 * SEQSCREEN_HMARGIN),
                1. - ((double)mouseY - SEQSCREEN_VMARGIN) /
                (h - 2 * SEQSCREEN_VMARGIN), event->buttons());
}

void SeqScreen::setRecord(bool on)
{
    recordMode = on;
}

void SeqScreen::setCurrentRecStep(int recStep)
{
    currentRecStep = recStep;
}
