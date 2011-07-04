/*!
 * @file arpwidget.h
 * @brief Member definitions for the ArpWidget GUI class.
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

#ifndef ARPWIDGET_H
#define ARPWIDGET_H

#include <QSignalMapper>
#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "midiarp.h"
#include "slider.h"
#include "arpscreen.h"

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

/*! @brief GUI class associated with and controlling a MidiArp worker
 *
 * It controls the MidiArp arpeggiator and
 * is created alongwith each MidiArp and embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData member. It manages a ArpWidget::ccList
 * for each
 * instance for MIDI controllers attributed through the MIDILearn
 * context menu. It instantiates a ArpScreen and interacts with it.
class ArpWidget : public QWidget
*/
class ArpWidget : public QWidget
{
  Q_OBJECT

  private:
    QComboBox *chIn;                        // Channel of input events
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QLabel *rangeInLabel, *indexInLabel;
    QComboBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QComboBox *repeatPatternThroughChord;
    QComboBox *triggerMode;
    QComboBox *patternPresetBox;
    QGroupBox *inputFilterBox, *randomBox, *envelopeBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QToolButton *latchModeButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;
    QAction *latchModeAction;
    QAction *deleteAction, *renameAction;
    QAction *cancelMidiLearnAction;
    QSignalMapper *learnSignalMapper, *forgetSignalMapper;
    QStringList midiCCNames;
    MidiArp *midiWorker;
    QLineEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
    Slider *attackTime, *releaseTime;
    bool modified;      /**< @brief Is set to True if unsaved parameter modifications exist */
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
    void loadPatternPresets();
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

/* PUBLIC MEMBERS */
  public:
/*!
 * @brief Constructor for ArpWidget. It creates the GUI and an ArpScreen
 * instance.
 *
 * @param p_midiWorker Associated MidiArp Object
 * @param portCount Number of available ALSA MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param parent The parent widget of this module, i.e. MainWindow
 */
    ArpWidget(MidiArp *p_midiWorker, int portCount, bool compactStyle,
            bool mutedAdd = false, QWidget* parent=0);
    ~ArpWidget();

    QString name;       /**< @brief The name of this ArpWidget as shown in the DockWidget TitleBar */
    int ID;                     /**< @brief Corresponds to the Engine::midiArpList index of the associated MidiArp */
    int parentDockID;           /**< @brief The index of the ArpWidget's parent DockWidget in Engine::moduleWindowList */
    QVector<MidiCC> ccList;     /**< @brief Contains MIDI controller - GUI element bindings */

    ArpScreen *screen;
    QStringList patternPresets, patternNames;
    QCheckBox *muteOut;

    void setChIn(int value);
    void setIndexIn(int index, int value);
    void setRangeIn(int index, int value);

    //these members are common to all modules
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
* @brief This function returns the MidiArp instance associated with this GUI
* Widget.
* @return MidiArp instance associated with this GUI
*/
    MidiArp *getMidiWorker();
/*!
* @brief Settor for the ArpWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void setChannelOut(int value);
/*!
* @brief Settor for the ArpWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void setPortOut(int value);
/*!
* @brief Accessor for ArpWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief This function sets ArpWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);

/* SIGNALS */
  signals:
/*! @brief Emitted to MainWindow::updatePatternPresets saving and deploying modified preset
 *  list.
 *  @param pname Name of the modified pattern
 *  @param pattern Text chain of the modified pattern
 *  @param index Set to the index of the pattern for removal, or to zero for appending a pattern
 * */
    void presetsChanged(const QString& pname, const QString& pattern, int index);
/*! @brief Emitted to MainWindow::removeLfo for module deletion.
 *  @param ID The internal ArpWidget::ID of the module to be removed
 * */
/*! Emitted to MainWindow::removeSeq for module deletion. */
    void moduleRemove(int ID);
/*! @brief Emitted to MainWindow::renameDock for module rename.
 *  @param mname New name of the module
 *  @param parentDockID SeqWidget::parentDockID of the module to rename
 * */
    void dockRename(const QString& mname, int parentDockID);
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
    //these slots are specific for the Arp module
    void updateRandomVelocityAmp(int value);
    void updateRandomTickAmp(int value);
    void updateRandomLengthAmp(int value);
    void updateAttackTime(int value);
    void updateReleaseTime(int value);
    void checkIfRandomSet();
    void checkIfEnvelopeSet();
    void checkIfInputFilterSet();
    void updateText(const QString& newtext);
    void updateRepeatPattern(int);
    void updateTriggerMode(int);
    void selectPatternPreset(int);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void openTextEditWindow(bool on);
    void storeCurrentPattern();
    void removeCurrentPattern();
    void setRandomVisible(bool on);
    void setEnvelopeVisible(bool on);
    void setInputFilterVisible(bool on);

    void updateChIn(int value);
    void updateIndexIn(int value);
    void updateRangeIn(int value);

 /*! @brief Slot for ArpWidget::latchModeAction.
  * Will cause notes remaining in MidiArp::latchBuffer until new
  * stakato note received */
    void setLatchMode(bool);

    //these slots are common to all modules
/*!
 *  @brief Slot for the ArpWidget::channelOut spinbox setting the output
 *  channel of this module.
 *  @param value Number of the output channel to send data to
 *
 */
    void updateChannelOut(int value);
/*!
* @brief Slot for the ArpWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void updatePortOut(int value);
/*!
* @brief Slot for the ArpWidget::muteOut checkbox.
* This function suppresses output of Arp data.
*
* It calls
* MidiArp::setMuted and ArpScreen::setMuted
* @param on Set to True for muting this module
*
*/
    void setMuted(bool on);
/*!
* @brief Slot for ArpWidget::deleteAction.
*
* This function displays a warning and then emits the
* ArpWidget::moduleRemove signal to MainWindow with the module ID as
* parameter.
*/
    void moduleDelete();
/*!
* @brief Slot for ArpWidget::renameAction.
*
* This function queries a new name then emits the ArpWidget::dockRename
* signal to MainWindow with the new name and the dockWidget ID to rename.
*/
    void moduleRename();
/*!
* @brief This function appends a new MIDI controller - GUI element
* binding to ArpWidget::ccList.
*
* Before appending, it checks whether this binding already exists.
* @param controlID The ID of the control GUI element (found
* in ArpWidget::midiCCNames)
* @param ccnumber The CC of the MIDI controller to be attributed
* @param channel The MIDI Channel of the MIDI controller to be attributed
* @param min The minimum value to which the controller range is mapped
* @param max The maximum value to which the controller range is mapped
*/
    void appendMidiCC(int controlID, int ccnumber, int channel, int min, int max);
/*!
* @brief This function removes a MIDI controller - GUI element
* binding from the ArpWidget::ccList.
*
* @param controlID The ID of the control GUI element (found
* in ArpWidget::midiCCNames)
* @param ccnumber The CC of the MIDI controller to be removed
* @param channel The MIDI Channel of the MIDI controller to be removed
*/
    void removeMidiCC(int controlID, int ccnumber, int channel);
/*!
* @brief Slot for ArpWidget::triggered signal created by MIDI-Learn context
* menu MIDI Learn action.
*
* This function sets Engine into
* MIDI Learn status for this module and controlID.
* It emits ArpWidget::setMidiLearn with the necessary module and GUI element
* information parameters.
* Engine will then wait for an incoming controller event and trigger the
* attribution by calling appendMidiCC.
*
* @param controlID The ID of the control GUI element (found
* in ArpWidget::midiCCNames)
*/
    void midiLearn(int controlID);
/*!
* @brief Slot for ArpWidget::triggered signal created by a
* MIDI-Learn context menu Forget action.
*
* This function removes a controller
* binding attribution by calling removeMidiCC.
*
* @param controlID The ID of the control GUI element (found
* in ArpWidget::midiCCNames)
*/
    void midiForget(int controlID);
/*!
* @brief Slot for ArpWidget::cancelMidiLearnAction in MIDI-Learn
* context menu. This function signals cancellation of the
* MIDI-Learn Process to Engine.
*
* It emits ArpWidget::setMidiLearn with controlID set to -1 meaning cancel.
*/
    void midiLearnCancel();
};

#endif
