/*
 *      arpscreen.cpp
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

#include <alsa/asoundlib.h>

#include "arpscreen.h"
#include "arpdata.h"


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
    QPolygon points(7);
    QPen pen;
    pen.setWidth(1);
    p.setFont(QFont("Helvetica", 8));
    p.setPen(pen);

    int l1, l2;
    int beat = 4;
    double nsteps = 0.0;
    double tempo = 1.0;
    double curstep = 0.0;
    int nlines = 0; 
    int notelen;
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
    int grooveTmp = 0;
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
                nsteps += tempo;
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
                tempo /= 2.0;
                if (tempo < minTempo)
                    minTempo /= 2.0;
                break;

            case '<':
                tempo *= 2.0;
                break;

            case '.':
                tempo = 1.0;
                break;
                
            case 'p':
                if (!chordMode)
                    nsteps += tempo;
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
    xscale = (w - 2 * ARPSCREEN_HMARGIN) / len;
    yscale = h - 2 * ARPSCREEN_VMARGIN;

    //Beat separators
    for (l1 = 0; l1 < nsteps + 1; l1++) {

        if (l1 < 10) {
            ofs = w / len * .5 - 4 + ARPSCREEN_HMARGIN;
        } else {
            ofs = w / len * .5 - 6 + ARPSCREEN_HMARGIN;
        }
        if ((bool)(l1%beat)) {
            p.setPen(QColor(60, 180, 60));
        } else {
            p.setPen(QColor(60, 180, 150));   
        }
        x = l1 * xscale;
        p.drawLine(ARPSCREEN_HMARGIN + x, ARPSCREEN_VMARGIN,
                ARPSCREEN_HMARGIN + x, h-ARPSCREEN_VMARGIN);

        if (l1 < nsteps) {

            //Beat numbers
            p.drawText(ofs + x, ARPSCREEN_VMARGIN, QString::number(l1+1));

            // Beat divisor separators
            p.setPen(QColor(40, 100, 40));

            for (l2 = 1; l2 < 1.0/minTempo; l2++) {
                x1 = x + l2 * xscale * minTempo;
                if (x1 < xscale*len)
                    p.drawLine(ARPSCREEN_HMARGIN + x1,
                            ARPSCREEN_VMARGIN, ARPSCREEN_HMARGIN + x1,
                            h - ARPSCREEN_VMARGIN);
            } 
        }
    }

    //Octave separators and numbers
    p.setPen(QColor(40, 120, 40));
    noctaves = maxOctave - minOctave + 1;
    for (l1 = 0; l1 < noctaves + 1; l1++) {
        ypos = yscale * l1 / noctaves + ARPSCREEN_VMARGIN;
        p.drawLine(ARPSCREEN_HMARGIN, ypos, w - ARPSCREEN_HMARGIN, ypos);        
        p.drawText(ARPSCREEN_HMARGIN / 2 - 3, 
                yscale * (l1 + 0.5) / noctaves + ARPSCREEN_VMARGIN + 4, 
                QString::number(noctaves - l1 + minOctave - 1));
    }    

    //Draw arpTicks
    curstep= 0.0;
    notelen = xscale/8;
    tempo = 1.0;
    vel = 0.8;
    octave = 0;
    chordMode = false;
    chordIndex = 0;


    for (l1 = 0; l1 < patternLen; l1++) 
    {
        c = a_pattern.at(l1);
        grooveTmp = (grooveIndex % 2) ? -grooveTick : grooveTick ;
        if (c.isDigit()) 
        {
            nlines = c.digitValue() + 1;
            if (!chordIndex) 
            {
                if (chordMode) chordIndex++;
                curstep += tempo; // * (1.0 + 0.01 * (double)grooveTmp); 
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
                    tempo /= 2.0;
                    break;

                case '<':
                    tempo *= 2.0;
                    break;

                case '.':
                    tempo = 1.0;
                    break;

                case 'p':
                    if (!chordMode)
                        curstep += tempo; // * (1.0 + 0.01 * (double)grooveTmp);
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

        if (c.isDigit()) 
        {
            octYoffset = (octave - minOctave) * (patternMaxIndex + 1);
            x = (curstep - tempo) * xscale;
//          notestreak_thick = h / (patternMaxIndex + 1) / noctaves / 2;
            if (nlines > 0) 
            {
                ypos = yscale - yscale * (nlines - 1 + octYoffset)
                            / (patternMaxIndex + 1) / noctaves
                            + ARPSCREEN_VMARGIN - 3 + notestreak_thick;
                xpos = ARPSCREEN_HMARGIN + x + notestreak_thick / 2;
                if (grooveIndex == currentIndex) 
                {
                    pen.setColor(QColor(140, 240, 140));
                    p.setPen(pen);
                    p.drawLine(ARPSCREEN_HMARGIN + x, ARPSCREEN_VMARGIN,
                            ARPSCREEN_HMARGIN + x, h);
                    pen.setWidth(notestreak_thick);
                    pen.setColor(QColor(80 + 60 * (vel - 0.8),
                               250, 120 + 60 * (vel - 0.8)));
                    p.setPen(pen);
                    p.drawLine(xpos, ypos,
                            xpos + notelen - notestreak_thick / 2, ypos);
                } else 
                {
                    pen.setWidth(notestreak_thick);
                    pen.setColor(QColor(80 + 60 * (vel - 0.8),
                                160 + 40 * (vel - 0.8),
                                80 + 60 * (vel - 0.8)));
                    p.setPen(pen);
                    p.drawLine(xpos, ypos,
                            xpos + notelen - notestreak_thick / 2, ypos);
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

void ArpScreen::setGrooveTick(int tick)
{
    grooveTick = tick;
    update();
}
void ArpScreen::setGrooveVelocity(int vel)
{
    grooveVelocity = vel;
    update();
}

void ArpScreen::setGrooveLength(int length)
{
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
    return QSize(ARPSCREEN_MINIMUM_WIDTH, ARPSCREEN_MINIMUM_HEIGHT); 
}

QSizePolicy ArpScreen::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::MinimumExpanding,
            QSizePolicy::MinimumExpanding);
}

