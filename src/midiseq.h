/*!
 * @file midiseq.h
 * @brief Member definitions for the MidiSeq MIDI worker class.
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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

#include "midiworker.h"
#include <vector>

/*! @brief MIDI worker class for the Seq Module. Implements a monophonic
 * step sequencer.
 *
 * The parameters of MidiSeq are controlled by the SeqWidget class.
 * The backend driver thread calls the Engine::echoCallback(), which will
 * query each module, in this case via
 * the MidiSeq::getNextFrame() method. MidiSeq will return a note from
 * its internal MidiSeq::data buffer as a function of the position of
 * the driver's transport. The MidiSeq::data buffer is populated by the
 * MidiSeq::getData() function at each modification done via
 * the SeqWidget. It is modified by drawing a sequence of notes on the
 * SeqWidget display or by recording incoming notes step by step. In all
 * cases the sequence has resolution, velocity, note length and
 * size attributes and single points can be tagged as muted, which will
 * avoid data output at the corresponding position.
 */
class MidiSeq : public MidiWorker  {

  private:
    int lastMouseLoc;
    int lastMouseY;
/**
 * @brief  calulates the next MidiSeq::framePtr as a
 * function of the current value, play direction, loop marker position
 * and orientation. It is called by MidiSeq::getNextFrame() so that upon
 * every new output note, the sequence pointer advances.
 */
    void advancePatternIndex();

  public:
    bool lastMute;              /**< Contains the mute state of the last waveForm point modified by mouse click*/
    bool recordMode;
    int vel, transp, notelength;
    int velDefer, transpDefer, notelengthDefer;
    int size, res;
    int currentRecStep;
    int loopMarker;
    int maxNPoints;        /*!< Maximum number of steps that have been used in the session */
    int nOctaves;
    int baseOctave;
    std::vector<Sample> customWave;
    std::vector<bool> muteMask;
    std::vector<Sample> data;

  public:
    MidiSeq();
    virtual ~MidiSeq() {}
    void updateWaveForm(int val);
    void updateNoteLength(int);
    void updateVelocity(int);
    void updateResolution(int);
    void updateSize(int);
    void updateLoop(int);
    void updateTranspose(int);
    void updateDispVert(int mode);

    void recordNote(int note);

    bool handleEvent(MidiEvent inEv, int64_t tick, int keep_rel = 0) override;
/*! @brief  sets the (controller) value of one point of the
 * MidiSeq::customWave array. It is used for handling drawing functionality.
 *
 * It is called by mouseEvent() function.
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
    int setCustomWavePoint(double mouseX, double mouseY);
/*! @brief  sets the MidiSeq::loopMarker member variable
 * used as a supplemental return point within the sequence.
 *
 * @param ix Absolute pattern index position of the marker. NOTE:
 * If ix is negative, the
 * marker acts to the left, whereas it acts to the right when mouseX is
 * positive. If ix is zero, the loopMarker will be removed.
 * @see MidiSeq::toggleMutePoint(), MidiSeq::setMutePoint(),
 * MidiSeq::setLoopMarkerMouse()
 */
    void setLoopMarker(int ix);
/*! @brief  sets the MidiSeq::loopMarker member variable
 * used as a supplemental return point within the sequence. It is called
 *  by the mouseEvent() function.
 * The normalized mouse coordinates are scaled to the waveform size and
 * resolution and then MidiSeq::setLoopMarker() is called
 *
 * @param mouseX Normalized horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0). If the mouseX parameter is negative, the
 * marker acts to the left, whereas it acts to the right when mouseX is
 * positive. If mouseX is zero, the loopMarker will be removed.
 * @see MidiSeq::toggleMutePoint(), MidiSeq::setMutePoint(),
 * MidiSeq::setLoopMarker()
 */
    void setLoopMarkerMouse(double mouseX);
/*! @brief  sets the mute state of one point of the
 * MidiSeq::muteMask array to the given state.
 *
 * It is called when the right mouse button is clicked on the
 * SeqScreen via the mouseEvent() function.
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
    int setMutePoint(double mouseX, bool muted);
/*! @brief  recalculates the MidiSeq::customWave as a
 * function of the current MidiSeq::res and MidiSeq::size values.
 *
 * It is called upon every change of MidiSeq::size and MidiSeq::res. It
 * repeats the current MidiSeq::customWave periodically if the new values
 * lead to a bigger size data array.
 */
    void resizeAll();
    void setRecordMode(int on);
/*! @brief  Called by SeqWidget::mouseEvent()
 */
    int mouseEvent(double mouseX, double mouseY, int buttons, int pressed);
    void setRecordedNote(int note);

/*! @brief  is called upon every change of parameters in
 * SeqWidget or upon input by mouse clicks on the SeqScreen.
 *
 * It fills the
 * MidiSeq::data buffer with Sample points, which it copies from the
 * MidiSeq::customWave data.
 *
 * @param data reference to an array the waveform is copied to
 */
    void getData(std::vector<Sample> * p_data);
/*! @brief  transfers the next Sample to returnNote
 * 
 * Transfers one Sample of data taken from the currently active sequence 
 * MidiSeq::data at the index framePtr into the returnNote to be read
 * by engine. This is called by Engine at every step.
 *
 * @param tick the current tick at which we request a note. This tick will be
 * used to calculate the nextTick which is quantized to the pattern
 */
    void getNextFrame(int64_t tick) override;
/*! @brief  toggles the mute state of one point of the
 * MidiSeq::muteMask array.
 *
 * It is called when the right mouse button is clicked on the
 * SeqScreen. The Sample.mute status at the given position in
 * MidiSeq::customWave is changed as well.
 *
 * @param mouseX Normalized Horizontal location of the mouse on the
 * SeqScreen (0.0 ... 1.0)
 * @see MidiSeq::setMutePoint
 */
    bool toggleMutePoint(double);

    void setFramePtr(int ix);
/*! @brief Checks if deferred parameter changes are pending and applies
 * them if so
 */
    void applyPendingParChanges();
    
    void setNextTick(uint64_t tick) override;
};

#endif
