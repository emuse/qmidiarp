/*!
 * @file qmidiarp_lfowidget_lv2.cpp
 * @brief Implements the the LV2 GUI for the QMidiArp Lfo plugin.
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

#include "qmidiarp_lfowidget_lv2.h"
#include "qmidiarp_lfo_lv2.h"

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

qmidiarp_lfowidget_lv2::qmidiarp_lfowidget_lv2 (
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function)
        : LfoWidget(NULL, NULL, 1, true, true, true, "LFO-LV2", 0)
{
    m_controller = ct;
    writeFunction = write_function;


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

    connect(amplitude, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(offset, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(resBox, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(sizeBox, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(freqBox, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(waveFormBox, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(loopBox, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(channelOut, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(chIn, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(ccnumberBox, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(ccnumberInBox, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));

    connect(muteOutAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableNoteOff, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableTrigByKbd, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableTrigLegato, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));

    connect(this, SIGNAL(mouseSig(double, double, int, int))
            , this, SLOT(mapMouse(double, double, int, int)));

    setStyleSheet(COMPACT_STYLE);

    waveIndex = 0;
    res = 4;
    size = 1;
    mouseXCur = 0.0;
    mouseYCur = 0.0;
    startoff = true;
}

void qmidiarp_lfowidget_lv2::port_event ( uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer )
{

    if (format == 0 && buffer_size == sizeof(float)) {


        float fValue = *(float *) buffer;
        res = resBox->currentText().toInt();
        size = sizeBox->currentText().toInt();

        if ((port_index < 26) && port_index >= 10) {
            receiveWavePoint(fValue);
        }
        else {

            switch (port_index) {
                case 2:
                        amplitude->setValue(fValue);
                break;
                case 3:
                        offset->setValue(fValue);
                break;
                case 4:
                        resBox->setCurrentIndex(fValue);
                break;
                case 5:
                        sizeBox->setCurrentIndex(fValue);
                break;
                case 6:
                        freqBox->setCurrentIndex(fValue);
                break;
                case 7:
                        channelOut->setCurrentIndex(fValue);
                break;
                case 8:
                        chIn->setCurrentIndex(fValue);
                break;
                case 9:
                        cursor->updateNumbers(res, size);
                        cursor->updatePosition(fValue);
                        cursor->update();
                break;
                case 26:
                        waveFormBox->setCurrentIndex(fValue);
                break;
                case 27:
                        loopBox->setCurrentIndex(fValue);
                break;
                case 28:
                        muteOutAction->setChecked((bool)fValue);
                        screen->setMuted(fValue);
                        screen->update();
                break;
                case 29: // these are the mouse ports
                case 30:
                case 31:
                case 32:
                break;
                case 33:
                        ccnumberBox->setValue(fValue);
                break;
                case 34:
                        ccnumberInBox->setValue(fValue);
                break;
                case 35:
                        enableNoteOff->setChecked((fValue > .5));
                break;
                case 36:
                        enableRestartByKbd->setChecked((bool)fValue);
                break;
                case 37:
                        enableTrigByKbd->setChecked((bool)fValue);
                break;
                case 38:
                        enableTrigLegato->setChecked((bool)fValue);
                break;
                case 39:
                        recordAction->setChecked((bool)fValue);
                break;
                case 40:
                        deferChangesAction->setChecked((bool)fValue);
                break;
                case 41:
                        //custom offset has changed
                        if (waveFormBox->currentIndex() == 5)
                            offset->setValue(fValue);
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
    updateParam(31, -1);
}

}
void qmidiarp_lfowidget_lv2::receiveWavePoint(float fValue)
{
    Sample sample;
    if (fValue < 0) {
        sample.muted = true;
        fValue = -fValue;
    }
    else sample.muted = false;
    waveIndex = (int)fValue  / 128;
    sample.value = ((int)fValue) % 128;
    sample.tick = waveIndex * TPQN / res;
    if (waveIndex >= data.count()) data.append(sample);
    else data.replace(waveIndex, sample);
    if (data.count() > (res * size) + 1) {
        data.resize(res * size);
    }
    if (data.count() == (res * size)) {
        sample.value = -1;
        sample.tick = size * TPQN;
        data.append(sample);
    }
    if ((data.count() == ((res * size) + 1)) && (screen)) {
        if (startoff) {
            startoff = false;
            updateParam(31, 0);
        }
        screen->updateData(data);
        screen->update();
    }
}

void qmidiarp_lfowidget_lv2::mapBool(bool on)
{
    float value = (float)on;
    if (muteOutAction == sender()) updateParam(28, value);
    else if (enableNoteOff == sender()) updateParam(35, value);
    else if (enableRestartByKbd == sender()) updateParam(36, value);
    else if (enableTrigByKbd == sender()) updateParam(37, value);
    else if (enableTrigLegato == sender()) updateParam(38, value);
    else if (recordAction == sender()) updateParam(39, value);
    else if (deferChangesAction == sender()) updateParam(40, value);
    else if (transportBox == sender()) updateParam(43, value);
}

void qmidiarp_lfowidget_lv2::mapMouse(double mouseX, double mouseY, int buttons, int pressed)
{
    updateParam(29, mouseX);
    updateParam(30, mouseY);
    updateParam(31, buttons);
    updateParam(32, pressed); //mouseMoved or pressed
}

void qmidiarp_lfowidget_lv2::mapParam(int value)
{
    if (amplitude == sender()) updateParam(2, value);
    else if (offset == sender()) updateParam(3, value);
    else if (resBox == sender()) updateParam(4, value);
    else if (sizeBox == sender()) updateParam(5, value);
    else if (freqBox == sender()) updateParam(6, value);
    else if (channelOut == sender()) updateParam(7, value);
    else if (chIn == sender()) updateParam(8, value);
    else if (waveFormBox == sender()) updateParam(26, value);
    else if (loopBox == sender()) updateParam(27, value);
    else if (ccnumberBox == sender()) updateParam(33, value);
    else if (ccnumberInBox == sender()) updateParam(34, value);
    else if (tempoSpin == sender()) updateParam(44, value);
}

void qmidiarp_lfowidget_lv2::updateParam(int index, float fValue) const
{
        writeFunction(m_controller, index, sizeof(float), 0, &fValue);
}
