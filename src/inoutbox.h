/*!
 * @file inoutbox.h
 * @brief Member definitions for the InOutBox GUI class.
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
 */

#ifndef INOUTBOX_H
#define INOUTBOX_H

#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>


#ifdef APPBUILD
#include <QInputDialog>
#include "parstore.h"
#include "prefs.h"
#endif

#include "midiworker.h"

/*! @brief GUI base class for module widgets, creates an in-out settings box
 *
 * The three module widget classes ArpWidget, SeqWidget and LfoWidget 
 * inherit from this class. It provides the input
 * output settings and widget and handlers and some other small functions
 * and member variables
*/
class InOutBox: public QWidget
{
  Q_OBJECT
  
    public:
    MidiWorker *midiWorker;
#ifdef APPBUILD
    QString name;       /**< @brief The name of this Widget as shown in the DockWidget TitleBar */
    GlobStore *globStore;
/*!
 * @brief Constructor for InOutBox. Creates the base class and widget for a module
 * 
 *
 * @param p_midiWorker Associated MidiWorker Base Object
 * @param p_globStore The Application-wide globStore widget
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param inOutVisible Add the module with visible InOutBox or not
 * @param name The name of the module preceded by its type (Arp: , etc...)
 */
    InOutBox(MidiWorker *p_midiWorker, GlobStore *p_globStore, 
            Prefs *p_prefs, bool inOutVisible, const QString& name);
    QAction *deleteAction, *renameAction, *cloneAction;
    int ID;             /**< @brief Corresponds to the Engine::midi*List index of the associated MidiSeq */
    int parentDockID;   /**< @brief The index of the Widget in Engine::moduleWidgetList */
    Prefs *prefs;
    ParStore *parStore;
    MidiControl *midiControl;
#else
    InOutBox(const QString& name);
#endif
    ~InOutBox();
    bool modified;      /**< @brief Is set to True if unsaved parameter modifications exist */
    bool dataChanged;
    bool needsGUIUpdate;
    QLabel *rangeInLabel, *indexInLabel;
    QGroupBox *inputFilterBox;
    QComboBox *chIn;                        // Channel of input events
    QComboBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QCheckBox *enableRestartByKbd;
    QCheckBox *enableTrigByKbd;
    QCheckBox *enableTrigLegato;
    QCheckBox *enableNoteIn;
    QCheckBox *enableVelIn;
    QCheckBox *enableNoteOff;
    QSpinBox  *ccnumberInBox;
    QSpinBox  *ccnumberBox;
    QAction *hideInOutBoxAction;
    QToolButton *hideInOutBoxButton;
    QAction *muteOutAction;
    QToolButton *muteOut;
    QAction *deferChangesAction;
    QToolButton *deferChangesButton;

    QWidget *inOutBoxWidget;

    virtual bool isModified();
    virtual void setModified(bool);
    virtual void checkIfInputFilterSet();
    virtual int getFramePtr() { return midiWorker->getFramePtr(); }
    virtual int getNextTick() { return midiWorker->nextTick; }

/*!
 * @brief ENUM for Internal MIDI Control IDs supported 
 * by the InOutBox widget
 * NOTE: These are used by the arp, lfo, and seq widgets
 */         
    enum INOUTBOX_CTRL_IDS {
        MUTE_BUTTON = 10,
        NOTE_LOW = 11,
        NOTE_HIGH = 12,
        PARAM_RESTORE = 13,
    };

#ifdef APPBUILD

/*!
* @brief Writes common module parameters to disk
* @param xml XML stream to write to
*/
    virtual void writeCommonData(QXmlStreamWriter& xml);
/*!
* @brief Reads common module parameters from disk
* @param xml XML stream to read from
*/
    virtual void readCommonData(QXmlStreamReader& xml);
/*!
* @brief Setter for the InOutBox::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    virtual void setPortOut(int value);
/*!
* @brief stores some module parameters in a parameter
* list object
*
* @param ix Position index in the parameter list
*/
    virtual void doStoreParams(int ix) = 0;
/*!
* @brief Restores some module parameters from the parameter
* list object
*
* @param ix Position index in the parameter list
*/
    virtual void doRestoreParams(int ix) = 0;
/**
 * @brief Copies the new values transferred from the
 * GrooveWidget into variables used by the main routine.
 *
 * @param p_grooveTick Groove amount for timing displacements
 * @param p_grooveVelocity Groove amount for velocity variations
 * @param p_grooveLength Groove amount for note length variations
 */
    virtual void newGrooveValues(int p_grooveTick, int p_grooveVelocity,
            int p_grooveLength);

/**
 * @brief Handles MIDI-learned controller events locally in each module
 * 
 * It is called by Engine::sendController() when a MIDI controller is received.
 */
    virtual void handleController(int ccnumber, int channel, int value) = 0;
/*!
 * @brief Updates the SeqScreen and other GUI elements with data from
 * the MidiSeq instance.
 *
 * It is called by Engine::updateDisplay(), which itself is
 * connected to the MTimer::timeout event. It runs in the MTimer thread.
 * It reads the waveform data and other settings from the MidiSeq instance
 * and sets GUI cursor, wave display and other elements accordingly. This
 * way, no memory allocations are done within the jack run thread, for
 * instance by MIDI controllers, since the Qt widgets are not called directly.
 * This function also checks whether parameter changes from ParStore are
 * pending and causes them to get transferred if so.
 */
    virtual void updateDisplay() = 0;
    virtual void updateIndicators();
    virtual void updateCursorPos() = 0;
    virtual void checkIfRestore(int64_t *restoreTick, bool *restoreFlag);
    virtual bool prepareNextFrame(bool echo_from_trig, int syncTol,
                int64_t tick, int64_t *restoreTick, bool *restoreFlag);
/*!
* @brief writes all parameters of this module to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    virtual void writeData(QXmlStreamWriter& xml) = 0;
/*!
* @brief allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It also advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml reference to QXmlStreamReader containing the open XML stream
*/
    virtual void skipXmlElement(QXmlStreamReader& xml);
#endif
    
  public slots:
/*!
* @brief Slot for InOutBox::deleteAction.
*
* This function displays a warning and then emits the
* InOutBox::moduleRemove signal to MainWindow with the module ID as
* parameter.
*/
    virtual void moduleDelete();
/*!
* @brief Slot for InOutBox::renameAction.
*
* This function queries a new name then emits the InOutBox::dockRename
* signal to MainWindow with the new name and the dockWidget ID to rename.
*/
    virtual void moduleRename();
/*!
* @brief Slot for InOutBox::cloneAction.
*
* This function emits the InOutBox::moduleClone()
* signal to MainWindow with the module ID and the dockWidget ID.
*/
    virtual void moduleClone();

    virtual void setInputFilterVisible(bool on);
    
/*!
* @brief Store common module parameters and call doStoreParams
* 
* Stores common module parameters to ParStore::list and then calls 
* InOutBox::doStoreParams(), which is reimplemented in each module widget
* to store module specific parameters.
* 
* @param ix The storage location index to write to
* @param empty Signal an empty location
*/
    virtual void storeParams(int ix, bool empty = 0);
/*!
* @brief Restore common module parameters and call doRestoreParams
* 
* Restores common module parameters from ParStore::list and then calls 
* InOutBox::doRestoreParams(), which is reimplemented in each module widget
* to restore module specific parameters.
* 
* @param ix The storage location index to read from
*/
    virtual void restoreParams(int ix);

    virtual void copyParamsFrom(InOutBox *fromWidget) { (void)fromWidget; };

/*!
* @brief Slot for the InOutBox::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    virtual void updateChannelOut(int value);
/*!
* @brief Slot for the InOutBox::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    virtual void updateCcnumberIn(int value);
    virtual void updatePortOut(int value);
    virtual void updateChIn(int value);
    virtual void updateIndexIn(int value);
    virtual void updateRangeIn(int value);
    virtual void updateEnableNoteIn(bool on);
    virtual void updateEnableVelIn(bool on);
    virtual void updateEnableNoteOff(bool on);
    virtual void updateEnableRestartByKbd(bool on);
    virtual void updateEnableTrigByKbd(bool on);
    virtual void updateTrigLegato(bool on);
    virtual void updateNRep(int nrep);
/*!
* @brief Slot for the InOutBox::ccnumberBox spinbox setting the output
* controller CC number of this module.
* @param val CC number to send data to
*
*/
    virtual void updateCcnumber(int val);
/*!
* @brief Slot for the InOutBox::deferChangesAction.
*
* Sets a flag in the midi worker causing parameter changes to become
* active/inactive only at pattern end.
*
* @param on Set to True for deferring parameter changes to pattern end
*/
    virtual void updateDeferChanges(bool on);
/*!
* @brief Slot for the InOutBox::muteOut checkbox.
* suppresses output of MIDI data.
*
* It sets MidiWorker::isMuted and causes a needsGUIUpdate
* @param on Set to True for muting this module
*
*/
    virtual void setMuted(bool on);

    bool getReverse() { return midiWorker->reverse; }

  signals:

/*! @brief Emitted to MainWindow::removeSeq for module deletion.
 *  */
    void removeModule();
/*! @brief Emitted to MainWindow::renameDock for module rename.
 *  @param mname New name of the module
 *  @param parentDockID parentDockID of the module to rename
 * */
    void dockRename(const QString& mname, int parentDockID);
/*! @brief Emitted to MainWindow::cloneSeq for module duplication.
 * */
    void cloneModule();
};
#endif
