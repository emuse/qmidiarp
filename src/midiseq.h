/*!
 * @file midiseq.h
 * @brief Member definitions for the MidiSeq MIDI worker class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2014 <qmidiarp-devel@lists.sourceforge.net>
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
    bool pingpong;      /*!< True when the play direction should alternate */
    bool reflect;      /*!< True when the current play direction will change at the next reflect point */
    int noteCount;
    bool seqFinished;
/**
 * @brief  allows forcing an integer value within the
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
 * @brief  calulates the next MidiSeq::currentIndex as a
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
    bool trigLegato; /*!< If True, trigger and restart upon legato input notes as well */
    bool enableLoop;
    bool gotKbdTrig;
    bool restartFlag; /*!< Signals frameptr reset on next getNextFrame() call */
    bool reverse;       /*!< True when the current play direction is backwards */
    int portOut;        /**< Output port index */
    int channelOut;
    bool isMuted;
    bool isMutedDefer;   /*!< Deferred Global mute state */
    bool deferChanges;  /*!< set by SeqWidget to defer parameter changes to pattern end */
    bool parChangesPending;    /*!< set when deferChanges is set and a parameter is changed */
    bool lastMute;              /**< Contains the mute state of the last waveForm point modified by mouse click*/
    bool recordMode;
    bool dataChanged;
    bool needsGUIUpdate;
    int vel, transp, notelength;
    int velDefer, transpDefer, notelengthDefer;
    int size, res;
    int currentRecStep;
    int curLoopMode;    /*!< Local storage of the currently active Loop mode */
    int loopMarker;
    int nPoints;        /*!< Number of steps to be played out */
    int maxNPoints;        /*!< Maximum number of steps that have been used in the session */
    int nextTick;
    int nOctaves;
    int baseOctave;
    int newGrooveTick, grooveTick, grooveVelocity, grooveLength, grooveIndex;
    QVector<Sample> customWave;
    QVector<bool> muteMask;
    QVector<Sample> data;

  public:
    MidiSeq();
    ~MidiSeq();
    void updateWaveForm(int val);
    void updateNoteLength(int);
    void updateVelocity(int);
    void updateResolution(int);
    void updateSize(int);
    void updateLoop(int);
    void updateTranspose(int);
    void updateQueueTempo(int);

    void recordNote(int note);
/*! @brief  sets MidiLfo::isMuted, which is checked by
 * SeqDriver and which suppresses data output globally if set to True.
 *
 * @param on Set to True to suppress data output to the Driver
 */
    void setMuted(bool);
/*! @brief  sets MidiSeq::deferChanges, which will cause a
 * parameter changes only at pattern end.
 *
 * @param on Set to True to defer changes to pattern end
 */
    void updateDeferChanges(bool on) { deferChanges = on; }
/**
 * @brief  does the actions related to a newly received note.
 *
 * It is called by Engine when a new note is received on the MIDI input port.

 * @param note The note value of the received note
 * @param velocity The note velocity
 * @param tick The time the note was received in internal ticks
 */
    bool handleEvent(MidiEvent inEv, int tick);
/*! @brief  sets the (controller) value of one point of the
 * MidiSeq::customWave array. It is used for handling drawing functionality.
 *
 * It is called by SeqWidget::mouseMoved or SeqWidget::mousePressed.
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
 *  by SeqWidget::mousePressed().
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
    void getData(QVector<Sample> *data);
/*! @brief  transfers one Sample of data taken from
 * the currently active waveform MidiLfo::data at the index frameptr.
 *
 * @param p_sample reference to a Sample structure receiving the data point
 */
    void getNextNote(Sample *p_sample, int tick);
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
/**
 * @brief  copies the new values transferred from the
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
/*! @brief Checks if deferred parameter changes are pending and applies
 * them if so
 */
    void applyPendingParChanges();
/**
 * @brief sets MidiSeq::nextTick and MidiSeq::currentIndex position
 * according to the specified tick.
 *
 * @param tick The current tick to which the module position should be
 * aligned.
 */
    void setNextTick(int tick);
};

#endif
