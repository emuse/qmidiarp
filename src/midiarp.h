/*!
 * @file midiarp.h
 * @brief MIDI worker class for the Arpeggiator Module. Implements the
 * functions providing note arpeggiation as a QThread.
 *
 * The parameters of MidiArp are controlled by the ArpWidget class.
 * A pointer to MidiArp is passed to the SeqDriver thread, which calls
 * the MidiArp::prepareCurrentNote member as a function of the position of
 * the ALSA queue. MidiArp will then call its
 * internal MidiArp::getNote function which produces an array of notes
 * stored in its internal output buffer. The notes in the array depend
 * on the active MidiArp::pattern, envelope, random and groove settings.
 * The note events consist of timing information
 * (tick and length), note values and velocity values. MidiArp::getNote
 * also advances the pattern index and emits the MidiArp::nextStep signal
 * to update the cursor position in the graphical ArpScreen display part
 * of ArpWidget. SeqDriver then
 * accesses this output buffer and sends it to the ALSA queue. SeqDriver
 * also calls MidiArp::handleNoteOn and MidiArp::handleNoteOff. These members
 * manage the arpeggiator input note buffer
 * also part of this class. Notes received on the ALSA input port will
 * therefore be added or removed from the buffer as SeqDriver transfers
 * them to this class.
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
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <alsa/asoundlib.h>
#include <main.h>


 /*!
 * @brief MIDI worker class for the Arpeggiator Module. Implements the
 * functions providing note arpeggiation as a QThread.
 *
 * The parameters of MidiArp are controlled by the ArpWidget class.
 * A pointer to MidiArp is passed to the SeqDriver thread, which calls
 * the MidiArp::prepareCurrentNote member as a function of the position of
 * the ALSA queue. MidiArp will then call its
 * internal MidiArp::getNote function which produces an array of notes
 * stored in its internal output buffer. The notes in the array depend
 * on the active MidiArp::pattern, envelope, random and groove settings.
 * The note events consist of timing information
 * (tick and length), note values and velocity values. MidiArp::getNote
 * also advances the pattern index and emits the MidiArp::nextStep signal
 * to update the cursor position in the graphical ArpScreen display part
 * of ArpWidget. SeqDriver then
 * accesses this output buffer and sends it to the ALSA queue. SeqDriver
 * also calls MidiArp::handleNoteOn and MidiArp::handleNoteOff. These members
 * manage the arpeggiator input note buffer
 * also part of this class. Notes received on the ALSA input port will
 * therefore be added or removed from the buffer as SeqDriver transfers
 * them to this class.
 */
class MidiArp : public QThread  {

  Q_OBJECT

  private:
    int nextNote[MAXCHORD], nextVelocity[MAXCHORD];
    int currentNoteTick, nextNoteTick, currentTick, arpTick;
    int currentNote[MAXCHORD], currentVelocity[MAXCHORD];
    int currentLength, nextLength;
    bool newCurrent, newNext, chordMode;
    int grooveTick, grooveVelocity, grooveLength, grooveIndex;
    int randomTick, randomVelocity, randomLength;
    double queueTempo;
    double stepWidth, len, vel;

    QVector<int> sustainBuffer;
    QVector<int> latchBuffer;
    QTimer *latchTimer;

    bool sustain, latch_mode;
    int octave, noteIndex[MAXCHORD], patternIndex;
 /*! @brief The input note buffer array of the Arpeggiator.
  *
  * It has two
  * array copies (first index 0:1) for avoiding access conflicts while
  * modifying its data.
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
    double old_attackfn[MAXNOTES];
    int noteBufPtr, noteCount, patternLen, patternMaxIndex, noteOfs;

    QMutex mutex;


/**
 * @brief This function resets all attributes the pattern
 * accumulates during run.
 *
 * It is called when the currentIndex revolves to restart the loop with
 * default velocity, step width, octave and length.
 *
 * @param evIn ALSA event to check
 * @return True if evIn is in the input range of the arp
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
 * It is called by MidiArp::run and calls MidiArp::getNote if the given
 * timing is ahead of the last timing information. It then transfers the
 * MidiArp::nextNote and MidiArp::nextVelocity arrays into
 * MidiArp::currentNote and MidiArp::currentVelocity.
 *
 * @param currentTick The current timing information in internal ticks
 */
    void updateNotes(int currentTick);
/**
 * @brief This is MidiArp's main note processor producing output notes
 * from input notes.
 *
 * It analyzes the MidiArp::pattern text and MidiArp::notes input buffer
 * to yield arrays of notes that have to be output at the given timing.
 */
    void getNote(int *tick, int note[], int velocity[], int *length);
    void prepareNextNote(int askedTick);
    int getPressedNoteCount();
    void removeNote(int *noteptr, int tick, int keep_rel); // Remove input Note from Arpeggio
    bool advancePatternIndex(bool reset);
    void deleteNoteAt(int index, int bufPtr);
    void tagAsReleased(int note, int tick, int bufPtr);
    void copyNoteBuffer();

  public:
    int chIn;       // Channel of input events
    int indexIn[2]; // Index input/output (for Controller events)
    int rangeIn[2]; // Parameter that is mapped, [0] low, [1] high boundary
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool isMuted;
    bool restartByKbd, trigByKbd;
    int repeatPatternThroughChord;
    double attack_time, release_time;
    int randomTickAmp, randomVelocityAmp, randomLengthAmp;
    QString pattern;
    QVector<int> returnNote, returnVelocity;
    int returnTick, returnIsNew, returnLength;

  public:
    MidiArp();
    ~MidiArp();
/**
 * @brief This function checks whether an ALSA event is eligible for this
 * arp module.
 *
 * Its response depends on the input filter settings, i.e. note range,
 * velocity range and channel.
 *
 * @param evIn ALSA event to check
 * @return True if evIn is in the input range of the arp
 */
    bool isArp(snd_seq_event_t *evIn);
/**
 * @brief This function checks whether this module is set to keyboard
 * trigger mode.
 *
 * Its response depends on MidiArp::trigByKbd and whether there are notes
 * pressed on the keyboard, i.e. whether the note was played stakato.
 *
 * @return True if the module accepts to be triggered
 */
    bool wantTrigByKbd();
/**
 * @brief This function does the actions related to a newly received note.
 *
 * It is called by SeqDriver when a new note is received on the ALSA input port.
 * The MidiArp::latchBuffer is purged if the note was played stakato.
 * Depending on the trigger settings, the Arp's timing is reset to
 * that of the note tick and/or the pattern index is reset. The note with
 * its attributes will then be inserted in the MidiArp::notes buffer, which
 * is sorted in accending note value order, and MidiArp::copyNoteBuffer is
 * called.
 *
 * @param note The note value of the received note
 * @param velocity The note velocity
 * @param tick The time the note was received in internal ticks
 */
    void handleNoteOn(int note, int velocity, int tick);
/**
 * @brief This function does the actions related to a note release detected
 * on the ALSA input port.
 *
 * It is called by SeqDriver when a NOTE_OFF event is received. The function
 * will go through checks regarding MidiArp::latchMode and MidiArp::sustain
 * and add the note to the respective MidiArp::latchBuffer and/or
 * MidiArp::sustainBuffer if required. If not, the note is either tagged
 * as released (provided MidiArp::release_time is set) or removed from
 * the buffer. The latter depends on the keep_rel argument.
 *
 * @param note The note value of the received note
 * @param tick The time the note was released in internal ticks
 * @param keep_rel Set this flag to 1 if the note is to be kept in the buffer
 * along with the release tick and tagged as a released note. 0 otherwise for
 * definite removal from the buffer.
 */
    void handleNoteOff(int note, int tick, int keep_rel);
/**
 * @brief This function represents the external interface to the
 * core of the arpeggiator engine.
 *
 * It is called by SeqDriver when a previously scheduled SND_SEQ_ECHO
 * event is received on the ALSA port.
 * It starts the MidiArp::run thread. In the thread, MidiArp::getNote
 * is called, which does the note processing depending on the
 * MidiArp::pattern and
 * leading to new data in MidiArp::returnNote, MidiArp::returnVelocity,
 * MidiArp::returnLength. SeqDriver then accesses this data directly
 * and outputs it to the ALSA queue.
 *
 * @param askedTick the timing of the note(s) to be output
 *
 */
    void prepareCurrentNote(int askedTick);
/**
 * @brief This function returns the timing of the next note to be
 * calculated and played out in internal ticks.
 *
 * It is called by SeqDriver immediately after accessing the current note
 * data. This next note timing information is used to
 * schedule a so called echo event which will trigger the next call to
 * prepareCurrentNote followed by an output of note data to the ALSA
 * queue.
 *
 * @return The timing of the next coming note in internal ticks.
 */
    int getNextNoteTick();
/**
 * @brief This function resets the pattern index and sets the current
 * timing of the arpeggio to currentTick.
 *
 * It is called by SeqDriver when the ALSA queue is started, or when
 * a stakato note is received while MidiArp::restartByKbd is set.
 *
 * @param currentTick The timing in internal ticks, relative to which
 * the following arpeggio notes are calculated.
 */
    void initArpTick(int currentTick);
/**
 * @brief This function ensures continuity of the release function
 * when the currentTick position jumps into
 * the past.
 *
 * It should be called whenever the transport position is looping. At
 * this time, this is the case when JACK Transport is looping.

 * @param currentTick The current time position in internal ticks.
 */
    void foldReleaseTicks(int currentTick);
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
/**
 * @brief This is the thread main function. It
 * calls MidiArp::updateNotes() and copies
 * the newly calculated MidiArp::currentNote and MidiArp::currentVelocity
 * parameter arrays into the return members, which can then be accessed
 * by the SeqDriver::run thread.
 */
    void run();

  signals:
/**
 * @brief Emitted to ArpScreen::update at every arpeggio step.
 *
 * It causes update of the cursor position and pattern display.
 * @param patternIndex The current index position in the MidiArp::pattern.
 */
    void nextStep(int patternIndex);

  public slots:
    void updatePattern(const QString&);
    void updateTriggerMode(int val);
    void updateRandomTickAmp(int);
    void updateRandomVelocityAmp(int);
    void updateRandomLengthAmp(int);
    void updateAttackTime(int);
    void updateQueueTempo(int);
    void updateReleaseTime(int);
    void setMuted(bool); //set mute
    void setSustain(bool, int); //set sustain
    void setLatchMode(bool); //set latch mode
    void purgeSustainBuffer(int sustick);
    void purgeLatchBuffer();
    void clearNoteBuffer();
};

#endif
