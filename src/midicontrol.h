/**
 * @file midicontrol.h
 * @brief Member definitions for the MidiControl QWidget class.
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

#ifndef MIDICONTROL_H
#define MIDICONTROL_H

#include <QAction>
#include <QSignalMapper>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

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
 * A MidiControl is instantiated by each of QMidiArp's module widgets.
 */
class MidiControl : public QWidget

{
  Q_OBJECT

  private:
    QAction *cancelMidiLearnAction;
    QSignalMapper *learnSignalMapper, *forgetSignalMapper;
    QStringList names;
    bool modified;
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

  public:
    MidiControl(const QStringList &p_names);
    ~MidiControl();
    int ID;
    int parentDockID;
    QVector<MidiCC> ccList;     /**< Contains MIDI controller - GUI element bindings */
/*!
* @brief Accessor for LfoWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief This function sets LfoWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);
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

  signals:
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

  public slots:
/*!
* @brief This function appends a new MIDI controller - GUI element
* binding to LfoWidget::ccList.
*
* Before appending, it checks whether this binding already exists.
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
* @param ccnumber The CC of the MIDI controller to be attributed
* @param channel The MIDI Channel of the MIDI controller to be attributed
* @param min The minimum value to which the controller range is mapped
* @param max The maximum value to which the controller range is mapped
*/
    void appendMidiCC(int controlID, int ccnumber, int channel, int min, int max);
/*!
* @brief This function removes a MIDI controller - GUI element
* binding from the LfoWidget::ccList.
*
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
* @param ccnumber The CC of the MIDI controller to be removed
* @param channel The MIDI Channel of the MIDI controller to be removed
*/
    void removeMidiCC(int controlID, int ccnumber, int channel);
/*!
* @brief Slot for LfoWidget::triggered signal created by MIDI-Learn context
* menu MIDI Learn action.
*
* This function sets Engine into
* MIDI Learn status for this module and controlID.
* It emits LfoWidget::setMidiLearn with the necessary module and GUI element
* information parameters.
* Engine will then wait for an incoming controller event and trigger the
* attribution by calling appendMidiCC.
*
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
*/
    void midiLearn(int controlID);
/*!
* @brief Slot for LfoWidget::triggered signal created by a MIDI-Learn
* context menu Forget action.
*
* This function removes a controller
* binding attribution by calling removeMidiCC.
*
* @param controlID The ID of the control GUI element (found in
* LfoWidget::midiCCNames)
*/
    void midiForget(int controlID);
/*!
* @brief Slot for LfoWidget::cancelMidiLearnAction in MIDI-Learn context
* menu. This function signals cancellation of the MIDI-Learn Process to
* Engine.
*
* It emits LfoWidget::setMidiLearn with controlID set to -1 meaning cancel.
*/
    void midiLearnCancel();
/*!
* @brief This function reads all parameters of this LFO from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief This function writes all parameters of this LFO to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);

    void setCcList(const QVector<MidiCC> &p_ccList);
};
#endif
