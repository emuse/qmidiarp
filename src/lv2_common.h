/*!
 * @file lv2_common.h
 * @brief URI definitions and mapping for QMidiArp LV2 plugins
 *
 *
 *      Copyright 2011 - 2023 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef LV2_COMMON_H
#define LV2_COMMON_H

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#define QMIDIARP_LV2_URI "https://git.code.sf.net/p/qmidiarp"
#define QMIDIARP_LV2_PREFIX QMIDIARP_LV2_URI "#"

typedef struct {
    LV2_URID atom_Object;
    LV2_URID atom_Blank;
    LV2_URID atom_Float;
    LV2_URID atom_Int;
    LV2_URID atom_Vector;
    LV2_URID atom_Long;
    LV2_URID atom_String;
    LV2_URID atom_eventTransfer;
    LV2_URID atom_Resource;
    LV2_URID time_Position;
    LV2_URID time_frame;
    LV2_URID time_barBeat;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_speed;
    LV2_URID midi_MidiEvent;
    LV2_URID atom_Sequence;
    LV2_URID hex_customwave;
    LV2_URID hex_mutemask;
    LV2_URID pattern_string;
    LV2_URID ui_up;
    LV2_URID ui_down;
    LV2_URID flip_wave;
} QMidiArpURIs;

static inline void map_uris(LV2_URID_Map* urid_map, QMidiArpURIs* uris) {
    uris->atom_Object         = urid_map->map(urid_map->handle, LV2_ATOM__Object);
    uris->atom_Blank          = urid_map->map(urid_map->handle, LV2_ATOM__Blank);
    uris->atom_Float          = urid_map->map(urid_map->handle, LV2_ATOM__Float);
    uris->atom_Int            = urid_map->map(urid_map->handle, LV2_ATOM__Int);
    uris->atom_Vector         = urid_map->map(urid_map->handle, LV2_ATOM__Vector);
    uris->atom_Long           = urid_map->map(urid_map->handle, LV2_ATOM__Long);
    uris->atom_String         = urid_map->map(urid_map->handle, LV2_ATOM__String);
    uris->atom_eventTransfer  = urid_map->map(urid_map->handle, LV2_ATOM__eventTransfer);
    uris->atom_Resource       = urid_map->map(urid_map->handle, LV2_ATOM__Resource);
    uris->time_Position       = urid_map->map(urid_map->handle, LV2_TIME__Position);
    uris->time_frame          = urid_map->map(urid_map->handle, LV2_TIME__frame);
    uris->time_barBeat        = urid_map->map(urid_map->handle, LV2_TIME__barBeat);
    uris->time_beatsPerMinute = urid_map->map(urid_map->handle, LV2_TIME__beatsPerMinute);
    uris->time_speed          = urid_map->map(urid_map->handle, LV2_TIME__speed);
    uris->midi_MidiEvent      = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
    uris->atom_Sequence       = urid_map->map(urid_map->handle, LV2_ATOM__Sequence);
    uris->hex_customwave      = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "WAVEHEX");
    uris->hex_mutemask        = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "MUTEHEX");
    uris->pattern_string      = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "ARPPATTERN");
    uris->ui_up               = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "UI_UP");
    uris->ui_down             = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "UI_DOWN");
    uris->flip_wave           = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "FLIP_WAVE");
}
#endif
