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

#include "lv2.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define QMIDIARP_SEQ_LV2_URI "https://git.code.sf.net/p/qmidiarp/seq"
#define QMIDIARP_SEQ_LV2_PREFIX QMIDIARP_SEQ_LV2_URI "#"


typedef struct {
    LV2_URID atom_Blank;
    LV2_URID atom_Float;
    LV2_URID atom_Long;
    LV2_URID atom_String;
    LV2_URID atom_Resource;
    LV2_URID time_Position;
    LV2_URID time_frame;
    LV2_URID time_barBeat;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_speed;
    LV2_URID hex_customwave;
    LV2_URID hex_mutemask;
} QMidiArpURIs;

class qmidiarp_seq_lv2 : public MidiSeq
{
public:

        qmidiarp_seq_lv2(double sample_rate, const LV2_Feature *const *host_features);

        ~qmidiarp_seq_lv2();

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
            WAVEDATA1 = 10, //output
            WAVEDATA2 = 11,
            WAVEDATA3 = 12,
            WAVEDATA4 = 13,
            WAVEDATA5 = 14,
            WAVEDATA6 = 15,
            WAVEDATA7 = 16,
            WAVEDATA8 = 17,
            WAVEDATA9 = 18,
            WAVEDATA10 = 19,
            WAVEDATA11 = 20,
            WAVEDATA12 = 21,
            WAVEDATA13 = 22,
            WAVEDATA14 = 23,
            WAVEDATA15 = 24,
            WAVEDATA16 = 25,
            LOOPMARKER = 26,
            LOOPMODE = 27,
            MUTE = 28,
            MOUSEX = 29,
            MOUSEY = 30,
            MOUSEBUTTON = 31,
            MOUSEPRESSED = 32,
            ENABLE_NOTEIN = 33,
            ENABLE_VELIN = 34,
            ENABLE_NOTEOFF = 35,
            ENABLE_RESTARTBYKBD = 36,
            ENABLE_TRIGBYKBD = 37,
            ENABLE_TRIGLEGATO = 38,
            RECORD = 39,
            DEFER = 40,
            CURR_RECSTEP = 41, //output
            TRANSPORT_CONTROL = 42,
            TRANSPORT_MODE = 43,
            TEMPO = 44
        };

        void connect_port(uint32_t port, void *data);
        void run(uint32_t nframes);
        void activate();
        void deactivate();
        void updatePos(const LV2_Atom_Object* obj);
        LV2_URID_Map *uridMap;
        QMidiArpURIs m_uris;

private:

        float *val[50];
        uint64_t curFrame;
        uint64_t nCalls;
        uint64_t tempoChangeTick;
        int curTick;
        Sample currentSample;
        int waveIndex;
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
        void updateParams();
        void sendWave();
        void sendSample(int ix, int port);

        uint32_t MidiEventID;
        uint64_t transportFramesDelta;  /**< Frames since last click start */
        float transportBpm;
        float transportSpeed;
        bool transportMode;
        QVector<uint> evQueue;
        QVector<uint> evTickQueue;
        int bufPtr;

        LV2_Event_Buffer *inEventBuffer;
        LV2_Event_Buffer *outEventBuffer;
        LV2_Atom_Sequence *transportControl;
};

#endif
