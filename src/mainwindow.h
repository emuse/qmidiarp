/*!
 * @file mainwindow.h
 * @brief Member definitions for the MainWindow top-level UI class.
 *
 *
 *      Copyright 2009 - 2023 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMainWindow>
#include <QToolBar>

#include "logwidget.h"
#include "midicctable.h"
#include "prefswidget.h"
#include "globstore.h"
#include "prefs.h"

#ifdef NSM
#include "nsm.h"
#endif


static const char ABOUTMSG[] =
            "<html> <p><b><big>" APP_NAME " " PACKAGE_VERSION "</big></b></p>"
            "<p>(C) 2009 - 2023 Frank Kober<br/>"
            "(C) 2011 Nedko Arnaudov<br/>"
            "(C) 2009 Guido Scholz<br/>"
            "(C) 2002-2003 Matthias Nagorni (SuSE AG Nuremberg)<br/></p>"
            "<p><b>Contributions</b><br/>"
            "Roy Vegard Ovesen (work on nsm support)<br/>"
            "Matthew McGuire (LFO phase setting)<br/></p>"
            "<p><b>Translations</b><br/>"
            "Pavel Fric<br/>"
            "Pedro Lopez-Cabanillas<br/>"
            "Robert Dietrich<br/></p>"
            "<p>For getting support please type <b>man qmidiarp</b> or go to<br/>"
            "<a href=\"http://qmidiarp.sourceforge.net\">"
            "http://qmidiarp.sourceforge.net</a></p>"
            APP_NAME " is licensed under the GPLv2.</b></p></html>";

/*!
 * The MainWindow class is the main UI that holds functions to manage global
 * QMidiArp parameters and modules and to load and save parameters to
 * disk. The constructor sets up all main window elements including
 * toolbars and menus. It instantiates the LogWidget, PrefsWidget,
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
    PrefsWidget *prefsWidget;
    GrooveWidget *grooveWidget;
    LogWidget *logWidget;
    GlobStore *globStore;
    Engine *engine;
    MidiCCTable *midiCCTable;
    Prefs *prefs;
    QString lastDir, filename;
    QStringList patternNames, patternPresets;
    QStringList recentFiles;
    QDockWidget *logWindow, *grooveWindow, *passWindow, *globStoreWindow;

    QToolBar *controlToolBar, *fileToolBar;
    QAction *runAction, *addArpAction;
    QAction *addLfoAction, *addSeqAction;
    QAction *fileNewAction, *fileOpenAction, *fileSaveAction, *fileSaveAsAction;
    QAction *fileQuitAction;
    QAction *midiClockAction, *jackSyncAction;
    QAction *showAllIOAction, *hideAllIOAction;
    QMenu* fileRecentlyOpenedFiles;

#ifdef NSM
    static nsm_client_t *nsm;
    QString configFile;
#endif
/*!
* @brief  opens a file dialog and calls MainWindow::openFile().
* It is called by MainWindow::fileOpen().
*/
    void chooseFile();
/*! @brief  checks whether parameter modifications
 * were done after the last save.
 *
 * If yes, it queries the user how to handle unsaved changes using a
 * message box.
 * @return True if the parameters can be overwritten
 */
    bool isSave();
    void updateWindowTitle();
/*!
* @brief  opens a QMidiArp XML session file named
* MainWindow::filename for write using QXmlStreamReader.
*
* It writes global and GUI parameters and and calls the
* block writers in the module widgets.
*
* @return True if write was successful
*/
    bool saveFile();
/*! @brief  opens a file dialog and appends the file extension to
 * the chosen name if not present. It is called by fileSaveAs.
 * @return True if a file name was successfully chosen
 */
    bool saveFileAs();
/*!
* @brief  returns the result of Engine::isModified
* @return True if unsaved parameter modifications exist in any module
*
*/
    bool isModified();

    void addModule(ModuleWidget *moduleWidget, MidiWorker *midiWorker, bool fromfile, 
                    ModuleWidget *clonefrom = nullptr);

/*!
* @brief  creates and adds a new MidiArp to Engine.
*
* It also creates and adds the associated ArpWidget
* and DockWidget to the corresponding lists in Engine. It sets
* the ManageBox::name and ManageBox::ID as the current count of the
* Engine::midiArpList.
*
* @param name Name attribute of the created arpeggiator module
* @param fromfile Set to True if module is added by a file read
* @param inOutVisible Set to True if In-Out panel should be shown
*/
    void addArp(const QString& name, bool fromfile = false,
            ModuleWidget *clonefrom = nullptr, bool inOutVisible = true);
/*!
* @brief  creates and adds a new MidiLfo to Engine.
*
* It also creates and adds the associated LfoWidget
* and DockWidget to the corresponding lists in Engine. It sets
* the ManageBox::name and ManageBox::ID as the current count of the
* Engine::midiLfoList.
*
* @param name Name attribute of this LFO module
* @param fromfile Set to True if module is added by a file read
* @param clonefrom Set to the ID to clone this module from,
* -1 for a new module (default)
* @param inOutVisible Set to True if In-Out panel should be shown
*/
    void addLfo(const QString& name, bool fromfile = false,
                ModuleWidget *clonefrom = nullptr, bool inOutVisible = true);
/*!
* @brief  creates and adds a new MidiSeq to Engine.
*
* It also creates and adds the associated SeqWidget
* and DockWidget to the corresponding lists in Engine. It sets
* the ManageBox::name and ManageBox::ID as the current count of the
* Engine::midiSeqList.
*
* @param name Name attribute of this module
* @param fromfile Set to True if module is added by a file read
* @param clonefrom Set to the ID to clone this module from,
* -1 for a new module (default)
* @param inOutVisible Set to True if In-Out panel should be shown
*/
    void addSeq(const QString& name, bool fromfile = false,
                ModuleWidget *clonefrom = nullptr, bool inOutVisible = true);
/*!
* @brief  wraps the given widget in a QDockWidget and adds
* it to the list in Engine.
*
* @param *moduleWidget The QWidget to be embedded
* @param count DockWidget list location at which the window is insertet
*/
    void appendDock(ModuleWidget *moduleWidget, int count);
/*!
* @brief  reads global parameter block from an XML session
* stream using the QXmlStreamReader passed by the caller.
*
* @param xml Reference to QXmlStreamReader containing the open XML stream
*/
    void readFilePartGlobal(QXmlStreamReader& xml);
/*!
* @brief  reads the module parameter blocks from the XML stream
*  by calling their read functions.
*
* It uses the first three letters of the module name to distinguish their
* type. It creates the according module components and calls their UI
* widgets, which fill them with the parameters found in the same stream.
* Calls Engine->NNNWidget->readData, where NNN is Arp, Lfo or Seq.
*
* @param xml Reference to QXmlStreamReader containing the open XML stream
*/
    void readFilePartModules(QXmlStreamReader& xml, const QString& qmaxVersion);
/*!
* @brief  reads the GUI settings block
* from the XML session stream passed by the caller.
*
* @param xml Reference to QXmlStreamReader containing the XML stream
*/
    void readFilePartGUI(QXmlStreamReader& xml);
/*!
* @brief  prepends a filename at the beginning of the recently
* opened files list.
*
* It is called by openFile if the opening was successful.
* @param fn Filename with full path to be prepended
* @param lst The list of recently opened files
* @see setupRecentFilesMenu
*/
    void addRecentlyOpenedFile(const QString &fn, QStringList &lst);
/*!
* @brief  checks whether a .qmidiarprc file is present
* and creates it with default settings, if not.
* @return True if the file existed, False if it was created now.
* @see readRcFile, updatePatternPresets
*
*/
    bool checkRcFile();
/*!
* @brief  writes the .qmidiarprc text resource file.
*
* It is called on program exit and upon modification of the
* Arp preset list.
* @see readRcFile, updatePatternPresets
*/
    void writeRcFile();
/*!
* @brief  reads all elements from the .qmidiarprc text
* resource file.
*
* The file contains the Arp preset patterns, the last
* GUI state, settings made in the Settings dialog (PrefsWidget) and
* the recent files and path.
* This function is called from the MainWindow constructor.
* @see readRcFile, updatePatternPresets
*/
    void readRcFile();
/*!
* @brief  checks if there are no more modules present and sets
* some GUI elements accordingly if so.
*
* It is called by removeArp, removeSeq and removeLfo.
* @see checkIfFirstModule
*/
    void checkIfLastModule();
/*!
* @brief  checks if there were no modules present, i.e.
* if the module we just created is the first one, and sets some GUI
* elements accordingly if so.
*
* It is called by addArp, addSeq and addLfo.
* @see checkIfLastModule
*/
    void checkIfFirstModule();
/*!
* @brief  removes and deletes all modules from the
* lists.
*
* It removes all module components, i.e. the Midi workers, UI widgets
* and DockWidgets. It disconnects jack transport and stops the
* transport if running.
*/
    void clear();
/*!
* @brief  disables or enables GUI elements depending on
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

/*! @brief Handler for system signals (SIGUSR1, SIGINT...).
 * This function writes a message to the pipe and leaves as soon as possible
 */
    static void handleSignal(int);
/*! @brief  sets up a QSocketNotifier forwarding UNIX signals
 * as Qt signals to provide Ladish L1 support.
 *
 * @return True if installation succeeded.
*/
    bool installSignalHandlers();
/*!
* @brief allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It also advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml reference to QXmlStreamReader containing the open XML stream
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
* @param p_alsamidi Start as ALSA MIDI client
* @param *execName Name of the application's executable
*/
    MainWindow(int p_portCount, bool p_alsamidi, char *execName);
    ~MainWindow();

    bool jackFailed;

/* SIGNALS */
  signals:
    void newTempo(int);
#ifdef NSM
    void nsmOpenFile(const QString & name);
#endif

/* PUBLIC SLOTS */
  public slots:
/*!
* @brief Slot for "New..." UI entries.
*
* This function calls MainWindow::clear
* and empties the current MainWindow::filename.
*/
    void fileNew();
/*!
* @brief Slot for "Open..." UI entries.
*
* This function calls MainWindow::isSave
*  and MainWindow::chooseFile if all changes are in saved state.
*/
    void fileOpen();
/*!
* @brief  opens a QMidiArp XML session file for reading
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
/*!
* @brief Slot for file Save GUI elements.
*
* This function calls either
* MainWindow::saveFileAs
* or MainWindow::saveFile depending on whether the MainWindow::filename is set.
*
*/
    void fileSave();
/*!
* @brief Slot for file SaveAs GUI elements.
*
* This function calls saveFileAs.
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
* @brief  removes and deletes a Seq module.
*
* It removes all components MidiSeq, SeqWidget and
* DockWidget from the corresponding lists in Engine.
*
* @param index The Engine::midiSeqList index of the sequencer to remove
*/
    void removeModule();
/*!
* @brief  duplicates and adds a MidiLfo to the Engine.
*
* @param ID List ID of the module to copy
*/
    void cloneModule();

    void helpAbout();
    void helpAboutQt();
/*! @brief Slot for tempo spinBox changes.
* This function forwards a new tempo value to the driver.
* @param tempo The new tempo to be set
*
*/
    void updateTempo(int tempo);
/*! @brief Slot for Engine::tempoUpdated() signal.
*
* This function displays a new tempo value in the tempo spin box.
* @param p_tempo The new tempo to be displayed
*
*/
    void displayTempo(double p_tempo);
/*! @brief Slot for MainWindow::runAction ToolButton.
* This function calls Engine::setStatus() and disables the
* MainWindow::tempoSpin box
* @param on True to set Transport to running state
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
* This function toggles the driver between Jack Transport and internal
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
    void showIO();
    void hideIO();
/*!
* @brief  appends or deletes an Arp pattern preset.
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
* @brief  populates the recent files menu.
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

#ifdef NSM
    static int cb_nsm_open(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata);
    static int cb_nsm_save(char **out_msg, void *userdata);

    int nsm_open(const char *name, const char *display_name, const char *client_id, char **out_msg);
    int nsm_save(char **out_msg);
#endif
};

#endif
