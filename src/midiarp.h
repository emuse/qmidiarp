/*!
 * @file midiarp.h
 * @brief Member definitions for the MidiArp MIDI worker class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QMutex>
#include <QWidget>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <main.h>

 /*!
 * @brief MIDI worker class for the Arpeggiator Module. Implements the
 * functions providing note arpeggiation as a QWidget.
 *
 * The parameters of MidiArp are controlled by the ArpWidget class.
 * A pointer to MidiArp is passed to the Engine, which calls
 * the MidiArp::prepareCurrentNote member as a function of the position of
 * the Driver queue. MidiArp will then call its
 * internal MidiArp::getNote function which produces an array of notes
 * stored in its internal output buffer. The notes in the array depend
 * on the active MidiArp::pattern, envelope, random and groove settings.
 * The note events consist of timing information
 * (tick and length), note values and velocity values. MidiArp::getNote
 * also advances the pattern index and emits the MidiArp::nextStep signal
 * to update the cursor position in the graphical ArpScreen display part
 * of ArpWidget. Engine then
 * accesses this output buffer and sends it to the Driver queue. Engine
 * also calls MidiArp::handleNoteOn and MidiArp::handleNoteOff. These members
 * manage the arpeggiator input note buffer
 * also part of this class. Notes received on the MIDI input port will
 * therefore be added or removed from the buffer as Engine transfers
 * them to this class.
 */
class MidiArp : public QWidget  {

  Q_OBJECT

  private:
    int nextNote[MAXCHORD]; /*!< Holds the note values to be output next
                                @see MidiArp::updateNotes */
    int nextVelocity[MAXCHORD]; /*!< Holds the associated velocities to be output next
                                    @see MidiArp::updateNotes, MidiArp::nextNote */
    int currentNoteTick, currentTick, arpTick;
    int currentNote[MAXCHORD], currentVelocity[MAXCHORD];
    int currentLength, nextLength;
    bool newCurrent, newNext, chordMode;
    bool restartFlag; /*!< Signals frameptr reset on next getNextFrame() call */
    int newGrooveTick, grooveTick, grooveVelocity, grooveLength, grooveIndex;
    int randomTick, randomVelocity, randomLength;
    double stepWidth, len, vel;

    QVector<int> sustainBuffer; /*!< Holds released note values when MidiArp::sustain is True */
    QVector<int> latchBuffer;   /*!< Holds released note values when MidiArp::latch_mode is True */
    QTimer *latchTimer;         /*!< Is started when a note is released and causes MidiArp::purgeLatchBuffer after
                                    a delay of 200ms except when another released note is detected before. */

    bool sustain, latch_mode;
    int octave, noteIndex[MAXCHORD], patternIndex;
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
    int noteCount;      /*!< The number of notes in the MidiArp::notes buffer */
    int patternLen;     /*!< Length of the arp text pattern */
    int patternMaxIndex;/*!< Maximum number of stacked notes in the pattern */
    int noteOfs;        /*!< The current index in a chord. @see repeatPatternThroughChord */
    int releaseNoteCount; /*!< The number of notes currently in release stage */

/**
 * @brief This function resets all attributes the pattern
 * accumulates during run.
 *
 * It is called when the currentIndex revolves to restart the loop with
 * default velocity, step width, octave and length.
*/
    void initLoop();
/**
 * @brief This function allows forcing an integer value within the
 * specified range (clip).
 *
 * @param value The value to be checked
 * @param min The minimum allowed return value
 * @param max The maximum allowed return value
 * @param outOfRange Is set to True if value was outside min|max range
 * @return The value clipped within the range
 */
    int clip(int value, int min, int max, bool *outOfRange);
/**
 * @brief This function updates the current note arrays with new values
 * obtained from MidiArp::getNote
 *
 * It is called by Engine::echoCallback() and calls MidiArp::getNote if the given
 * timing is ahead of the last timing information. It then transfers the
 * MidiArp::nextNote and MidiArp::nextVelocity arrays into
 * MidiArp::currentNote and MidiArp::currentVelocity.
 *
 */
    void updateNotes();
/**
 * @brief This is MidiArp's main note processor producing output notes
 * from input notes.
 *
 * It analyzes the MidiArp::pattern text and MidiArp::notes input buffer
 * to yield arrays of notes that have to be output at the given timing.
 * Only in case of an arpeggio step involving chords, these arrays have
 * sizes > 1.
 * @param tick The timing of the requested arpeggio step
 * @param note The array of notes to be filled
 * @param velocity The associated array of velocites to be filled
 * @param length The note length for this arpeggio step
 */
    void getNote(int *tick, int note[], int velocity[], int *length);
/**
 * @brief This function returns the number of notes present at the MIDI
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
 * @brief This function returns the number of notes present at the MIDI
 * input port.
 *
 * This is the number of notes currently pressed on the keyboard. Note
 * that the input MidiArp::notes buffer size can be different from this
 * number, since it can contain notes in release state or in the
 * MidiArp::latchBuffer.
 *
 * @return Number of notes present at the MIDI input port.
 */
    void removeNote(int *noteptr, int tick, int keep_rel);
/**
 * @brief This function calculates the index of the next arpeggio
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
 * @brief This function removes a note inside the MidiArp::notes input
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
 * @brief This function sets the released flag (value 1) for the note
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
 * @brief This function performs a copy within MidiArp::notes from the
 * currently active index to the inactive index
 */
    void copyNoteBuffer();

  public:
    int chIn;       /*!< Input channel state set by ArpWidget */
    int indexIn[2]; /*!< Note range filter 0: lower, 1: upper limit, set by ArpWidget */
    int rangeIn[2]; /*!< Velocity range filter 0: lower, 1: upper limit, set by ArpWidget */
    int portOut;    /*!< Output port, set by ArpWidget */
    int channelOut; /*!< Output channel, set by ArpWidget */
    bool isMuted;   /*!< Mute state set by ArpWidget */
    bool restartByKbd; /*!< If True, restart pattern at 0 upon new received note, set by ArpWidget */
    bool trigByKbd; /*!< If True, trigger current note tick by tick of received note, set by ArpWidget */
    bool gotKbdTrig;
    int repeatPatternThroughChord; /*!< Repeat mode "Static", "Up", "Down", set by ArpWidget */
    double attack_time;/*!< Attack time in seconds, set by ArpWidget */
    double release_time;/*!< Release time in seconds, set by ArpWidget */
    int randomTickAmp; /*!< Amplitude of timing randomization, set by ArpWidget */
    int randomVelocityAmp; /*!< Amplitude of velocity randomization, set by ArpWidget */
    int randomLengthAmp; /*!< Amplitude of length randomization, set by ArpWidget */
    QString pattern; /*!< Holds the the arpeggio pattern text */

    QVector<int> returnNote; /*!< Holds the notes of the currently active arpeggio step */
    QVector<int> returnVelocity; /*!< Holds the velocities of the currently active arpeggio step */
    int returnTick; /*!< Holds the time in internal ticks of the currently active arpeggio step */
    int returnLength; /*!< Holds the note length of the currently active arpeggio step */
    int returnIsNew;
    int nextTick;

  public:
    MidiArp();
    ~MidiArp();
    void updatePattern(const QString&);
    void updateTriggerMode(int val);
    void updateRandomTickAmp(int);
    void updateRandomVelocityAmp(int);
    void updateRandomLengthAmp(int);
    void updateAttackTime(int);
    void updateReleaseTime(int);
/*! @brief This function sets MidiArp::isMuted, which is checked by
 * Engine and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to the Driver
 */
    void setMuted(bool);
/**
 * @brief This function does the actions related to a note on or off
 * event detected on the MIDI input port.
 *
 * It is called by Engine when a NOTE ON or OFF event is received.
 * In case of NOTE ON, depending on the trigger settings, the Arp's timing
 * is reset to that of the note tick and/or the pattern index is reset.
 * The note with its attributes will then be inserted in the MidiArp::notes
 * buffer, which is sorted in accending note value order, and
 * MidiArp::copyNoteBuffer is called. The MidiArp::latchBuffer is purged
 * if the note was played stakato. In case of NOTE OFF, the function
 * will go through checks regarding MidiArp::latchMode and MidiArp::sustain
 * and add the note from the respective MidiArp::latchBuffer and/or
 * MidiArp::sustainBuffer if required. If not, the note is either tagged
 * as released (provided MidiArp::release_time is set) or removed from
 * the buffer. The latter depends on the keep_rel argument.
 *
 * @param note The note value of the received note
 * @param velocity The note velocity
 * @param tick The time the note was released in internal ticks
 * @param keep_rel Set this flag to 1 if the note is released and to be
 * kept in the buffer along with the release tick and tagged as a
 * released note. 0 otherwise for definite removal from the buffer.
 */
    bool handleEvent(MidiEvent inEv, int tick, int keep_rel = 0);
/**
 * @brief This function represents the external interface to the
 * core of the arpeggiator engine.
 *
 * It is called by Engine when a previously scheduled echo
 * event is received from the driver.
 * It starts the MidiArp::run thread. In the thread, MidiArp::getNote
 * is called, which does the note processing depending on the
 * MidiArp::pattern and
 * leading to new data in MidiArp::returnNote, MidiArp::returnVelocity,
 * MidiArp::returnLength. Engine then accesses this data directly
 * and outputs it to the Driver's queue.
 *
 * @param askedTick the timing of the note(s) to be output
 *
 */
    void prepareCurrentNote(int askedTick);
/**
 * @brief This function resets the pattern index and sets the current
 * timing of the arpeggio to currentTick.
 *
 * It is called by Engine when the transport is started, or when
 * a stakato note is received while MidiArp::restartByKbd is set.
 *
 * @param currentTick The timing in internal ticks, relative to which
 * the following arpeggio notes are calculated.
 */
    void initArpTick(int tick);
/**
 * @brief This function ensures continuity of the release function
 * when the currentTick position jumps into
 * the past.
 *
 * It should be called whenever the transport position is looping. At
 * this time, this is the case when JACK Transport is looping.

 * @param tick The current time position in internal ticks.
 */
    void foldReleaseTicks(int tick);
/**
 * @brief This function seeds new random values for the three parameters
 * concerned, timing (tick), velocity and length.
 *
 * This function is
 * called by MidiArp::getNote at every new note. It uses the
 * MidiArp::randomTickAmp, MidiArp::randomVelocityAmp and
 * MidiArp::randomLengthAmp settings coming from ArpWidget.
 * The values are then used by MidiArp::getNote.
 */
    void newRandomValues();
/**
 * @brief This function copies the new values transferred from the
 * GrooveWidget into variables used by MidiArp::getNote.
 *
 * @param p_grooveTick Groove amount for timing displacements
 * @param p_grooveVelocity Groove amount for velocity variations
 * @param p_grooveLength Groove amount for note length variations
 */
    void newGrooveValues(int p_grooveTick, int p_grooveVelocity,
            int p_grooveLength);
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
  * This function is called by ArpWidget::setLatchMode
  */
    void setLatchMode(bool);
 /*! @brief Calls MidiArp::removeNote for all notes in MidiArp::sustainBuffer
  * and then clears sustainBuffer.
  *
  * This function is called by MidiArp::setSustain
  * @param sustick Time in internal ticks at which the controller was received */
    void purgeSustainBuffer(int sustick);
 /*! @brief sets MidiArp::noteCount to zero and clears MidiArp::latchBuffer. */
    void clearNoteBuffer();

  signals:
/**
 * @brief Emitted to ArpScreen::update at every arpeggio step.
 *
 * It causes update of the cursor position and pattern display.
 * @param patternIndex The current index position in the MidiArp::pattern.
 */
    void nextStep(int patternIndex);

  public slots:
 /*! @brief Slot for MidiArp::latchTimer. Calls MidiArp::removeNote for
  * all notes in MidiArp::latchBuffer and then clears latchBuffer.
  */
    void purgeLatchBuffer();

};

#endif
