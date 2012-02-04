/*!
 * @file mainwindow.h
 * @brief Member definitions for the MainWindow top-level UI class.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QMessageBox>
#include <QMainWindow>
#include <QString>
#include <QToolBar>

#include "logwidget.h"
#include "engine.h"
#include "midicctable.h"
#include "passwidget.h"
#include "globstore.h"
#include "groovewidget.h"
#include "config.h"

#ifdef JACK_SESSION
#include <jack/session.h>
#endif


static const char ABOUTMSG[] =
            "<html> <p><b><big>" APP_NAME " " PACKAGE_VERSION "</big></b></p>"
            "<p>(C) 2009-2011 Frank Kober<br/>"
            "(C) 2011 Nedko Arnaudov<br/>"
            "(C) 2009 Guido Scholz<br/>"
            "(C) 2002-2003 Matthias Nagorni (SuSE AG Nuremberg)<br/></p>"
            "<p>For getting support please type <b>man qmidiarp</b> or go to<br/>"
            "<a href=\"http://qmidiarp.sourceforge.net\">"
            "http://qmidiarp.sourceforge.net</a></p>"
            APP_NAME " is licensed under the GPL.</b></p></html>";

/*!
 * The MainWindow class is the main UI that holds functions to manage global
 * QMidiArp parameters and modules and to load and save parameters to
 * disk. The constructor sets up all main window elements including
 * toolbars and menus. It instantiates the LogWidget, PassWidget,
 * MidiCCTable and their DockWidget windows. It also instantiates the
 * Engine widget holding the lists of modules.

 * @brief Top-level UI class. Instantiates Engine
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

  private:
    static int sigpipe[2];
    bool alsaMidi;
    QSpinBox *tempoSpin;
    PassWidget *passWidget;
    GrooveWidget *grooveWidget;
    LogWidget *logWidget;
    GlobStore *globStore;
    Engine *engine;
    MidiCCTable *midiCCTable;
    QString lastDir, filename;
    QStringList patternNames, patternPresets;
    QStringList recentFiles;
    QDockWidget *logWindow, *grooveWindow, *passWindow;

    QToolBar *controlToolBar, *fileToolBar;
    QAction *runAction, *addArpAction;
    QAction *addLfoAction, *addSeqAction;
    QAction *fileNewAction, *fileOpenAction, *fileSaveAction, *fileSaveAsAction;
    QAction *fileQuitAction;
    QAction *midiClockAction, *jackSyncAction;
    QAction *viewGlobAction;
    QMenu* fileRecentlyOpenedFiles;

/*!
* @brief This function opens a file dialog and calls either
* MainWindow::openTextFile or
* MainWindow::openFile depending on the selected file extension.
* It is called by MainWindow::fileOpen.
*/
    void chooseFile();
/*! @brief This function checks whether parameter modifications
 * were done after the last save.
 *
 * If yes, it queries the user how to handle unsaved changes using a
 * message box.
 * @return True if the parameters can be overwritten
 */
    bool isSave();
    void updateWindowTitle();
/*!
* @brief This function opens a QMidiArp XML session file named
* MainWindow::filename for write using QXmlStreamReader.
*
* It writes global and GUI parameters and and calls the
* block writers in the module widgets.
*
* @return True if write was successful
*/
    bool saveFile();
/*! @brief This function opens a file dialog and appends the file extension to
 * the chosen name if not present. It is called by fileSaveAs.
 * @return True if a file name was successfully chosen
 */
    bool saveFileAs();
/*!
* @brief This function returns the result of Engine::isModified
* @return True if unsaved parameter modifications exist in any module
*
*/
    bool isModified();
/*!
* @brief This function creates and adds a new MidiArp to Engine.
*
* It also creates and adds the associated ArpWidget
* and DockWidget to the corresponding lists in Engine. It sets
* the ArpWidget::name and ArpWidget::ID as the current count of the
* Engine::MidiArpList.
*
* @param name Name attribute of the created arpeggiator module
*/
    void addArp(const QString&);
/*!
* @brief This function creates and adds a new MidiLfo to Engine.
*
* It also creates and adds the associated LfoWidget
* and DockWidget to the corresponding lists in Engine. It sets
* the LfoWidget::name and SeqWidget::ID as the current count of the
* Engine::MidiLfoList.
*
* @param name Name attribute of this LFO module
*/
    void addLfo(const QString&);
/*!
* @brief This function creates and adds a new MidiSeq to Engine.
*
* It also creates and adds the associated SeqWidget
* and DockWidget to the corresponding lists in Engine. It sets
* the SeqWidget::name and SeqWidget::ID as the current count of the
* Engine::MidiSeqList.
*
* @param name Name attribute of this arpeggiator module
*/
    void addSeq(const QString&);
/*!
* @brief This function wraps the given widget in a QDockWidget and adds
* it to the list in Engine.
*
* @param *moduleWidget The QWidget to be embedded
*
*/
    void appendDock(QWidget *moduleWidget, const QString& name, int count);
/*!
* @brief This function reads global parameter block from an XML session
* stream using the QXmlStreamReader passed by the caller.
*
* @param xml Reference to QXmlStreamReader containing the open XML stream
*/
    void readFilePartGlobal(QXmlStreamReader& xml);
/*!
* @brief This function reads the module parameter blocks from the XML stream
*  by calling their read functions.
*
* It uses the first three letters of the module name to distinguish their
* type. It creates the according module components and calls their UI
* widgets, which fill them with the parameters found in the same stream.
* Calls Engine->NNNWidget->readData, where NNN is Arp, Lfo or Seq.
*
* @param xml Reference to QXmlStreamReader containing the open XML stream
*/
    void readFilePartModules(QXmlStreamReader& xml);
/*!
* @brief This function reads the GUI settings block
* from the XML session stream passed by the caller.
*
* @param xml Reference to QXmlStreamReader containing the XML stream
*/
    void readFilePartGUI(QXmlStreamReader& xml);
/*!
* @brief This function prepends a filename at the beginning of the recently
* opened files list.
*
* It is called by openFile if the opening was successful.
* @param fn Filename with full path to be prepended
* @param lst The list of recently opened files
* @see appendRecentlyOpenedFile, setupRecentFilesMenu
*/
    void addRecentlyOpenedFile(const QString &fn, QStringList &lst);
/*!
* @brief This function appends a filename at the end of the recently
* opened files list.
*
* It is called by openFile if the opening was successful.
* @param fn Filename with full path to be appended
* @param lst The list of recently opened files
* @see addRecentlyOpenedFile, setupRecentFilesMenu
*/
    void appendRecentlyOpenedFile(const QString &fn, QStringList &lst);
/*!
* @brief This function checks whether a .qmidiarprc file is present
* and creates it with default settings, if not.
* @return True if the file existed, False if it was created now.
* @see readRcFile, updatePatternPresets
*
*/
    bool checkRcFile();
/*!
* @brief This function writes the .qmidiarprc text resource file.
*
* It is called on program exit and upon modification of the
* Arp preset list.
* @see readRcFile, updatePatternPresets
*/
    void writeRcFile();
/*!
* @brief This function reads all elements from the .qmidiarprc text
* resource file.
*
* The file contains the Arp preset patterns, the last
* GUI state, settings made in the Settings dialog (PassWidget) and
* the recent files and path.
* This function is called from the MainWindow constructor.
* @see readRcFile, updatePatternPresets
*/
    void readRcFile();
/*!
* @brief This function checks if there are no more modules present and sets
* some GUI elements accordingly if so.
*
* It is called by removeArp, removeSeq and removeLfo.
* @see checkIfFirstModule
*/
    void checkIfLastModule();
/*!
* @brief This function checks if there were no modules present, i.e.
* if the module we just created is the first one, and sets some GUI
* elements accordingly if so.
*
* It is called by addArp, addSeq and addLfo.
* @see checkIfLastModule
*/
    void checkIfFirstModule();
/*!
* @brief This function removes and deletes all modules from the
* lists.
*
* It removes all module components, i.e. the Midi workers, UI widgets
* and DockWidgets. It disconnects jack transport and stops the
* transport if running.
*/
    void clear();
/*!
* @brief This function disables or enables GUI elements depending on
* synchronization mode
*
* It distinguishes between internal and external (MIDI Clock or
* JACK Transport) sync. This is necessary since some operations would
* break the synchronization, and some others would become useless.
* @param on True sets the GUI for external synchronization, False
* returns it to internal clock state.
*
* @see midiClockToggle, jackSyncToggle
*/
    void setGUIforExtSync(bool on);
/*!
* @brief This function restarts the transport position if rolling and does
* nothing if it is stopped
*
*/
    void resetTransport();

/*! @brief Handler for system signals (SIGUSR1, SIGINT...).
 * This function writes a message to the pipe and leaves as soon as possible
 */
    static void handleSignal(int);
/*! @brief This function sets up a QSocketNotifier forwarding UNIX signals
 * as Qt signals to provide Ladish L1 support.
 *
 * @return True if installation succeeded.
*/
    bool installSignalHandlers();
/*!
* @brief This function allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml Reference to QXmlStreamReader containing the open XML stream
*/
    void skipXmlElement(QXmlStreamReader& xml);


  protected:
/*!
* @brief Handler for close events either by quit or if the user closes
* the window.
*
*/
    void closeEvent(QCloseEvent*);

/* PUBLIC MEMBERS */
  public:
/*!
* @param p_portCount Number of registered MIDI output ports
*/
    MainWindow(int p_portCount, bool p_alsamidi);
    ~MainWindow();
/*!
* @brief This function opens a QMidiArp XML session file for reading
* using QXmlStreamReader.
*
* It queries XML block elements and calls the block readers
* MainWindow::readFilePartGlobal, MainWindow::readFilePartModules,
* MainWindow::readFilePartGUI. It sets MainWindow::lastDir according to
* the file path given with fn and calls MainWindow::updateWindowTitle.
* It updates MainWindow::recentFiles list.
*
* @param fn File name to open including its absolute path
*/
    void openFile(const QString&);

/* SIGNALS */
  signals:
    void newTempo(int);

/* PUBLIC SLOTS */
  public slots:
/*!
* @brief Slot for "New..." UI entries. This function calls MainWindow::clear
* and empties the current MainWindow::filename.
*/
    void fileNew();
/*!
* @brief Slot for "Open..." UI entries. This function calls MainWindow::isSave
*  and MainWindow::chooseFile if all changes are in saved state.
*/
    void fileOpen();
/*!
* @brief Slot for file Save GUI elements. This function calls either
* MainWindow::saveFileAs
* or MainWindow::saveFile depending on whether the MainWindow::filename is set.
*
*/
    void fileSave();
/*!
* @brief Slot for file SaveAs GUI elements. This function calls saveFileAs.
*
*/
    void fileSaveAs();
/*!
* @brief Slot for "Add Arpeggiator" menu entry and toolbutton.
*
* It asks for
* the module name and calls MainWindow::addArp with that name.
*/
    void arpNew();
/*!
* @brief Slot for "Add LFO" menu entry and toolbutton.
*
* It asks for
* the module name and calls MainWindow::addLfo with that name.
*/
    void lfoNew();
/*!
* @brief Slot for "Add Sequencer" menu entry and toolbutton.
*
* It asks for
* the module name and calls MainWindow::addSeq with that name.
*/
    void seqNew();
/*!
* @brief This function renames the TitleBar of a DockWidget
* with the passed name
*
* @param name New name attribute of the DockWidget
* @param index The widget ID of the DockWidget to rename
*/
    void renameDock(const QString& name, int index);
/*!
* @brief This function removes and deletes an Arpeggiator module.
*
* It removes all components MidiArp, ArpWidget and
* DockWidget from the corresponding lists in Engine.
*
* @param index The Engine::midiArpList index of the arpeggiator to remove
*/
    void removeArp(int index);
/*!
* @brief This function removes and deletes an LFO module.
*
* It removes all components MidiLfo, LfoWidget and
* DockWidget from the corresponding lists in Engine.
*
* @param index The Engine::midiLfoList index of the LFO to
*/
    void removeLfo(int index);
/*!
* @brief This function removes and deletes a Seq module.
*
* It removes all components MidiSeq, SeqWidget and
* DockWidget from the corresponding lists in Engine.
*
* @param index The Engine::MidiSeqList index of the sequencer to remove
*/
    void removeSeq(int index);
/*!
* @brief This function duplicates and adds a MidiLfo to the Engine.
*
* @param ID List ID of the module to copy
*/
    void cloneLfo(int ID);
/*!
* @brief This function duplicates and adds a MidiSeq to the Engine.
*
* @param ID List ID of the module to copy
*/
    void cloneSeq(int ID);

    void helpAbout();
    void helpAboutQt();
/*! @brief Slot for tempo spinBox changes.
* This function forwards a new tempo value to SeqDriver.
* @param tempo The new tempo to be set
*
*/
    void updateTempo(int tempo);
/*! @brief Slot for runQueue ToolButton.
* This function emits the MainWindow::runQueue signal with the state passed by the caller.
* @param on True to set Queue to running state or to restart
*
*/
    void updateTransportStatus(bool on);
/*! @brief Slot for midiClock ToolButton.
* This function toggles SeqDriver between MIDI Clock and internal clock
* operation.
* @param on True sets SeqDriver to MIDI Clock operation, false returns to
* internal clock.
*
*/
    void midiClockToggle(bool on);
/*! @brief Slot for jackSync ToolButton.
* This function toggles SeqDriver between Jack Transport and internal
* clock operation.
* @param on True sets driver to JACK Transport operation, false
* returns to internal clock.
*
* @see jackSyncToggle
*/
    void jackSyncToggle(bool on);
/*! @brief Slot for the JackDriver::j_shutdown() signal.
* This function switches the sync to internal clock operation and
* deactivates the Jack Transport Action.
* @see jackSyncToggle
*/
    void jackShutdown();
/*! @brief Slot for "Midi Controllers" menu action. This function displays
 * the MidiCC Dialog window.
*/
    void showMidiCCDialog();
/*!
* @brief This function appends or deletes an Arp pattern preset.
*
* If index = 0, it appends an Arp pattern. If index > 0 it deletes
* the Arp pattern at index from the list.
* It deploys the new pattern preset list to all Arp modules by calling
* Engine::updatePatternPresets and writes
* the new preset list to the resource file.
* This function is a signal slot for ArpWidget::presetsChanged and
* called whenever the preset list is modified in one Arp module.
*
* @param n Name of the preset to append if index = 0
* @param p Text sequence of te preset to append if index = 0
* @param index List index of the preset to delete or 0 to append a preset
* @see ArpWidget::presetsChanged, Engine::updatePatternPresets
*/
    void updatePatternPresets(const QString& n, const QString& p, int index);
/*! @brief Slot for fileRecentlyOpenedFiles.
* This function proceeds to open the file selected in the recent
* files menu.
*
* @see recentFileActivated, setupRecentFilesMenu
*/
    void recentFileActivated(QAction*);
/*!
* @brief This function populates the recent files menu.
*
* It is called by the constructor MainWindow::MainWindow
* @see recentFileActivated, addRecentlyOpenedFile
*/
    void setupRecentFilesMenu();
/*! @brief Slot to give response to an incoming pipe message (Ladish L1).
 *
 * This function calls fileSave upon reception of SIGUSR1 and close upon
 * reception of SIGINT.
 * @param fd UNIX signal number
*/
    void signalAction(int);

/*! @brief Slot to give response to an incoming Jack Session event.
 *
 * @param ev Type of the event (internal to QMidiarp)
*/
    void jsAction(int ev);
    void ctb_update_orientation(Qt::Orientation orient);
    void ftb_update_orientation(Qt::Orientation orient);
};

#endif
