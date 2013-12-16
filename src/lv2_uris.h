/*!
 * @file lv2_uris.h
 * @brief URI definitions and mapping for QMidiArp LV2 plugins
 *
 * @section LICENSE
 *
 *      Copyright 2011 - 2013 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef LV2_URIS_H
#define LV2_URIS_H
#include "lv2.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

typedef struct {
    LV2_URID atom_Blank;
    LV2_URID atom_Float;
    LV2_URID atom_Int;
    LV2_URID atom_Vector;
    LV2_URID atom_Long;
    LV2_URID atom_String;
    LV2_URID atom_Resource;
    LV2_URID time_Position;
    LV2_URID time_frame;
    LV2_URID time_barBeat;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_speed;
    LV2_URID ui_state;
    LV2_URID hex_customwave;
    LV2_URID hex_mutemask;
} QMidiArpURIs;

static inline void map_uris(LV2_URID_Map* urid_map, QMidiArpURIs* uris) {
    uris->atom_Blank          = urid_map->map(urid_map->handle, LV2_ATOM__Blank);
    uris->atom_Float          = urid_map->map(urid_map->handle, LV2_ATOM__Float);
    uris->atom_Int            = urid_map->map(urid_map->handle, LV2_ATOM__Int);
    uris->atom_Long           = urid_map->map(urid_map->handle, LV2_ATOM__Long);
    uris->atom_String         = urid_map->map(urid_map->handle, LV2_ATOM__String);
    uris->atom_Resource       = urid_map->map(urid_map->handle, LV2_ATOM__Resource);
    uris->time_Position       = urid_map->map(urid_map->handle, LV2_TIME__Position);
    uris->time_frame          = urid_map->map(urid_map->handle, LV2_TIME__frame);
    uris->time_barBeat        = urid_map->map(urid_map->handle, LV2_TIME__barBeat);
    uris->time_beatsPerMinute = urid_map->map(urid_map->handle, LV2_TIME__beatsPerMinute);
    uris->time_speed          = urid_map->map(urid_map->handle, LV2_TIME__speed);
    uris->hex_customwave      = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "WAVEHEX");
    uris->hex_mutemask        = urid_map->map(urid_map->handle, QMIDIARP_LV2_PREFIX "MUTEHEX");
}
#endif
