/**
 * @file engine.h
 * @brief Header file for the Engine class
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
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <QThread>
#include <QDockWidget>
#include <QList>
#include "seqdriver.h"
#include "jackdriver.h"
#include "midiarp.h"
#include "arpwidget.h"
#include "midilfo.h"
#include "lfowidget.h"
#include "midicontrol.h"
#include "midiseq.h"
#include "seqwidget.h"
#include "groovewidget.h"
#include "globstore.h"

/*!
 * @brief 5ms timer calling the updateDisplay() function periodically
 *
 * This class produces timeout() signals every 5ms. The signal
 * is connected to the Engine::updateDisplay() method. This mechanism is used
 * to update many GUI elements as a function of changes happening in the
 * backend's realtime thread. For example, cursor redrawing cannot be called
 * directly from the driver. The modules' updateDisplay() functions therefore
 * only read the position and redraw their screens within this MTimer thread. 
 * 
 * MTimer consumes about 10 times less CPU than QTimer with the
 * same interval.
 *
 */
class MTimer : public QThread
{
    Q_OBJECT

public:
    MTimer();

signals:
    void timeout();

protected:
    void run();
};

/*!
 * @brief Core Engine Thread. Instantiates SeqDriver and JackDriver.
 *
 * For each module type there is a QList for each of
 * its components (for example MidiArp and ArpWidget). In parallel there is
 * a common list for all modules containing their DockWidgets.
 * Engine also instantiates the MIDI Driver backend and processes MIDI
 * events coming in and going out. It dispatches incoming events to the
 * worker modules and schedules resulting events back to the driver.
 * Controller events are dispatched to the modules as requiered by their
 * MIDI Learn MidiCCList.
 *
 */
class Engine : public QThread  {

  Q_OBJECT

  private:
    QList<MidiArp *> midiArpList;
    QList<ArpWidget *> arpWidgetList;
    QList<QDockWidget *> moduleWindowList;
    QList<MidiLfo *> midiLfoList;
    QList<LfoWidget *> lfoWidgetList;
    QList<MidiSeq *> midiSeqList;
    QList<SeqWidget *> seqWidgetList;

    int portCount;
    bool modified;
    int midiLearnID, midiLearnWindowID, midiLearnModuleID;
    bool midiLearnFlag;
    bool useMidiClock;

    int restoreModIx;
    QChar restoreModType;
    int restoreModWindowIndex;
    int restoreRequest;
    int restoreModule;
    int restoreTick;
    int schedRestoreLocation; /**< When Engine requests restore during running this is the location, it is -1 otherwise */
    double tempo;
    double requestedTempo;

    //From SeqDriver
    int schedDelayTicks;
    int nextMinLfoTick;
    int nextMinSeqTick;
    int nextMinArpTick;
    int currentTick;
    int requestTick;
    Sample seqSample;
    bool sendLogEvents;
    int logEventCount;
    QVector<MidiEvent> logEventBuffer;
    QVector<int> logTickBuffer;

    MTimer *dispTimer;

    static bool midi_event_received_callback(void * context, MidiEvent ev);
    static void tick_callback(void * context, bool echo_from_trig);
    static void tr_state_cb(bool tr_state, void * context);
    static void tempo_callback(double bpm, void *context);
  public:
    int grooveTick, grooveVelocity, grooveLength;
    bool midiControllable;
    bool status;
    bool ready;
    GlobStore *globStoreWidget;
    GrooveWidget *grooveWidget;
    JackDriver *jackSync;
    DriverBase *driver;
    MidiControl *midiControl;

  public:
    Engine(GlobStore *p_globStore, GrooveWidget *p_grooveWidget, int p_portCount, bool p_alsamidi, QWidget* parent=0);
    ~Engine();
    int getPortCount();
    bool isModified();


    void addModuleWindow(QDockWidget *moduleWindow);
    void removeModuleWindow(QDockWidget *moduleWindow);
    QDockWidget *moduleWindow(int index);
    int moduleWindowCount();
    void updateIDs(int curID);

    void addMidiArp(MidiArp *midiArp);
    void addArpWidget(ArpWidget *arpWidget);
    void removeMidiArp(MidiArp *midiArp);
    void removeArpWidget(ArpWidget *arpWidget);
    int midiArpCount();
    int arpWidgetCount();
    MidiArp *midiArp(int index);
    ArpWidget *arpWidget(int index);

    void addMidiLfo(MidiLfo *midiLfo);
    void addLfoWidget(LfoWidget *lfoWidget);
    void removeMidiLfo(MidiLfo *midiLfo);
    void removeLfoWidget(LfoWidget *lfoWidget);
    int midiLfoCount();
    int lfoWidgetCount();
    MidiLfo *midiLfo(int index);
    LfoWidget *lfoWidget(int index);

    void addMidiSeq(MidiSeq *midiSeq);
    void addSeqWidget(SeqWidget *seqWidget);
    void removeMidiSeq(MidiSeq *midiSeq);
    void removeSeqWidget(SeqWidget *seqWidget);
    int midiSeqCount();
    int seqWidgetCount();
    MidiSeq *midiSeq(int index);
    SeqWidget *seqWidget(int index);

    int getClientId();
    void setTempo(double bpm);
    void sendGroove();
    void showAllIOPanels(bool on);

  signals:
/**
 * @brief This signal is connected to the LogWidget::appendEvent() slot
 *
 * @param ev MidiEvent received by Engine
 * @param tick Set to the tick value at which the event was received
 */
    void midiEventReceived(MidiEvent ev, int tick);
/**
 * @brief This signal is connected to the MainWindow::updateTempo() slot
 *
 * It is emitted in case of a tempo change detected on the driver, for
 * instance when the Jack Transport Master has changed its tempo
 *
 * @param bpm New tempo to display
 */
    void tempoUpdated(double bpm);

  public slots:
/*!
* @brief  renames the TitleBar of a DockWidget
* with the passed name
*
* @param name New name attribute of the DockWidget
* @param windowID window ID of the DockWidget to rename
* @param widgetID widgetID of the module to rename
*/
    void renameDock(const QString& name, int windowID, int widgetID);
/*!
* @brief  Sets the transport status running or stopped
*
* Clears all MidiArp note buffers and calls the appropriate transport
* start/stop functions in the driver backend.
* 
* @param on Run or Stop
*/
    void setStatus(bool);
/**
 * @brief Sets the modified flag, which is queried before
 * loading a new session file or quitting qmidiarp.
 *
 * @param m Set to True if parameter modifications are present
 */
    void setModified(bool);
/**
 * @brief Slot for ArpWidget::presetsChanged() signal
 * 
 * Called by an individual ArpWidget when its pattern presets changed. It
 * causes all other ArpWidget modules to store the modified presets as
 * well. It saves the preset list to the .qmidiarprc resource file.
 * 
 * @param n Name of the modified preset
 * @param p Pattern string of the modified preset
 * @param index location within the preset list
 * 
 */
    void updatePatternPresets(const QString& n, const QString& p, int index);
/**
 * @brief Dispatches a controller MIDI event to all concerned widgets
 *
 * Concerned widgets are those containing MIDI-learnable elements. 
 *
 * @param ccnumber MIDI Control Event number
 * @param channel MIDI Control Event channel
 * @param value MIDI Control Event value
 */
    void sendController(int ccnumber, int channel, int value);
/**
 * @brief Checks all concerned widgets if they requested a MIDI learn
 *
 * Causes a module that requested MIDI-learn to learn this controller
 * event.
 *
 * @param ccnumber MIDI Control Event number
 * @param channel MIDI Control Event channel
 */
    void learnController(int ccnumber, int channel);
/**
 * @brief Engine internal MIDI Controller handler for MIDI learn
 *
 * Only the tempo is currently MIDI-learnable and controllable
 *
 * @param ccnumber MIDI Control Event number
 * @param channel MIDI Control Event channel
 * @param value MIDI Control Event value
 */
    void handleController(int ccnumber, int channel, int value);
/**
 * @brief Slot for MidiControl::setMidiLearn(). Sets Engine into MIDI Learn status for
 * moduleID and controlID.
 *
 * Engine will then wait for an incoming controller event and trigger the
 * attribution by calling MidiControl::appendMidiCC().
 *
 * @param moduleWindowID dockWidget ID of the module
 * @param moduleID ID of the module (index in the moduleWidgetList)
 * @param controlID ID of the controllable widget requesting MIDI learn
 */
    void setMidiLearn(int moduleWindowID, int moduleID, int controlID);
/**
 * @brief turns on and off MIDI controller handling globally
 *
 * This is a slot for PassWidget::updateControlSetting() called when the
 * Settings window checkbox is clicked.
 */
    void setMidiControllable(bool on);
/**
 * @brief sets the QStyle of all module widgets to CompactStyle
 *
 * This is a slot for PassWidget::updateCompactStyle() called when the
 * Settings window checkbox is clicked.
 */
    void setCompactStyle(bool on);
/**
 * @brief sets grooveTick member of all module widgets
 *
 * This is a slot for GrooveWidget::updateGrooveTick() called when the
 * GrooveWidget::grooveTick slider is moved.
 */
    void setGrooveTick(int grooveTick);
/**
 * @brief sets grooveVelocity member of all module widgets
 *
 * This is a slot for GrooveWidget::updateGrooveVelocity() called when the
 * GrooveWidget::grooveVelocity slider is moved.
 */
    void setGrooveVelocity(int grooveVelocity);
/**
 * @brief sets grooveLength member of all module widgets
 *
 * This is a slot for GrooveWidget::updateGrooveLength() called when the
 * GrooveWidget::grooveLength slider is moved.
 */
    void setGrooveLength(int grooveLength);
/**
 * @brief turns on and off the recording and transfer of received MIDI
 * events to the LogWidget via the midiEventReceived signal
 *
 * This is a slot for LogWidget::enableLogToggle() called when the
 * log window checkbox is clicked.
 */
    void setSendLogEvents(bool on);
/**
 * @brief turns on and off MIDI realtime clock synchronization
 *
 * It stops the transport (Engine status) and calls
 * DriverBase::setUseMidiClock(). This is a slot for
 * MainWindow::midiClockToggle() called when the toolbar button is clicked.
 */
    void setUseMidiClock(bool on);
/**
 * @brief turns on and off JACK Transport synchronization
 *
 * It calls DriverBase::setUseJackTransport(). This is a slot for
 * MainWindow::jackSyncToggle() called when the toolbar button is clicked.
 */
    void setUseJackTransport(bool on);
/**
 * @brief called by the driver at the time a MIDI event is received.
 *
 * It queries all module midi workers for direct event eligibility and if
 * not routes it to all module's handleController() methods. If logging
 * is enabled, it stores the event in the logEventBuffer, which
 * is regularly transferred to the LogWidget by updateDisplay().
 *
 * @param inEv MidiEvent structure that should be handled
 */
    bool eventCallback(MidiEvent inEv);
/**
 * @brief core function called by the driver every time an echo is pending
 *
 * It queries all module midi workers whether new data should be generated
 * at the tick time of that echo (this is the case when the current tick
 * reaches the module's nextTick. If new data is needed it gets it,
 * composes and sends the new MIDI events back in the driver's queue along
 * with their tick time at which they should be played out.
 * It also updates the cursors and sets the module and global indicator
 * pacman. It causes parameter restore() in case global restores
 * are pending.
 *
* @param echo_from_trig True if this echo was generated by a trigger through
* and incoming MIDI event
 */
    void echoCallback(bool echo_from_trig);
    void resetTicks(int curtick);
/*!
* @brief Called by the display MTimer event loop

* Dispatches the periodic call by the MTimer thread to all widgets, which
* will perform their associated operations. 
*/
    void updateDisplay();
/*!
* @brief causes all modules to restore their parameters from its
* ParStore::list at index ix.
*
* @param ix ParStore::list index from which all module parameters are to be restored
*/
    void restore(int ix);
/*!
* @brief causes Engine to call restore() when the restore conditions are met
*
* @param ix ParStore::list index from which all module parameters are to be restored
*/
    void schedRestore(int ix);
/*!
* @brief causes all modules to remove their entries in the ParStore::list
* at index ix
*
* @param ix ParStore::list index at which module parameters are to be removed
*/
    void removeParStores(int ix);
/*!
* @brief causes all modules to store their current parameters in their
* ParStore::list at index ix
*
* @param ix ParStore::list index at which all module parameters are to be stored
*/
    void store(int ix);
/*!
* @brief causes Engine to call restore() when the timing and
* restore type conditions are met
* @param ix ParStore::list index from which all module parameters are to be restored
*/
    void requestRestore(int ix);
/*!
* @brief slot for GlobStore::updateGlobRestoreTimeModule signal
*
* Makes the module with index windowIndex trigger global store switches when its cursor
* reaches the end. It determines the type (Arp, LFO, Seq) and the index of the selected
* module the module storage lists and stores these in local variables.
*
* @param windowIndex moduleWindowList index of the module to become switch
* trigger when its cursor reaches the end of the pattern
*/
    void updateGlobRestoreTimeModule(int windowIndex);

};

#endif
