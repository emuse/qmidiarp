/*!
 * @file midiarp_lv2.cpp
 * @brief Implements an LV2 plugin inheriting from MidiArp
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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

#include <cstdio>
#include <cmath>
#include "midiarp_lv2.h"

MidiArpLV2::MidiArpLV2 (
    double sample_rate, const LV2_Feature *const *host_features )
    :MidiArp()
{
    MidiEventID = 0;
    sampleRate = sample_rate;
    curFrame = 0;
    inEventBuffer = NULL;
    outEventBuffer = NULL;
    tempo = 120.0f;
    internalTempo = 120.0f;

    transportBpm = 120.0f;
    transportFramesDelta = 0;
    curTick = 0;
    tempoChangeTick = 0;
    hostTransport = true;
    transportSpeed = 0;
    trStartingTick = 0;
    transportAtomReceived = false;

    sendPatternFlag = false;
    ui_up = false;

    bufPtr = 0;

    LV2_URID_Map *urid_map;


    /* Scan host features for URID map */

    for (int i = 0; host_features[i]; ++i) {
        if (::strcmp(host_features[i]->URI, LV2_URID_URI "#map") == 0) {
            urid_map = (LV2_URID_Map *) host_features[i]->data;
            if (urid_map) {
                MidiEventID = urid_map->map(urid_map->handle, LV2_MIDI_EVENT_URI);
                break;
            }
        }
    }
    if (!urid_map) {
        printf("Host does not support urid:map.\n");
        return;
    }

    lv2_atom_forge_init(&forge, urid_map);

    /* Map URIS */
    QMidiArpURIs* const uris = &m_uris;
    map_uris(urid_map, uris);

    uridMap = urid_map;
}


MidiArpLV2::~MidiArpLV2 (void)
{
}

void MidiArpLV2::connect_port ( uint32_t port, void *seqdata )
{
    switch(port) {
    case 0:
        inEventBuffer = (LV2_Atom_Sequence*)seqdata;
        break;
    case 1:
        outEventBuffer = (const LV2_Atom_Sequence*)seqdata;
        break;
    default:
        val[port - 2] = (float *) seqdata;
        break;
    }
}

void MidiArpLV2::updatePosAtom(const LV2_Atom_Object* obj)
{
    if (!hostTransport) return;

    QMidiArpURIs* const uris = &m_uris;

    uint64_t pos1 = transportFramesDelta;
    float bpm1 = tempo;
    int speed1 = transportSpeed;

    // flag that the host sends transport information via atom port and
    // that we will no longer process designated port events
    transportAtomReceived = true;

    LV2_Atom *bpm = NULL, *speed = NULL, *pos = NULL;
    lv2_atom_object_get(obj,
                        uris->time_frame, &pos,
                        uris->time_beatsPerMinute, &bpm,
                        uris->time_speed, &speed,
                        NULL);

    if (bpm && bpm->type == uris->atom_Float) bpm1 = ((LV2_Atom_Float*)bpm)->body;
    if (pos && pos->type == uris->atom_Long)  pos1 = ((LV2_Atom_Long*)pos)->body;
    if (speed && speed->type == uris->atom_Float) speed1 = ((LV2_Atom_Float*)speed)->body;

    updatePos(pos1, bpm1, speed1);
}

void MidiArpLV2::updatePos(uint64_t pos, float bpm, int speed, bool ignore_pos)
{
    if (transportBpm != bpm) {
        /* Tempo changed */
        transportBpm = bpm;
        tempo = transportBpm;
        transportSpeed = 0;
    }

    if (!ignore_pos) {
        const float frames_per_beat = 60.0f / transportBpm * sampleRate;
        transportFramesDelta = pos;
        tempoChangeTick = pos * TPQN / frames_per_beat;
    }    
    if (transportSpeed != speed) {
        /* Speed changed, e.g. 0 (stop) to 1 (play) */
        transportSpeed = speed;
        if (transportSpeed) {
            curFrame = transportFramesDelta;
            foldReleaseTicks(trStartingTick - tempoChangeTick);
            setNextTick(tempoChangeTick);
        } 

        trStartingTick = tempoChangeTick;
    }
}

void MidiArpLV2::run ( uint32_t nframes )
{
    const QMidiArpURIs* uris = &m_uris;
    const uint32_t capacity = outEventBuffer->atom.size;

    lv2_atom_forge_set_buffer(&forge, (uint8_t*)outEventBuffer, capacity);
    lv2_atom_forge_sequence_head(&forge, &m_frame, 0);

    sendPattern(pattern);
    updateParams();


    if (inEventBuffer) {
        LV2_ATOM_SEQUENCE_FOREACH(inEventBuffer, event) {
            // Control Atom Input
            if (event && (event->body.type == uris->atom_Object
                        || event->body.type == uris->atom_Blank)) {
                const LV2_Atom_Object* obj = (LV2_Atom_Object*)&event->body;
                /* interpret atom-objects: */
                if (obj->body.otype == uris->time_Position) {
                    /* Received position information, update */
                    if (hostTransport) updatePosAtom(obj);
                }
                else if (obj->body.otype == uris->ui_up) {
                    /* UI was activated */
                    ui_up = true;
                    sendPatternFlag = true;
                }
                else if (obj->body.otype == uris->ui_down) {
                    /* UI was closed */
                    ui_up = false;
                }
                else if (obj->body.otype == uris->pattern_string) {
                    /* UI sends pattern string */
                    const LV2_Atom* a0 = NULL;
                    lv2_atom_object_get(obj, uris->pattern_string, &a0, 0);
                    if (a0 && a0->type == uris->atom_String) {
                        const char* p = (const char*)LV2_ATOM_BODY(a0);
                        
                        std::string newPattern = p;
                        updatePattern(newPattern);
                        sendPatternFlag = false;
                    }
                }
            }
            // MIDI Input
            if (event && event->body.type == MidiEventID) {
                uint8_t *di = (uint8_t *) LV2_ATOM_BODY(&event->body);
                MidiEvent inEv = {0, 0, 0, 0};
                if ( (di[0] & 0xf0) == 0x90 ) {
                    inEv.type = EV_NOTEON;
                    inEv.value = di[2];
                }
                else if ( (di[0] & 0xf0) == 0x80 ) {
                    inEv.type = EV_NOTEON;
                    inEv.value = 0;
                }
                else if ( (di[0] & 0xf0) == 0xb0 ) {
                    inEv.type = EV_CONTROLLER;
                    inEv.value = di[2];
                }
                else inEv.type = EV_NONE;

                inEv.channel = di[0] & 0x0f;
                inEv.data=di[1];
                int tick = ((uint64_t)(curFrame - transportFramesDelta) 
                        * nframes
                        + (uint64_t)(&event->time.frames) % nframes)
                        * TPQN*tempo/nframes/60/sampleRate 
                        + tempoChangeTick;
                        
                //printf("curFrame %d \n", curFrame - transportFramesDelta);
                // Set ticks to zero whenever notes with stopped
                // transport are received.
                // Also, when note offs are received when transport is
                // not rolling, these notes should be removed without
                // release.
                bool unmatched = false;
                if ((hostTransport) && (transportSpeed == 0)) {
                    tick = 2;
                    unmatched = handleEvent(inEv, tick - 2, 0);
                } 
                else {
                    unmatched = handleEvent(inEv, tick - 2, 1);
                }
                if (unmatched) //if event is unmatched, forward it
                    forgeMidiEvent((int)((uint64_t)(&event->time.frames) % nframes), di, 3);
            }
        }
    }


        // MIDI Output
    for (uint32_t f = 0 ; f < nframes; f++) {
        curTick = (uint64_t)(curFrame - transportFramesDelta)
                        *TPQN*tempo/60/sampleRate + tempoChangeTick;
        if ((curTick >= nextTick) && (transportSpeed)) {
            getNextFrame(curTick);
            if (!isMuted) {
                if (hasNewNotes && returnVelocity[0]) {
                    int l2 = 0;
                    while(returnNote[l2] >= 0) {
                        unsigned char d[3];
                        d[0] = 0x90 + channelOut;
                        d[1] = returnNote[l2];
                        d[2] = returnVelocity[l2];
                        forgeMidiEvent(f, d, 3);
                        evTickQueue[bufPtr] = curTick + returnLength;
                        evQueue[bufPtr] = returnNote[l2];
                        bufPtr++;
                        l2++;
                    }
                }
            }
            float pos = (float)getFramePtr();
            *val[CURSOR_POS] = pos;
        }

        // Note Off Queue handling
        int noteofftick = evTickQueue[0];
        int idx = 0;
        for (int l1 = 0; l1 < bufPtr; l1++) {
            int tmptick = evTickQueue[l1];
            if (noteofftick > tmptick) {
                idx = l1;
                noteofftick = tmptick;
            }
        }
        if ( (bufPtr) && ((curTick >= noteofftick)
                || (hostTransport && !transportSpeed)) ) {
            int outval = evQueue[idx];
            for (int l4 = idx ; l4 < (bufPtr - 1);l4++) {
                evQueue[l4] = evQueue[l4 + 1];
                evTickQueue[l4] = evTickQueue[l4 + 1];
            }
            bufPtr--;

            unsigned char d[3];
            d[0] = 0x80 + channelOut;
            d[1] = outval;
            d[2] = 127;
            forgeMidiEvent(f, d, 3);
        }
        curFrame++;
    }
}

void MidiArpLV2::forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size)
{
    QMidiArpURIs* const uris = &m_uris;
    LV2_Atom midiatom;
    midiatom.type = uris->midi_MidiEvent;
    midiatom.size = size;
    lv2_atom_forge_frame_time(&forge, f);
    lv2_atom_forge_raw(&forge, &midiatom, sizeof(LV2_Atom));
    lv2_atom_forge_raw(&forge, buffer, size);
    lv2_atom_forge_pad(&forge, sizeof(LV2_Atom) + size);
}

void MidiArpLV2::updateParams()
{
    attack_time     = *val[ATTACK];

    if (release_time != *val[RELEASE]) {
        updateReleaseTime(*val[RELEASE]);
    }

    if (randomTickAmp != *val[RANDOM_TICK]) {
        updateRandomTickAmp(*val[RANDOM_TICK]);
    }

    if (randomLengthAmp != *val[RANDOM_LEN]) {
        updateRandomLengthAmp(*val[RANDOM_LEN]);
    }

    if (randomVelocityAmp != *val[RANDOM_VEL]) {
        updateRandomVelocityAmp(*val[RANDOM_VEL]);
    }
    if (octMode != *val[OCTAVE_MODE]) {
        updateOctaveMode(*val[OCTAVE_MODE]);
    }
    if (latch_mode != (bool)*val[LATCH_MODE]) {
        setLatchMode((bool)*val[LATCH_MODE]);
    }
    
    octLow     =   (int)*val[OCTAVE_LOW];
    octHigh     =   (int)*val[OCTAVE_HIGH];


    if (deferChanges != ((bool)*val[DEFER])) deferChanges = ((bool)*val[DEFER]);
    if (isMuted != (bool)*val[MUTE] && !parChangesPending) setMuted((bool)(*val[MUTE]));

    indexIn[0]   =   (int)*val[INDEX_IN1];
    indexIn[1]   =   (int)*val[INDEX_IN2];
    rangeIn[0]   =   (int)*val[RANGE_IN1];
    rangeIn[1]   =   (int)*val[RANGE_IN2];

    restartByKbd =  (bool)*val[ENABLE_RESTARTBYKBD];
    trigByKbd =     (bool)*val[ENABLE_TRIGBYKBD];
    trigLegato =    (bool)*val[ENABLE_TRIGLEGATO];

    repeatPatternThroughChord = (int)*val[REPEAT_MODE];
    channelOut =      (int)*val[CH_OUT];
    chIn =            (int)*val[CH_IN];

    if (internalTempo != *val[TEMPO]) {
        internalTempo = *val[TEMPO];
        initTransport();
    }

    if (hostTransport != (bool)(*val[TRANSPORT_MODE])) {
        hostTransport = (bool)(*val[TRANSPORT_MODE]);
        initTransport();
    }

    if (hostTransport && !transportAtomReceived) {
        updatePos(  (uint64_t)*val[HOST_POSITION],
                    (float)*val[HOST_TEMPO],
                    (int)*val[HOST_SPEED],
                    false);
    }
}

void MidiArpLV2::initTransport()
{
    if (!hostTransport) {
        transportFramesDelta = curFrame;
        if (curTick > 0) tempoChangeTick = curTick;
        transportBpm = internalTempo;
        tempo = internalTempo;
        transportSpeed = 1;
    }
    else transportSpeed = 0;

    setNextTick(tempoChangeTick);
}

void MidiArpLV2::sendPattern(const std::string & p)
{
    if (!(ui_up && sendPatternFlag)) return;

    sendPatternFlag = false;

    const QMidiArpURIs* uris = &m_uris;

    const char* c = p.c_str();


    /* prepare forge buffer and initialize atom-sequence */
    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&forge, 0);
    lv2_atom_forge_object(&forge, &frame, 1, uris->pattern_string);

    /* forge container object of type 'pattern_string' */
    lv2_atom_forge_property_head(&forge, uris->pattern_string, 0);
    lv2_atom_forge_string(&forge, c, strlen(c));

    /* close-off frame */
    lv2_atom_forge_pop(&forge, &frame);
}

static LV2_State_Status MidiArpLV2_state_restore ( LV2_Handle instance,
    LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature *const * )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);

    if (pPlugin == NULL) return LV2_STATE_ERR_UNKNOWN;

    QMidiArpURIs* const uris = &pPlugin->m_uris;

    uint32_t type = uris->atom_String;

    if (type == 0) return LV2_STATE_ERR_BAD_TYPE;

    size_t size = 0;

    uint32_t key = uris->pattern_string;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    const char *value1
        = (const char *) (*retrieve)(handle, key, &size, &type, &flags);

    if (size < 2) return LV2_STATE_ERR_UNKNOWN;

    pPlugin->advancePatternIndex(true);

    pPlugin->updatePattern(value1);
    pPlugin->sendPatternFlag = true;
    return LV2_STATE_SUCCESS;
}

static LV2_State_Status MidiArpLV2_state_save ( LV2_Handle instance,
    LV2_State_Store_Function store, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature *const * )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);

    if (pPlugin == NULL) return LV2_STATE_ERR_UNKNOWN;

    QMidiArpURIs* const uris = &pPlugin->m_uris;

    uint32_t type = uris->atom_String;

    if (type == 0) return LV2_STATE_ERR_BAD_TYPE;

    flags |= (LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

    const char* c = pPlugin->pattern.c_str();

    size_t size = strlen(c) + 1;
    uint32_t key = uris->pattern_string;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    LV2_State_Status result = (*store)(handle, key, c, size, type, flags);

    return result;
}

static const LV2_State_Interface MidiArpLV2_state_interface =
{
    MidiArpLV2_state_save,
    MidiArpLV2_state_restore
};

void MidiArpLV2::activate (void)
{
    initTransport();
}

void MidiArpLV2::deactivate (void)
{
    transportSpeed = 0;
    clearNoteBuffer();
}

static LV2_Handle MidiArpLV2_instantiate (
    const LV2_Descriptor *, double sample_rate, const char *,
    const LV2_Feature *const *host_features )
{
    return new MidiArpLV2(sample_rate, host_features);
}

static void MidiArpLV2_connect_port (
    LV2_Handle instance, uint32_t port, void *data )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);
    if (pPlugin)
        pPlugin->connect_port(port, data);
}

static void MidiArpLV2_run ( LV2_Handle instance, uint32_t nframes )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);
    if (pPlugin)
        pPlugin->run(nframes);
}

static void MidiArpLV2_activate ( LV2_Handle instance )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);
    if (pPlugin)
        pPlugin->activate();
}

static void MidiArpLV2_deactivate ( LV2_Handle instance )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);
    if (pPlugin)
        pPlugin->deactivate();
}

static void MidiArpLV2_cleanup ( LV2_Handle instance )
{
    MidiArpLV2 *pPlugin = static_cast<MidiArpLV2 *> (instance);
    if (pPlugin)
        delete pPlugin;
}

static const void *MidiArpLV2_extension_data ( const char * uri)
{
    static const LV2_State_Interface state_iface =
                { MidiArpLV2_state_save, MidiArpLV2_state_restore };
    if (!strcmp(uri, LV2_STATE__interface)) {
        return &state_iface;
    }
    else return NULL;
}

static const LV2_Descriptor MidiArpLV2_descriptor =
{
    QMIDIARP_ARP_LV2_URI,
    MidiArpLV2_instantiate,
    MidiArpLV2_connect_port,
    MidiArpLV2_activate,
    MidiArpLV2_run,
    MidiArpLV2_deactivate,
    MidiArpLV2_cleanup,
    MidiArpLV2_extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &MidiArpLV2_descriptor : NULL);
}
