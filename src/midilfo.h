/*!
 * @file midilfo.h
 * @brief Member definitions for the MidiLfo MIDI worker class.
 *
 *
 *      Copyright 2009 - 2019 <qmidiarp-devel@lists.sourceforge.net>
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

#include <vector>
#include "midiworker.h"


/*! @brief MIDI worker class for the LFO Module. Implements a sequencer
 * for controller data as a QObject.
 *
 * The parameters of MidiLfo are controlled by the LfoWidget class.
 * The backend driver thread calls the Engine::echoCallback(), which will
 * query each module, in this case via
 * the MidiLfo::getNextFrame() method. MidiLfo will fill a frame from
 * its internal MidiLfo::data buffer as a function of the position of
 * the driver's transport. MidiLfo::frame is then accessed by Engine. It
 * has size 1 except for resolution higher than 16th notes.
 * The MidiLfo::data buffer is populated by the getData() function
 * at each modification done via the LfoWidget. It can consist of
 * a classic waveform calculation or a hand-drawn waveform. In all cases
 * the waveform has resolution, offset and size attributes and single
 * points can be tagged as muted, which will avoid data output at the
 * corresponding position.
 */
class MidiLfo : public MidiWorker  {

  private:
    int lastMouseLoc;   /*!< The X location of the last modification of the wave, used for interpolation*/
    int lastMouseY;     /*!< The Y location at the last modification of the wave, used for interpolation*/
    int recValue;
    int lastSampleValue;
/*! @brief  recalculates the MidiLfo::customWave as a function
 * of a new offset value.
 *
 * It is called by MidiLfo::updateOffset() in case a custom wave is active.
 * @param cwoffs New offset value
 */
    void updateCustomWaveOffset(int cwoffs);

  public:
    bool recordMode, isRecording;
    bool lastMute;                  /**< Contains the mute state of the last waveForm point modified by mouse click*/
    int old_res;
    int freq, amp, offs;
    int phase;                      /*!< Starting Phase position of the waveform based on resolution */
    int size;                       /*!< Size of the waveform in quarter notes */
    int res;                        /*!< Resolution of the waveform in ticks per quarter note */
    int frameSize;                  /*!< Current size of a vector returned by MidiLfo::getNextFrame() */
    int maxNPoints;                 /*!< Maximum number of steps that have been used in the session */
    int waveFormIndex;              /*!< Index of the waveform to produce
                                        @par 0: Sine
                                        @par 1: Sawtooth Up
                                        @par 2: Triangle
                                        @par 3: Sawtooth Down
                                        @par 4: Square
                                        @par 5: Use Custom Wave */
    int cwmin;                      /*!< The minimum of MidiLfo::customWave */
    std::vector<Sample> customWave; /*!< Vector of Sample points holding the custom drawn wave */
    std::vector<bool> muteMask;     /*!< Vector of booleans with mute state information for each wave point */
    std::vector<Sample> frame;      /*!< Vector of Sample points holding the current frame for transfer */
    std::vector<Sample> data;

  public:
    MidiLfo();
    virtual ~MidiLfo() {}
    void updateWaveForm(int val);
    void updateFrequency(int);
    void updateAmplitude(int);
    void updateOffset(int);
    void updatePhase(int);
    void updateResolution(int);
    void updateSize(int);
    void updateLoop(int);
    void record(int value);
    void setRecordMode(bool on);
/*! @brief  Called by LfoWidget::mouseEvent()
 */
    int mouseEvent(double mouseX, double mouseY, int buttons, int pressed);
/*!
* @brief  determines the minimum of the current waveform and
* sets the LfoWidget::offset slider accordingly.
*
* It also sets MidiLfo::cwmin. When a new waveform is drawn, its minimum
* offset from 0 changes and the offset controller has to be adapted in range.
*
*/
    void newCustomOffset();

/*! @brief  sets the (controller) value of one point of the
 * MidiLfo::customWave array. It is used for handling drawing functionality.
 *
 * It is called by the mouseEvent() function.
 * The normalized mouse coordinates are scaled to the waveform size and
 * resolution and to the controller range (0 ... 127). The function
 * interpolates potentially missing waveform points between two events
 * if the mouse buttons were not released.
 *
 * @returns index in the wave vector that has been set
 * @param mouseX Normalized horizontal location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @param mouseY Normalized verical location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @param newpt Set to true if the mouse button was newly clicked before
 * the move
 *
 * @see MidiLfo::toggleMutePoint(), MidiLfo::setMutePoint()
 */
    int setCustomWavePoint(double mouseX, double mouseY, bool newpt);
/*! @brief  sets the mute state of one point of the
 * MidiLfo::muteMask array to the given state.
 *
 * The method is called when the right mouse button is clicked on the
 * LfoScreen via the mouseEvent() function.
 * If calculated waveforms are active, only the MidiLfo::muteMask is
 * changed. If a custom waveform is active, the Sample.mute status
 * at the given position is changed as well.
 *
 * @returns index in the wave vector that has been set
 * @param mouseX Normalized Horizontal location of the mouse on the
 * LfoScreen (0.0 ... 1.0)
 * @param muted mute state to set for the given position
 *
 * @see MidiLfo::toggleMutePoint()
 */
    int setMutePoint(double mouseX, bool muted);
/*! @brief  recalculates the MidiLfo::customWave as a
 * function of the current MidiLfo::res and MidiLfo::size values.
 *
 * It is called upon every change of MidiLfo::size and MidiLfo::res. It
 * repeats the current MidiLfo::customWave periodically if the new values
 * lead to a bigger size data array.
 */
    void resizeAll();
/*! @brief  copies the current MidiLfo::data array into
 * MidiLfo::customWave.
 *
 * It is called when a waveform modification by the user is attempted
 * while in calculated waveform mode. (MidiLfo::waveFormIndex 1 ... 4).
 */
    void copyToCustom();
/*! @brief  flips the MidiLfo::customWave array about its middle value
 *
 * It is called by LfoWidget when the vertical flip button is pressed.
 */
    void flipWaveVertical();
/*! @brief  sets the MidiLfo::framePtr to the given value.
 *
 * It is called when the Transport starts.
 * @param idx Index to which the framePtr is set
 */
    void setFramePtr(int idx);
/**
 * @brief  does the actions related to a newly received event.
 *
 * It is called by Engine when a new event is received on the MIDI input port.

 * @param inEv MidiEvent to check and process or not
 * @param tick The time the event was received in internal ticks
 * @return True if inEv is in not the input range of the module (event is unmatched)
 */
    bool handleEvent(MidiEvent inEv, int tick);
/*! @brief  is the main calculator for the data contained
 * in a waveform.
 *
 * It is called upon every change of parameters in LfoWidget or upon
 * input by mouse clicks on the LfoScreen. It fills the
 * MidiLfo::data buffer with Sample points, which it either calculates
 * or which it copies from the MidiLfo::customWave data.
 *
 * @param *data reference to an array the waveform is copied to
 */
    void getData(std::vector<Sample> *data);
/*! @brief fills the MidiLfo::frame with Sample data points taken from
 * the currently active waveform MidiLfo::data.
 *
 * MidiLfo::frame is then accessed by Engine::echoCallback() and sequenced
 * to the driver backend.
 *
 * @param tick current tick
 */
    void getNextFrame(int tick);
/*! @brief  toggles the mute state of one point of the
 * MidiLfo::muteMask array.
 *
 * The function is called when the right mouse button is clicked on the
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

/*! @brief Checks if deferred parameter changes are pending and applies
 * them if so
 */
    void applyPendingParChanges();
/**
 * @brief sets MidiLfo::nextTick and MidiLfo::framePtr position
 * according to the specified tick.
 *
 * @param tick The current tick to which the module position should be
 * aligned.
 */
    void setNextTick(int tick);
};

#endif
