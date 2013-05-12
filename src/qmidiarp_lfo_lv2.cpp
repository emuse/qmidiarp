/*!
 * @file qmidiarp_lfo_lv2.cpp
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
#include "qmidiarp_lfo_lv2.h"
#include "qmidiarp_lfowidget_lv2.h"

#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h" // deprecated.
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"

#define LV2_MIDI_EVENT_URI "http://lv2plug.in/ns/ext/midi#MidiEvent"
#define LV2_TIME_URI "http://lv2plug.in/ns/ext/time"

qmidiarp_lfo_lv2::qmidiarp_lfo_lv2 (
    double sample_rate, const LV2_Feature *const *host_features )
    :MidiLfo()
{
    eventID = 0;
    sampleRate = sample_rate;
    curFrame = 0;
    inLfoFrame = 0;
    inEventBuffer = NULL;
    outEventBuffer = NULL;
    getNextFrame(0);
    waveIndex = 0;
    mouseXCur = 0;
    mouseYCur = 0;

    for (int i = 0; host_features[i]; ++i) {
        if (::strcmp(host_features[i]->URI, LV2_URID_MAP_URI) == 0) {
            LV2_URID_Map *urid_map
                = (LV2_URID_Map *) host_features[i]->data;
            if (urid_map) {
                eventID = urid_map->map(
                    urid_map->handle, LV2_MIDI_EVENT_URI);
                break;
            }
        }
        else
        if (::strcmp(host_features[i]->URI, LV2_URI_MAP_URI) == 0) {
            LV2_URI_Map_Feature *uri_map_feature
                = (LV2_URI_Map_Feature *) host_features[i]->data;
            if (uri_map_feature) {
                eventID = uri_map_feature->uri_to_id(
                    uri_map_feature->callback_data,
                    LV2_EVENT_URI, LV2_MIDI_EVENT_URI);
                break;
            }
        }
    }
}


qmidiarp_lfo_lv2::~qmidiarp_lfo_lv2 (void)
{
}

void qmidiarp_lfo_lv2::connect_port ( uint32_t port, void *data )
{
    switch(PortIndex(port)) {
    case MidiIn:
        inEventBuffer = (LV2_Event_Buffer *) data;
        break;
    case MidiOut:
        outEventBuffer = (LV2_Event_Buffer *) data;
        break;
    default:
        val[port - 2] = (float *) data;
        break;
    }
}

void qmidiarp_lfo_lv2::run ( uint32_t nframes )
{
    LV2_Event_Iterator iter_out;
    lv2_event_buffer_reset(outEventBuffer, outEventBuffer->stamp_type, outEventBuffer->data);
    lv2_event_begin(&iter_out, outEventBuffer);

    if (inEventBuffer) {
        LV2_Event_Iterator iter;
        lv2_event_begin(&iter, inEventBuffer);
        while (lv2_event_is_valid(&iter)) {
            uint8_t   *data;
            LV2_Event *event = lv2_event_get(&iter, &data);
            if (event && event->type == eventID) {
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
                int tick = (uint64_t)curFrame*nframes*TPQN*120/60/sampleRate;
                (void)handleEvent(inEv, tick);
            }
            lv2_event_increment(&iter);
        }
    }

    if ((curFrame % 12) == 5) updateParams();
    if (!(curFrame % 12)) sendWave();

    for (uint f = 0 ; f < nframes; f++) {
        int curtick = ((uint64_t)f + curFrame*nframes)*TPQN*120/60/sampleRate;
        if (curtick >= nextTick) {
            if (curtick > frame.at(inLfoFrame).tick) {
                //printf("inlfoframe %d curtick %d - nextTick %d\n", inLfoFrame, curtick, nextTick);
                if (!frame.at(inLfoFrame).muted && !isMuted) {
                    unsigned char d[3];
                    d[0] = 0xb0 + channelOut;
                    d[1] = ccnumber;
                    d[2] = frame.at(inLfoFrame).value;
                    lv2_event_write(&iter_out, f, 0, eventID, 3, d);
                }
                inLfoFrame++;
                if (inLfoFrame >= frameSize) {
                    frameptr = getFramePtr();
                    float pos = (float)frameptr;
                    *val[CURSOR_POS - 2] = pos;
                    getNextFrame(curtick);
                    inLfoFrame = 0;
                }
            }
        }
    }
    curFrame++;
}

void qmidiarp_lfo_lv2::updateParams()
{
    bool changed = false;
    if (mouseXCur != *val[27] || mouseYCur != *val[28]) {
        changed = false;
        mouseXCur = *val[27];
        mouseYCur = *val[28];
        int ix = mouseEvent(mouseXCur, mouseYCur, *val[29], *val[30]);
        if (*val[30]) lastMouseIndex = ix; // mouse pressed
        getData(&data);
        // Update all wave data points since the last mouse event
        int dir = (lastMouseIndex > ix) ? -1 : 1;
        for (int l1 = lastMouseIndex; l1*dir < (ix + dir)*dir; l1+=dir) {
            sendSample(l1, l1 % 16);
        }
        lastMouseIndex = ix; // mouse pressed
        return;
    }

    if (amp != *val[0]) {
        changed = true;
        updateAmplitude(*val[0]);
    }

    if (offs != *val[1]) {
        changed = true;
        updateOffset(*val[1]);
    }

    if (res != lfoResValues[(int)*val[2]]) {
        changed = true;
        updateResolution(lfoResValues[(int)*val[2]]);
    }

    if (size != 1+ (int)*val[3]) {
        changed = true;
        updateSize(1 + (int)*val[3]);
    } // TODO: get correct size values

    if (freq != lfoFreqValues[(int)*val[4]]) {
        changed = true;
        updateFrequency(lfoFreqValues[(int)*val[4]]);
    }

    if (waveFormIndex != (int)*val[24]) {
        changed = true;
        updateWaveForm(*val[24]);
    }

    if (curLoopMode != (*val[25])) updateLoop(*val[25]);
    isMuted = (bool)(*val[26]);

    enableNoteOff = (bool)*val[33];
    restartByKbd = (bool)(*val[34]);
    trigByKbd = (bool)(*val[35]);
    trigLegato = (bool)(*val[36]);

    channelOut = ((int)*val[5]);
    chIn = ((int)*val[6]);
    ccnumber = ((int)*val[31]);
    ccnumberIn = ((int)*val[32]);

    if (changed) {
        getData(&data);
        waveIndex = 0;
        dataChanged = true;
    }
}

void qmidiarp_lfo_lv2::sendWave()
{
    if (!dataChanged) return;
    int ct = data.count();
    // The 16 ports are swept to transmit the data in parallel
    for (int l1 = 0; l1 < 16; l1++) {
        waveIndex %= ct;
        sendSample(waveIndex, l1);
        if (!waveIndex && (l1 > 0)) {
            dataChanged = false;
            return;
        }
        waveIndex++;
    }
}

void qmidiarp_lfo_lv2::sendSample(int ix, int port)
{
    // wave data and index are encodred into a single float
        *val[WAVEDATA1 + port - 2] = (float)(data.at(ix).value
             + ix*128) * ((data.at(ix).muted == false) ? 1 : -1);
}

void qmidiarp_lfo_lv2::activate (void)
{
    curFrame = 0;
    nextTick = 0;
    inLfoFrame = 0;
    getNextFrame(0);
}

void qmidiarp_lfo_lv2::deactivate (void)
{
    curFrame = 0;
    nextTick = 0;
    inLfoFrame = 0;
}

static LV2_Handle qmidiarp_lfo_lv2_instantiate (
    const LV2_Descriptor *, double sample_rate, const char *,
    const LV2_Feature *const *host_features )
{
    return new qmidiarp_lfo_lv2(sample_rate, host_features);
}

static void qmidiarp_lfo_lv2_connect_port (
    LV2_Handle instance, uint32_t port, void *data )
{
    qmidiarp_lfo_lv2 *pPlugin = static_cast<qmidiarp_lfo_lv2 *> (instance);
    if (pPlugin)
        pPlugin->connect_port(port, data);
}

static void qmidiarp_lfo_lv2_run ( LV2_Handle instance, uint32_t nframes )
{
    qmidiarp_lfo_lv2 *pPlugin = static_cast<qmidiarp_lfo_lv2 *> (instance);
    if (pPlugin)
        pPlugin->run(nframes);
}

static void qmidiarp_lfo_lv2_activate ( LV2_Handle instance )
{
    qmidiarp_lfo_lv2 *pPlugin = static_cast<qmidiarp_lfo_lv2 *> (instance);
    if (pPlugin)
        pPlugin->activate();
}

static void qmidiarp_lfo_lv2_deactivate ( LV2_Handle instance )
{
    qmidiarp_lfo_lv2 *pPlugin = static_cast<qmidiarp_lfo_lv2 *> (instance);
    if (pPlugin)
        pPlugin->deactivate();
}

static void qmidiarp_lfo_lv2_cleanup ( LV2_Handle instance )
{
    qmidiarp_lfo_lv2 *pPlugin = static_cast<qmidiarp_lfo_lv2 *> (instance);
    if (pPlugin)
        delete pPlugin;
}

static const void *qmidiarp_lfo_lv2_extension_data ( const char * )
{
    return NULL;
}

static LV2UI_Handle qmidiarp_lfo_lv2ui_instantiate (
    const LV2UI_Descriptor *, const char *, const char *,
    LV2UI_Write_Function write_function,
    LV2UI_Controller controller, LV2UI_Widget *widget,
    const LV2_Feature *const * )
{
    qmidiarp_lfowidget_lv2 *pWidget = new qmidiarp_lfowidget_lv2(controller, write_function);
    *widget = pWidget;
    return pWidget;
}

static void qmidiarp_lfo_lv2ui_cleanup ( LV2UI_Handle ui )
{
    qmidiarp_lfowidget_lv2 *pWidget = static_cast<qmidiarp_lfowidget_lv2 *> (ui);
    if (pWidget)
        delete pWidget;
}

static void qmidiarp_lfo_lv2ui_port_event (
    LV2UI_Handle ui, uint32_t port_index,
    uint32_t buffer_size, uint32_t format, const void *buffer )
{
    qmidiarp_lfowidget_lv2 *pWidget = static_cast<qmidiarp_lfowidget_lv2 *> (ui);
    if (pWidget)
        pWidget->port_event(port_index, buffer_size, format, buffer);
}

static const void *qmidiarp_lfo_lv2ui_extension_data ( const char * )
{
    return NULL;
}

static const LV2_Descriptor qmidiarp_lfo_lv2_descriptor =
{
    QMIDIARP_LFO_LV2_URI,
    qmidiarp_lfo_lv2_instantiate,
    qmidiarp_lfo_lv2_connect_port,
    qmidiarp_lfo_lv2_activate,
    qmidiarp_lfo_lv2_run,
    qmidiarp_lfo_lv2_deactivate,
    qmidiarp_lfo_lv2_cleanup,
    qmidiarp_lfo_lv2_extension_data
};

static const LV2UI_Descriptor qmidiarp_lfo_lv2ui_descriptor =
{
    QMIDIARP_LFO_LV2UI_URI,
    qmidiarp_lfo_lv2ui_instantiate,
    qmidiarp_lfo_lv2ui_cleanup,
    qmidiarp_lfo_lv2ui_port_event,
    qmidiarp_lfo_lv2ui_extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &qmidiarp_lfo_lv2_descriptor : NULL);
}

LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor ( uint32_t index )
{
    return (index == 0 ? &qmidiarp_lfo_lv2ui_descriptor : NULL);
}

