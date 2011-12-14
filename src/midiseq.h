/*!
 * @file midiseq.h
 * @brief Member definitions for the MidiSeq MIDI worker class.
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

#ifndef MIDISEQ_H
#define MIDISEQ_H

#include <QObject>
#include <QString>
#include <QVector>
#include <main.h>

#ifndef SAMPLE_H
#define SAMPLE_H

/*! @brief Structure holding elements of a MIDI note or controller representing
 * one point of a waveform
 */
    struct Sample {
        int value;
        int tick;
        bool muted;
    };
#endif

/*! @brief MIDI worker class for the Seq Module. Implements a monophonic
 * step sequencer as a QObject.
 *
 * The parameters of MidiSeq are controlled by the SeqWidget class.
 * A pointer to MidiSeq is passed to the SeqDriver thread, which calls
 * the MidiSeq::getNextNote member as a function of the position of
 * the Driver's queue. MidiSeq will return a note from its internal
 * MidiSeq::data buffer. The MidiSeq::data buffer is populated by the
 * MidiSeq::getData function at each modification done via
 * the SeqWidget. It is modified by drawing a sequence of notes on the
 * SeqWidget display or by recording incoming notes step by step. In all
 * cases the sequence has resolution, velocity, note length and
 * size attributes and single points can be tagged as muted, which will
 * avoid data output at the corresponding position.
 */
class MidiSeq : public QObject  {

  Q_OBJECT

  private:
    double queueTempo;
    int lastMouseLoc;
    int currentIndex;
    bool backward;       /*!< True when the sequence should start backward */
    bool reverse;       /*!< True when the current play direction is backwards */
    bool pingpong;      /*!< True when the play direction should alternate */
    bool reflect;      /*!< True when the current play direction will change at the next reflect point */
    int curLoopMode;    /*!< Local storage of the currently active Loop mode */
    int noteCount;
    bool recordMode;
    bool seqFinished;
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
 * @brief This function calulates the next MidiSeq::currentIndex as a
 * function of the current value, play direction, loop marker position
 * and orientation. It is called by MidiSeq::getNextNote() so that upon
 * every new output note, the sequence pointer advances.
 */
    void advancePatternIndex();

  public:
    int chIn;           /**< Channel of input events */
    bool enableNoteIn;
    bool enableNoteOff;
    bool enableVelIn;
    bool restartByKbd;
    bool trigByKbd;
    bool enableLoop;
    bool gotKbdTrig;
    bool restartFlag; /*!< Signals frameptr reset on next getNextFrame() call */
    int portOut;        /**< Output port index */
    int channelOut;
    bool isMuted;
    int vel, transp, notelength;
    int size, res, waveFormIndex;
    int currentRecStep;
    int loopMarker;
    int nextTick;
    int nOctaves;
    int baseOctave;
    int newGrooveTick, grooveTick, grooveVelocity, grooveLength, grooveIndex;
    QVector<Sample> customWave;
    QVector<bool> muteMask;

  public:
    MidiSeq();
    ~MidiSeq();
    void updateWaveForm(int val);
    void updateVelocity(int);
    void updateLoop(int);
    void updateTranspose(int);
    void updateQueueTempo(int);

    void recordNote(int note);
/*! @brief This function sets MidiLfo::isMuted, which is checked by
 * SeqDriver and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to the Driver
 */
    void setMuted(bool);
/**
 * @brief This function does the actions related to a newly received note.
 *
 * It is called by Engine when a new note is received on the MIDI input port.

 * @param note The note value of the received note
 * @param velocity The note velocity
 * @param tick The time the note was received in internal ticks
 */
    bool handleEvent(MidiEvent inEv, int tick);
/*! @brief This function sets the (controller) value of one point of the
 * MidiSeq::customWave array. It is used for handling drawing functionality.
 *
 * The member is called by SeqWidget::mouseMoved or SeqWidget::mousePressed.
 * The normalized mouse coordinates are scaled to the waveform size and
 * resolution and to the controller range (0 ... 127). The function
 * interpolates potentially missing waveform points between two events
 * if the mouse buttons were not released.
 *
 * @param mouseX Normalized horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0)
 * @param mouseY Normalized verical location of the mouse on the
 * SeqScreen (0.0 ... 1.0)
 * @see MidiSeq::toggleMutePoint(), MidiSeq::setMutePoint()
 */
    void setCustomWavePoint(double mouseX, double mouseY);
/*! @brief This function sets the MidiSeq::loopMarker member variable
 * used as a supplemental return point within the sequence. It is called
 *  by SeqWidget::mousePressed().
 * The normalized mouse coordinates are scaled to the waveform size and
 * resolution.
 *
 * @param mouseX Normalized horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0). If the mouseX parameter is negative, the
 * marker acts to the left, whereas it acts to the right when mouseX is
 * positive. If mouseX is zero, the loopMarker will be removed.
 * @see MidiSeq::toggleMutePoint(), MidiSeq::setMutePoint()
 */
    void setLoopMarker(double mouseX);
/*! @brief This function sets the mute state of one point of the
 * MidiSeq::muteMask array to the given state.
 *
 * The member is called when the right mouse button is clicked on the
 * SeqScreen.
 * If calculated waveforms are active, only the MidiSeq::muteMask is
 * changed. If a custom waveform is active, the Sample.mute status
 * at the given position is changed as well.
 *
 * @param mouseX Normalized horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0)
 * @param muted mute state to set for the given position
 *
 * @see MidiSeq::toggleMutePoint()
 */
    void setMutePoint(double mouseX, bool muted);
/*! @brief This function recalculates the MidiSeq::customWave as a
 * function of the current MidiSeq::res and MidiSeq::size values.
 *
 * It is called upon every change of MidiSeq::size and MidiSeq::res. It
 * repeats the current MidiSeq::customWave periodically if the new values
 * lead to a bigger size data array.
 */
    void resizeAll();
/*! @brief Currently not in use. This function will copy the current
 * MidiSeq::data array into MidiSeq::customWave.
 */
    void copyToCustom();
    void setRecordMode(int on);
    void setRecordedNote(int note);

/*! @brief This function is called upon every change of parameters in
 * SeqWidget or upon input by mouse clicks on the SeqScreen.
 *
 * It fills the
 * MidiSeq::data buffer with Sample points, which it copies from the
 * MidiSeq::customWave data.
 *
 * @param data reference to an array the waveform is copied to
 */
    void getData(QVector<Sample> *data);
/*! @brief This function transfers one Sample of data taken from
 * the currently active waveform MidiLfo::data at the index frameptr.
 *
 * @param p_sample reference to a Sample structure receiving the data point
 */
    void getNextNote(Sample *p_sample, int tick);
/*! @brief This function toggles the mute state of one point of the
 * MidiSeq::muteMask array.
 *
 * The member is called when the right mouse button is clicked on the
 * SeqScreen. The Sample.mute status at the given position in
 * MidiSeq::customWave is changed as well.
 *
 * @param mouseX Normalized Horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0)
 * @see MidiSeq::setMutePoint
 */
    bool toggleMutePoint(double);
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

    void setCurrentIndex(int ix);
    int getCurrentIndex() {return currentIndex; }

  signals:
    void nextStep(int currentIndex);
    void noteEvent(int note, int velocity);

};

#endif
