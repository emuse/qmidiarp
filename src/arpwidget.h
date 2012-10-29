/*!
 * @file arpwidget.h
 * @brief Member definitions for the ArpWidget GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef ARPWIDGET_H
#define ARPWIDGET_H

#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "midiarp.h"
#include "slider.h"
#include "arpscreen.h"
#include "midicontrol.h"
#include "managebox.h"
#include "parstore.h"

/*! @brief GUI class associated with and controlling a MidiArp worker
 *
 * It controls the MidiArp arpeggiator and
 * is created alongwith each MidiArp and embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData member. It manages a ArpWidget::ccList
 * for each
 * instance for MIDI controllers attributed through the MIDILearn
 * context menu. It instantiates a ArpScreen and interacts with it.
class ArpWidget : public QWidget
*/
class ArpWidget : public QWidget
{
  Q_OBJECT

  private:
    QComboBox *chIn;                        // Channel of input events
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QLabel *rangeInLabel, *indexInLabel;
    QComboBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QComboBox *repeatPatternThroughChord;
    QComboBox *triggerMode;
    QComboBox *patternPresetBox;
    QGroupBox *inputFilterBox, *randomBox, *envelopeBox;
    QWidget *inOutBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QToolButton *latchModeButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;
    QAction *latchModeAction;
    MidiArp *midiWorker;
    QLineEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
    Slider *attackTime, *releaseTime;
    bool modified;      /**< @brief Is set to True if unsaved parameter modifications exist */
    int patternPresetBoxIndex;

    void loadPatternPresets();
/* PUBLIC MEMBERS */
  public:
/*!
 * @brief Constructor for ArpWidget. It creates the GUI and an ArpScreen
 * instance.
 *
 * @param p_midiWorker Associated MidiArp Object
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param parent The parent widget of this module, i.e. MainWindow
 */
    ArpWidget(MidiArp *p_midiWorker, int portCount, bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true, QWidget* parent=0);
    ~ArpWidget();

    ParStore *parStore;
    MidiControl *midiControl;
    ArpScreen *screen;
    ManageBox *manageBox;

    QStringList patternPresets, patternNames;
    QPushButton *muteOut;
    QAction* hideInOutBoxAction;

    void setChIn(int value);
    void setIndexIn(int index, int value);
    void setRangeIn(int index, int value);

    //these members are common to all modules
/*!
* @brief This function reads all parameters of this module from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief This function writes all parameters of this module to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief This function stores some module parameters in a parameter
* list object
*
* @param Position index in the parameter list
*/
    void storeParams(int ix, bool empty = false);
/*!
* @brief This function restores some module parameters from the parameter
* list object
*
* @param Position index in the parameter list
*/
    void restoreParams(int ix);
/*!
* @brief This function returns the MidiArp instance associated with this GUI
* Widget.
* @return MidiArp instance associated with this GUI
*/
    MidiArp *getMidiWorker();
/*!
* @brief Settor for the ArpWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void setChannelOut(int value);
/*!
* @brief Settor for the ArpWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void setPortOut(int value);
/*!
* @brief Accessor for ArpWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief This function sets ArpWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);

    int getNextTick() { return midiWorker->nextTick; }

/* SIGNALS */
  signals:
/*! @brief Emitted to MainWindow::updatePatternPresets saving and deploying modified preset
 *  list.
 *  @param pname Name of the modified pattern
 *  @param pattern Text chain of the modified pattern
 *  @param index Set to the index of the pattern for removal, or to zero for appending a pattern
 * */
    void presetsChanged(const QString& pname, const QString& pattern, int index);

/* PUBLIC SLOTS */
  public slots:
    //these slots are specific for the Arp module
    void updateRandomVelocityAmp(int value);
    void updateRandomTickAmp(int value);
    void updateRandomLengthAmp(int value);
    void updateAttackTime(int value);
    void updateReleaseTime(int value);
    void checkIfRandomSet();
    void checkIfEnvelopeSet();
    void checkIfInputFilterSet();
    void updateText(const QString& newtext);
    void updateRepeatPattern(int);
    void updateTriggerMode(int);
    void selectPatternPreset(int);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void openTextEditWindow(bool on);
    void storeCurrentPattern();
    void removeCurrentPattern();
    void setRandomVisible(bool on);
    void setEnvelopeVisible(bool on);
    void setInputFilterVisible(bool on);
    void setInOutBoxVisible(bool on);

    void updateChIn(int value);
    void updateIndexIn(int value);
    void updateRangeIn(int value);

    void updateDisplay();

 /*! @brief Slot for ArpWidget::latchModeAction.
  * Will cause notes remaining in MidiArp::latchBuffer until new
  * stakato note received */
    void setLatchMode(bool);

    //these slots are common to all modules
/*!
 *  @brief Slot for the ArpWidget::channelOut spinbox setting the output
 *  channel of this module.
 *  @param value Number of the output channel to send data to
 *
 */
    void updateChannelOut(int value);
/*!
* @brief Slot for the ArpWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void updatePortOut(int value);
/*!
* @brief Slot for the ArpWidget::muteOut checkbox.
* This function suppresses output of Arp data.
*
* It calls
* MidiArp::setMuted and ArpScreen::setMuted
* @param on Set to True for muting this module
*
*/
    void setMuted(bool on);
    void handleController(int ccnumber, int channel, int value);
    int getGrooveIndex() { return midiWorker->getGrooveIndex(); }
};

#endif
