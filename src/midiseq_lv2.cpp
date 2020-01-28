/*!
 * @file midiseq_lv2.cpp
 * @brief Implements an LV2 plugin inheriting from MidiSeq
 *
 *
 *      Copyright 2009 - 2019 <qmidiarp-devel@lists.sourceforge.net>
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
#include "midiseq_lv2.h"

MidiSeqLV2::MidiSeqLV2 (
    double sample_rate, const LV2_Feature *const *host_features )
    :MidiSeq()
{
    MidiEventID = 0;
    sampleRate = sample_rate;
    curFrame = 0;
    inEventBuffer = NULL;
    outEventBuffer = NULL;
    getData(&data);
    mouseXCur = 0;
    mouseYCur = 0;
    mouseEvCur = 0;
    tempo = 120.0f;
    internalTempo = 120.0f;
    lastMouseIndex = 0;
    dispVertIndex = 0;

    transportBpm = 120.0f;
    transportFramesDelta = 0;
    curTick = 0;
    tempoChangeTick = 0;
    hostTransport = true;
    transportSpeed = 0;
    transportAtomReceived = false;

    transpFromGui = 0;
    velFromGui = 256;

    bufPtr = 0;
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
        printf("Host does not support urid:map.\n");
        return;
    }

    lv2_atom_forge_init(&forge, urid_map);

    /* Map URIS */
    QMidiArpURIs* const uris = &m_uris;
    map_uris(urid_map, uris);
    uridMap = urid_map;
}


MidiSeqLV2::~MidiSeqLV2 (void)
{
}

void MidiSeqLV2::connect_port ( uint32_t port, void *seqdata )
{
    switch(port) {
    case 0:
        inEventBuffer = (LV2_Atom_Sequence*)seqdata;
        break;
    case 1:
        outEventBuffer = (const LV2_Atom_Sequence*)seqdata;
        break;
    default:
        val[port - 2] = (float *)seqdata;
        break;
    }
}

void MidiSeqLV2::updatePosAtom(const LV2_Atom_Object* obj)
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

void MidiSeqLV2::updatePos(uint64_t pos, float bpm, int speed, bool ignore_pos)
{
    if (transportBpm != bpm) {
        /* Tempo changed */
        transportBpm = bpm;
        tempo = transportBpm;
        transportSpeed = 0;
    }

    if (!ignore_pos && (transportBpm > 0)) {
        const float frames_per_beat = 60.0f / transportBpm * sampleRate;
        transportFramesDelta = pos;
        tempoChangeTick = pos * TPQN / frames_per_beat;
    }
    if (transportSpeed != speed) {
        /* Speed changed, e.g. 0 (stop) to 1 (play) */
        transportSpeed = speed;
        curFrame = transportFramesDelta;
        if (transportSpeed) {
            setNextTick(tempoChangeTick);
        }
    }
    //printf("transportBpm %f, transportFramesDelta %d\n", transportBpm, transportFramesDelta);
}

void MidiSeqLV2::run (uint32_t nframes )
{
    const QMidiArpURIs* uris = &m_uris;
    const uint32_t capacity = outEventBuffer->atom.size;

    lv2_atom_forge_set_buffer(&forge, (uint8_t*)outEventBuffer, capacity);
    lv2_atom_forge_sequence_head(&forge, &m_frame, 0);

    sendWave();
    updateParams();

    if (inEventBuffer) {
        LV2_ATOM_SEQUENCE_FOREACH(inEventBuffer, event) {
            // Control Atom Input
            if (event && (event->body.type == uris->atom_Object
                        || event->body.type == uris->atom_Blank)) {
                const LV2_Atom_Object* obj = (LV2_Atom_Object*)&event->body;
                if (obj->body.otype == uris->time_Position) {
                    /* Received position information, update */
                    if (hostTransport) updatePosAtom(obj);
                }
                else if (obj->body.otype == uris->ui_up) {
                    /* UI was activated */
                    ui_up = true;
                    dataChanged = true;
                }
                else if (obj->body.otype == uris->ui_down) {
                    /* UI was closed */
                    ui_up = false;
                }
            }
            // MIDI Input
            else if (event && event->body.type == MidiEventID) {
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
                int tick = ((uint64_t)(curFrame - transportFramesDelta) * nframes
                            +(uint64_t)(&event->time.frames) % nframes)
                            *TPQN*tempo/nframes/60/sampleRate + tempoChangeTick;
                if (handleEvent(inEv, tick - 2)) //if event is unmatched, forward it
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
            if (!returnNote.muted && !isMuted) {
                unsigned char d[3];
                d[0] = 0x90 + channelOut;
                d[1] = returnNote.value;
                d[2] = vel;
                forgeMidiEvent(f, d, 3);
                evTickQueue[bufPtr] = curTick + notelength / 4;
                evQueue[bufPtr] = returnNote.value;
                bufPtr++;
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

void MidiSeqLV2::forgeMidiEvent(uint32_t f, const uint8_t* const buffer, uint32_t size)
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

void MidiSeqLV2::updateParams()
{
    bool changed = false;

    if (loopMarker != (int)*val[LOOPMARKER]) {
        changed = true;
        setLoopMarker((int)*val[LOOPMARKER]);
    }
    
    if (dispVertIndex != (int)*val[DISPLAY_ZOOM]) {
        changed = true;
        dispVertIndex = (int)*val[DISPLAY_ZOOM];
        updateDispVert(dispVertIndex);
    }

    if (mouseXCur != *val[MOUSEX] || mouseYCur != *val[MOUSEY]
                || mouseEvCur != *val[MOUSEPRESSED]) {
        int ix = 1;
        int evtype = 0;
        changed = true;
        mouseXCur = *val[MOUSEX];
        mouseYCur = *val[MOUSEY];
        if ((mouseEvCur == 2) && (*val[MOUSEPRESSED] != 2))
            evtype = 1;
        else
            evtype = *val[MOUSEPRESSED];

        mouseEvCur = *val[MOUSEPRESSED];

        if (mouseEvCur == 2) return; // mouse was released
        ix = mouseEvent(mouseXCur, mouseYCur, *val[MOUSEBUTTON], evtype);
        if (evtype == 1) lastMouseIndex = ix; // if we have a new press event set last point index here
    }

    if (currentRecStep != *val[CURR_RECSTEP]) {
        changed = true;
        *val[CURR_RECSTEP] = currentRecStep;
    }

    if (velFromGui != *val[VELOCITY]) {
        velFromGui = *val[VELOCITY];
        updateVelocity(velFromGui);
    }

    if (notelength != sliderToTickLen(*val[NOTELENGTH])) {
        updateNoteLength(sliderToTickLen(*val[NOTELENGTH]));
    }

    if (res != seqResValues[(int)*val[RESOLUTION]]) {
        changed = true;
        updateResolution(seqResValues[(int)*val[RESOLUTION]]);
    }

    if (size != seqSizeValues[(int)*val[SIZE]]) {
        changed = true;
        updateSize(seqSizeValues[(int)*val[SIZE]]);
    }

    if (transpFromGui != (int)*val[TRANSPOSE]) {
        transpFromGui = (int)*val[TRANSPOSE];
        updateTranspose(transpFromGui);
    }

    if (curLoopMode != (*val[LOOPMODE])) updateLoop(*val[LOOPMODE]);

    if (recordMode != ((bool)*val[RECORD])) {
        setRecordMode((bool)*val[RECORD]);
    }

    if (deferChanges != ((bool)*val[DEFER])) deferChanges = ((bool)*val[DEFER]);
    if (isMuted != (bool)*val[MUTE] && !parChangesPending) setMuted((bool)(*val[MUTE]));

    enableNoteIn =   (int)*val[ENABLE_NOTEIN];
    enableVelIn =    (int)*val[ENABLE_VELIN];
    enableNoteOff = (bool)*val[ENABLE_NOTEOFF];
    restartByKbd =  (bool)*val[ENABLE_RESTARTBYKBD];
    trigByKbd =     (bool)*val[ENABLE_TRIGBYKBD];
    trigLegato =    (bool)*val[ENABLE_TRIGLEGATO];

    channelOut =      (int)*val[CH_OUT];
    chIn =            (int)*val[CH_IN];
    indexIn[0]   =   (int)*val[INDEX_IN1];
    indexIn[1]   =   (int)*val[INDEX_IN2];
    rangeIn[0]   =   (int)*val[RANGE_IN1];
    rangeIn[1]   =   (int)*val[RANGE_IN2];

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

    if (changed) {
        getData(&data);
        dataChanged = true;
    }
}

void MidiSeqLV2::initTransport()
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

void MidiSeqLV2::sendWave()
{
    if (!(dataChanged && ui_up)) return;
    dataChanged = false;

    const QMidiArpURIs* uris = &m_uris;
    int ct = res * size + 1; // last element in wave is an end tag
    int tempArray[ct];

    for (int l1 = 0; l1 < ct; l1++) {
        tempArray[l1]=data[l1].value*((data[l1].muted) ? -1 : 1);
    }

    /* forge container object of type 'hex_customwave' */
    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&forge, 0);
    lv2_atom_forge_object(&forge, &frame, 1, uris->hex_customwave);

    /* Send customWave to UI */
    lv2_atom_forge_property_head(&forge, uris->hex_customwave, 0);
    lv2_atom_forge_vector(&forge, sizeof(int), uris->atom_Int,
        ct, tempArray);

    /* close-off frame */
    lv2_atom_forge_pop(&forge, &frame);
}

#ifdef BUILD_LV2_UI
//=== The following comes from the synthv1 plugin by rncbc ====
QApplication *MidiSeqLV2::g_qAppInstance = nullptr;
unsigned int  MidiSeqLV2::qAppCount = 0;


void MidiSeqLV2::qAppInstantiate(void)
{
	if (qApp == nullptr && g_qAppInstance == nullptr) {
		static int s_argc = 1;
		static const char *s_argv[] = { __func__, nullptr };
		g_qAppInstance = new QApplication(s_argc, (char **) s_argv);
	}

	if (g_qAppInstance)
		qAppCount++;
}


void MidiSeqLV2::qAppCleanup (void)
{
	if (g_qAppInstance && --qAppCount == 0) {
		delete g_qAppInstance;
		g_qAppInstance = nullptr;
	}
}


QApplication *MidiSeqLV2::qAppInstance(void)
{
	return g_qAppInstance;
}
// ====
#endif

static LV2_State_Status MidiSeqLV2_state_restore ( LV2_Handle instance,
    LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature* const* )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);

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

    if (size < 2) return LV2_STATE_ERR_UNKNOWN;

    pPlugin->setFramePtr(0);
    pPlugin->maxNPoints = (size - 1 ) / 2;

    for (l1 = 0; l1 <  pPlugin->maxNPoints; l1++) {
        pPlugin->muteMask[l1] = (value1[2 * l1 + 1] == '1');
    }

    key = uris->hex_customwave;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    const char *value
        = (const char *) (*retrieve)(handle, key, &size, &type, &flags);

    if (size < 2) return LV2_STATE_ERR_UNKNOWN;

    Sample sample;
    int step = TPQN / pPlugin->res;
    int lt = 0;

    for (l1 = 0; l1 <  pPlugin->maxNPoints; l1++) {
        int hi = 0;
        int lo = 0;
        if (value[2*l1] <= '9' && value[2*l1] >= '0') hi = value[2*l1] - '0';
        if (value[2*l1] <= 'f' && value[2*l1] >= 'a') hi = value[2*l1] - 'a' + 10;

        if (value[2*l1 + 1] <= '9' && value[2*l1 + 1] >= '0') lo = value[2*l1 + 1] - '0';
        if (value[2*l1 + 1] <= 'f' && value[2*l1 + 1] >= 'a') lo = value[2*l1 + 1] - 'a' + 10;

        sample.value = hi * 16 + lo;
        sample.tick = lt;
        sample.muted = pPlugin->muteMask[l1];
        pPlugin->customWave[l1] = sample;
        lt+=step;
    }

    pPlugin->getData(&pPlugin->data);
    pPlugin->dataChanged = true;
    return LV2_STATE_SUCCESS;
}

static LV2_State_Status MidiSeqLV2_state_save ( LV2_Handle instance,
    LV2_State_Store_Function store, LV2_State_Handle handle,
    uint32_t flags, const LV2_Feature* const* )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);
    
    if (pPlugin == NULL) return LV2_STATE_ERR_UNKNOWN;

    QMidiArpURIs* const uris = &pPlugin->m_uris;

    uint32_t type = uris->atom_String;

    if (type == 0) return LV2_STATE_ERR_BAD_TYPE;

    flags |= (LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);

    const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    int l1;
    char bt[pPlugin->maxNPoints * 2 + 1];
    
    for (l1 = 0; l1 < pPlugin->maxNPoints; l1++) {
        bt[2*l1] = hexmap[(pPlugin->customWave[l1].value  & 0xF0) >> 4];
        bt[2*l1 + 1] = hexmap[pPlugin->customWave[l1].value  & 0x0F];
    }
    bt[pPlugin->maxNPoints * 2] = '\0';
    
    const char *value = bt;
    
    size_t size = strlen(value) + 1;
    
    uint32_t key = uris->hex_customwave;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    store(handle, key, value, size, type, flags);

    for (l1 = 0; l1 < pPlugin->maxNPoints; l1++) {
        bt[2*l1] = '0';
        bt[2*l1 + 1] = hexmap[pPlugin->muteMask[l1]];
    }

    const char *value1 = bt;

    size = strlen(value1) + 1;
    key = uris->hex_mutemask;
    if (!key) return LV2_STATE_ERR_NO_PROPERTY;

    LV2_State_Status result = (*store)(handle, key, value1, size, type, flags);

    return result;
}

static const LV2_State_Interface MidiSeqLV2_state_interface =
{
    MidiSeqLV2_state_save,
    MidiSeqLV2_state_restore
};

void MidiSeqLV2::activate (void)
{
    initTransport();
}

void MidiSeqLV2::deactivate (void)
{
    transportSpeed = 0;
}

static LV2_Handle MidiSeqLV2_instantiate (
    const LV2_Descriptor *, double sample_rate, const char *,
    const LV2_Feature *const *host_features )
{
#ifdef BUILD_LV2_UI
	MidiSeqLV2::qAppInstantiate();
#endif
    return new MidiSeqLV2(sample_rate, host_features);
}

static void MidiSeqLV2_connect_port (
    LV2_Handle instance, uint32_t port, void *data )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);
    if (pPlugin)
        pPlugin->connect_port(port, data);
}

static void MidiSeqLV2_run ( LV2_Handle instance, uint32_t nframes )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);
    if (pPlugin)
        pPlugin->run(nframes);
}

static void MidiSeqLV2_activate ( LV2_Handle instance )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);
    if (pPlugin)
        pPlugin->activate();
}

static void MidiSeqLV2_deactivate ( LV2_Handle instance )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);
    if (pPlugin)
        pPlugin->deactivate();
}

static void MidiSeqLV2_cleanup ( LV2_Handle instance )
{
    MidiSeqLV2 *pPlugin = static_cast<MidiSeqLV2 *> (instance);
    if (pPlugin)
        delete pPlugin;

#ifdef BUILD_LV2_UI
    MidiSeqLV2::qAppCleanup();
#endif
}

static const void *MidiSeqLV2_extension_data ( const char * uri)
{
    static const LV2_State_Interface state_iface =
                { MidiSeqLV2_state_save, MidiSeqLV2_state_restore };
    if (!strcmp(uri, LV2_STATE__interface)) {
        return &state_iface;
    }
    else return NULL;
}

static const LV2_Descriptor MidiSeqLV2_descriptor =
{
    QMIDIARP_SEQ_LV2_URI,
    MidiSeqLV2_instantiate,
    MidiSeqLV2_connect_port,
    MidiSeqLV2_activate,
    MidiSeqLV2_run,
    MidiSeqLV2_deactivate,
    MidiSeqLV2_cleanup,
    MidiSeqLV2_extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &MidiSeqLV2_descriptor : NULL);
}

