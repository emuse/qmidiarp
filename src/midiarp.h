/*!
 * @file midiarp.h
 * @brief Member definitions for the MidiArp MIDI worker class.
 *
 *
 *      Copyright 2009 - 2017 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef MIDIARP_H
#define MIDIARP_H

#include <string>
#include "midiworker.h"

 /*!
 * @brief MIDI worker class for the Arpeggiator Module. Implements the
 * functions providing note arpeggiation.
 *
 * The parameters of MidiArp are controlled by the ArpWidget class.
 * The backend driver thread calls the Engine::echoCallback(), which will
 * query each module, in this case via
 * the MidiArp::getNextFrame() method. MidiArp will then call its
 * internal MidiArp::getNote() function which produces an array of notes
 * stored in its internal output buffer. The notes in the array depend
 * on the active MidiArp::pattern, envelope, random and groove settings.
 * The note events consist of timing information
 * (tick and length), note values and velocity values. MidiArp::getNote()
 * also advances the pattern index. Engine::tickCallback() then
 * accesses this output buffer and sends it to the backend driver. Engine
 * also calls MidiArp::handleEvent() in particular to store incoming notes
 * in its note buffer. 
 */
class MidiArp : public MidiWorker  {

  private:
    int nextNote[MAXCHORD]; /*!< Holds the note values to be output next
                                @see MidiArp::updateNotes */
    int nextVelocity[MAXCHORD]; /*!< Holds the associated velocities to be output next
                                    @see MidiArp::updateNotes, MidiArp::nextNote */
    int arpTick;
    int nextLength;
    bool chordMode;
    bool purgeReleaseFlag; /*!< Causes MidiArp::getNote() to call MidiArp::purgeReleaseNotes() */
    int patternIndex; /*!< Holds the current position within the pattern text*/
    int randomTick, randomVelocity, randomLength;
    int sustainBufferCount, latchBufferCount;
    int lastLatchTick;
    double stepWidth, len, vel;
    int sustainBuffer[MAXNOTES]; /*!< Holds released note values when MidiArp::sustain is True */
    int latchBuffer[MAXNOTES];   /*!< Holds released note values when MidiArp::latch_mode is True */

    bool sustain;
    int noteIndex[MAXCHORD], chordSemitone[MAXCHORD];
    int semitone;
 /*! @brief The input note buffer array of the Arpeggiator, which has
  * two array copies.
  *
  * @par The first index (0:1) selects the copy to be modified
  * avoiding access conflicts while modifying its data.
  * @par The second index selects the columns corresponding to
  * - 0: Note value
  * - 1: Note velocity
  * - 2: Timing of the note event in internal ticks (NOTE_ON or NOTE_OFF)
  * - 3: Release tag (0: note is kept in buffer, 1: MidiArp::getNote
  * will decrease this note's velocity until it reaches 0, and then
  * definitely remove the by a MidiArp::removeNote call.
  *
  * @par The third index is the note index in the buffer.
  * */
    int notes[2][4][MAXNOTES];

 /*! @brief The storage copy of dynamic attack values.
  *
  * These values are to be multiplied with the
  * velocity at each new arpeggiator step. Its index corresponds
  * to that of the third MidiArp::notes buffer index.
  * */
    double old_attackfn[MAXNOTES];
    int noteBufPtr;     /*!< Pointer to the currently active note buffer copy */
    int patternLen;     /*!< Length of the arp text pattern */
    int noteOfs;        /*!< The current index in a chord. @see repeatPatternThroughChord */
    int octOfs;        /*!< The currently active octave shift. @see repeatPatternThroughChord */
    int octIncr;        /*!< The octave increment at repeat end. @see repeatPatternThroughChord */
    int releaseNoteCount; /*!< The number of notes currently in release stage */

/**
 * @brief  resets all attributes the pattern
 * accumulates during run.
 *
 * It is called when the currentIndex revolves to restart the loop with
 * default velocity, step width, octave and length.
*/
    void initLoop();
/**
 * @brief This is MidiArp's main note processor producing output notes
 * from input notes.
 *
 * It analyzes the MidiArp::pattern text and MidiArp::notes input buffer
 * to yield arrays of notes that have to be sent at the given timing.
 * The calculated note data is stored in arrays, copied again by
 * getNextFrame() and the copy is accessed by Engine::echoCallback().
 * Only in case of an arpeggio step involving chords, these arrays have
 * sizes > 1.
 * @param tick The timing of the notes to be scheduled
 * @param note The array of notes to be filled
 * @param velocity The associated array of velocites to be filled
 * @param length The note length for this arpeggio step
 */
    void getNote(int *tick, int note[], int velocity[], int *length);
/**
 * @brief  returns the number of notes present at the MIDI
 * input port.
 *
 * This is the number of notes currently pressed on the keyboard. Note
 * that the input MidiArp::notes buffer size can be different from this
 * number, since it can contain notes in release state or in the
 * MidiArp::latchBuffer.
 *
 * @return Number of notes present at the MIDI input port.
 */
    int getPressedNoteCount();

/**
 * @brief Adds an incoming note to the note buffer
 *
 * This function is called when a NOTE ON event is received. The 
 * specified note is 
 * 
 * @param note note data
 * @param val note velocity value
 * @param tick the tick position of the event
  * 
 */
    void addNote(int note, int vel, int tick);
/**
 * @brief Either deletes a note or tags the note as released
 *
 * This function is called when the latch and sustain buffers are 
 * cleared. The specified note is either 
 * deleted via MidiArp::deleteNoteAt() or tagged as released if the 
 * release function is active and if the keep_rel flag is set to 1. 
 *
 * @param noteptr pointer to the note to be looked for
 * @param tick the current tick position
 * @param keep_rel If set to 1 and MidiArp::release_time is set, the 
 * note is marked as released. If set to 0, the note will be deleted
 * 
 */
    void removeNote(int *noteptr, int tick, int keep_rel);
/**
 * @brief Handles a released incoming note
 *
 * This function is called when a NOTE OFF event is received. One note
 * of the type specified is either deleted from the buffer or tagged 
 * as released if the release time of the module is set > 0.
 * 
 * @param note note data
 * @param tick the tick position of the event
 * @param keep_rel If set to 1 and MidiArp::release_time is set, the 
 * note is marked as released. If set to 0, the note will be deleted
 * 
 */
    void releaseNote(int note, int tick, bool keep_rel);
/**
 * @brief  Deletes a note inside the MidiArp::notes input
 * note buffer.
 *
 * The note  at the given index is deleted from the buffer with the
 * given bufPtr index (0 or 1), and notes at higher positions in the
 * buffer are moved in position.
 * @param index Index of the note to delete from the buffer
 * @param bufPtr Buffer copy to work with
 */
    void deleteNoteAt(int index, int bufPtr);
/**
 * @brief  sets the released flag (value 1) for the note
 * at the given index
 *
 * This operation is done for the buffer with given bufPtr index (0 or 1).
 * A released flag set will cause getNote to diminish the velocity of
 * this note at each arpeggio step, and to remove it when the velocity
 * reaches zero.
 *
 * @param note Note value to be tagged as released
 * @param tick The time in internal ticks at which the note was released
 * @param bufPtr The index of the MidiArp::notes buffer currently in use
 */
    void tagAsReleased(int note, int tick, int bufPtr);
/**
 * @brief  performs a copy within MidiArp::notes from the
 * currently active index to the inactive index
 */
    void copyNoteBuffer();
/**
 * @brief Advances octOfs according to the settings. Called when the octave
 * reaches an edge condition (at octave range or outside permitted range)
 * @param reset Set to True in order to set the octave shift to zero
 */
    void checkOctaveAtEdge(bool reset);

  public:
    bool latch_mode; /*!< If True hold notes released earlier than latch delay in latch buffer */
    bool hasNewNotes; /*!< True when getNextFrame() was called with a tick causing new note calculation */
    int repeatPatternThroughChord; /*!< Repeat mode "Static", "Up", "Down", "As Played" set by ArpWidget */
    double attack_time;/*!< Attack time in seconds, set by ArpWidget */
    double release_time;/*!< Release time in seconds, set by ArpWidget */
    int randomTickAmp; /*!< Amplitude of timing randomization, set by ArpWidget */
    int randomVelocityAmp; /*!< Amplitude of velocity randomization, set by ArpWidget */
    int randomLengthAmp; /*!< Amplitude of length randomization, set by ArpWidget */
    int trigDelayTicks; /*!< Ticks to wait for playing out notes after trigger in delayed trig mode */

    std::string pattern; /*!< Holds the the arpeggio pattern text */
    int maxOctave;      /*!< Maximum octave shift found in the pattern */
    int minOctave;      /*!< Minimum octave shift found in the pattern */
    double minStepWidth; /*!< Minimum step width of the pattern for quantization purposes*/
    double nSteps;      /*!< Musical length of the pattern in beats */
    int patternMaxIndex;/*!< Maximum number of stacked notes in the pattern */
    int octMode;        /*!< The octave Mode 0=up, 1=down, 2=pingpong. @see repeatPatternThroughChord */
    int octLow;        /*!< The lower octave limit. @see repeatPatternThroughChord */
    int octHigh;        /*!< The higher octave limit. @see repeatPatternThroughChord */

    int returnNote[MAXCHORD]; /*!< Holds the notes of the currently active arpeggio step */
    int returnVelocity[MAXCHORD]; /*!< Holds the velocities of the currently active arpeggio step */
    int returnTick; /*!< Holds the time in internal ticks of the currently active arpeggio step */
    int returnLength; /*!< Holds the note length of the currently active arpeggio step */

  public:
    MidiArp();
    virtual ~MidiArp() {}
    std::string stripPattern(const std::string& p_pattern);
    void updatePattern(const std::string&);
    void updateRandomTickAmp(int);
    void updateRandomVelocityAmp(int);
    void updateRandomLengthAmp(int);
    void updateAttackTime(int);
    void updateReleaseTime(int);
/**
 * @brief  calculates the index of the next arpeggio
 * step and revolves it if necessary.
 *
 * The next step depends on the MidiArp::repeatPatternThrough chord mode.
 * The pattern index can be simply reset to zero if the reset flag is
 * set to True.
 * @param reset Set to True in order to set the pattern index to zero
 * @return True if the pattern index is now zero
 */
    bool advancePatternIndex(bool reset);
/**
 * @brief  does the actions related to a newly received event.
 *
 * It is called by Engine when a new event is received on the MIDI input port.

 * @param inEv MidiEvent to check and process or not
 * @param tick The time the event was received in internal ticks
 * @param keep_rel If set to True, a NOTE_OFF event should cause the note to
 * remain in the release buffer. It will definitely be removed if keep_rel is false
 * @return True if inEv is in not the input range of the module (event is unmatched)
 */
    bool handleEvent(MidiEvent inEv, int tick, int keep_rel = 0);
/**
 * @brief Causes calculation of a new note set at a step and copies it
 * to arrays accessed by Engine.
 *
 * It is called by Engine::echoCallback() when a previously scheduled echo
 * event is received from the driver. It calls MidiArp::getNote().
 * It copies the new data to MidiArp::returnNote, MidiArp::returnVelocity,
 * MidiArp::returnLength. Engine::echoCallback() then accesses this
 * data directly and transfers it to the driver.
 *
 * @param askedTick the current transport position in ticks.
 *
 */
    void getNextFrame(int askedTick);
/**
 * @brief  resets the pattern index and sets the current
 * timing of the arpeggio to currentTick.
 *
 * It is called by Engine when the transport is started, or when
 * a stakato note is received while MidiArp::restartByKbd is set.
 *
 * @param tick The timing in internal ticks, relative to which
 * the following arpeggio notes are calculated.
 */
    void initArpTick(int tick);
/**
 * @brief  ensures continuity of the release function
 * when the currentTick position jumps into
 * the past.
 *
 * It should be called whenever the transport position is looping. At
 * this time, this is the case when JACK Transport is looping.

 * @param tick The current time position in internal ticks.
 */
    void foldReleaseTicks(int tick);
/**
 * @brief  seeds new random values for the three parameters
 * concerned, timing (tick), velocity and length.
 *
 * It is
 * called by MidiArp::getNote at every new note. It uses the
 * MidiArp::randomTickAmp, MidiArp::randomVelocityAmp and
 * MidiArp::randomLengthAmp settings coming from ArpWidget.
 * The values are then used by MidiArp::getNote.
 */
    void newRandomValues();
 /*! @brief Set by Engine when MidiCC #64 is received.
  *
  * Will cause notes remaining in MidiArp::sustainBuffer until
  * set to false.
  * @param sustain Set to True to cause hold mode
  * @param tick Time in internal ticks at which the controller was received */
    void setSustain(bool sustain, int tick);
 /*! @brief Will cause notes remaining in MidiArp::latchBuffer until new
  * stakato note received
  *
  *  It is called by ArpWidget::setLatchMode
  */
    void setLatchMode(bool);
 /*! @brief Calls MidiArp::removeNote for all notes in MidiArp::sustainBuffer
  * and then clears sustainBuffer.
  *
  *  It is called by MidiArp::setSustain
  * @param sustick Time in internal ticks at which the controller was received */
    void purgeSustainBuffer(int sustick);
 /*! @brief sets MidiArp::noteCount to zero and clears MidiArp::latchBuffer. */
    void clearNoteBuffer();
/*! @brief Checks if deferred parameter changes are pending and applies
 * them if so
 */
    void applyPendingParChanges();

/*! @brief Sets octave mode and octave increment accordingly, resets the
 * current octave shift
 */
    void updateOctaveMode(int val);

 /*! @brief Calls MidiArp::removeNote for
  * all notes in MidiArp::latchBuffer and then clears latchBuffer.
  * @param latchtick Time of note release in internal ticks
  */
    void purgeLatchBuffer(int latchtick);

 /*! @brief Untags notes tagged as released in the specified buffer.
  * 
  * @param bufptr Buffer pointer (0 or 1) to the buffer to act on
  */
    void purgeReleaseNotes(int bufptr);
/**
 * @brief sets MidiArp::nextTick and MidiArp::patternIndex position
 * according to the specified tick.
 *
 * @param tick The current tick to which the module position should be
 * aligned.
 */
    void setNextTick(int tick);

};

#endif
