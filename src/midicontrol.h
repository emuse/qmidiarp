/**
 * @file midicontrol.h
 * @brief Member definitions for the MidiControl QWidget class.
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

#ifndef MIDICONTROL_H
#define MIDICONTROL_H

#include <QAction>
#include <QSignalMapper>
#include <QStringList>
#include <QVector>
#include <QXmlStreamWriter>

#include <cstdio>
#include "main.h"

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
 * @brief Manages the list of MIDI-controllable Widgets for a Module.
 *
 * MidiControl is instantiated by each of QMidiArp's module widgets.
 */
class MidiControl : public QObject

{
  Q_OBJECT

  private:
    QAction *cancelMidiLearnAction;
/*!
* @brief Forwards the MIDI-learn context menu action signal to the
* MidiControl::midiLearn() slot along with the ID of the
* controllable QWidget
*/
    QSignalMapper *learnSignalMapper;
/*!
* @brief Forwards the MIDI-forget context menu action signal to the
* MidiControl::midiForget() slot along with the ID of the
* controllable QWidget
*/
    QSignalMapper *forgetSignalMapper;
    bool newCCPending;
    bool modified;
    MidiCC pendingCC;

  public:
    MidiControl(QWidget *parent = 0);
    ~MidiControl();
    QStringList names;      /**< Contains a list of convenient names
                                    of controllable widgets */
    int ID;
    int parentDockID;
    QVector<MidiCC> ccList; /**< Contains MIDI controller - GUI element bindings */
/*!
* @brief Accessor for the modified flag.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief Sets the modified flag.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);
/*!
* @brief Creates and attributes a MIDI-Learn context menu to the passed QWidget.
*
* The menu entries are connected to MidiControl::learnSignalMapper and
* MidiControl::forgetSignalMapper for dispatching actions
*
* @param name Convenient name for attribution to the controllable QWidget
* @param widget QWidget to which the context menu is attributed
* @param count Internal identifier of the controllable QWidget
*/
    void addMidiLearnMenu(const QString &name, QWidget *widget, int count = 0);

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
/*! @brief Connected to Engine::setMidiLearn() to listen for incoming events.
*  @param parentdockid parentDockID of the module requesting MIDI learn
*  @param controlID ID of the GUI element to be assigned to the controller
*/
    void setMidiLearn(int parentdockid, int controlID);

  public slots:
/*!
* @brief Calls MidiControl::requestAppendMidiCC() and MidiControl::update()
* successively
*
* Before appending, it checks whether this binding already exists.
* @param controlID The ID of the control GUI element
* @param ccnumber The CC of the MIDI controller to be attributed
* @param channel The MIDI Channel of the MIDI controller to be attributed
* @param min The minimum value to which the controller range is mapped
* @param max The maximum value to which the controller range is mapped
*/
    void appendMidiCC(int controlID, int ccnumber, int channel, int min, int max);

/*!
* @brief Schedules appending of a new MIDI controller - GUI element
* binding to MidiControl::ccList.
*
* It will be appended on the call of MidiControl::update().
* Before appending, it checks whether this binding already exists.
* @param controlID The ID of the control GUI element
* @param ccnumber The CC of the MIDI controller to be attributed
* @param channel The MIDI Channel of the MIDI controller to be attributed
* @param min The minimum value to which the controller range is mapped
* @param max The maximum value to which the controller range is mapped
*/
    void requestAppendMidiCC(int controlID, int ccnumber, int channel, int min, int max);
/*!
* @brief Does the actual append of the pending controller scheduled by
* MidiControl::requestAppendMidiCC().
*/
    void update();
/*!
* @brief removes a MIDI controller - GUI element binding from the MidiControl::ccList.
*
* @param controlID The ID of the control GUI element
* @param ccnumber The CC of the MIDI controller to be removed
* @param channel The MIDI Channel of the MIDI controller to be removed
*/
    void removeMidiCC(int controlID, int ccnumber, int channel);
/*!
* @brief Slot for the MidiControl::learnSignalMapper signal forwarded from
* the MIDI-Learn context menu "MIDI Learn" actions.
*
* It emits the MidiControl::setMidiLearn() signal with the necessary
* module and GUI element information parameters to Engine::setMidiLearn().
* @param controlID The ID of the control GUI element
*/
    void midiLearn(int controlID);
/*!
* @brief Slot for the MidiControl::forgetSignalMapper signal forwarded from
* the MIDI-Learn context menu "MIDI Forget" actions.
*
* Removes a controller binding attribution by calling
* MidiControl::removeMidiCC().
*
* @param controlID The ID of the control GUI element
*/
    void midiForget(int controlID);
/*!
* @brief Slot for the MidiControl::cancelMidiLearnAction in MIDI-Learn
* context menu.
*
* Signals cancellation of the MIDI-Learn Process to
* Engine.
*
* It emits MidiControl::setMidiLearn() with controlID set to -1 meaning cancel.
*/
    void midiLearnCancel();
/*!
* @brief Reads all parameters of this Object from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief Writes all parameters of this Object to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief Copies the CC list from an existing one, used to copy
* params from in lfowidget and seqwidget
*
* @param p_ccList QVector<MidiCC> to copy from
*/
    void setCcList(const QVector<MidiCC> &p_ccList);
};
#endif
