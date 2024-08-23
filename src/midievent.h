/*!
 * @file midievent.h
 * @brief Defines the MidiEvent structure and the midi_event_type enum
 *
 *
 *      Copyright 2009 - 2024 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef SAMPLE_H
#define SAMPLE_H

/*! @brief Structure holding elements of a MIDI note or controller representing
 * one point of a waveform
 */
    struct Sample {
        int data;
        int value;
        int tick;
        bool muted;
    };
#endif

#ifndef MIDIEVENT_H
#define MIDIEVENT_H


/*! @brief Structure holding elements of a MIDI event
 */
typedef struct {
        int type;
        int channel;
        int data;
        int value;
    } MidiEvent;

#ifdef APPBUILD
#include <QMetaType>
Q_DECLARE_METATYPE (MidiEvent)
#endif

/*! @brief Sequencer event type enum in analogy to the ALSA snd_seq_event_types */
enum midi_event_type {
    EV_SYSTEM = 0,
    EV_RESULT,

    EV_NOTE = 5,
    EV_NOTEON,
    EV_NOTEOFF,
    EV_KEYPRESS,

    EV_CONTROLLER = 10,
    EV_PGMCHANGE,
    EV_CHANPRESS,
    EV_PITCHBEND,
    EV_CONTROL14,
    EV_NONREGPARAM,
    EV_REGPARAM,

    EV_SONGPOS = 20,
    EV_SONGSEL,
    EV_QFRAME,
    EV_TIMESIGN,
    EV_KEYSIGN,

    EV_START = 30,
    EV_CONTINUE,
    EV_STOP,
    EV_SETPOS_TICK,
    EV_SETPOS_TIME,
    EV_TEMPO,
    EV_CLOCK,
    EV_TICK,
    EV_QUEUE_SKEW,
    EV_SYNC_POS,

    EV_TUNE_REQUEST = 40,
    EV_RESET,
    EV_SENSING,

    EV_ECHO = 50,
    EV_SYSEX = 130,
    EV_BOUNCE,

    EV_NONE = 255
};

#endif
