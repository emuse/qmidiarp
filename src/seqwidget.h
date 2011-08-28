/*!
 * @file seqwidget.h
 * @brief Member definitions for the SeqWidget GUI class.
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
 */

#ifndef SEQWIDGET_H
#define SEQWIDGET_H

#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QTextStream>
#include <QCheckBox>
#include <QAction>
#include <QToolButton>
#include <QVector>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "midiseq.h"
#include "slider.h"
#include "seqscreen.h"
#include "midicontrol.h"
#include "managebox.h"

/*! @brief This array holds the currently available resolution values.
 */
const int seqResValues[5] = {1, 2, 4, 8, 16};

/*!
 * @brief GUI class associated with and controlling a MidiSeq worker

 * It controls the MidiSeq sequencer and
 * is created alongwith each MidiSeq and embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData member. It manages a SeqWidget::ccList
 * for each
 * instance for MIDI controllers attributed through the MIDILearn
 * context menu. It instantiates an SeqScreen and interacts with it.
 *
*/
class SeqWidget : public QWidget
{
    Q_OBJECT

    QAction *copyToCustomAction;

    MidiSeq *midiWorker;
    QVector<Sample> data;
    bool modified;      /**< Is set to True if unsaved parameter modifications exist */
    bool lastMute;      /**< Contains the mute state of the last waveForm point modified by mouse click*/
/*!
* @brief This function allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It also advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml reference to QXmlStreamReader containing the open XML stream
*/
    void skipXmlElement(QXmlStreamReader& xml);
/*!
* @brief This function populates the SeqWidget::waveForms list with
* waveform names, currently only one.
*
* This might be used in the future for handling sequence presets
*
* @par Currently there is only one waveform
*   - 0 Custom
*
*/
    void loadWaveForms();

    bool recordMode;    /**< Is set to True if incoming notes are to be step-recorded*/

/* PUBLIC MEMBERS */
  public:
/*!
 * @brief Constructor for SeqWidget. It creates the GUI and an SeqScreen
 * instance.
 *
 * @param p_midiWorker Associated MidiSeq Object
 * @param portCount Number of available ALSA MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param parent The parent widget of this module, i.e. MainWindow
 */
    SeqWidget(MidiSeq *p_midiWorker, int portCount, bool compactStyle,
            bool mutedAdd = false, QWidget* parent=0);
    ~SeqWidget();

    MidiControl *midiControl;
    SeqScreen *screen;
    ManageBox *manageBox;

    QStringList waveForms;
    QComboBox *chIn;
    QComboBox *channelOut, *portOut;
    QComboBox *waveFormBox, *resBox, *sizeBox, *freqBox;
    QCheckBox *muteOut;
    QCheckBox *enableNoteIn;
    QCheckBox *enableNoteOff;
    QCheckBox *enableVelIn;
    QCheckBox *enableRestartByKbd;
    QCheckBox *enableTrigByKbd;
    QCheckBox *enableLoop;
    QCheckBox *dispVert[4];
    Slider *velocity, *transpose, *notelength;
    QAction *recordAction;
    int dispVertical;

    void setChIn(int value);
    void setEnableNoteIn(bool on);
    void setEnableVelIn(bool on);
    QVector<Sample> getCustomWave();

/*!
* @brief This function returns the MidiSeq instance associated with this GUI
* Widget.
* @return MidiSeq instance associated with this GUI
*/
    MidiSeq *getMidiWorker();

/*!
* @brief This function reads all parameters of this module from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief This function reads all module parameters of this module from an old
* QMidiArp .qma text stream.
*
* @param arpText QTextStream to read from
*/
    void readDataText(QTextStream& arpText);
/*!
* @brief This function writes all parameters of this module to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief This function copies all Seq module GUI parameters from
* fromWidget
*
* @param fromWidget pointer to the SeqWidget parameters are to be taken from
*/
    void copyParamsFrom(SeqWidget *fromWidget);
/*!
* @brief This function is obsolete.
* It writes to an old QMidiArp .qma text file passed as a stream
* by MainWindow.
*
* @param arpText QTextStream to write to
*/
    void setChannelOut(int value);
/*!
* @brief Settor for the SeqWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void setPortOut(int value);
/*!
* @brief Accessor for SeqWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief This function sets SeqWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);

/* SIGNALS */
  signals:
/*! @brief Currently not in use. */
    void patternChanged();

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

    void updateChIn(int value);
    void updateEnableNoteIn(bool on);
    void updateEnableVelIn(bool on);
    void updateEnableNoteOff(bool on);
    void updateEnableRestartByKbd(bool on);
    void updateEnableTrigByKbd(bool on);
    void updateEnableLoop(bool on);
    void setRecord(bool on);
    void setDispVert(int mode);
    void updateDispVert(int mode);

/*!
* @brief Slot currently not in use.
*
* This function calls the midi worker
* MidiSeq::copyToCustom, which copies the
* current sequence to the custom sequence buffer
*
* It switches the waveForm combobox to index 0 and calls
* SeqWidget::updateWaveForm.
*
*/
    void copyToCustom();
/*!
* @brief Receiver for incoming note events. This function either sets the
* global transpose and velocity or records the received note.
*
* Depending on the state of the SeqWidget::enableNoteIn and
* SeqWidget::enableVelIn checkboxes, global transpose and velocity
* are set according to the received note. If recordMode is set,
* MidiSeq::recordNote is called, which will replace the current sequence
* sample by the new note. Then, the screen is updated with the
* new sequence (waveform).
*
* @param note Note value (0 ... 127) of the received note event.
* @param velocity Velocity value (0 ... 127) of the received note event.
*
*/
    void processNote(int note, int velocity);

/*!
* @brief Slot for the SeqScreen::mouseMoved signal. This function
* mutes or sets a wave point when the mouse is moved with held buttons.
*
* The mouse events are generated by the SeqScreen.
* It sets a MidiSeq::setCustomWavePoint and calls SeqWidget::newCustomOffset
* if the left button is held while moving. It sets a MidiSeq::setMutePoint
* if the right button is held while moving.
*
* @param mouseX Normalized mouse position on SeqScreen in X
* direction (0.0 ... 1.0)
* @param mouseY Normalized mouse position on SeqScreen in Y
* direction (0.0 ... 1.0)
* @param buttons 1 for left mouse button, 2 for right mouse button
*
*/
    void mouseMoved(double, double, int);
/*!
* @brief Slot for the SeqScreen::mousePressed signal. This function mutes or
* sets a wave point when a mouse button is pressed before moving.
*
* The mouse events are generated by the SeqScreen.
* It sets a MidiSeq::setCustomWavePoint and calls SeqWidget::newCustomOffset
* if the left button is pressed. It toggles a MidiSeq::setMutePoint
* if the right button is pressed before moving.
*
* @param mouseX Normalized mouse position on SeqScreen in X
* direction (0.0 ... 1.0)
* @param mouseY Normalized mouse position on SeqScreen in Y
* direction (0.0 ... 1.0)
* @param buttons 1 for left mouse button, 2 for right mouse button
*
*/
    void mousePressed(double, double, int);

/*!
* @brief Slot for the SeqWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void updateChannelOut(int value);
/*!
* @brief Slot for the SeqWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void updatePortOut(int value);
/*!
* @brief Slot for the SeqWidget::muteOut checkbox.
* This function suppresses output of Seq data.
*
* It callsS
* MidiSeq::setMuted and SeqScreen::setMuted
* @param on Set to True for muting this module
*
*/
    void setMuted(bool on);

    int getCurrentIndex() { return midiWorker->getCurrentIndex(); }
    int getNextTick() { return midiWorker->nextTick; }
};

#endif
