/*!
 * @file qmidiarp_arp_lv2.cpp
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
#include "qmidiarp_arp_lv2.h"
#include "qmidiarp_arpwidget_lv2.h"

#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"

#define LV2_MIDI_EVENT_URI "http://lv2plug.in/ns/ext/midi#MidiEvent"
#define LV2_TIME_URI "http://lv2plug.in/ns/ext/time"

qmidiarp_arp_lv2::qmidiarp_arp_lv2 (
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
    patternSendTrials = 0;

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

    /* Map URIS */
    QMidiArpURIs* const uris = &m_uris;

    uris->atom_Blank          = urid_map->map(urid_map->handle, LV2_ATOM__Blank);
    uris->atom_Float          = urid_map->map(urid_map->handle, LV2_ATOM__Float);
    uris->atom_Long           = urid_map->map(urid_map->handle, LV2_ATOM__Long);
    uris->atom_String         = urid_map->map(urid_map->handle, LV2_ATOM__String);
    uris->atom_Resource       = urid_map->map(urid_map->handle, LV2_ATOM__Resource);
    uris->time_Position       = urid_map->map(urid_map->handle, LV2_TIME__Position);
    uris->time_frame          = urid_map->map(urid_map->handle, LV2_TIME__frame);
    uris->time_barBeat        = urid_map->map(urid_map->handle, LV2_TIME__barBeat);
    uris->time_beatsPerMinute = urid_map->map(urid_map->handle, LV2_TIME__beatsPerMinute);
    uris->time_speed          = urid_map->map(urid_map->handle, LV2_TIME__speed);
    uris->pattern_string      = urid_map->map(urid_map->handle, QMIDIARP_ARP_LV2_PREFIX "PATTERNSTRING");

    uridMap = urid_map;
}


qmidiarp_arp_lv2::~qmidiarp_arp_lv2 (void)
{
}

void qmidiarp_arp_lv2::connect_port ( uint32_t port, void *data )
{
    switch(PortIndex(port)) {
    case MidiIn:
        inEventBuffer = (LV2_Atom_Sequence*)data;
        break;
    case MidiOut:
        outEventBuffer = (LV2_Event_Buffer *) data;
        break;
    case TRANSPORT_CONTROL:
        transportControl = (LV2_Atom_Sequence*)data;
        break;
    default:
        val[port - 2] = (float *) data;
        break;
    }
}

void qmidiarp_arp_lv2::updatePos(const LV2_Atom_Object* obj)
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

void qmidiarp_arp_lv2::run ( uint32_t nframes )
{
    LV2_Event_Iterator iter_out;
    lv2_event_buffer_reset(outEventBuffer, outEventBuffer->stamp_type, outEventBuffer->data);
    lv2_event_begin(&iter_out, outEventBuffer);

    const QMidiArpURIs* uris = &m_uris;
    const LV2_Atom_Sequence* atomIn = transportControl;
    LV2_Atom_Event* atomEv = lv2_atom_sequence_begin(&atomIn->body);


    if (!(nCalls % 12)) updateParams();

        // Position stuff
    while (!lv2_atom_sequence_is_end(&atomIn->body, atomIn->atom.size, atomEv)) {
        if (atomEv->body.type == uris->atom_Blank) {
            const LV2_Atom_Object* obj = (LV2_Atom_Object*)&atomEv->body;
            if (obj->body.otype == uris->time_Position) {
                /* Received position information, update */
                if (transportMode) updatePos(obj);
            }
            atomEv = lv2_atom_sequence_next(atomEv);
        }
    }

        // MIDI Input
    if (inEventBuffer) {
        LV2_ATOM_SEQUENCE_FOREACH(inEventBuffer, event) {
            if (event && event->body.type == MidiEventID) {
                uint8_t *data = (uint8_t *) LV2_ATOM_BODY(&event->body);
                MidiEvent inEv;
                if ( (data[0] & 0xf0) == 0x90 ) {
                    inEv.type = EV_NOTEON;
                    inEv.value = data[2];
                }
                else if ( (data[0] & 0xf0) == 0x80 ) {
                    inEv.type = EV_NOTEON;
                    inEv.value = 0;
                }
                else if ( (data[0] & 0xf0) == 0xb0 ) {
                    inEv.type = EV_CONTROLLER;
                    inEv.value = data[2];
                }
                else inEv.type = EV_NONE;

                inEv.channel = data[0] & 0x0f;
                inEv.data=data[1];
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
                            lv2_event_write(&iter_out, f, 0, MidiEventID, 3, d);
                            evTickQueue.replace(bufPtr, curTick + returnLength);
                            evQueue.replace(bufPtr, returnNote.at(l2));
                            bufPtr++;
                            l2++;
                        }
                    }
                }
            }
            float pos = (float)getGrooveIndex();
            *val[CURSOR_POS - 2] = pos;
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
        if ((bufPtr) && (curTick >= noteofftick) && (transportSpeed)) {
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
            lv2_event_write(&iter_out, f, 0, MidiEventID, 3, d);
        }
        curFrame++;
    }
    nCalls++;
}

void qmidiarp_arp_lv2::updateParams()
{
    if (!sendPatternFlag) {
        // decode from float into four pattern characters and append them
        QString newpattern;
        uint32_t n;
        unsigned char c;
        for (int l1 = 0; l1 < 16; l1++) {
            n = *val[l1 + WAVEIN1 - 2] * 8192;
            for (int l2 = 0; l2 < 4; l2++) {
                c = (n >> (24 - (l2 * 8))) & 0xff;
                if (c != 0) newpattern.append(c);
            }
        }

        if (newpattern != pattern) {
            updatePattern(newpattern);
        }
    }
    else {
        sendPattern(pattern);
    }

    if (attack_time != *val[0]) {
        updateAttackTime(*val[0]);
    }

    if (release_time != *val[1]) {
        updateReleaseTime(*val[1]);
    }

    if (randomTickAmp != *val[2]) {
        updateRandomTickAmp(*val[2]);
    }

    if (randomLengthAmp != *val[3]) {
        updateRandomLengthAmp(*val[3]);
    }

    if (randomVelocityAmp != *val[4]) {
        updateRandomVelocityAmp(*val[4]);
    }


    if (deferChanges != ((bool)*val[38])) deferChanges = ((bool)*val[38]);
    if (isMuted != (bool)*val[26] && !parChangesPending) setMuted((bool)(*val[26]));

    indexIn[0]   =   (int)*val[31];
    indexIn[1]   =   (int)*val[32];
    rangeIn[0]   =   (int)*val[33];
    rangeIn[1]   =   (int)*val[34];

    restartByKbd =  (bool)*val[35];
    trigByKbd =     (bool)*val[39];
    trigLegato =    (bool)*val[27];

    repeatPatternThroughChord = (int)*val[36];
    channelOut =      (int)*val[5];
    chIn =            (int)*val[6];

    if (internalTempo != *val[42]) {
        internalTempo = *val[42];
        if (!transportMode) {
            transportFramesDelta = curFrame;
            tempoChangeTick = curTick;
            transportBpm = internalTempo;
            tempo = internalTempo;
            setNextTick(tempoChangeTick);
            prepareCurrentNote(nextTick);
        }
    }

    if (transportMode != (bool)(*val[41])) {
        transportMode = (bool)(*val[41]);
        if (transportMode) {
            transportSpeed = 0;
        }
        else {
            transportSpeed = 0;
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

void qmidiarp_arp_lv2::sendPattern(const QString & p)
{
    // encode into floats and send pattern to GUI via WAVEDATA ports
    *val[37] = 1.0;
    if (patternSendTrials < 2) {
        patternSendTrials++;
        return;
    }

    int l1, l2;
    int ix = 0;
    uint32_t n;
    unsigned char c;
    for (l1 = 0; l1 < 16; l1++) {
        n = 0;
        for (l2 = 24; l2 > 0; l2-=8) {
            if (ix < p.count()) c = p.at(ix).cell(); else c = 0;
            n |= ( c << l2 );
            ix++;
        }
        *val[l1 + WAVEDATA1 - 2] = (float)n / 8192. ;
    }

    patternSendTrials++;
    if (patternSendTrials > 3) {
        *val[37] = 0.0;
        patternSendTrials = 0;
        sendPatternFlag = false;
    }
}

static LV2_State_Status qmidiarp_arp_lv2_state_restore ( LV2_Handle instance,
    LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature *const *features )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);

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
    std::string tmpString1 = value1;

    pPlugin->advancePatternIndex(true);
    pPlugin->updatePattern(QString::fromStdString(tmpString1));
    pPlugin->sendPatternFlag = true;

    return LV2_STATE_SUCCESS;
}

static LV2_State_Status qmidiarp_arp_lv2_state_save ( LV2_Handle instance,
    LV2_State_Store_Function store, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature *const *features )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);

    if (pPlugin == NULL) return LV2_STATE_ERR_UNKNOWN;

    QMidiArpURIs* const uris = &pPlugin->m_uris;

    uint32_t type = uris->atom_String;

    if (type == 0) return LV2_STATE_ERR_BAD_TYPE;


    const std::string tempString = pPlugin->pattern.toStdString();
    const char *value = pPlugin->pattern.toLatin1();

    size_t size = strlen(value) + 1;
    uint32_t key = uris->pattern_string;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    LV2_State_Status result = (*store)(handle, key, value, size, type, flags);

    return result;
}

static const LV2_State_Interface qmidiarp_arp_lv2_state_interface =
{
    qmidiarp_arp_lv2_state_save,
    qmidiarp_arp_lv2_state_restore
};

void qmidiarp_arp_lv2::activate (void)
{
}

void qmidiarp_arp_lv2::deactivate (void)
{
}

static LV2_Handle qmidiarp_arp_lv2_instantiate (
    const LV2_Descriptor *, double sample_rate, const char *,
    const LV2_Feature *const *host_features )
{
    return new qmidiarp_arp_lv2(sample_rate, host_features);
}

static void qmidiarp_arp_lv2_connect_port (
    LV2_Handle instance, uint32_t port, void *data )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);
    if (pPlugin)
        pPlugin->connect_port(port, data);
}

static void qmidiarp_arp_lv2_run ( LV2_Handle instance, uint32_t nframes )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);
    if (pPlugin)
        pPlugin->run(nframes);
}

static void qmidiarp_arp_lv2_activate ( LV2_Handle instance )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);
    if (pPlugin)
        pPlugin->activate();
}

static void qmidiarp_arp_lv2_deactivate ( LV2_Handle instance )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);
    if (pPlugin)
        pPlugin->deactivate();
}

static void qmidiarp_arp_lv2_cleanup ( LV2_Handle instance )
{
    qmidiarp_arp_lv2 *pPlugin = static_cast<qmidiarp_arp_lv2 *> (instance);
    if (pPlugin)
        delete pPlugin;
}

static const void *qmidiarp_arp_lv2_extension_data ( const char * uri)
{
    static const LV2_State_Interface state_iface =
                { qmidiarp_arp_lv2_state_save, qmidiarp_arp_lv2_state_restore };
    if (!strcmp(uri, LV2_STATE__interface)) {
        return &state_iface;
    }
    else return NULL;
}

static LV2UI_Handle qmidiarp_arp_lv2ui_instantiate (
    const LV2UI_Descriptor *, const char *, const char *,
    LV2UI_Write_Function write_function,
    LV2UI_Controller controller, LV2UI_Widget *widget,
    const LV2_Feature *const * )
{
    qmidiarp_arpwidget_lv2 *pWidget = new qmidiarp_arpwidget_lv2(controller, write_function);
    *widget = pWidget;
    return pWidget;
}

static void qmidiarp_arp_lv2ui_cleanup ( LV2UI_Handle ui )
{
    qmidiarp_arpwidget_lv2 *pWidget = static_cast<qmidiarp_arpwidget_lv2 *> (ui);
    if (pWidget)
        delete pWidget;
}

static void qmidiarp_arp_lv2ui_port_event (
    LV2UI_Handle ui, uint32_t port_index,
    uint32_t buffer_size, uint32_t format, const void *buffer )
{
    qmidiarp_arpwidget_lv2 *pWidget = static_cast<qmidiarp_arpwidget_lv2 *> (ui);
    if (pWidget)
        pWidget->port_event(port_index, buffer_size, format, buffer);
}

static const void *qmidiarp_arp_lv2ui_extension_data ( const char * )
{
    return NULL;
}

static const LV2_Descriptor qmidiarp_arp_lv2_descriptor =
{
    QMIDIARP_ARP_LV2_URI,
    qmidiarp_arp_lv2_instantiate,
    qmidiarp_arp_lv2_connect_port,
    qmidiarp_arp_lv2_activate,
    qmidiarp_arp_lv2_run,
    qmidiarp_arp_lv2_deactivate,
    qmidiarp_arp_lv2_cleanup,
    qmidiarp_arp_lv2_extension_data
};

static const LV2UI_Descriptor qmidiarp_arp_lv2ui_descriptor =
{
    QMIDIARP_ARP_LV2UI_URI,
    qmidiarp_arp_lv2ui_instantiate,
    qmidiarp_arp_lv2ui_cleanup,
    qmidiarp_arp_lv2ui_port_event,
    qmidiarp_arp_lv2ui_extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &qmidiarp_arp_lv2_descriptor : NULL);
}

LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor ( uint32_t index )
{
    return (index == 0 ? &qmidiarp_arp_lv2ui_descriptor : NULL);
}

