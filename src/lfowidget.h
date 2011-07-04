/*!
 * @file lfowidget.h
 * @brief Member definitions for the LfoWidget GUI class.
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

#ifndef LFOWIDGET_H
#define LFOWIDGET_H

#include <QAction>
#include <QComboBox>
#include <QCheckBox>
#include <QSignalMapper>
#include <QSpinBox>
#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "midilfo.h"
#include "slider.h"
#include "lfoscreen.h"

/*! This array holds the currently available resolution values.
 */
const int lfoResValues[9] = {1, 2, 4, 8, 16, 32, 64, 96, 192};
/*! This array holds the currently available frequency values.
 */
const int lfoFreqValues[14] = {1, 2, 4, 8, 16, 24, 32, 64, 96, 128, 160, 192, 224, 256};
#ifndef MIDICC_H

/*! @brief Structure holding all elements of a MIDI controller allocated to
 * a QMidiArp parameter assigned via MIDI learn
 */
struct MidiCC {
        QString name;   /**< @brief Name of the assigned parameter (GUI element)*/
        int min;        /**< @brief Value output when the CC value is 0 */
        int max;        /**< @brief Value output when the CC value is 127 */
        int ccnumber;   /**< @brief MIDI CC number of the assigned controller event */
        int channel;    /**< @brief MIDI channel on which the controller has to come in */
        int ID;         /**< @brief Internal ID of the assigned parameter (GUI element)*/
    };
#define MIDICC_H
#endif

/*!
 * @brief GUI class associated with and controlling a MidiLfo worker

 * It controls the MidiLfo LFO and
 * is created alongwith each MidiLfo and embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData member. It manages a LfoWidget::ccList
 * for each
 * instance for MIDI controllers attributed through the MIDILearn
 * context menu. It instantiates an LfoScreen and interacts with it.
 *
 *
*/
class LfoWidget : public QWidget
{
    Q_OBJECT

    QAction *deleteAction, *renameAction, *cloneAction;
    QAction *cancelMidiLearnAction;
    QSignalMapper *learnSignalMapper, *forgetSignalMapper;
    QStringList midiCCNames; /**< List of from GUI-element names for index = ControlID */

    MidiLfo *midiWorker;
    QVector<Sample> data;
    bool modified;              /**< Is set to True if unsaved parameter modifications exist */
    bool lastMute;              /**< Contains the mute state of the last waveForm point modified by mouse click*/
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
* @brief This function populates the LfoWidget::waveForms list with
* waveform names.
*
* @par Currently they are
*   - 0 Sine
*   - 1 Saw up
*   - 2 Triangle
*   - 3 Saw Down
*   - 4 Square
*   - 5 Custom
*
*/
    void loadWaveForms();
/*!
* @brief This function creates and attributes a MIDI-Learn context menu
* to the passed QWidget.
*
* The menu is connected to a QSignalMapper for dispatching its actions
*
* @param *widget QWidget to which the context menu is attributed
* @param count Internal identifier of the controllable QWidget
*/
    void addMidiLearnMenu(QWidget *widget, int count = 0);

/*!
* @brief This function determines the minimum of the current waveform and
* sets the LfoWidget::offset slider accordingly.
*
* It also sets MidiLfo::cwmin. When a new waveform is drawn, its minimum
* offset from 0 changes and the offset controller has to be adapted in range.
*
*/
    void newCustomOffset();

/* PUBLIC MEMBERS */
  public:
/*!
 * @brief Constructor for LfoWidget. It creates the GUI and an LfoScreen
 * instance.
 *
 * @param p_midiWorker Associated MidiLfo Object
 * @param portCount Number of available ALSA MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param parent The parent widget of this module, i.e. MainWindow
 */
    LfoWidget(MidiLfo *p_midiWorker, int portCount, bool compactStyle,
            bool mutedAdd = false, QWidget* parent=0);
    ~LfoWidget();

    QString name;               /**< The name of this LfoWidget as shown in the DockWidget TitleBar */
    int ID;                     /**< Corresponds to the Engine::midiLfoList index of the associated MidiLfo */
    int parentDockID;           /**< The index of the LfoWidget's parent DockWidget in Engine::moduleWindowList */
    QVector<MidiCC> ccList;     /**< Contains MIDI controller - GUI element bindings */

    LfoScreen *screen;
    QStringList waveForms;
    QComboBox *chIn;
    QSpinBox  *ccnumberInBox;
    QSpinBox  *ccnumberBox;
    // Output channel / port (ALSA Sequencer)
    QComboBox *channelOut, *portOut;
    QComboBox *resBox, *sizeBox;
    QCheckBox *muteOut;
    Slider *frequency, *amplitude, *offset;
    QAction *recordAction;
    QComboBox *waveFormBox, *freqBox;

    void setChIn(int value);
    QVector<Sample> getCustomWave();

/*!
* @brief This function returns the MidiLfo instance associated with this GUI
* Widget.
* @return MidiLfo instance associated with this GUI
*/
    MidiLfo *getMidiWorker();

/*!
* @brief This function reads all parameters of this LFO from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief This function reads all LFO parameters of this module from an old
* QMidiArp .qma text stream.
*
* @param arpText QTextStream to read from
*/
    void readDataText(QTextStream& arpText);
/*!
* @brief This function writes all parameters of this LFO to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief This function copies all LFO module GUI parameters from
* fromWidget
*
* @param fromWidget pointer to the LfoWidget parameters are to be taken from
*/
    void copyParamsFrom(LfoWidget *fromWidget);
/*!
* @brief Settor for the LfoWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void setChannelOut(int value);
/*!
* @brief Settor for the LfoWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void setPortOut(int value);
/*!
* @brief Accessor for LfoWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief This function sets LfoWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);

/* SIGNALS */
  signals:
/*! @brief Currently not in use. */
    void patternChanged();
/*! @brief Emitted to MainWindow::removeLfo for module deletion.
 *  @param ID The internal LfoWidget::ID of the module to remove
 *  */
    void moduleRemove(int ID);
/*! @brief Emitted to MainWindow::renameDock for module rename.
 *  @param mname New name of the module
 *  @param parentDockID SeqWidget::parentDockID of the module to rename
 * */
    void dockRename(const QString& mname, int parentDockID);
/*! @brief Emitted to MainWindow::cloneLfo for module duplication.
 *  @param ID MidiLfo::ID of the module to clone
 * */
    void moduleClone(int ID);
/*! @brief Emitted to Engine::setMidiLearn to listen for incoming events.
 *  @param parentDockID SeqWidget::parentDockID of the module to rename
 *  @param ID SeqWidget::ID of the module receiving the MIDI controller
 *  @param controlID ID of the GUI element to be assigned to the controller
 *  */
    void setMidiLearn(int parentDockID, int ID, int controlID);
/*! @brief Forwarded context menu action by signalMapper to call MIDI-Learn/Forget functions.
 *  @param controlID ID of the GUI element requesting the MIDI controller
 *  */
    void triggered(int controlID);

/* PUBLIC SLOTS */
  public slots:
    void updateChIn(int value);
    void updateCcnumberIn(int value);
    void updateScreen(int value);
    void setRecord(bool on);

/*!
* @brief Slot for the LfoWidget::waveFormBox combobox setting the waveform
* of this module.
* @param val Waveform index to choose as present in LfoWidget::loadWaveForms.
*
*/
    void updateWaveForm(int);
/*!
* @brief Slot for the LfoWidget::resBox combobox. Sets the resolution
* of the LFO.
*
* It sets MidiLfo::res and updates the LfoScreen of this module.
* @param val Resolution index from lfoResValues to set.
*
*/
    void updateRes(int);
/*!
* @brief Slot for the LfoWidget::sizeBox combobox. Sets the waveform size
* of the LFO.
*
* It sets MidiLfo::size and updates the LfoScreen of this module.
* @param val Size (number of bars) of the waveform.
*
*/
    void updateSize(int);
/*!
* @brief Slot for the LfoWidget::ccnumberBox spinbox setting the output
* controller CC number of this module.
* @param val CC number to send data to
*
*/
    void updateCcnumber(int val);
/*!
* @brief Slot for the LfoWidget::freqBox combobox. Sets the frequency
* of the LFO.
*
* It sets MidiLfo::freq for this and updates the LfoScreen of this module.
* @param val Frequency index from lfoFreqValues to set.
*
*/
    void updateFreq(int val);
/*!
* @brief Slot for the LfoWidget::amplitude slider. Sets the amplitude
* of the waveform for this LFO.
*
* It sets MidiLfo::amp and updates the LfoScreen of this module.
* @param val Amplitude (0 ... 127) of the calculated waveform.
*
*/
    void updateAmp(int val);
/*!
* @brief Slot for the LfoWidget::offset slider. Sets the offset
* of the waveform for this LFO.
*
* This function updates the LfoScreen of this module.
* @param val Offset (0 ... 127) of the waveform.
*
*/
    void updateOffs(int val);

/*!
* @brief Slot for the LfoScreen::mouseMoved signal. This function
* mutes or sets a wave point when the mouse is moved with held buttons.
*
* The mouse events are generated by the LfoScreen.
* It sets a MidiLfo::setCustomWavePoint and calls LfoWidget::newCustomOffset
* if the left button is held while moving. It sets a MidiLfo::setMutePoint
* if the right button is held while moving.
*
* @param mouseX Normalized mouse position on LfoScreen in X
* direction (0.0 ... 1.0)
* @param mouseY Normalized mouse position on LfoScreen in Y
* direction (0.0 ... 1.0)
* @param buttons 1 for left mouse button, 2 for right mouse button
*
*/
    void mouseMoved(double, double, int);
/*!
* @brief Slot for the LfoScreen::mousePressed signal. This function
* mutes or sets a wave point when a mouse button is pressed before moving.
*
* The mouse events are generated by the LfoScreen.
* It sets a MidiLfo::setCustomWavePoint and calls LfoWidget::newCustomOffset
* if the left button is pressed. It toggles a MidiLfo::setMutePoint
* if the right button is pressed before moving.
*
* @param mouseX Normalized mouse position on LfoScreen in X
* direction (0.0 ... 1.0)
* @param mouseY Normalized mouse position on LfoScreen in Y
* direction (0.0 ... 1.0)
* @param buttons 1 for left mouse button, 2 for right mouse button
*
*/
    void mousePressed(double, double, int);
/*!
* @brief Slot for the LfoScreen::mouseWheel signal. This function changes the
* offset by the steps the mouse wheel was turned.
*
* The mouse events are generated by the LfoScreen.
* @param step number of steps and direction the offset will be changed by
*
*/
    void mouseWheel(int);
/*!
* @brief Slot currently not in use.
*
* This function calls the midi worker
* MidiLfo::copyToCustom, which copies the current waveform to the custom
* waveform buffer
*
* It switches the waveForm combobox to index 5 and calls
* LfoWidget::updateWaveForm.
*
*/
    void copyToCustom();

/*!
* @brief Slot for the LfoWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void updateChannelOut(int value);
/*!
* @brief Slot for the LfoWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void updatePortOut(int value);
/*!
* @brief Slot for the LfoWidget::muteOut checkbox.
* This function suppresses output of LFO data.
*
* It calls
* MidiLfo::setMuted and LfoScreen::setMuted
* @param on Set to True for muting this module
*
*/
    void setMuted(bool on);
/*!
* @brief Slot for LfoWidget::deleteAction.
*
* This function displays a warning and then emits the
* LfoWidget::moduleRemove signal to MainWindow with the module ID as
* parameter.
*/
    void moduleDelete();
/*!
* @brief Slot for LfoWidget::renameAction.
*
* This function queries a new name then emits the LfoWidget::dockRename
* signal to MainWindow with the new name and the dockWidget ID to rename.
*/
    void moduleRename();
/*!
* @brief Slot for SeqWidget::cloneAction.
*
* This function emits the SeqWidget::dockClone
* signal to MainWindow with the module ID and the dockWidget ID.
*/
    void moduleClone();
/*!
* @brief This function appends a new MIDI controller - GUI element
* binding to LfoWidget::ccList.
*
* Before appending, it checks whether this binding already exists.
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
* @param ccnumber The CC of the MIDI controller to be attributed
* @param channel The MIDI Channel of the MIDI controller to be attributed
* @param min The minimum value to which the controller range is mapped
* @param max The maximum value to which the controller range is mapped
*/
    void appendMidiCC(int controlID, int ccnumber, int channel, int min, int max);
/*!
* @brief This function removes a MIDI controller - GUI element
* binding from the LfoWidget::ccList.
*
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
* @param ccnumber The CC of the MIDI controller to be removed
* @param channel The MIDI Channel of the MIDI controller to be removed
*/
    void removeMidiCC(int controlID, int ccnumber, int channel);
/*!
* @brief Slot for LfoWidget::triggered signal created by MIDI-Learn context
* menu MIDI Learn action.
*
* This function sets Engine into
* MIDI Learn status for this module and controlID.
* It emits LfoWidget::setMidiLearn with the necessary module and GUI element
* information parameters.
* Engine will then wait for an incoming controller event and trigger the
* attribution by calling appendMidiCC.
*
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
*/
    void midiLearn(int controlID);
/*!
* @brief Slot for LfoWidget::triggered signal created by a MIDI-Learn
* context menu Forget action.
*
* This function removes a controller
* binding attribution by calling removeMidiCC.
*
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
*/
    void midiForget(int controlID);
/*!
* @brief Slot for LfoWidget::cancelMidiLearnAction in MIDI-Learn context
* menu. This function signals cancellation of the MIDI-Learn Process to
* Engine.
*
* It emits LfoWidget::setMidiLearn with controlID set to -1 meaning cancel.
*/
    void midiLearnCancel();
};

#endif
