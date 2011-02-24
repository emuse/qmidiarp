/*!
 * @file midiseq.h
 * @brief MIDI worker class for the Seq Module. Implements a monophonic
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
#include <alsa/asoundlib.h>
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
    int chIn;       // Channel of input events
    bool enableNoteIn; // Index input/output (for Controller events)
    bool enableVelIn; // Parameter that is mapped, [0] low, [1] high boundary
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool isMuted;
    int vel, transp, notelength;
    int size, res, waveFormIndex;
    int currentRecStep;
    QVector<Sample> customWave;
    QVector<bool> muteMask;

  public:
    MidiSeq();
    ~MidiSeq();

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
    void updateWaveForm(int val);
    void setCustomWavePoint(double, double);
/*! @brief This function sets the mute state of one point of the
 * MidiSeq::muteMask array to the given state.
 *
 * The member is called when the right mouse button is clicked on the
 * SeqScreen.
 * If calculated waveforms are active, only the MidiSeq::muteMask is
 * changed. If a custom waveform is active, the Sample.mute status
 * at the given position is changed as well.
 *
 * @param mouseX Normalized Horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0)
 * @param muted mute state to set for the given position
 *
 * @see MidiSeq::toggleMutePoint()
 */
    void setMutePoint(double, bool);
    void resizeAll();
    void copyToCustom();
    void setRecordedNote(int note);

/**
 * @brief This function checks whether an ALSA event is eligible for this
 * module.
 *
 * Its response depends on the input filter settings, i.e. note,
 * velocity and channel.
 *
 * @param evIn ALSA event to check
 * @return True if evIn is in the input range of the module
 */
    bool isSeq(snd_seq_event_t *evIn);
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

  signals:
    void nextStep(int currentIndex);

};

#endif
