/*!
 * @file qmidiarp_lfo_lv2.cpp
 * @brief Implements the MidiArp MIDI worker class for the Arpeggiator Module.
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

#include "qmidiarp_lfo_lv2.h"
#include <cstdio>

#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h" // deprecated.
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"

#define LV2_MIDI_EVENT_URI "http://lv2plug.in/ns/ext/midi#MidiEvent"

qmidiarp_lfo_lv2::qmidiarp_lfo_lv2 (
    double sample_rate, const LV2_Feature *const *host_features )
//    : MidiLfo()
{
    eventID = 0;
    sampleRate = sample_rate;
    curFrame = 0;
    inLfoFrame = 0;
    inEventBuffer = NULL;
    outEventBuffer = NULL;
    getNextFrame(0);
    midiLfo = new MidiLfo();

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
        //setParamPort(ParamIndex(port - ParamBase), (float *) data);
        break;
    }
}


void qmidiarp_lfo_lv2::run ( uint32_t nframes )
{
    LV2_Event_Iterator iter, iter_out;
    lv2_event_begin(&iter, inEventBuffer);
    lv2_event_buffer_reset(outEventBuffer, outEventBuffer->stamp_type, outEventBuffer->data);
    lv2_event_begin(&iter_out, outEventBuffer);

        if (inEventBuffer) {
            while (lv2_event_is_valid(&iter)) {
                uint8_t   *data;
                LV2_Event *event = lv2_event_get(&iter, &data);
                if (event && event->type == eventID) {
                    uint64_t eventtime = (uint64_t)event->frames*nframes;
                    MidiEvent inEv;
                    inEv.type=data[0];
                    inEv.data=data[1];
                    inEv.value=data[2];
                    int tick = eventtime*TPQN*120/60/sampleRate;
                    (void)midiLfo->handleEvent(inEv, tick);
                }
                lv2_event_increment(&iter);
            }
            inEventBuffer = NULL;
        }

        for (uint f = 0 ; f < nframes; f++) {
            uint64_t curtick = ((uint64_t)f + curFrame*nframes)*TPQN*120/60/sampleRate;
            if (curtick >= midiLfo->nextTick) {
                //frameptr = getFramePtr();
                //lfoWidget(l1)->cursor->updatePosition(frameptr);
                if (curtick > midiLfo->frame.at(inLfoFrame).tick) {
                    //printf("inlfoframe %d curtick %d - nextTick %d\n", inLfoFrame, curtick, nextTick);
                    if (!midiLfo->frame.at(inLfoFrame).muted && !midiLfo->isMuted) {
                        unsigned char data[3];
                        data[0] = 0xb0 + midiLfo->channelOut;
                        data[1] = midiLfo->ccnumber;
                        data[2] = midiLfo->frame.at(inLfoFrame).value;
                        lv2_event_write(&iter_out, f, 0, eventID, 3, data);
                    }
                    inLfoFrame++;
                    if (inLfoFrame >= midiLfo->frameSize) {
                        midiLfo->getNextFrame(curtick);
                        inLfoFrame = 0;
                    }
                }
            }
        }
        curFrame++;
}


void qmidiarp_lfo_lv2::activate (void)
{
    curFrame = 0;
    midiLfo->nextTick = 0;
    inLfoFrame = 0;
    midiLfo->getNextFrame(0);
}


void qmidiarp_lfo_lv2::deactivate (void)
{
    curFrame = 0;
    midiLfo->nextTick = 0;
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

/*
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
*/

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
/*
static const LV2UI_Descriptor qmidiarp_lfo_lv2ui_descriptor =
{
    QMIDIARP_LFO_LV2UI_URI,
    qmidiarp_lfo_lv2ui_instantiate,
    qmidiarp_lfo_lv2ui_cleanup,
    qmidiarp_lfo_lv2ui_port_event,
    qmidiarp_lfo_lv2ui_extension_data
};

*/
LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
    return (index == 0 ? &qmidiarp_lfo_lv2_descriptor : NULL);
}

/*
LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor ( uint32_t index )
{
    return (index == 0 ? &qmidiarp_lfo_lv2ui_descriptor : NULL);
}
*/
