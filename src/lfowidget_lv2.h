/*!
 * @file lfowidget_lv2.h
 * @brief Headers for the LV2 GUI for the QMidiArp Lfo plugin.
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

#ifndef QMIDIARP_LFOWIDGET_LV2_H
#define QMIDIARP_LFOWIDGET_LV2_H

#include <QBoxLayout>
#include "lfowidget.h"
#include "lv2_common.h"

#define QMIDIARP_LFO_LV2_URI QMIDIARP_LV2_URI "/lfo"
#define QMIDIARP_LFO_LV2_PREFIX QMIDIARP_LFO_LV2_URI "#"
#define QMIDIARP_LFO_LV2UI_URI QMIDIARP_LFO_LV2_PREFIX "ui"

class LfoWidgetLV2 : public LfoWidget
{
  Q_OBJECT

  public:
        enum PortIndex {
            MidiIn = 0,
            MidiOut = 1,
            AMPLITUDE = 2,
            OFFSET = 3,
            RESOLUTION = 4,
            SIZE = 5,
            FREQUENCY = 6,
            CH_OUT = 7,
            CH_IN = 8,
            CURSOR_POS = 9, //output
            WAVEFORM = 10,
            LOOPMODE = 11,
            MUTE = 12,
            MOUSEX = 13,
            MOUSEY = 14,
            MOUSEBUTTON = 15,
            MOUSEPRESSED = 16,
            CC_OUT = 17,
            CC_IN = 18,
            INDEX_IN1 = 19,
            INDEX_IN2 = 20,
            RANGE_IN1 = 21,
            RANGE_IN2 = 22,
            ENABLE_NOTEOFF = 23,
            ENABLE_RESTARTBYKBD = 24,
            ENABLE_TRIGBYKBD = 25,
            ENABLE_TRIGLEGATO = 26,
            RECORD = 27,
            DEFER = 28,
            SPARE = 29,
            TRANSPORT_MODE = 30,
            TEMPO = 31,
            WaveOut = 32,
            HOST_TEMPO = 33,
            HOST_POSITION = 34,
            HOST_SPEED = 35
        };

    LfoWidgetLV2(
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function,
        const LV2_Feature *const *host_features
        );
    ~LfoWidgetLV2();

    void port_event(uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer);
    void sendUIisUp(bool on);

  public slots:
    void mapParam(int value);
    void mapBool(bool on);
    void mapMouse(double mouseX, double mouseY, int buttons, int pressed);
    void receiveWave(LV2_Atom* atom);
    void receiveWavePoint(int index, int value);
    void sendFlipWaveVertical();

  protected:
    void updateParam(int index, float fValue) const;

  private:
    LV2UI_Controller     m_controller;
    LV2UI_Write_Function writeFunction;
    QVector<Sample> data1;
    QCheckBox* transportBox;
    QSpinBox* tempoSpin;
    QMidiArpURIs m_uris;
    LV2_Atom_Forge forge;
    LV2_Atom_Forge_Frame frame;

    int res, size;
    double mouseXCur, mouseYCur;
    bool copiedToCustomFlag;
};

#endif
