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

        connect(muteOutAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
        connect(enableNoteOff, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
        connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
        connect(enableTrigByKbd, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
        connect(enableTrigLegato, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));

        connect(this, SIGNAL(mouseSig(double, double, int, bool))
                , this, SLOT(mapMouse(double, double, int, bool)));

        setStyleSheet(COMPACT_STYLE);

        waveIndex = 0;
        mouseXCur = 0.0;
        mouseYCur = 0.0;
}

void qmidiarp_lfowidget_lv2::port_event ( uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer )
{
    if (format == 0 && buffer_size == sizeof(float)) {

        float fValue = *(float *) buffer;
        int res = resBox->currentText().toInt();
        int size = sizeBox->currentText().toInt();
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
                    screen->setMuted(muteOutAction->isChecked());
                    screen->update();
            break;
            case 26:
                    waveFormBox->setCurrentIndex(fValue);
            break;
            case 27:
                    loopBox->setCurrentIndex(fValue);
            break;
            case 28:
                    muteOutAction->setChecked((bool)fValue);
            break;
            case 29:
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
            default: // ports 10 to 15 are the 16 wave TX ports
                    Sample sample;
                    waveIndex = abs((int)fValue ) / 128;
                    sample.value = ((int)fValue) % 128;
                    if (sample.value < 0) {
                        sample.muted = true;
                        sample.value = -sample.value;
                    }
                    else sample.muted = false;
                    sample.tick = waveIndex * TPQN / res;
                    if (waveIndex >= data.count()) data.append(sample);
                    else data.replace(waveIndex, sample);
                    //qWarning("data size %d - waveindex %d", data.count(), waveIndex);
                    if (data.count() > (res * size) + 1) {
                        data.resize(res * size);
                    }
                    if (data.count() == (res * size)) {
                        sample.value = -1;
                        sample.tick = size * TPQN;
                        data.append(sample);
                    }
                    if (data.count() == ((res * size) + 1)) {
                        screen->updateData(data);
                        screen->update();
                    }
            break;
        }
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
}

void qmidiarp_lfowidget_lv2::mapMouse(double mouseX, double mouseY, int buttons, bool pressed)
{
    updateParam(29, (float)mouseX);
    updateParam(30, (float)mouseY);
    updateParam(31, (float)buttons);
    updateParam(32, (float)(pressed)); //mouseMoved or pressed
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
}

void qmidiarp_lfowidget_lv2::updateParam(int index, float fValue) const
{
        writeFunction(m_controller, index, sizeof(float), 0, &fValue);
}
