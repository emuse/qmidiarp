/*!
 * @file qmidiarp_arpwidget_lv2.cpp
 * @brief Headers for the LV2 GUI for the QMidiArp Arp plugin.
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

#ifndef QMIDIARP_ARPWIDGET_LV2_H
#define QMIDIARP_ARPWIDGET_LV2_H

#include "arpwidget.h"
#include "midiarp.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"


#define QMIDIARP_ARP_LV2UI_URI QMIDIARP_ARP_LV2_PREFIX "ui"

class qmidiarp_arpwidget_lv2 : public ArpWidget
{
  Q_OBJECT

  public:

    qmidiarp_arpwidget_lv2(
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function
        );

    void port_event(uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer);

  public slots:
    void mapParam(int value);
    void mapBool(bool on);
    void updatePattern(const QString&);
    void receivePattern(int port_index, float fValue);

  protected:
    void updateParam(int index, float fValue) const;

  private:
    LV2UI_Controller     m_controller;
    LV2UI_Write_Function writeFunction;
    QVector<Sample> data1;
    QCheckBox* transportBox;
    QSpinBox* tempoSpin;
    QString newPattern;

    int waveIndex;
    int res, size;
    double mouseXCur, mouseYCur;
    bool startoff;
    bool receivePatternFlag;
};

#endif
