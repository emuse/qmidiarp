/*!
 * @file seqwidget.h
 * @brief Member definitions for the SeqWidget GUI class.
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
 */

#ifndef SEQWIDGET_H
#define SEQWIDGET_H

#include <QSignalMapper>

#include "midiseq.h"
#include "inoutbox.h"
#include "slider.h"
#include "seqscreen.h"
#include "cursor.h"


/*!
 * @brief GUI class associated with and controlling a MidiSeq worker

 * It controls the MidiSeq step sequencer, which is transferred as a pointer
 * when SeqWidget is instantiated. It is embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData() function. It instantiates a SeqScreen
 * and interacts with it.
 *
*/
class SeqWidget : public InOutBox
{
    Q_OBJECT

    MidiSeq *midiSeq;
    bool lastMute;      /**< Contains the mute state of the last waveForm point modified by mouse click*/
    bool recordMode;    /**< Is set to True if incoming notes are to be step-recorded*/

/* PUBLIC MEMBERS */
  public:
#ifdef APPBUILD
/*!
 * @brief Constructor for SeqWidget. It creates the GUI and an SeqScreen
 * instance.
 *
 * @param p_midiSeq Associated MidiSeq Object
 * @param p_globStore The Application-wide globStore widget
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param inOutVisible Add the module with visible InOutBox or not
 * @param name The name of the module preceded by its type (Arp: , etc...)
 */
    SeqWidget(MidiSeq *p_midiSeq, GlobStore *p_globStore, Prefs *p_prefs,
            bool inOutVisible = true,
            const QString& name = "");

#else
    SeqWidget();
#endif

    QVector<Sample> data;
    SeqScreen *screen;
    Cursor *cursor;

    QComboBox *resBox, *sizeBox, *freqBox;
    QComboBox *loopBox;
    QCheckBox *dispVert[4];
    Slider *velocity, *transpose, *notelength;
    QAction *recordAction;
    QSignalMapper *dispSignalMapper;
    int dispVertIndex;
    int resBoxIndex;
    int sizeBoxIndex;

    QVector<Sample> getCustomWave();
    QVector<bool> getMuteMask();

/*!
 * @brief ENUM for Internal MIDI Control IDs supported 
 * by the Sequencer widget
 */         
    enum SEQWIDGET_CTRL_IDS {
        SEQ_VELOCITY = 1,
        SEQ_NOTE_LENGTH = 2,
        SEQ_RECORD = 3,
        SEQ_RESOLUTION = 4,
        SEQ_SIZE = 5,
        SEQ_LOOP_MODE = 6,
        SEQ_TRANSPOSE = 7,
        SEQ_CHANNEL_OUT = 8,
    };

#ifdef APPBUILD
/*!
* @brief returns the MidiSeq instance associated with this GUI
* Widget.
* @return MidiSeq instance associated with this GUI
*/
    MidiSeq *getMidiWorker();

/*!
* @brief reads all parameters of this module from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief writes all parameters of this module to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief copies all Seq module GUI parameters from
* fromWidget
*
* @param fromWidget pointer to the SeqWidget parameters are to be taken from
*/
    void copyParamsFrom(SeqWidget *fromWidget);

    void doStoreParams(int ix);
    void doRestoreParams(int ix);
    void updateDisplay();
    void handleController(int ccnumber, int channel, int value);
    void updateCursorPos() {cursor->updatePosition(getFramePtr()); }
#endif

/* SIGNALS */
  signals:
/*! @brief Currently not in use. */
    void patternChanged();
/*! @brief Is re-emitted when a SeqScreen::mouseEvent() is received
 * and no pointer to a MidiWorker is present (LV2 build). */
    void mouseSig(double, double, int, int);

/* PUBLIC SLOTS */
  public slots:
/*!
* @brief Slot currently not in use.
* @param val Waveform index to choose as present in SeqWidget::loadWaveForms.
*
*/
    void updateWaveForm(int);
/*!
* @brief Slot for the SeqWidget::resBox combobox. Sets the resolution
* of the sequencer.
*
* It sets MidiSeq::res and updates the SeqScreen of this module.
* @param val Resolution index from SeqWidget::seqResValues to set.
*
*/
    void updateRes(int);
/*!
* @brief Slot for the SeqWidget::sizeBox combobox. Sets the waveform size
* of the sequencer.
*
* It sets MidiSeq::size and updates the SeqScreen of this module.
* @param val Size (number of bars) of the waveform.
*
*/
    void updateSize(int);
/*!
* @brief Slot for the SeqWidget::loopBox combobox. Sets the loop mode
* of the sequencer.
*
* It sets MidiSeq::reverse, MidiSeq::pingpong and MidiSeq::enableLoop
* @param val Combination index ranging from 0 to 5
*
*/
    void updateLoop(int);
/*!
* @brief Slot for the SeqWidget::velocity slider. Sets the note velocity
* of the sequencer.
*
* @param val New note velocity (0 ... 127).
*
*/
    void updateVelocity(int val);
/*!
* @brief Slot for the SeqWidget::transpose slider. Sets the global transpose
* of the sequencer in semitones (-24 ... 24).
*
* @param val New global transpose of the sequencer in semitones (-24 ... 24).
*
*/
    void updateNoteLength(int val);
    void updateTranspose(int val);

    void setRecord(bool on);
    void setDispVert(int mode);
    void updateDispVert(int mode);

/*!
* @brief Slot for the SeqScreen::mouseEvent signal.
* 
* Mutes or sets a wave point when the mouse is pressed or
* released or moved with held buttons.
* The mouse events are generated by the SeqScreen.
* It calls MidiSeq::mouseEvent()
* or emits the mouseSig() signal if no pointer to a MIDI worker was
* transferred (LV2 build).
*
* @param mouseX Normalized mouse position on SeqScreen in X
* direction (0.0 ... 1.0)
* @param mouseY Normalized mouse position on SeqScreen in Y
* direction (0.0 ... 1.0)
* @param buttons 1 for left mouse button, 2 for right mouse button
* @param pressed 0 for mouse moved, 1 for mouse pressed, 2 for mouse released
*
*/
    void mouseEvent(double, double, int, int pressed);

    int getLoopMarker() { return midiSeq->loopMarker; }
    bool getReverse() { return midiSeq->reverse; }
    int sliderToTickLen(int val) { return (val * TPQN / 64); }
    int tickLenToSlider(int val) { return (val * 64 / TPQN); }
};

#endif
