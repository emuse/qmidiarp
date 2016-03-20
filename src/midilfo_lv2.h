/*!
 * @file midilfo_lv2.h
 * @brief Headers for the MidiLfo LV2 plugin class
 *
 * @section LICENSE
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

#ifndef QMIDIARP_LFO_LV2_H
#define QMIDIARP_LFO_LV2_H

#include "midilfo.h"
#include "lv2_common.h"

#define QMIDIARP_LFO_LV2_URI QMIDIARP_LV2_URI "/lfo"
#define QMIDIARP_LFO_LV2_PREFIX QMIDIARP_LFO_LV2_URI "#"



class MidiLfoLV2 : public MidiLfo
{
public:

        MidiLfoLV2(double sample_rate, const LV2_Feature *const *host_features);

        ~MidiLfoLV2();

        enum FloatField {
            AMPLITUDE = 0,
            OFFSET = 1,
            RESOLUTION = 2,
            SIZE = 3,
            FREQUENCY = 4,
            CH_OUT = 5,
            CH_IN = 6,
            CURSOR_POS = 7, //output
            WAVEFORM = 8,
            LOOPMODE = 9,
            MUTE = 10,
            MOUSEX = 11,
            MOUSEY = 12,
            MOUSEBUTTON = 13,
            MOUSEPRESSED = 14,
            CC_OUT = 15,
            CC_IN = 16,
            ENABLE_NOTEOFF = 17,
            ENABLE_RESTARTBYKBD = 18,
            ENABLE_TRIGBYKBD = 19,
            ENABLE_TRIGLEGATO = 20,
            RECORD = 21,
            DEFER = 22,
            SPARE = 23, //output
            TRANSPORT_MODE = 24,
            TEMPO = 25,
            WaveOut = 26,
            HOST_TEMPO = 27,
            HOST_POSITION = 28,
            HOST_SPEED = 29
        };

        void connect_port(uint32_t port, void *data);
        void run(uint32_t nframes);
        void activate();
        void deactivate();
        void updatePosAtom(const LV2_Atom_Object* obj);
        void updatePos(uint64_t position, float bpm, int speed, bool ignore_pos=false);
        void initTransport();
        void sendWave();
        LV2_URID_Map *uridMap;
        QMidiArpURIs m_uris;
        LV2_Atom_Forge forge;
        LV2_Atom_Forge_Frame m_lv2frame;

private:

        float *val[30];
        uint64_t curFrame;
        uint64_t nCalls;
        uint64_t tempoChangeTick;
        int curTick;
        int inLfoFrame;
        double mouseXCur;
        double mouseYCur;
        int mouseEvCur;
        int lastMouseIndex;
        int buttonsCur;
        double internalTempo;
        double sampleRate;
        double tempo;
        bool ui_up;
        bool transportAtomReceived;
        void updateParams();
        void forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size);

        uint32_t MidiEventID;
        uint64_t transportFramesDelta;  /**< Frames since last click start */
        float transportBpm;
        float transportSpeed;
        bool hostTransport;

        LV2_Atom_Sequence *inEventBuffer;
        const LV2_Atom_Sequence *outEventBuffer;
};

#endif
