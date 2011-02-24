/*!
 * @file midilfo.h
 * @brief MIDI worker class for the LFO Module. Implements a sequencer
 * for controller data as a QObject.
 *
 * The parameters of MidiLfo are controlled by the LfoWidget class.
 * A pointer to MidiLfo is passed to the SeqDriver thread, which calls
 * the MidiLfo::getNextFrame member as a function of the position of
 * the ALSA queue. MidiLfo will return an array of controller values
 * representing a frame of its internal MidiLfo::data buffer. This frame
 * has size 1 except for resolution higher than 16th notes.
 * The MidiLfo::data buffer is populated by the MidiLfo::getData function
 * at each modification done via the LfoWidget. It can consist of
 * a classic waveform calculation or a hand-drawn waveform. In all cases
 * the waveform has resolution, offset and size attributes and single
 * points can be tagged as muted, which will avoid data output at the
 * corresponding position.
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

#ifndef MIDILFO_H
#define MIDILFO_H

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

/*! @brief MIDI worker class for the LFO Module. Implements a sequencer
 * for controller data as a QObject.
 *
 * The parameters of MidiLfo are controlled by the LfoWidget class.
 * A pointer to MidiLfo is passed to the SeqDriver thread, which calls
 * the MidiLfo::getNextFrame member as a function of the position of
 * the ALSA queue. MidiLfo will return an array of controller values
 * representing a frame of its internal MidiLfo::data buffer. This frame
 * has size 1 except for resolution higher than 16th notes.
 * The MidiLfo::data buffer is populated by the MidiLfo::getData function
 * at each modification done via the LfoWidget. It can consist of
 * a classic waveform calculation or a hand-drawn waveform. In all cases
 * the waveform has resolution, offset and size attributes and single
 * points can be tagged as muted, which will avoid data output at the
 * corresponding position.
 */
class MidiLfo : public QObject  {

  Q_OBJECT

  private:
    double queueTempo;  /*!< current tempo of the ALSA queue, not in use here */
    int lastMouseLoc;   /*!< The X location of the last modification of the wave, used for interpolation*/
    int lastMouseY;     /*!< The Y location at the last modification of the wave, used for interpolation*/
    int frameptr;       /*!< position of the currently output frame in the MidiArp::data waveform */
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
    QVector<Sample> data;

  public:
    int portOut;    /*!< ALSA output port number */
    int channelOut; /*!< ALSA output channel */
    int ccnumber;   /*!< MIDI Controller CC number to output */
    bool isMuted;   /*!< Global mute state */
    int freq, amp, offs;
    int size;       /*!< Size of the waveform in quarter notes */
    int res;        /*!< Resolution of the waveform in ticks per quarter note */
    int waveFormIndex;          /*!< Index of the waveform to produce
                                    @par 0: Sine
                                    @par 1: Sawtooth Up
                                    @par 2: Triangle
                                    @par 3: Sawtooth Down
                                    @par 4: Square
                                    @par 5: Use Custom Wave */
    int cwmin;                  /*!< The minimum of MidiArp::customWave */
    QVector<Sample> customWave; /*!< Vector of Sample points holding the custom drawn wave */
    QVector<bool> muteMask;     /*!< Vector of booleans with mute state information for each wave point */

  public:
    MidiLfo();
    ~MidiLfo();
    void updateWaveForm(int val);
    void updateFrequency(int);
    void updateAmplitude(int);
    void updateOffset(int);
    void updateQueueTempo(int);
/*! @brief This function sets MidiLfo::isMuted, which is checked by
 * SeqDriver and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to ALSA
 */
    void setMuted(bool on);
    void setCustomWavePoint(double, double, bool);
/*! @brief This function sets the mute state of one point of the
 * MidiLfo::muteMask array to the given state.
 *
 * The member is called when the right mouse button is clicked on the
 * LfoScreen.
 * If calculated waveforms are active, only the MidiLfo::muteMask is
 * changed. If a custom waveform is active, the Sample.mute status
 * at the given position is changed as well.
 *
 * @param mouseX Normalized Horizontal location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @param muted mute state to set for the given position
 *
 * @see MidiLfo::toggleMutePoint()
 */
    void setMutePoint(double mouseX, bool muted);
    void resizeAll();
    void copyToCustom();
    void resetFramePtr();
    void updateCustomWaveOffset(int);
/*! @brief This function is the main calculator for the data contained
 * in a waveform.
 *
 * It is called upon every change of parameters in LfoWidget or upon
 * input by mouse clicks on the LfoScreen. It fills the
 * MidiLfo::data buffer with Sample points, which it either calculates
 * or which it copies from the MidiLfo::customWave data.
 *
 * @param data reference to an array the waveform is copied to
 */
    void getData(QVector<Sample> *data);
/*! @brief This function transfers a frame of Sample data points taken from
 * the currently active waveform MidiLfo::data.
 *
 * @param data reference to an array the frame is copied to
 */
    void getNextFrame(QVector<Sample> *p_data);
/*! @brief This function toggles the mute state of one point of the
 * MidiLfo::muteMask array.
 *
 * The member is called when the right mouse button is clicked on the
 * LfoScreen.
 * If calculated waveforms are active, only the MidiLfo::muteMask is
 * changed. If a custom waveform is active, the Sample.mute status
 * at the given position is changed as well.
 *
 * @param mouseX Normalized Horizontal location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @see MidiLfo::setMutePoint
 */
    bool toggleMutePoint(double mouseX);
};

#endif
