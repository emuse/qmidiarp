/*!
 * @file managebox.h
 * @brief Headers for the ManageBox QWidget UI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef MANAGEBOX_H
#define MANAGEBOX_H

#include <QWidget>

/*!
 * The ManageBox class is a small QWidget container holding the three
 * buttons for deleting, renaming and duplicating module widgets.
 *
 * @brief Small QWidget container for module management buttons.
 * Instantiated by every module widget.
 */
class ManageBox : public QWidget

{
  Q_OBJECT

  private:
    QString namePrefix;
    QAction *deleteAction, *renameAction, *cloneAction;

  public:
    ManageBox(const QString & nameprefix);
    ~ManageBox();

    QString name;       /**< @brief The name of this Widget as shown in the DockWidget TitleBar */
    int ID;             /**< @brief Corresponds to the Engine::midi*List index of the associated MidiSeq */
    int parentDockID;   /**< @brief The index of the ArpWidget's parent DockWidget in Engine::moduleWindowList */


  signals:

/*! @brief Emitted to MainWindow::removeSeq for module deletion.
 *  @param ID The internal module ID of the module to remove
 *  */
    void moduleRemove(int ID);
/*! @brief Emitted to MainWindow::renameDock for module rename.
 *  @param mname New name of the module
 *  @param parentDockID parentDockID of the module to rename
 *  @param ID widgetID of the module to rename
 * */
    void dockRename(const QString& mname, int parentDockID, int ID);
/*! @brief Emitted to MainWindow::cloneSeq for module duplication.
 *  @param ID module ID of the module to clone
 * */
    void moduleClone(int ID);

  public slots:
/*!
* @brief Slot for ManageBox::deleteAction.
*
* This function displays a warning and then emits the
* ManageBox::moduleRemove signal to MainWindow with the module ID as
* parameter.
*/
    void moduleDelete();
/*!
* @brief Slot for ManageBox::renameAction.
*
* This function queries a new name then emits the ManageBox::dockRename
* signal to MainWindow with the new name and the dockWidget ID to rename.
*/
    void moduleRename();
/*!
* @brief Slot for ManageBox::cloneAction.
*
* This function emits the ManageBox::dockClone
* signal to MainWindow with the module ID and the dockWidget ID.
*/
    void moduleClone();
};

#endif
