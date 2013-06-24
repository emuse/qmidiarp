/*!
 * @file qmidiarp_arpwidget_lv2.cpp
 * @brief Implements the the LV2 GUI for the QMidiArp Arp plugin.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2013 <qmidiarp-devel@lists.sourceforge.net>
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

#include "qmidiarp_arpwidget_lv2.h"
#include "qmidiarp_arp_lv2.h"

#include <unistd.h>
#include <ctime>

#ifndef COMPACT_STYLE
#define COMPACT_STYLE "QLabel { font: 7pt; } \
    QComboBox { font: 7pt; max-height: 15px;} \
    QToolButton { max-height: 20px;} \
    QSpinBox { font: 7pt; max-height: 20px;} \
    QCheckBox { font: 7pt; max-height: 20px;} \
    QGroupBox { font: 7pt; }"

#endif

qmidiarp_arpwidget_lv2::qmidiarp_arpwidget_lv2 (
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function)
        : ArpWidget(NULL, NULL, 1, true, true, true, "ARP-LV2", 0)
{
    m_controller = ct;
    writeFunction = write_function;
    receivePatternFlag = true;

    transportBox = new QCheckBox(this);
    QLabel *transportBoxLabel = new QLabel(tr("&Sync with Host"),this);
    transportBoxLabel->setBuddy(transportBox);
    transportBox->setToolTip(tr("Sync to Transport from Host"));
    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(120);
    tempoSpin->setKeyboardTracking(false);
    tempoSpin->setToolTip(tr("Tempo of internal clock"));
    connect(transportBox, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(transportBox, SIGNAL(toggled(bool)), tempoSpin, SLOT(setDisabled(bool)));
    transportBox->setChecked(false);

    inOutBox->layout()->addWidget(transportBoxLabel);
    inOutBox->layout()->addWidget(transportBox);
    inOutBox->layout()->addWidget(tempoSpin);

    connect(attackTime, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(releaseTime, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(randomTick, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(randomLength, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(randomVelocity, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(indexIn[0], SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(indexIn[1], SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(rangeIn[0], SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(rangeIn[1], SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(triggerMode, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(channelOut, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(chIn, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(patternPresetBox, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(repeatPatternThroughChord, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(patternText, SIGNAL(textChanged(const QString&)), this,
            SLOT(updatePattern(const QString&)));

    connect(muteOutAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));

    setStyleSheet(COMPACT_STYLE);

    waveIndex = 0;
    res = 4;
    size = 4;
    mouseXCur = 0.0;
    mouseYCur = 0.0;
    startoff = true;
}

void qmidiarp_arpwidget_lv2::port_event ( uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer )
{

    if (format == 0 && buffer_size == sizeof(float)) {


        float fValue = *(float *) buffer;

        if ((port_index < 26) && (port_index >= 10)) {
            receivePattern(port_index, fValue);
        }
        else {

            switch (port_index) {
                case 2:
                        attackTime->setValue(fValue);
                break;
                case 3:
                        releaseTime->setValue(fValue);
                break;
                case 4:
                        randomTick->setValue(fValue);
                break;
                case 5:
                        randomLength->setValue(fValue);
                break;
                case 6:
                        randomVelocity->setValue(fValue);
                break;
                case 7:
                        channelOut->setCurrentIndex(fValue);
                break;
                case 8:
                        chIn->setCurrentIndex(fValue);
                break;
                case 9:
                        screen->updateScreen((int)fValue);
                        screen->update();
                break;
                case 26:

                break;
                case 27:
                        patternPresetBox->setCurrentIndex(fValue);
                break;
                case 28:
                        muteOutAction->setChecked((bool)fValue);
                        screen->setMuted(fValue);
                        screen->update();
                break;
                case 29: // these are the mouse ports (not in use with arp)
                case 30:
                case 31:
                case 32:
                break;
                case 33:
                        indexIn[0]->setValue(fValue);
                break;
                case 34:
                        indexIn[1]->setValue(fValue);
                break;
                case 35:
                        rangeIn[0]->setValue(fValue);
                break;
                case 36:
                        rangeIn[1]->setValue(fValue);
                break;
                case 37:
                        triggerMode->setCurrentIndex(fValue);
                break;
                case 38:
                        repeatPatternThroughChord->setCurrentIndex(fValue);
                break;
                case 39:
                        if ((bool)fValue != receivePatternFlag) {
                            qWarning("setting receive flag in GUI %d", (int)fValue);
                            receivePatternFlag = (bool)fValue;
                        }
                break;
                case 40:
                        deferChangesAction->setChecked((bool)fValue);
                break;
                case 41:
                        //spare
                break;
                case 42: // metronome port
                break;
                case 43:
                        transportBox->setChecked((bool)fValue);
                break;
                case 44:
                        tempoSpin->setValue((int)fValue);
                break;
                default: // ports 10 to 25 are the 16 wave TX ports
                break;
            }
        }
    }
    // this is a dirty way to provoke retransmission of wave data at
    // ui start. Mouse button is set to -1 and reset to 0 when data has
    // been received.
    if (startoff) {
    }

}
void qmidiarp_arpwidget_lv2::updatePattern(const QString& p_pattern)
{
    if (p_pattern.count() > 64) return;


    QChar c;
    QString pattern = p_pattern;
    int patternLen = p_pattern.length();
    int patternMaxIndex = 0;
    double minStepWidth = 1.0;
    int minOctave = 0;
    int maxOctave = 0;

    double stepwd = 1.0;
    double nsteps = 0.;
    int chordindex = 0;
    bool chordmd = false;
    int oct = 0;
    int npoints = 0;

    // strip off trailing control tokens
    if (patternLen) {
        c = (pattern.at(patternLen - 1));
        while (!c.isDigit() && (c != 'p') && (c != ')')) {
            pattern = pattern.left(patternLen - 1);
            patternLen--;
            if (patternLen < 1) break;
            c = (pattern.at(patternLen - 1));
        }
    }

    // encode into floats and send pattern to GUI via WAVEDATA ports

    int l1, l2;
    int ix = 0;
    int32_t n;
    for (l1 = 0; l1 < 16; l1++) {
        n = 0;
        for (l2 = 24; l2 > 0; l2-=8) {
            if (ix >= pattern.count()) break;
            n |= ( (char)pattern.at(ix).toLatin1() << l2 );
            ix++;
        }
        updateParam(l1 + 45, (float)n / 8192.);
    }

    // determine some useful properties of the arp pattern,
    // number of octaves, step width and number of steps in beats and
    // number of points

    for (l1 = 0; l1 < patternLen; l1++) {
        c = pattern.at(l1);

        if (c.isDigit()) {
            if (!chordindex) {
                nsteps += stepwd;
                npoints++;
                if (chordmd) chordindex++;
            }
            if (c.digitValue() > patternMaxIndex)
                patternMaxIndex = c.digitValue();
        }
        switch(c.toAscii()) {
            case '(':
                chordmd = true;
                chordindex = 0;
                break;

            case ')':
                chordmd = false;
                chordindex = 0;
                break;

            case '>':
                stepwd *= .5;
                if (stepwd < minStepWidth)
                    minStepWidth *= .5;
                break;

            case '<':
                stepwd *= 2.0;
                break;

            case '.':
                stepwd = 1.0;
                break;

            case 'p':
                if (!chordmd)
                    nsteps += stepwd;
                    npoints++;
                break;

            case '+':
                oct++;
                if (oct > maxOctave)
                    maxOctave++;
                break;

            case '-':
                oct--;
                if (oct < minOctave)
                    minOctave--;
                break;

            case '=':
                oct=0;
                break;

            default:
                ;
        }
    }
    screen->updateScreen(pattern, minOctave, maxOctave, minStepWidth,
                    nsteps, patternMaxIndex);
    screen->update();

}

void qmidiarp_arpwidget_lv2::receivePattern(int port_index, float fValue)
{
    int l1 = port_index - 10;
    if (!l1) newPattern.fill(QChar(0), 64);

    uint32_t n;
    unsigned char c;
    n = (uint32_t)(fValue*8192.);

    for (int l2 = 0; l2 < 3; l2++) {
        c = (n >> (24 - (l2 * 8))) & 0xff;
        newPattern.replace(l1*3 + l2, 1, c);
    }

    QString txPattern = newPattern.remove(QChar(0));
    updatePattern(txPattern);
    patternText->setText(txPattern);
    screen->update();
}

void qmidiarp_arpwidget_lv2::mapBool(bool on)
{
    float value = (float)on;
    if (muteOutAction == sender()) updateParam(28, value);
    else if (deferChangesAction == sender()) updateParam(40, value);
    else if (latchModeAction == sender()) updateParam(41, value);
    else if (transportBox == sender()) updateParam(43, value);
}

void qmidiarp_arpwidget_lv2::mapParam(int value)
{
    if (attackTime == sender()) updateParam(2, value);
    else if (releaseTime == sender()) updateParam(3, value);
    else if (randomTick == sender()) updateParam(4, value);
    else if (randomLength == sender()) updateParam(5, value);
    else if (randomVelocity == sender()) updateParam(6, value);
    else if (channelOut == sender()) updateParam(7, value);
    else if (chIn == sender()) updateParam(8, value);
    else if (patternPresetBox == sender()) updateParam(27, value);
    else if (indexIn[0] == sender()) updateParam(33, value);
    else if (indexIn[1] == sender()) updateParam(34, value);
    else if (rangeIn[0] == sender()) updateParam(35, value);
    else if (rangeIn[1] == sender()) updateParam(36, value);
    else if (triggerMode == sender()) updateParam(37, value);
    else if (repeatPatternThroughChord == sender()) updateParam(38, value);
    else if (tempoSpin == sender()) updateParam(44, value);
}

void qmidiarp_arpwidget_lv2::updateParam(int index, float fValue) const
{
        writeFunction(m_controller, index, sizeof(float), 0, &fValue);
}
