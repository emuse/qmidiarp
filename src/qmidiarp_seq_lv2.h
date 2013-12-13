/*!
 * @file qmidiarp_seq_lv2.h
 * @brief Headers for the MidiSeq LV2 plugin class
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

#ifndef QMIDIARP_SEQ_LV2_H
#define QMIDIARP_SEQ_LV2_H

#include "midiseq.h"
#include "seqwidget.h"
#include "lv2_common.h"

#define QMIDIARP_SEQ_LV2_URI QMIDIARP_LV2_URI "/seq"
#define QMIDIARP_SEQ_LV2_PREFIX QMIDIARP_SEQ_LV2_URI "#"


class qmidiarp_seq_lv2 : public MidiSeq
{
public:

        qmidiarp_seq_lv2(double sample_rate, const LV2_Feature *const *host_features);

        ~qmidiarp_seq_lv2();
        /* this enum is for the float value array and shifted by 2 compared with the
         * float port indices */
        enum FloatField {
            VELOCITY = 0,
            NOTELENGTH = 1,
            RESOLUTION = 2,
            SIZE = 3,
            TRANSPOSE = 4,
            CH_OUT = 5,
            CH_IN = 6,
            CURSOR_POS = 7, //output
            LOOPMARKER = 8,
            LOOPMODE = 9,
            MUTE = 10,
            MOUSEX = 11,
            MOUSEY = 12,
            MOUSEBUTTON = 13,
            MOUSEPRESSED = 14,
            ENABLE_NOTEIN = 15,
            ENABLE_VELIN = 16,
            ENABLE_NOTEOFF = 17,
            ENABLE_RESTARTBYKBD = 18,
            ENABLE_TRIGBYKBD = 19,
            ENABLE_TRIGLEGATO = 20,
            RECORD = 21,
            DEFER = 22,
            CURR_RECSTEP = 23, //output
            TRANSPORT_CONTROL = 24,
            TRANSPORT_MODE = 25,
            TEMPO = 26,
            WAV_CONTROL = 27,
            WAV_NOTIFY = 28
        };

        void connect_port(uint32_t port, void *data);
        void run(uint32_t nframes);
        void activate();
        void deactivate();
        void updatePos(const LV2_Atom_Object* obj);
        LV2_URID_Map *uridMap;
        QMidiArpURIs m_uris;
        LV2_Atom_Forge forge;
        LV2_Atom_Forge_Frame frame;

private:

        float *val[30];
        uint64_t curFrame;
        uint64_t nCalls;
        uint64_t tempoChangeTick;
        int curTick;
        Sample currentSample;
        double mouseXCur;
        double mouseYCur;
        int mouseEvCur;
        int lastMouseIndex;
        int buttonsCur;
        int transpFromGui;
        int velFromGui;
        double internalTempo;
        double sampleRate;
        double tempo;
        bool ui_up;
        void updateParams();
        void sendWave();

        uint32_t MidiEventID;
        uint64_t transportFramesDelta;  /**< Frames since last click start */
        float transportBpm;
        float transportSpeed;
        bool transportMode;
        QVector<uint> evQueue;
        QVector<uint> evTickQueue;
        int bufPtr;

        LV2_Atom_Sequence *inEventBuffer;
        LV2_Event_Buffer *outEventBuffer;
        LV2_Atom_Sequence *transportControl;
        const LV2_Atom_Sequence *control;
        LV2_Atom_Sequence *notify;
};

#endif
