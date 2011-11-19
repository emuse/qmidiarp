/*!
 * @file midilfo.h
 * @brief Member definitions for the MidiLfo MIDI worker class.
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
 * A pointer to MidiLfo is passed to the Engine, which calls
 * the MidiLfo::getNextFrame member as a function of the position of
 * the Driver's queue. MidiLfo will return an array of controller values
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
    double queueTempo;  /*!< current tempo of the transport, not in use here */
    int lastMouseLoc;   /*!< The X location of the last modification of the wave, used for interpolation*/
    int lastMouseY;     /*!< The Y location at the last modification of the wave, used for interpolation*/
    int frameptr;       /*!< position of the currently output frame in the MidiArp::data waveform */
    bool reverse;       /*!< True when the play direction is backwards */
    bool pingpong;       /*!< True when the play direction alternates */
    int curLoopMode;    /*!< Local storage of the currently active Loop mode */
    int recValue;
    int lastSampleValue;
    bool seqFinished;   /*!< When True, all output events are muted, used when NOTE OFF is received */
    int noteCount;      /*!< The number of keys currently pressed on keyboard */
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
/*! @brief This function recalculates the MidiLfo::customWave as a function
 * of a new offset value.
 *
 * It is called by MidiLfo::updateOffset() in case a custom wave is active.
 * @param cwoffs New offset value
 */
    void updateCustomWaveOffset(int cwoffs);

  public:
    bool enableNoteOff;
    bool enableVelIn;
    bool restartByKbd;
    bool trigByKbd;
    bool enableLoop;
    bool gotKbdTrig;
    bool restartFlag; /*!< Signals frameptr reset on next getNextFrame() call */
    int portOut;    /*!< MIDI output port number */
    int channelOut; /*!< MIDI output channel */
    bool recordMode, isRecording;
    int old_res;
    int ccnumber;   /*!< MIDI Controller CC number to output */
    bool isMuted;   /*!< Global mute state */
    int freq, amp, offs, ccnumberIn, chIn;
    int size;       /*!< Size of the waveform in quarter notes */
    int res;        /*!< Resolution of the waveform in ticks per quarter note */
    int waveFormIndex;          /*!< Index of the waveform to produce
                                    @par 0: Sine
                                    @par 1: Sawtooth Up
                                    @par 2: Triangle
                                    @par 3: Sawtooth Down
                                    @par 4: Square
                                    @par 5: Use Custom Wave */
    int cwmin;                  /*!< The minimum of MidiLfo::customWave */
    int nextTick;
    int newGrooveTick, grooveTick, grooveVelocity, grooveLength, grooveIndex;
    QVector<Sample> customWave; /*!< Vector of Sample points holding the custom drawn wave */
    QVector<bool> muteMask;     /*!< Vector of booleans with mute state information for each wave point */

  public:
    MidiLfo();
    ~MidiLfo();
    void updateWaveForm(int val);
    void updateFrequency(int);
    void updateAmplitude(int);
    void updateOffset(int);
    void updateResolution(int);
    void updateSize(int);
    void updateLoop(int);
    void updateQueueTempo(int);
    void record(int value);
/*! @brief This function sets MidiLfo::isMuted, which is checked by
 * Engine and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to the Driver
 */
    void setMuted(bool on);
/*! @brief This function sets the (controller) value of one point of the
 * MidiLfo::customWave array. It is used for handling drawing functionality.
 *
 * The member is called by LfoWidget::mouseMoved or LfoWidget::mousePressed.
 * The normalized mouse coordinates are scaled to the waveform size and
 * resolution and to the controller range (0 ... 127). The function
 * interpolates potentially missing waveform points between two events
 * if the mouse buttons were not released.
 *
 * @param mouseX Normalized horizontal location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @param mouseY Normalized verical location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @param newpt Set to true if the mouse button was newly clicked before
 * the move
 *
 * @see MidiLfo::toggleMutePoint(), MidiLfo::setMutePoint()
 */
    void setCustomWavePoint(double mouseX, double mouseY, bool newpt);
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
/*! @brief This function recalculates the MidiLfo::customWave as a
 * function of the current MidiLfo::res and MidiLfo::size values.
 *
 * It is called upon every change of MidiLfo::size and MidiLfo::res. It
 * repeats the current MidiLfo::customWave periodically if the new values
 * lead to a bigger size data array.
 */
    void resizeAll();
/*! @brief This function copies the current MidiLfo::data array into
 * MidiLfo::customWave.
 *
 * It is called when a waveform modification by the user is attempted
 * while in calculated waveform mode. (MidiLfo::waveFormIndex 1 ... 4).
 */
    void copyToCustom();
/*! @brief This function sets the MidiLfo::frameptr to the given value.
 *
 * It is called when the Transport starts.
 * @param idx Index to which the frameptr is set
 */
    void setFramePtr(int idx);
/**
 * @brief This function does the actions related to a newly received event.
 *
 * It is called by Engine when a new event is received on the MIDI input port.

 * @param inEv MidiEvent to check and process or not
 * @param tick The time the event was received in internal ticks
 * @return True if inEv is in not the input range of the module (event is unmatched)
 */
    bool handleEvent(MidiEvent inEv, int tick);
/*! @brief This function is the main calculator for the data contained
 * in a waveform.
 *
 * It is called upon every change of parameters in LfoWidget or upon
 * input by mouse clicks on the LfoScreen. It fills the
 * MidiLfo::data buffer with Sample points, which it either calculates
 * or which it copies from the MidiLfo::customWave data.
 *
 * @param *data reference to an array the waveform is copied to
 */
    void getData(QVector<Sample> *data);
/*! @brief This function transfers a frame of Sample data points taken from
 * the currently active waveform MidiLfo::data.
 *
 * @param *p_data reference to an array the frame is copied to
 */
    void getNextFrame(QVector<Sample> *p_data, int tick);
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
    int getFramePtr() { return frameptr;}

  signals:
    void nextStep(int frameptr);
};

#endif
