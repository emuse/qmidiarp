/*!
 * @file midiarp_lv2.cpp
 * @brief Implements an LV2 plugin inheriting from MidiArp
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

#include <cstdio>
#include <cmath>
#include "midiarp_lv2.h"
#include "arpwidget_lv2.h"

MidiArpLV2::MidiArpLV2 (
    double sample_rate, const LV2_Feature *const *host_features )
    :MidiArp()
{
    MidiEventID = 0;
    sampleRate = sample_rate;
    curFrame = 0;
    nCalls = 0;
    inEventBuffer = NULL;
    outEventBuffer = NULL;
    tempo = 120.0f;
    internalTempo = 120.0f;

    transportBpm = 120.0f;
    transportFramesDelta = 0;
    curTick = 0;
    tempoChangeTick = 0;
    transportMode = false;
    transportSpeed = 1;

    sendPatternFlag = false;
    ui_up = false;

    bufPtr = 0;
    evQueue.resize(JQ_BUFSZ);
    evTickQueue.resize(JQ_BUFSZ);

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
        qWarning("Host does not support urid:map.");
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

void MidiArpLV2::updatePos(const LV2_Atom_Object* obj)
{
    QMidiArpURIs* const uris = &m_uris;

    LV2_Atom *bpm = NULL, *speed = NULL, *pos = NULL;
    lv2_atom_object_get(obj,
                        uris->time_frame, &pos,
                        uris->time_beatsPerMinute, &bpm,
                        uris->time_speed, &speed,
                        NULL);
    if (bpm && bpm->type == uris->atom_Float) {
        if (transportBpm != ((LV2_Atom_Float*)bpm)->body) {
            /* Tempo changed */
            transportBpm = ((LV2_Atom_Float*)bpm)->body;
            tempo = transportBpm;
        }
    }
    if (pos && pos->type == uris->atom_Long) {
        /* Position changed */
        const float frames_per_beat = 60.0f / transportBpm * sampleRate;
        const uint64_t position = ((LV2_Atom_Long*)pos)->body;

        transportFramesDelta = position;
        tempoChangeTick = position / frames_per_beat * TPQN;
    }
    if (speed && speed->type == uris->atom_Float) {
        if (transportSpeed != ((LV2_Atom_Float*)speed)->body) {
            /* Speed changed, e.g. 0 (stop) to 1 (play) */
            transportSpeed = ((LV2_Atom_Float*)speed)->body;
            if (transportSpeed) {
                curFrame = transportFramesDelta;
                setNextTick(tempoChangeTick);
                newRandomValues();
                prepareCurrentNote(tempoChangeTick);
            }
            else {
                curFrame = transportFramesDelta;
            }
        }
    }
    //~ if (changed) qWarning("frames %d ticks %d tempo %f status %f", transportFramesDelta
        //~ , tempoChangeTick, transportBpm, transportSpeed);
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
            if (event && event->body.type == uris->atom_Blank) {
                const LV2_Atom_Object* obj = (LV2_Atom_Object*)&event->body;
                /* interpret atom-objects: */
                if (obj->body.otype == uris->time_Position) {
                    /* Received position information, update */
                    if (transportMode) updatePos(obj);
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
                        QString newPattern = QString::fromUtf8(p);
                        QString txPattern = newPattern.remove(QChar(0));
                        updatePattern(txPattern);
                        sendPatternFlag = false;
                    }
                }
            }
            // MIDI Input
            if (event && event->body.type == MidiEventID) {
                uint8_t *di = (uint8_t *) LV2_ATOM_BODY(&event->body);
                MidiEvent inEv;
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
                int tick = (uint64_t)(curFrame - transportFramesDelta)
                            *TPQN*tempo/60/sampleRate + tempoChangeTick;
                (void)handleEvent(inEv, tick - 2); //we don't need to pre-schedule
            }
        }
    }


        // MIDI Output
    for (uint f = 0 ; f < nframes; f++) {
        curTick = (uint64_t)(curFrame - transportFramesDelta)
                        *TPQN*tempo/60/sampleRate + tempoChangeTick;
        if ((curTick >= nextTick) && (transportSpeed)) {
            newRandomValues();
            prepareCurrentNote(curTick);
            if (!isMuted) {
                if (!returnNote.isEmpty()) {
                    if (returnIsNew && returnVelocity.at(0)) {
                        int l2 = 0;
                        while(returnNote.at(l2) >= 0) {
                            unsigned char d[3];
                            d[0] = 0x90 + channelOut;
                            d[1] = returnNote.at(l2);
                            d[2] = returnVelocity.at(l2);
                            forgeMidiEvent(f, d, 3);
                            evTickQueue.replace(bufPtr, curTick + returnLength);
                            evQueue.replace(bufPtr, returnNote.at(l2));
                            bufPtr++;
                            l2++;
                        }
                    }
                }
            }
            float pos = (float)getGrooveIndex();
            *val[CURSOR_POS] = pos;
        }

        // Note Off Queue handling
        int noteofftick = evTickQueue.first();
        int tmptick = 0;
        int idx = 0;
        for (int l1 = 0; l1 < bufPtr; l1++) {
            tmptick = evTickQueue.at(l1);
            if (noteofftick > tmptick) {
                idx = l1;
                noteofftick = tmptick;
            }
        }
        if ((bufPtr) && (curTick >= noteofftick)) {
            int outval = evQueue.at(idx);
            for (int l4 = idx ; l4 < (bufPtr - 1);l4++) {
                evQueue.replace(l4, evQueue.at(l4 + 1));
                evTickQueue.replace(l4, evTickQueue.at(l4 + 1));
            }
            bufPtr--;

            unsigned char d[3];
            d[0] = 0x90 + channelOut;
            d[1] = outval;
            d[2] = 0;
            forgeMidiEvent(f, d, 3);
        }
        curFrame++;
    }
    nCalls++;
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
    if (attack_time != *val[ATTACK]) {
        updateAttackTime(*val[ATTACK]);
    }

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
        if (!transportMode) {
            transportFramesDelta = curFrame;
            tempoChangeTick = curTick;
            transportBpm = internalTempo;
            tempo = internalTempo;
            setNextTick(tempoChangeTick);
            prepareCurrentNote(nextTick);
        }
    }

    if (transportMode != (bool)(*val[TRANSPORT_MODE])) {
        transportMode = (bool)(*val[TRANSPORT_MODE]);
        transportSpeed = 0;
        if (!transportMode) {
            transportFramesDelta = curFrame;
            tempoChangeTick = curTick;
            transportBpm = internalTempo;
            tempo = internalTempo;
            setNextTick(tempoChangeTick);
            prepareCurrentNote(nextTick);
            transportSpeed = 1;
        }
    }
}

void MidiArpLV2::sendPattern(const QString & p)
{
    if (!(ui_up && sendPatternFlag)) return;

    sendPatternFlag = false;

    const QMidiArpURIs* uris = &m_uris;
    QByteArray byteArray = p.toUtf8();
    const char* c = byteArray.constData();


    /* prepare forge buffer and initialize atom-sequence */
    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&forge, 0);
    lv2_atom_forge_blank(&forge, &frame, 1, uris->pattern_string);

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
    QString newpattern = QString::fromUtf8(value1);
    pPlugin->updatePattern(newpattern);
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


    QByteArray byteArray = pPlugin->pattern.toUtf8();
    const char* c = byteArray.constData();

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
}

void MidiArpLV2::deactivate (void)
{
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
