/*!
 * @file midiworker.h
 * @brief Member definitions for the MidiWorker class.
 *
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QObject>
#include <QVector>

#include <main.h>


/*! @brief MIDI worker base class for QMidiArp modules.
 *
 * The three Midi Module classes inherit from this class. It provides common
 * input output settings variables and some other small functions that all
 * modules have in common
*/
class MidiWorker : public QObject  {

  Q_OBJECT

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
    bool enableLoop;
    bool gotKbdTrig;
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
    int nextTick; /*!< Holds the next tick at which note events will be played out */
    int noteCount;      /*!< The number of notes in the MidiWorker::notes buffer */
    int newGrooveTick, grooveTick, grooveVelocity, grooveLength;
    int framePtr;       /*!< position of the currently output frame in sequence/wave/pattern */
    int nPoints;        /*!< Number of steps in pattern or sequence or wave */
    bool dataChanged; /*!< Flag set to true by recording loop and queried by disp update */
    bool needsGUIUpdate;

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
    virtual void updateQueueTempo(int);
    virtual void updateTriggerMode(int val);
    virtual int getFramePtr() { return framePtr; }
    virtual void getNextFrame(int tick) = 0;
};

#endif
