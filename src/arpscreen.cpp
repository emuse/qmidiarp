/*!
 * @file arpscreen.cpp
 * @brief Implementation of the ArpScreen class
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

#include "arpscreen.h"


ArpScreen::ArpScreen(QWidget* parent) : QWidget (parent)
{
    setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
    a_pattern=" ";
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    isMuted = false;
}

ArpScreen::~ArpScreen()
{
}

// Paint event handler.
void ArpScreen::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int l1, l2;
    int beat = 4;
    double nsteps = 0.0;
    double stepWidth = 1.0;
    double curstep = 0.0;
    int nlines = 0;
    int notelen;
    int dx = 0;
    int ypos, xpos;
    int octYoffset;
    int w = QWidget::width();
    int h = QWidget::height();
    int notestreak_thick = 2;

    double len;
    int xscale, yscale, ofs;
    int x, x1;
    int patternMaxIndex, patternLen;
    bool chordMode = false;
    int octave = 0;
    int maxOctave = 0;
    int minOctave = 0;
    double minTempo = 1.0;
    int noctaves = 1;
    double vel =1.0;
    double v = 0;
    int grv_cur_sft = 0;
    int grv_cur_len = 0;
    int grv_cur_vel = 0;
    int grooveIndex = 0;
    int chordIndex = 0;
    l2 = 0;
    QChar c;

    patternLen = a_pattern.length();
    patternMaxIndex = 0;

    for (l1 = 0; l1 < patternLen; l1++)
    {
        c = a_pattern.at(l1);

        if (c.isDigit()) {
            if (!chordIndex) {
                nsteps += stepWidth;
                if (chordMode) chordIndex++;
            }
            if (c.digitValue() > patternMaxIndex)
                patternMaxIndex = c.digitValue();
        }
        switch(c.toAscii()) {
            case '(':
                chordMode = true;
                chordIndex = 0;
                break;

            case ')':
                chordMode = false;
                chordIndex = 0;
                break;

            case '>':
                stepWidth /= 2.0;
                if (stepWidth < minTempo)
                    minTempo /= 2.0;
                break;

            case '<':
                stepWidth *= 2.0;
                break;

            case '.':
                stepWidth = 1.0;
                break;

            case 'p':
                if (!chordMode)
                    nsteps += stepWidth;
                break;

            case '+':
                octave++;
                if (octave > maxOctave)
                    maxOctave++;
                break;

            case '-':
                octave--;
                if (octave < minOctave)
                    minOctave--;
                break;

            case '=':
                octave=0;
                break;

            case '/':
                vel += 0.2;
                break;

            case '\\':
                vel -= 0.2;
                break;

            case 'd':
                notelen *= 2;
                break;

            case 'h':
                notelen /= 2;
                break;

            default:
                ;
        }

    }


    //Green Filled Frame
    if (isMuted)
        p.fillRect(0, 0, w, h, QColor(70, 70, 70));
    else
        p.fillRect(0, 0, w, h, QColor(10, 50, 10));

    p.setViewport(0, 0, w, h);
    p.setWindow(0, 0, w, h);
    p.setPen(QColor(20, 160, 20));
    p.drawRect(0, 0, w - 1, h - 1);

    //Grid
    len = nsteps;
    xscale = (w - 2 * ARPSCR_HMARG) / len;
    yscale = h - 2 * ARPSCR_VMARG;

    //Beat separators
    for (l1 = 0; l1 < nsteps + 1; l1++) {

        if (l1 < 10) {
            ofs = w / len * .5 - 4 + ARPSCR_HMARG;
        } else {
            ofs = w / len * .5 - 6 + ARPSCR_HMARG;
        }
        if ((bool)(l1%beat)) {
            p.setPen(QColor(60, 180, 60));
        } else {
            p.setPen(QColor(60, 180, 150));
        }
        x = l1 * xscale;
        p.drawLine(ARPSCR_HMARG + x, ARPSCR_VMARG,
                ARPSCR_HMARG + x, h-ARPSCR_VMARG);

        if (l1 < nsteps) {

            //Beat numbers
            p.drawText(ofs + x, ARPSCR_VMARG, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(40, 100, 40));

            for (l2 = 1; l2 < 1.0/minTempo; l2++) {
                x1 = x + l2 * xscale * minTempo;
                if (x1 < xscale*len)
                    p.drawLine(ARPSCR_HMARG + x1,
                            ARPSCR_VMARG, ARPSCR_HMARG + x1,
                            h - ARPSCR_VMARG);
            }
        }
    }

    //Octave separators and numbers
    p.setPen(QColor(40, 120, 40));
    noctaves = maxOctave - minOctave + 1;
    for (l1 = 0; l1 < noctaves + 1; l1++) {
        ypos = yscale * l1 / noctaves + ARPSCR_VMARG;
        p.drawLine(ARPSCR_HMARG, ypos, w - ARPSCR_HMARG, ypos);
        p.drawText(ARPSCR_HMARG / 2 - 3,
                yscale * (l1 + 0.5) / noctaves + ARPSCR_VMARG + 4,
                QString::number(noctaves - l1 + minOctave - 1));
    }

    //Draw arpTicks
    curstep= 0.0;
    notelen = xscale/8;
    stepWidth = 1.0;
    vel = 0.8;
    octave = 0;
    chordMode = false;
    chordIndex = 0;


    for (l1 = 0; l1 < patternLen; l1++)
    {
        c = a_pattern.at(l1);
        if (c.isDigit())
        {
            nlines = c.digitValue() + 1;
            if (!chordIndex)
            {
                if (chordMode) chordIndex++;
                curstep += stepWidth;
                grooveIndex++;
            }
        }
        else
        {
            switch (c.toAscii())
            {
                case '(':
                    chordMode = true;
                    chordIndex = 0;
                    break;

                case ')':
                    chordMode = false;
                    chordIndex = 0;
                    break;

                case '>':
                    stepWidth /= 2.0;
                    break;

                case '<':
                    stepWidth *= 2.0;
                    break;

                case '.':
                    stepWidth = 1.0;
                    break;

                case 'p':
                    if (!chordMode)
                        curstep += stepWidth;
                        grooveIndex++;
                   break;

                case '+':
                    octave++;
                    break;

                case '-':
                    octave--;
                    break;

                case '=':
                    octave=0;
                    break;

                case '/':
                    vel += 0.2;
                    break;

                case '\\':
                    vel -= 0.2;
                    break;

                case 'd':
                    notelen *= 2;
                    break;

                case 'h':
                    notelen /= 2;
                    break;

                default:
                    ;
            }
        }

        grv_cur_sft = ((grooveIndex % 2)) ? 0 : grooveTick ;
        grv_cur_len = ((grooveIndex % 2)) ? grooveLength : -grooveLength ;
        grv_cur_vel = ((grooveIndex % 2)) ? grooveVelocity : -grooveVelocity ;

        if (c.isDigit()) {
            octYoffset = (octave - minOctave) * (patternMaxIndex + 1);
            x = (curstep - stepWidth + 0.01 * (double)grv_cur_sft * stepWidth) * xscale;
            dx = notelen * (1.0 + 0.005 * (double)grv_cur_len);
            v = vel * (1.0 + 0.005 * (double)grv_cur_vel) - .8;

            if (nlines > 0) {
                pen.setWidth(notestreak_thick);
                pen.setColor(QColor(80 + 60 * v, 160 + 40 * v, 80 + 60 * v));
                p.setPen(pen);
                ypos = yscale - yscale * (nlines - 1 + octYoffset)
                            / (patternMaxIndex + 1) / noctaves
                            + ARPSCR_VMARG - 3 + notestreak_thick;
                xpos = ARPSCR_HMARG + x + pen.width() / 2;
                p.drawLine(xpos, ypos, xpos + dx - pen.width(), ypos);
                // Cursor
                if (grooveIndex == currentIndex) {
                    pen.setWidth(notestreak_thick * 2);
                    p.setPen(pen);
                    ypos = h - 2;
                    p.drawLine(xpos, ypos, xpos + dx - pen.width(), ypos);
                }
                pen.setWidth(1);
            }
        }
    }
}

void ArpScreen::updateScreen(const QString& pattern)
{
    a_pattern = pattern;
    update();
}

void ArpScreen::updateScreen(int p_index)
{
    currentIndex = p_index;
    update();
}

void ArpScreen::newGrooveValues(int tick, int vel, int length)
{
    grooveTick = tick;
    grooveVelocity = vel;
    grooveLength = length;
    update();
}

void ArpScreen::setMuted(bool on)
{
    isMuted = on;
    update();
}

QSize ArpScreen::sizeHint() const
{
    return QSize(ARPSCR_MIN_W, ARPSCR_MIN_H);
}

QSizePolicy ArpScreen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}

