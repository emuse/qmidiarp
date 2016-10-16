/*!
 * @file inoutbox.h
 * @brief Member definitions for the InOutBox GUI class.
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
 */

#ifndef INOUTBOX_H
#define INOUTBOX_H

#include <QString>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>

/*! @brief GUI class creating an input/output parameter box
 *
*/
class InOutBox: public QWidget
{
  Q_OBJECT
  
	public:
#ifdef APPBUILD
	InOutBox( int portCount, bool compactStyle,
		bool inOutVisible, const QString& name);
    QAction *deleteAction, *renameAction, *cloneAction;
    QString name;       /**< @brief The name of this Widget as shown in the DockWidget TitleBar */
    int ID;             /**< @brief Corresponds to the Engine::midi*List index of the associated MidiSeq */
    int parentDockID;   /**< @brief The index of the ArpWidget's parent DockWidget in Engine::moduleWindowList */
#else
	InOutBox(bool compactStyle,
		bool inOutVisible, const QString& name);
#endif
    bool modified;      /**< @brief Is set to True if unsaved parameter modifications exist */
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
	   
	QWidget *inOutBoxWidget;

    virtual void setChIn(int value);
    virtual void setIndexIn(int index, int value);
    virtual void setRangeIn(int index, int value);
    virtual bool isModified();
    virtual void setModified(bool);
    virtual void checkIfInputFilterSet();
/*!
* @brief Setter for the InOutBox::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/

    virtual void setChannelOut(int value);

#ifdef APPBUILD
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
* @param Position index in the parameter list
*/
    virtual void doStoreParams(int ix, bool empty) = 0;
/*!
* @brief restores some module parameters from the parameter
* list object
*
* @param Position index in the parameter list
*/
    virtual void doRestoreParams(int ix) = 0;
#endif
	
  public slots:
/*!
* @brief Slot for ManageBox::deleteAction.
*
* This function displays a warning and then emits the
* ManageBox::moduleRemove signal to MainWindow with the module ID as
* parameter.
*/
    virtual void moduleDelete();
/*!
* @brief Slot for ManageBox::renameAction.
*
* This function queries a new name then emits the ManageBox::dockRename
* signal to MainWindow with the new name and the dockWidget ID to rename.
*/
    virtual void moduleRename();
/*!
* @brief Slot for ManageBox::cloneAction.
*
* This function emits the ManageBox::dockClone
* signal to MainWindow with the module ID and the dockWidget ID.
*/
    virtual void moduleClone();

    virtual void setInputFilterVisible(bool on);
    
	virtual void storeParams(int ix, bool empty = 0);
	virtual void restoreParams(int ix);
	

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
};
#endif
