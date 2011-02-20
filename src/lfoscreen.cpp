/*
 *      lfoscreen.cpp
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

#include <QPolygon>
#include <QPainter>
#include <QPaintDevice>
#include <QPen>
#include <QPixmap>
#include <QBrush>
#include <QSizePolicy>
#include <QSize>

#include "lfoscreen.h"
#include "arpdata.h"


LfoScreen::LfoScreen(QWidget* parent) : QWidget (parent)
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    mouseX = 0;
    mouseY = 0;
    isMuted = false;
}

LfoScreen::~LfoScreen()
{
}

// Paint event handler.
void LfoScreen::paintEvent(QPaintEvent*)
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
    int notestreak_thick = 2;
    int ofs;
    int x, x1;
    int octave = 0;
    int maxOctave = 1;
    int minOctave = 0;
    int beatRes = 1.0;
    int beatDiv = 0;
    int noctaves = 2;
    l2 = 0;
    QChar c;

    //Beryll Filled Frame
    if (isMuted)
        p.fillRect(0, 0, w, h, QColor(70, 70, 70));
    else
        p.fillRect(0, 0, w, h, QColor(50, 10, 10));
    p.setViewport(0, 0, w, h);
    p.setWindow(0, 0, w, h);
    p.setPen(QColor(160, 20, 20));
    p.drawRect(0, 0, w - 1, h - 1);

    //Grid
    if (p_data.isEmpty()) return;
    nsteps = p_data.at(p_data.count() - 1).tick / TICKS_PER_QUARTER;
    beatRes = (p_data.count() - 1) / nsteps;
    beatDiv = (beatRes * nsteps > 64) ? 64 / nsteps : beatRes;
    npoints = beatRes * nsteps;
    xscale = (w - 2 * LFOSCREEN_HMARGIN) / nsteps;
    yscale = h - 2 * LFOSCREEN_VMARGIN;

    //Beat separators
    for (l1 = 0; l1 < nsteps + 1; l1++) {

        if (l1 < 10) {
            ofs = w / nsteps * .5 - 4 + LFOSCREEN_HMARGIN;
        } else {
            ofs = w / nsteps * .5 - 6 + LFOSCREEN_HMARGIN;
        }
        if ((bool)(l1%beat)) {
            p.setPen(QColor(180, 100, 60));
        } else {
            p.setPen(QColor(180, 100, 100));
        }
        x = l1 * xscale;
            p.drawLine(LFOSCREEN_HMARGIN + x, LFOSCREEN_VMARGIN,
                LFOSCREEN_HMARGIN + x, h-LFOSCREEN_VMARGIN);

        if (l1 < nsteps) {
            p.setPen(QColor(180, 150, 100));

            //Beat numbers
            p.drawText(ofs + x, LFOSCREEN_VMARGIN, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(120, 60, 20));
            x1 = x;
            for (l2 = 1; l2 < beatDiv; l2++) {
                x1 = x + l2 * xscale / beatDiv;
                if (x1 < xscale * nsteps)
                    p.drawLine(LFOSCREEN_HMARGIN + x1,
                            LFOSCREEN_VMARGIN, LFOSCREEN_HMARGIN + x1,
                            h - LFOSCREEN_VMARGIN);
            }
        }
    }

    //Horizontal separators and numbers
    p.setPen(QColor(180, 120, 40));
    noctaves = maxOctave - minOctave + 1;
    for (l1 = 0; l1 < noctaves + 2; l1++) {
        ypos = yscale * l1 / noctaves + LFOSCREEN_VMARGIN;
        p.drawLine(LFOSCREEN_HMARGIN, ypos, w - LFOSCREEN_HMARGIN, ypos);
        p.drawText(1,
                yscale * (l1) / noctaves + LFOSCREEN_VMARGIN + 4,
                QString::number((noctaves - l1) * 128 / noctaves));
    }

    //Draw function

    octave = 0;

    pen.setWidth(notestreak_thick);
    p.setPen(pen);
    for (l1 = 0; l1 < npoints; l1++) {

        octYoffset = 0;
        x = l1 * xscale * nsteps / npoints;
        ypos = yscale - yscale * p_data.at(l1).value / 128
                        + LFOSCREEN_VMARGIN;
        xpos = LFOSCREEN_HMARGIN + x + notestreak_thick / 2;
        if (p_data.at(l1).muted) {
            pen.setColor(QColor(100, 40, 5));
            p.setPen(pen);
        }
        else {
            pen.setColor(QColor(180, 130, 50));
            p.setPen(pen);
        }
        p.drawLine(xpos, ypos,
                        xpos + (xscale / beatRes) - notestreak_thick / 2, ypos);
    }
    pen.setWidth(1);
}


void LfoScreen::updateScreen(const QVector<Sample>& data)
{
    p_data = data;
    update();
}

void LfoScreen::setMuted(bool on)
{
    isMuted = on;
    update();
}

QSize LfoScreen::sizeHint() const
{
    return QSize(LFOSCREEN_MINIMUM_WIDTH, LFOSCREEN_MINIMUM_HEIGHT);
}

QSizePolicy LfoScreen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}

void LfoScreen::mouseMoveEvent(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    bool cl = false;

    mouseX = clip(mouseX, LFOSCREEN_HMARGIN, w - LFOSCREEN_HMARGIN, &cl);
    mouseY = clip(mouseY, LFOSCREEN_VMARGIN + 1, h - LFOSCREEN_VMARGIN, &cl);
    emit mouseMoved(((double)mouseX - LFOSCREEN_HMARGIN) /
                            ((double)w - 2 * LFOSCREEN_HMARGIN + .2),
                1. - ((double)mouseY - LFOSCREEN_VMARGIN) /
                (h - 2 * LFOSCREEN_VMARGIN), event->buttons());
}

void LfoScreen::mousePressEvent(QMouseEvent *event)
{
    mouseX = event->x();
    mouseY = event->y();
    bool cl = false;

    mouseX = clip(mouseX, LFOSCREEN_HMARGIN, w - LFOSCREEN_HMARGIN, &cl);
    mouseY = clip(mouseY, LFOSCREEN_VMARGIN + 1, h - LFOSCREEN_VMARGIN, &cl);

    emit mousePressed(((double)mouseX - LFOSCREEN_HMARGIN) /
                            ((double)w - 2 * LFOSCREEN_HMARGIN + .2),
                1. - ((double)mouseY - LFOSCREEN_VMARGIN) /
                (h - 2 * LFOSCREEN_VMARGIN), event->buttons());
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
