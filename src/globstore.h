/*!
 * @file globstore.h
 * @brief Headers for the GlobStore UI class.
 *
 *
 *      Copyright 2009 - 2025 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef GLOBSTORE_H
#define GLOBSTORE_H

#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QList>
#include <QString>

#include "indicator.h"
#include "midicontrol.h"

/*!
 * The GlobStore class is a small QWidget UI that allows storing
 * and restoring global parameters of all modules for QMidiArp into
 * the ParStore::list. It creates a top row of controls and indicators
 * for each module as well as the first column of the storages. 
 * The storage locations themselves are handled by
 * ParStore. GlobStore is instantiated by MainWindow, and a pointer to
 * it is given to Engine and ParStore. 

 * @brief Global Parameter Storage UI. Instantiated by MainWindow.
 */
class GlobStore : public QWidget

{
  Q_OBJECT

  private:
    QSignalMapper *storeSignalMapper;
    int activeStore;
    int currentRequest;
    int schedRestoreVal;
    bool schedRestore;
    bool needsGUIUpdate;
    int dispReqIx, dispReqSelected;

    bool modified;

  public:
    GlobStore(QWidget* parent=0);
    ~GlobStore();
    MidiControl *midiControl;
    QComboBox *timeModuleBox;
    QComboBox *timeModeBox;
    QComboBox *switchAtBeatBox;
    QHBoxLayout* indivButtonLayout;
    Indicator *indicator;
    QList<QWidget*> widgetList;
    int switchAtBeat; /**< number of beats after which parameter restore is done in Engine */

/*!
 * @brief ENUM for Internal MIDI Control IDs supported 
 * by the GlobStore widget 
 */         
    enum GLOBSTORE_CTRL_IDS {
        GLOB_RESTORE = 0,
    };

/*!
* @brief creates and adds a new group of global parameter
*  storage and retrieval buttons
*/
    void addLocation();
/*!
* @brief adds a new module with name name to GlobStore
*
* @param name Name of the module to be added
*/
    void addModule(const QString& name);
/*!
* @brief removes the module with name name from GlobStore
*
* @param moduleWidgetIndex index in the Engine::moduleWidgetList of the module to remove
*/
    void removeModule(int moduleWidgetIndex);
/*!
* @brief reads all parameters of this Object from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief writes all parameters of this Object to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
    void handleController(int ccnumber, int channel, int value);
    bool isModified() { return modified;};
    void setModified(bool on) { modified = on; };
#ifdef APPBUILD
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
#endif

  signals:

/*!
* @brief emitted to Engine::store()

* Causes Engine to store all modules' parameters at their ParStore::list
* at index ix
*
* @param ix ParStore::list index at which all module parameters are to be stored
*/
  void store(int ix);
/*!
* @brief signel emitted to Engine::requestRestore(int)
*
* Causes Engine to restore all modules' parameters from their
* ParStore::list at index ix when the restore condition is given
*
* @param ix ParStore::list index from which the parameters are restored
*/
  void requestRestore(int ix);
/*!
* @brief signel emitted to Engine::updateGlobRestoreTimeModule()
* 
* Causes Engine to make the module at index windowIndex in the
* Engine::moduleWidgetList to trigger a global restore when its cursor
* reaches the end.
*
* @param windowIndex Engine::moduleWidgetList index of the module to become
* switch trigger
*/
  void updateGlobRestoreTimeModule(int windowIndex);
/*!
* @brief signal emitted to Engine::removeParStores(int)
*
* Causes Engine to remove one ParStore::list location from all modules
*
* @param ix ParStore::list index to be removed from all modules
*/
  void removeParStores(int ix);

  public slots:
/*!
* @brief removes the group of global parameter
*  storage and retrieval buttons at index ix
*
* @param ix Index at which the group is removed, if ix is -1, the last
* group is removed
*/
    void removeLocation(int ix = -1);
/*!
* @brief slot for the global save buttons
*
* Adds a storage location if needed and emits store() to Engine
*
* @param ix Index in the ParStore::list
*/
    void storeAll(int ix);
/*!
* @brief emits the updateGlobRestoreTimeModule() signal to
* Engine::updateGlobRestoreTimeModule()
*
* It determines the name of the module and transfers this to Engine by
* the signal
* @param ix Index in the module window store list, i.e. the dockWidget list
*/
    void updateTimeModule(int ix);
/*!
* @brief slot for the TimeModeBox ComboBox
*
* Updates the restoring condition type which can be 0 (restore at end of
* module patterns) or 1 (switch after a number of beats chosen by the
* switchAtBeatBox).
*
* @param ix set to 0 for switching at end of modules, 1 for switching after
* a fixed number of beats
*/
    void updateTimeModeBox(int ix);
/*!
* @brief slot for the SwitchAtBeatBox ComboBox
*
* Updates the switchAtBeat attribute.
*
* @param ix an integer number of beats after which restoring is done in
* Engine
*/
    void updateSwitchAtBeat(int ix);
/*!
* @brief handles the ParStore button colors as a function
* of selection state.
*
* It attributes blueish color to all buttons at index ix if selected is 2
* and green color if selected is 1. It will remove color attributes from
* the remaining buttons.
*
* @param ix Storage index of the storage button to act on
* @param selected Color state to attribute to the button, 1 = green, 2 = blueish
*/
    void setDispState(int ix, int selected);
/*!
* @brief will cause a flag to be set, which causes updateDisplay()
*  to call setDispState() at the next occasion.
*
* This function is used by handleController(), since setDispState()
* cannot be called directly from the realtime thread which sends the controller.
*
* @param ix Storage index of the storage button to act on
* @param selected Color state to attribute to the button, 1 = green, 2 = blueish
*/
    void requestDispState(int ix, int selected);
/*!
* @brief sets the color of location button row to the specified color index
*
* @param column ModuleWindow index of which button color is to be set
* @param row Location index at which button color is to be set
* @param color Location color (0: no color, 1: active color, 2: pending color)
*/
    void setBGColorAt(int column, int row, int color);
/*!
* @brief slot for each location's global restore button
*
* Sets the location index to restore from the caller widget "index" property and
* emits the requestRestore() signal to Engine.
*/
    void mapRestoreSignal();
/*!
* @brief slot for each location's global store button
*
* Sets the location index to restore from the caller widget "index" property and
* emits the store() signal to Engine.
*/
    void mapStoreSignal();
/*!
* @brief Called by the parent widget as part of the display MTimer event loop

* Sets the indicator position and handles storage requests as a function of the
* GUI requests and series parameters
*/
    void updateDisplay();
};

#endif
