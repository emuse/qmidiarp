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
 * the ALSA queue. MidiSeq will return a note from its internal
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

  public:
    int chIn;           /**< Channel of input events */
    bool enableNoteIn;
    bool enableNoteOff;
    bool enableVelIn;
    bool restartByKbd;
    bool trigByKbd;
    bool enableLoop;
    int portOut;        /**< Output port index */
    int channelOut;
    bool isMuted;
    int vel, transp, notelength;
    int size, res, waveFormIndex;
    int currentRecStep;
    int nextTick;
    int nOctaves;
    int baseOctave;
    QVector<Sample> customWave;
    QVector<bool> muteMask;

  public:
    MidiSeq();
    ~MidiSeq();
    void updateWaveForm(int val);
    void updateVelocity(int);
    void updateTranspose(int);
    void updateQueueTempo(int);

    void recordNote(int note);
/*! @brief This function sets MidiLfo::isMuted, which is checked by
 * SeqDriver and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to ALSA
 */
    void setMuted(bool);
/**
 * @brief This function checks whether this module is set to keyboard
 * trigger mode.
 *
 * Its response depends on MidiSeq::restartByKbd and (TODO) whether there are notes
 * pressed on the keyboard, i.e. whether the note was played stakato.
 *
 * @return True if the module accepts to be triggered
 */
    bool wantTrigByKbd();
/**
 * @brief This function does the actions related to a newly received note.
 *
 * It is called by SeqDriver when a new note is received on the ALSA input port.

 * @param note The note value of the received note
 * @param velocity The note velocity
 * @param tick The time the note was received in internal ticks
 */
    void handleNote(int note, int velocity, int tick);
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

/**
 * @brief This function checks whether an ALSA event is eligible for this
 * module.
 *
 * Its response depends on the input filter settings, i.e. note,
 * velocity and channel.
 *
 * @param inEv MidiEvent event to check
 * @return True if inEv is in the input range of the module
 */
    bool wantEvent(MidiEvent inEv);
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
    void getNextNote(Sample *p_sample);
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
    void setCurrentIndex(int ix);
    int getCurrentIndex() {return currentIndex; }

  signals:
    void nextStep(int currentIndex);
    void noteEvent(int note, int velocity);

};

#endif
