/*!
 * @file arpwidget_lv2.cpp
 * @brief Headers for the LV2 GUI for the QMidiArp Arp plugin.
 *
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QBoxLayout>
#include "lv2_common.h"
#include "arpwidget.h"

#define QMIDIARP_ARP_LV2_URI QMIDIARP_LV2_URI "/arp"
#define QMIDIARP_ARP_LV2_PREFIX QMIDIARP_ARP_LV2_URI "#"
#define QMIDIARP_ARP_LV2UI_URI QMIDIARP_ARP_LV2_PREFIX "ui"

class ArpWidgetLV2 : public ArpWidget
{
  Q_OBJECT

  public:

    enum PortIndex {
            MidiIn = 0,
            MidiOut = 1,
            ATTACK = 2,
            RELEASE = 3,
            RANDOM_TICK = 4,
            RANDOM_LEN = 5,
            RANDOM_VEL = 6,
            CH_OUT = 7,
            CH_IN = 8,
            CURSOR_POS = 9, //output
            ENABLE_RESTARTBYKBD = 10,
            ENABLE_TRIGBYKBD = 11,
            MUTE = 12,
            LATCH_MODE = 13,
            OCTAVE_MODE = 14,
            OCTAVE_LOW = 15,
            OCTAVE_HIGH = 16,
            INDEX_IN1 = 17,
            INDEX_IN2 = 18,
            RANGE_IN1 = 19,
            RANGE_IN2 = 20,
            ENABLE_TRIGLEGATO = 21,
            REPEAT_MODE = 22,
            RPATTERNFLAG = 23,
            DEFER = 24,
            PATTERN_PRESET = 25,
            TRANSPORT_MODE = 26,
            TEMPO = 27,
            HOST_TEMPO = 28,
            HOST_POSITION = 29,
            HOST_SPEED = 30
    };

    ArpWidgetLV2(
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function,
        const LV2_Feature *const *host_features
        );
    ~ArpWidgetLV2();

    void port_event(uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer);
    void sendUIisUp(bool on);

  public slots:
    void mapParam(int value);
    void mapBool(bool on);
    void updatePattern(const QString&);
    void receivePattern(LV2_Atom* atom);
    void sendPattern(const QString & p);

  protected:
    void updateParam(int index, float fValue) const;

  private:
    LV2UI_Controller     m_controller;
    LV2UI_Write_Function writeFunction;
    QCheckBox* transportBox;
    QSpinBox* tempoSpin;
    QMidiArpURIs m_uris;
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame frame;

    QString newPattern;
    double mouseXCur, mouseYCur;
    bool receivePatternFlag;
    bool receivedPatternOnce;
};

#endif
