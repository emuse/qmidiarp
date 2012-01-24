/*!
 * @file globstore.h
 * @brief Headers for the GlobStore UI class.
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
#ifndef GLOBSTORE_H
#define GLOBSTORE_H

#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QList>
#include <QSignalMapper>
#include <QString>

#include "indicator.h"

/*!
 * The GlobStore class is a small QGroupBox UI that allows storing
 * and restoring global parameters of all modules for QMidiArp.
 * It is instantiated by MainWindow.
 * It is shown in the bottom part of the MainWindow and is the central
 * widget for it.

 * @brief Global Parameter Storage UI. Instantiated by MainWindow.
 */
class GlobStore : public QGroupBox

{
  Q_OBJECT

  private:
    QSignalMapper *storeSignalMapper;
    QSignalMapper *restoreSignalMapper;
    QHBoxLayout* rowLayout;
    int activeStore;
    int currentRequest;

  public:
    GlobStore(QWidget* parent=0);
    ~GlobStore();

    QComboBox *timeModuleBox;
    QComboBox *timeModeBox;
    QComboBox *switchAtBeatBox;
    Indicator *indicator;
    QList<QWidget*> widgetList;
    int switchAtBeat; /**< number of beats after which parameter restore is done in Engine */

/*!
* @brief creates and adds a new group of global parameter
*  storage and retrieval buttons
*/
    void add();

  signals:

/*!
* @brief emitted to Engine::globStore(ix)
*
* @param ix ParStore::list index at which all module parameters are to be stored
*/
  void globStore(int);
/*!
* @brief emitted to Engine::requestGlobRestore(int)
*
* Causes all modules to restore their parameters from their
* ParStore::list at index ix
*
* @param ix Index at which the group is removed, if ix is -1, the last
* group is removed
*/
  void requestGlobRestore(int);
/*!
* @brief emitted to Engine, which will make the
* module at index ix trigger global store switches when its cursor
* reaches the end.
*
* @param name Name of the module to become time switch trigger
*/
  void updateGlobRestoreTimeModule(const QString&);
  void removeParStores(int);

  public slots:
/*!
* @brief removes the group of global parameter
*  storage and retrieval buttons at index ix
*
* @param ix Index at which the group is removed, if ix is -1, the last
* group is removed
*/
    void remove(int ix = -1);
/*!
* @brief causes all module widgets to store their current
* parameters in a global parameter store list setup in each module widget.
*
* @param ix Index in the module store lists
*/
    void store(int ix);
/*!
* @brief causes all module widgets to set their parameters
* to those found at index ix in their global parameter store lists
*
* @param ix Index in the module store lists
*/
    void restore(int ix);
/*!
* @brief emits the GlobStore::updateGlobRestoreTimeModule() signal to
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
* GlobStore::switchAtBeatBox).
*
* @param ix set to 0 for switching at end of modules, 1 for switching after
* a fixed number of beats
*/
    void updateTimeModeBox(int ix);
/*!
* @brief slot for the SwitchAtBeatBox ComboBox
*
* Updates the GlobRestore::switchAtBeat attribute.
*
* @param ix an integer number of beats after which restoring is done in
* Engine
*/
    void updateSwitchAtBeat(int ix);
/*!
* @brief handles the GlobStore button colors as a function
* of selection state.
*
* It attributes yellow color to the button at index ix if selected is 2
* and green color if selected is 1. It will remove color attributes from
* the remaining buttons.
*
* @param ix Index of the storage button to act on
* @param selected Color state to attribute to button ix, 1 = green, 2 = yellow
*/
    void setDispState(int ix, int selected);
};

#endif
