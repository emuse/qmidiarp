/*!
 * @file seqwidget_lv2.h
 * @brief Headers for the LV2 GUI for the QMidiArp Seq plugin.
 *
 *
 *      Copyright 2009 - 2024 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef QMIDIARP_SEQWIDGET_LV2_H
#define QMIDIARP_SEQWIDGET_LV2_H

#include "seqwidget.h"
#include "lv2_common.h"
#include <QApplication>
#include <QWindow>

#define QMIDIARP_SEQ_LV2_URI QMIDIARP_LV2_URI "/seq"
#define QMIDIARP_SEQ_LV2_PREFIX QMIDIARP_SEQ_LV2_URI "#"
#define QMIDIARP_SEQ_LV2UI_URI QMIDIARP_SEQ_LV2_PREFIX "ui"
#define QMIDIARP_SEQ_LV2UI_X11_URI QMIDIARP_SEQ_LV2_PREFIX "ui_x11"

class SeqWidgetLV2 : public SeqWidget
{
  Q_OBJECT

  public:
        /* this enum is for the port indices and shifted by 2 with respect to the
         * float array indices used for data transfer */

    enum PortIndex {
            MidiIn = 0,
            MidiOut = 1,
            VELOCITY = 2,
            NOTELENGTH = 3,
            RESOLUTION = 4,
            SIZE = 5,
            TRANSPOSE = 6,
            CH_OUT = 7,
            CH_IN = 8,
            CURSOR_POS = 9, //output
            LOOPMARKER = 10,
            LOOPMODE = 11,
            MUTE = 12,
            MOUSEX = 13,
            MOUSEY = 14,
            MOUSEBUTTON = 15,
            MOUSEPRESSED = 16,
            ENABLE_NOTEIN = 17,
            ENABLE_VELIN = 18,
            ENABLE_NOTEOFF = 19,
            ENABLE_RESTARTBYKBD = 20,
            ENABLE_TRIGBYKBD = 21,
            ENABLE_TRIGLEGATO = 22,
            INDEX_IN1 = 23,
            INDEX_IN2 = 24,
            RANGE_IN1 = 25,
            RANGE_IN2 = 26,
            RECORD = 27,
            DEFER = 28,
            CURR_RECSTEP = 29, //output
            TRANSPORT_MODE = 30,
            TEMPO = 31,
            HOST_TEMPO = 32,
            HOST_POSITION = 33,
            HOST_SPEED = 34,
            DISPLAY_ZOOM = 35
    };

    SeqWidgetLV2(
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function,
        const LV2_Feature *const *host_features
        );
    ~SeqWidgetLV2();

    void port_event(uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer);

    void sendUIisUp(bool on);
    bool isIdleClosed();
    
    static void qAppInstantiate();
    static void qAppCleanup();

    static QApplication *qAppInstance();

  public slots:
    void mapParam(int value);
    void mapBool(bool on);
    void mapMouse(double mouseX, double mouseY, int buttons, int pressed);
    void receiveWave(LV2_Atom* atom);
    void receiveWavePoint(int index, int value);

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
    bool uiIsUp;
    double mouseXCur, mouseYCur;
    
    static QApplication *g_qAppInstance;
    static unsigned int  qAppCount;
};

#endif
