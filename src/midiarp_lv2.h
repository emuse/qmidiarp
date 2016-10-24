/*!
 * @file midiarp_lv2.h
 * @brief Headers for the MidiArp LV2 plugin class
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

#ifndef QMIDIARP_ARP_LV2_H
#define QMIDIARP_ARP_LV2_H

#include "midiarp.h"
#include "lv2_common.h"

#define QMIDIARP_ARP_LV2_URI QMIDIARP_LV2_URI "/arp"
#define QMIDIARP_ARP_LV2_PREFIX QMIDIARP_ARP_LV2_URI "#"


class MidiArpLV2 : public MidiArp
{
public:

        MidiArpLV2(double sample_rate, const LV2_Feature *const *host_features);

        ~MidiArpLV2();

        enum FloatField {
            ATTACK = 0,
            RELEASE = 1,
            RANDOM_TICK = 2,
            RANDOM_LEN = 3,
            RANDOM_VEL = 4,
            CH_OUT = 5,
            CH_IN = 6,
            CURSOR_POS = 7, //output
            ENABLE_RESTARTBYKBD = 8,
            ENABLE_TRIGBYKBD = 9,
            MUTE = 10,
            LATCH_MODE = 11,
            OCTAVE_MODE = 12,
            OCTAVE_LOW = 13,
            OCTAVE_HIGH = 14,
            INDEX_IN1 = 15,
            INDEX_IN2 = 16,
            RANGE_IN1 = 17,
            RANGE_IN2 = 18,
            ENABLE_TRIGLEGATO = 19,
            REPEAT_MODE = 20,
            RPATTERNFLAG = 21,
            DEFER = 22,
            PATTERN_PRESET = 23,
            TRANSPORT_MODE = 24,
            TEMPO = 25,
            HOST_TEMPO = 26,
            HOST_POSITION = 27,
            HOST_SPEED = 28
        };

        void connect_port(uint32_t port, void *data);
        void run(uint32_t nframes);
        void activate();
        void deactivate();
        void updatePosAtom(const LV2_Atom_Object* obj);
        void updatePos(uint64_t position, float bpm, int speed, bool ignore_pos=false);
        void initTransport();
        LV2_URID_Map *uridMap;
        QMidiArpURIs m_uris;
        LV2_Atom_Forge forge;
        LV2_Atom_Forge_Frame m_frame;

        bool sendPatternFlag;

private:

        float *val[30];
        uint64_t curFrame;
        uint64_t nCalls;
        uint64_t tempoChangeTick;
        int curTick;
        double internalTempo;
        double sampleRate;
        double tempo;
        bool ui_up;
        bool transportAtomReceived;
        void updateParams();
        void sendPattern(const QString & p);
        void forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size);

        uint32_t MidiEventID;
        uint64_t transportFramesDelta;  /**< Frames since last click start */
        float transportBpm;
        float transportSpeed;
        bool hostTransport;
        QVector<uint> evQueue;
        QVector<uint> evTickQueue;
        int bufPtr;

        LV2_Atom_Sequence *inEventBuffer;
        const LV2_Atom_Sequence *outEventBuffer;
};

#endif
