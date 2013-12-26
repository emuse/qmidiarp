/*!
 * @file midilfo_lv2.cpp
 * @brief Implements an LV2 plugin inheriting from MidiLfo
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
#include "midilfo_lv2.h"
#include "lfowidget_lv2.h"

MidiLfoLV2::MidiLfoLV2 (
    double sample_rate, const LV2_Feature *const *host_features )
    :MidiLfo()
{
    MidiEventID = 0;
    sampleRate = sample_rate;
    curFrame = 0;
    nCalls = 0;
    inLfoFrame = 0;
    inEventBuffer = NULL;
    control = NULL;
    notify = NULL;
    transportControl = NULL;
    outEventBuffer = NULL;
    getNextFrame(0);
    mouseXCur = 0;
    mouseYCur = 0;
    mouseEvCur = 0;
    tempo = 120.0f;
    internalTempo = 120.0f;
    lastMouseIndex = 0;

    transportBpm = 120.0f;
    transportFramesDelta = 0;
    curTick = 0;
    tempoChangeTick = 0;
    transportMode = false;
    transportSpeed = 1;
    dataChanged = true;
    ui_up = false;


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


MidiLfoLV2::~MidiLfoLV2 (void)
{
}

void MidiLfoLV2::connect_port ( uint32_t port, void *seqdata )
{
    switch(port) {
    case 0:
        inEventBuffer = (LV2_Atom_Sequence*)seqdata;
        break;
    case 1:
        outEventBuffer = (const LV2_Atom_Sequence*)seqdata;
        break;
    case TRANSPORT_CONTROL + 2:
        transportControl = (LV2_Atom_Sequence*)seqdata;
        break;
    case WAV_CONTROL + 2:
        control = (const LV2_Atom_Sequence*)seqdata;
        break;
    case WAV_NOTIFY + 2:
        notify = (LV2_Atom_Sequence*)seqdata;
        break;
    default:
        val[port - 2] = (float *) seqdata;
        break;
    }
}

void MidiLfoLV2::updatePos(const LV2_Atom_Object* obj)
{
    QMidiArpURIs* const uris = &m_uris;

    bool changed = false;

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
            changed = true;
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
                inLfoFrame = 0;
                setNextTick(tempoChangeTick);
                getNextFrame(nextTick);
            }
            else {
                curFrame = transportFramesDelta;
                inLfoFrame = 0;
            }
            changed = true;
        }
    }
    //~ if (changed) qWarning("frames %d ticks %d tempo %f status %f", transportFramesDelta
        //~ , tempoChangeTick, transportBpm, transportSpeed);
}

void MidiLfoLV2::run ( uint32_t nframes )
{
    const uint32_t capacity = outEventBuffer->atom.size;
    const QMidiArpURIs* uris = &m_uris;

    updateParams();
    if (isRecording) {
        getData(&data);
    }
    sendWave();

        // Position stuff
    if (transportControl) {
        LV2_Atom_Event* atomEv = lv2_atom_sequence_begin(&transportControl->body);
        while (!lv2_atom_sequence_is_end(&transportControl->body, transportControl->atom.size, atomEv)) {
            if (atomEv->body.type == uris->atom_Blank) {
                const LV2_Atom_Object* obj = (LV2_Atom_Object*)&atomEv->body;
                if (obj->body.otype == uris->time_Position) {
                    /* Received position information, update */
                    if (transportMode) updatePos(obj);
                }
            }
            atomEv = lv2_atom_sequence_next(atomEv);
        }
    }

        // Process incoming events from GUI
    if (control) {
        LV2_Atom_Event* ev = lv2_atom_sequence_begin(&control->body);
        /* for each message from UI... */
        while(!lv2_atom_sequence_is_end(&control->body, control->atom.size, ev)) {
            if (ev->body.type == uris->atom_Blank) {
                const LV2_Atom_Object* obj = (LV2_Atom_Object*)&ev->body;
                /* interpret atom-objects: */
                if (obj->body.otype == uris->ui_up) {
                    /* UI was activated */
                    ui_up = true;
                    dataChanged = true;
                }
                else if (obj->body.otype == uris->ui_down) {
                    /* UI was closed */
                    ui_up = false;
                }
            }
            ev = lv2_atom_sequence_next(ev);
        }
    }

        // MIDI Input
    if (inEventBuffer) {
        LV2_ATOM_SEQUENCE_FOREACH(inEventBuffer, event) {
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
                (void)handleEvent(inEv, tick);
            }
        }
    }


        // MIDI and Wave Control Output
    lv2_atom_forge_set_buffer(&forge, (uint8_t*)outEventBuffer, capacity);
    lv2_atom_forge_sequence_head(&forge, &m_lv2frame, 0);
    for (uint f = 0 ; f < nframes; f++) {
        curTick = (uint64_t)(curFrame - transportFramesDelta)
                        *TPQN*tempo/60/sampleRate + tempoChangeTick;
        if ((curTick >= nextTick) && (transportSpeed)) {
            if (curTick > frame.at(inLfoFrame).tick) {
                if (!frame.at(inLfoFrame).muted && !isMuted) {
                    unsigned char d[3];
                    d[0] = 0xb0 + channelOut;
                    d[1] = ccnumber;
                    d[2] = frame.at(inLfoFrame).value;
                    forgeMidiEvent(f, d, 3);
                    *val[WaveOut] = (float)d[2] / 128;
                }
                inLfoFrame++;
                if (inLfoFrame >= frameSize) {
                    frameptr = getFramePtr();
                    float pos = (float)frameptr;
                    *val[CURSOR_POS] = pos;
                    getNextFrame(curTick);
                    inLfoFrame = 0;
                }
            }
        }
        curFrame++;
    }
    nCalls++;
}

void MidiLfoLV2::forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size)
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

void MidiLfoLV2::updateParams()
{
    bool changed = false;


    if (amp != *val[AMPLITUDE]) {
        changed = true;
        updateAmplitude(*val[AMPLITUDE]);
    }

    if (offs != *val[OFFSET]) {
        changed = true;
        updateOffset(*val[OFFSET]);
        *val[OFFSET] = offs;
    }

    if (mouseXCur != *val[MOUSEX] || mouseYCur != *val[MOUSEY]
                || mouseEvCur != *val[MOUSEPRESSED]) {
        int ix = 1;
        int evtype = 0;
        changed = true;
        mouseXCur = *val[MOUSEX];
        mouseYCur = *val[MOUSEY];
        if ((mouseEvCur == 2) && (*val[MOUSEPRESSED] != 2) )
            evtype = 1;
        else if (*val[MOUSEPRESSED] != -1)
            evtype = *val[MOUSEPRESSED];

        if (*val[MOUSEPRESSED] != -1) mouseEvCur = *val[MOUSEPRESSED];

        if (mouseEvCur == 2) return; // mouse was released
        //qWarning("mouse event X: %f - Y: %f - Type: %d - Button %d",
        //    mouseXCur, mouseYCur, (int)*val[MOUSEBUTTON], mouseEvCur);
        if (*val[MOUSEPRESSED] != -1) ix = mouseEvent(mouseXCur, mouseYCur, *val[MOUSEBUTTON], evtype);
        if (evtype == 1) lastMouseIndex = ix; // if we have a new press event set last point index here
    }

    if (res != lfoResValues[(int)*val[RESOLUTION]]) {
        changed = true;
        updateResolution(lfoResValues[(int)*val[RESOLUTION]]);
    }

    if (size != lfoSizeValues[(int)*val[SIZE]]) {
        changed = true;
        updateSize(lfoSizeValues[(int)*val[SIZE]]);
    }

    if (freq != lfoFreqValues[(int)*val[FREQUENCY]]) {
        changed = true;
        updateFrequency(lfoFreqValues[(int)*val[FREQUENCY]]);
    }

    if (waveFormIndex != (int)*val[WAVEFORM]) {
        changed = true;
        updateWaveForm(*val[WAVEFORM]);
    }

    if (curLoopMode != (*val[LOOPMODE])) updateLoop(*val[LOOPMODE]);
    if (recordMode != ((bool)*val[RECORD])) {
        setRecordMode((bool)*val[RECORD]);
    }
    if (deferChanges != ((bool)*val[DEFER])) deferChanges = ((bool)*val[DEFER]);
    if (isMuted != (bool)*val[MUTE] && !parChangesPending) setMuted((bool)(*val[MUTE]));

    ccnumber =       (int)*val[CC_OUT];
    ccnumberIn =     (int)*val[CC_IN];
    enableNoteOff = (bool)*val[ENABLE_NOTEOFF];
    restartByKbd =  (bool)*val[ENABLE_RESTARTBYKBD];
    trigByKbd =     (bool)*val[ENABLE_TRIGBYKBD];
    trigLegato =    (bool)*val[ENABLE_TRIGLEGATO];

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
            getNextFrame(nextTick);
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
            getNextFrame(nextTick);
            transportSpeed = 1;
        }
    }

    if (changed) {
        getData(&data);
        dataChanged = true;
    }
}

void MidiLfoLV2::sendWave()
{
    if (!(dataChanged && ui_up)) return;
    dataChanged = false;

    const QMidiArpURIs* uris = &m_uris;
    const uint32_t capacity = notify->atom.size;
    int ct = res * size + 1; // last element in wave is an end tag
    int tempArray[ct];

    for (int l1 = 0; l1 < ct; l1++) {
        tempArray[l1]=data.at(l1).value*((data.at(l1).muted) ? -1 : 1);
    }

    //size_t size = strlen(value) + 1;
    /* prepare forge buffer and initialize atom-sequence */
    lv2_atom_forge_set_buffer(&forge, (uint8_t*)notify, capacity);
    lv2_atom_forge_sequence_head(&forge, &m_lv2frame, 0);

    /* forge container object of type 'hex_customwave' */
    LV2_Atom_Forge_Frame lv2frame;
    lv2_atom_forge_frame_time(&forge, 0);
    lv2_atom_forge_blank(&forge, &lv2frame, 1, uris->hex_customwave);

    /* Send customWave to UI */
    lv2_atom_forge_property_head(&forge, uris->hex_customwave, 0);
    lv2_atom_forge_vector(&forge, sizeof(int), uris->atom_Int,
        ct, tempArray);

    /* close-off frame */
    lv2_atom_forge_pop(&forge, &lv2frame);
}

static LV2_State_Status MidiLfoLV2_state_restore ( LV2_Handle instance,
    LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature *const *features )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);

    if (pPlugin == NULL) return LV2_STATE_ERR_UNKNOWN;

    QMidiArpURIs* const uris = &pPlugin->m_uris;

    uint32_t type = uris->atom_String;

    if (type == 0) return LV2_STATE_ERR_BAD_TYPE;

    size_t size = 0;
    int l1;
    uint32_t key = uris->hex_mutemask;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    const char *value1
        = (const char *) (*retrieve)(handle, key, &size, &type, &flags);

    QByteArray tmpArray1 = QByteArray::fromHex(value1);

    if (size < 2 || !tmpArray1.count()) return LV2_STATE_ERR_UNKNOWN;

    pPlugin->setFramePtr(0);
    pPlugin->maxNPoints = tmpArray1.count();

    for (l1 = 0; l1 < tmpArray1.count(); l1++) {
        pPlugin->muteMask.replace(l1, tmpArray1.at(l1));
    }


    key = uris->hex_customwave;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    const char *value
        = (const char *) (*retrieve)(handle, key, &size, &type, &flags);

    if (size < 2) return LV2_STATE_ERR_UNKNOWN;

    QByteArray tmpArray = QByteArray::fromHex(value);

    Sample sample;
    int step = TPQN / pPlugin->res;
    int lt = 0;
    for (l1 = 0; l1 < tmpArray.count(); l1++) {
        sample.value = tmpArray.at(l1);
        sample.tick = lt;
        sample.muted = pPlugin->muteMask.at(l1);
        pPlugin->customWave.replace(l1, sample);
        lt+=step;
    }
    pPlugin->getData(&pPlugin->data);
    pPlugin->sendWave();

    return LV2_STATE_SUCCESS;
}

static LV2_State_Status MidiLfoLV2_state_save ( LV2_Handle instance,
    LV2_State_Store_Function store, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature *const *features )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);

    if (pPlugin == NULL) return LV2_STATE_ERR_UNKNOWN;

    QMidiArpURIs* const uris = &pPlugin->m_uris;

    uint32_t type = uris->atom_String;

    if (type == 0) return LV2_STATE_ERR_BAD_TYPE;

    QByteArray tempArray;

    tempArray.clear();
    int l1;
    for (l1 = 0; l1 < pPlugin->maxNPoints; l1++) {
        tempArray.append(pPlugin->customWave.at(l1).value);
    }

    const QByteArray hexArray = tempArray.toHex();
    const char *value = hexArray.constData();

    size_t size = strlen(value) + 1;
    uint32_t key = uris->hex_customwave;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    store(handle, key, value, size, type, flags);

    tempArray.clear();

    for (l1 = 0; l1 < pPlugin->maxNPoints; l1++) {
        tempArray.append(pPlugin->muteMask.at(l1));
    }

    const QByteArray hexArray1 = tempArray.toHex();
    const char *value1 = hexArray1.constData();

    size = strlen(value1) + 1;
    key = uris->hex_mutemask;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    LV2_State_Status result = (*store)(handle, key, value1, size, type, flags);

    return result;
}

static const LV2_State_Interface MidiLfoLV2_state_interface =
{
    MidiLfoLV2_state_save,
    MidiLfoLV2_state_restore
};

void MidiLfoLV2::activate (void)
{
}

void MidiLfoLV2::deactivate (void)
{
}

static LV2_Handle MidiLfoLV2_instantiate (
    const LV2_Descriptor *, double sample_rate, const char *,
    const LV2_Feature *const *host_features )
{
    return new MidiLfoLV2(sample_rate, host_features);
}

static void MidiLfoLV2_connect_port (
    LV2_Handle instance, uint32_t port, void *data )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);
    if (pPlugin)
        pPlugin->connect_port(port, data);
}

static void MidiLfoLV2_run ( LV2_Handle instance, uint32_t nframes )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);
    if (pPlugin)
        pPlugin->run(nframes);
}

static void MidiLfoLV2_activate ( LV2_Handle instance )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);
    if (pPlugin)
        pPlugin->activate();
}

static void MidiLfoLV2_deactivate ( LV2_Handle instance )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);
    if (pPlugin)
        pPlugin->deactivate();
}

static void MidiLfoLV2_cleanup ( LV2_Handle instance )
{
    MidiLfoLV2 *pPlugin = static_cast<MidiLfoLV2 *> (instance);
    if (pPlugin)
        delete pPlugin;
}

static const void *MidiLfoLV2_extension_data ( const char * uri)
{
    static const LV2_State_Interface state_iface =
                { MidiLfoLV2_state_save, MidiLfoLV2_state_restore };
    if (!strcmp(uri, LV2_STATE__interface)) {
        return &state_iface;
    }
    else return NULL;
}

static LV2UI_Handle MidiLfoLV2ui_instantiate (
    const LV2UI_Descriptor *, const char *, const char *,
    LV2UI_Write_Function write_function,
    LV2UI_Controller controller, LV2UI_Widget *widget,
    const LV2_Feature *const *host_features )
{
    LfoWidgetLV2 *pWidget = new LfoWidgetLV2(
                controller, write_function, host_features);
    *widget = pWidget;
    return pWidget;
}

static void MidiLfoLV2ui_cleanup ( LV2UI_Handle ui )
{
    LfoWidgetLV2 *pWidget = static_cast<LfoWidgetLV2 *> (ui);
    if (pWidget)
        delete pWidget;
}

static void MidiLfoLV2ui_port_event (
    LV2UI_Handle ui, uint32_t port_index,
    uint32_t buffer_size, uint32_t format, const void *buffer )
{
    LfoWidgetLV2 *pWidget = static_cast<LfoWidgetLV2 *> (ui);
    if (pWidget)
        pWidget->port_event(port_index, buffer_size, format, buffer);
}

static const void *MidiLfoLV2ui_extension_data ( const char * )
{
    return NULL;
}

static const LV2_Descriptor MidiLfoLV2_descriptor =
{
    QMIDIARP_LFO_LV2_URI,
    MidiLfoLV2_instantiate,
    MidiLfoLV2_connect_port,
    MidiLfoLV2_activate,
    MidiLfoLV2_run,
    MidiLfoLV2_deactivate,
    MidiLfoLV2_cleanup,
    MidiLfoLV2_extension_data
};

static const LV2UI_Descriptor MidiLfoLV2ui_descriptor =
{
    QMIDIARP_LFO_LV2UI_URI,
    MidiLfoLV2ui_instantiate,
    MidiLfoLV2ui_cleanup,
    MidiLfoLV2ui_port_event,
    MidiLfoLV2ui_extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &MidiLfoLV2_descriptor : NULL);
}

LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor ( uint32_t index )
{
    return (index == 0 ? &MidiLfoLV2ui_descriptor : NULL);
}

