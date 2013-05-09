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
        connect(channelOut, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
        connect(chIn, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
}

void qmidiarp_lfowidget_lv2::port_event ( uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer )
{
    if (format == 0 && buffer_size == sizeof(float)) {
        float fValue = *(float *) buffer;
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
            default:
            break;
        }
    }
}

void qmidiarp_lfowidget_lv2::mapParam(int value)
{
    if (amplitude == sender()) updateParam(2, value);
    if (offset == sender()) updateParam(3, value);
    if (resBox == sender()) updateParam(4, value);
    if (sizeBox == sender()) updateParam(5, value);
    if (freqBox == sender()) updateParam(6, value);
    if (channelOut == sender()) updateParam(7, value);
    if (chIn == sender()) updateParam(8, value);
}

void qmidiarp_lfowidget_lv2::updateParam(int index, float fValue) const
{
        writeFunction(m_controller, index, sizeof(float), 0, &fValue);
}
