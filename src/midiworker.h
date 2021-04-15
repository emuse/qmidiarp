/*!
 * @file midiworker.h
 * @brief Member definitions for the MidiWorker class.
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

#ifndef MIDIWORKER_H
#define MIDIWORKER_H

#include "main.h"
#include <cstdlib>
#include <cstdio>
#include <cstdint>

/*! @brief MIDI worker base class for QMidiArp modules.
 *
 * The three Midi Module classes inherit from this class. It provides common
 * input output settings variables and some other small functions that all
 * modules have in common
*/
class MidiWorker {

  public:
    double queueTempo;  /*!< current tempo of the transport, not in use here */
    int chIn;           /**< Channel of input events */
    int indexIn[2]; /*!< Note range filter 0: lower, 1: upper limit, set by InOutBox */
    int rangeIn[2]; /*!< Velocity range filter 0: lower, 1: upper limit, set by InOutBox */
    bool enableNoteIn;
    bool enableNoteOff;
    bool enableVelIn;
    bool restartByKbd;
    bool trigByKbd;
    bool trigLegato; /*!< If True, trigger and restart upon legato input notes as well */
    int triggerMode; /*!< Current Trigger mode index */
    bool enableLoop; /*!< Enables looping of sequence or wave, determined by the loopMode */
    bool gotKbdTrig; /*!< Set by MidiWorker::handleEvent() when the module was triggered by a new keyboard stroke */
    bool restartFlag; /*!< Signals frameptr reset on next getNextFrame() call */
    bool backward;       /*!< True when the sequence should start backward */
    bool pingpong;      /*!< True when the play direction should alternate */
    bool reflect;      /*!< True when the current play direction will change at the next reflect point */
    bool reverse;       /*!< True when the current play direction is backwards */
    int curLoopMode;    /*!< Local storage of the currently active Loop mode */
    bool seqFinished;   /*!< When True, all output events are muted, used when NOTE OFF is received */
    bool deferChanges;  /*!< When True, defer parameter changes to pattern end */
    bool parChangesPending;    /*!< set when deferChanges is set and a parameter is changed */
    int portOut;    /*!< MIDI output port number */
    int channelOut; /*!< MIDI output channel */
    int ccnumber;   /*!< MIDI Controller CC number to output */
    int ccnumberIn;
    bool isMuted;   /*!< Global mute state */
    bool isMutedDefer;   /*!< Deferred Global mute state */
    int64_t nextTick; /*!< Holds the next tick at which note events will be played out */
    int noteCount;      /*!< The number of notes in the MidiWorker::notes buffer */
    int newGrooveTick, grooveTick, grooveVelocity, grooveLength;
    int framePtr;       /*!< position of the currently output frame in sequence/wave/pattern */
    int nRepetitions;  /*!< number of repetitions set by parStore at each restore */
    int currentRepetition;  /*!< current repetition pointer of the pattern since pattern was restored */
    int nPoints;        /*!< Number of steps in pattern or sequence or wave */
    bool dataChanged; /*!< Flag set to true by recording loop and queried by InOutBox::updateDisplay() */
    bool needsGUIUpdate; /*!< Flag set to true when MidiWorker members changed and queried by InOutBox::updateDisplay() */

  public:
    MidiWorker();
/*! @brief sets MidiWorker::isMuted, which is checked by
 * Engine and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to the Driver
 */
    virtual void setMuted(bool on);

/*! @brief  sets MidiWorker::deferChanges, which will cause a
 * parameter changes only at pattern end.
 *
 * @param on Set to True to defer changes to pattern end
 */
    virtual void updateDeferChanges(bool on) { deferChanges = on; }
/**
 * @brief  does the actions related to a newly received event.
 *
 * It is called by Engine when a new event is received on the MIDI input port.

 * @param inEv MidiEvent to check and process or not
 * @param tick The time the event was received in internal ticks
 * @param keep_rel Only relevant for the Arp module. If set to True, 
 * a NOTE_OFF event should cause the note to
 * remain in the release buffer. It will definitely be removed if keep_rel is false
 * @return True if inEv is in not the input range of the module (event is unmatched)
 */
    virtual bool handleEvent(MidiEvent inEv, int64_t tick, int keep_rel = 0) = 0;
/**
 * @brief allows forcing an integer value within the
 * specified range (clip).
 *
 * @param value The value to be checked
 * @param min The minimum allowed return value
 * @param max The maximum allowed return value
 * @param outOfRange Is set to True if value was outside min|max range
 * @return The value clipped within the range
 */
    virtual int clip(int value, int min, int max, bool *outOfRange);
    virtual int getFramePtr() { return framePtr; }
/*! @brief  transfers the next Midi data Frame to an intermediate internal object
 * 
 * @param tick the current tick at which we request a note. This tick will be
 * used to calculate the nextTick which is quantized to the pattern
 */
    virtual void getNextFrame(int64_t tick) = 0;
/**
 * @brief sets MidiSeq::nextTick and MidiSeq::framePtr position
 * according to the specified tick.
 *
 * @param tick The current tick to which the module position should be
 * aligned.
 */
    virtual void setNextTick(uint64_t tick) = 0;
/**
 * @brief  Implemented in Arp only. Ensures continuity of the Arp's 
 * release function when the currentTick position jumps into
 * the past.
 *
 * It should be called whenever the transport position is looping. At
 * this time, this is the case when JACK Transport is looping.

 * @param tick The current time position in internal ticks.
 */
    virtual void foldReleaseTicks(int64_t tick) { (void)tick; };
};

#endif
