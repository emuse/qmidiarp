/*!
 * @file qmidiarp_arp_lv2.h
 * @brief Headers for the MidiArp LV2 plugin class
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

#ifndef QMIDIARP_ARP_LV2_H
#define QMIDIARP_ARP_LV2_H

#include "midiarp.h"
#include "arpwidget.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define QMIDIARP_ARP_LV2_URI "https://git.code.sf.net/p/qmidiarp/arp"
#define QMIDIARP_ARP_LV2_PREFIX QMIDIARP_ARP_LV2_URI "#"


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
    LV2_URID pattern_string;
} QMidiArpURIs;

class qmidiarp_arp_lv2 : public MidiArp
{
public:

        qmidiarp_arp_lv2(double sample_rate, const LV2_Feature *const *host_features);

        ~qmidiarp_arp_lv2();

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
            SPARE3 = 26,
            SPARE4 = 27,
            MUTE = 28,
            MOUSEX = 29,
            MOUSEY = 30,
            MOUSEBUTTON = 31,
            MOUSEPRESSED = 32,
            INDEX_IN1 = 33,
            INDEX_IN2 = 34,
            RANGE_IN1 = 35,
            RANGE_IN2 = 36,
            TRIGGER_MODE = 37,
            REPEAT_MODE = 38,
            RPATTERNFLAG = 39,
            DEFER = 40,
            SPARE2 = 41, //output
            TRANSPORT_CONTROL = 42,
            TRANSPORT_MODE = 43,
            TEMPO = 44,
            WAVEIN1 = 45,
            WAVEIN2 = 46,
            WAVEIN3 = 47,
            WAVEIN4 = 48,
            WAVEIN5 = 49,
            WAVEIN6 = 50,
            WAVEIN7 = 51,
            WAVEIN8 = 52,
            WAVEIN9 = 53,
            WAVEIN10 = 54,
            WAVEIN11 = 55,
            WAVEIN12 = 56,
            WAVEIN13 = 57,
            WAVEIN14 = 58,
            WAVEIN15 = 59,
            WAVEIN16 = 60
        };

        void connect_port(uint32_t port, void *data);
        void run(uint32_t nframes);
        void activate();
        void deactivate();
        void updatePos(const LV2_Atom_Object* obj);
        void sendPattern(const QString & p);
        LV2_URID_Map *uridMap;
        QMidiArpURIs m_uris;
        bool sendPatternFlag;

private:

        float *val[61];
        uint64_t curFrame;
        uint64_t nCalls;
        uint64_t tempoChangeTick;
        int curTick;
        int patternSendTrials;
        double internalTempo;
        double sampleRate;
        double tempo;
        void updateParams();

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
};

#endif
