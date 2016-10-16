/*!
 * @file arpwidget.h
 * @brief Member definitions for the ArpWidget GUI class.
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

#ifdef APPBUILD
#include "globstore.h"
#include "midicontrol.h"
#include "parstore.h"
#endif

#include "main.h"
#include "midiarp.h"
#include "inoutbox.h"
#include "slider.h"
#include "arpscreen.h"

/*! @brief GUI class associated with and controlling a MidiArp worker
 *
 * It controls the MidiArp arpeggiator, which is transferred as a pointer
 * when ArpWidget is instantiated. It is embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData() function. It instantiates an ArpScreen
 * and interacts with it.
class ArpWidget : public QWidget
*/
class ArpWidget :  public InOutBox
{
  Q_OBJECT

    MidiArp *midiWorker;
#ifdef APPBUILD
    GlobStore *globStore;
#endif
    bool modified;      /**< @brief Is set to True if unsaved parameter modifications exist */
    bool needsGUIUpdate;

    QGroupBox *randomBox, *envelopeBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QToolButton *latchModeButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;
    int patternPresetBoxIndex;

    void loadPatternPresets();
/* PUBLIC MEMBERS */
  public:
/*!
 * @brief Constructor for ArpWidget. It creates the GUI and an ArpScreen
 * instance.
 *
 * @param p_midiWorker Pointer to the associated MidiArp object
 * @param p_globStore Pointer to the GlobStore widget
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param inOutVisible If set to True, the module will show its In-Out panel
 * @param name Name string of the module
 * @param parent The parent widget of this module, i.e. MainWindow
 */
#ifdef APPBUILD
    ArpWidget(MidiArp *p_midiWorker, GlobStore *p_globStore,
            int portCount, bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true,
            const QString& name = "");

    ParStore *parStore;
    MidiControl *midiControl;
#else
    ArpWidget(
            bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true);
#endif
    ~ArpWidget();

    ArpScreen *screen;

    QComboBox *repeatPatternThroughChord;
    QComboBox *patternPresetBox;
    QComboBox *octaveModeBox;
    QComboBox *octaveLowBox;
    QComboBox *octaveHighBox;
    Slider *randomVelocity, *randomTick, *randomLength;
    Slider *attackTime, *releaseTime;
    QLineEdit *patternText;

    QStringList patternPresets, patternNames;
    QAction *muteOutAction;
    QAction *deferChangesAction;
    QToolButton *muteOut;
    QAction *latchModeAction;



#ifdef APPBUILD
    //these members are common to all modules
/*!
* @brief reads all parameters of this module from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief writes all parameters of this module to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief returns the MidiArp instance associated with this GUI
* Widget.
* @return MidiArp instance associated with this GUI
*/
    MidiArp *getMidiWorker();
/*!
* @brief Accessor for ArpWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief sets ArpWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);
/*!
* @brief stores some module parameters in a parameter
* list object
*
* @param Position index in the parameter list
*/
    void doStoreParams(int ix, bool empty);
/*!
* @brief restores some module parameters from the parameter
* list object
*
* @param Position index in the parameter list
*/
    void doRestoreParams(int ix);
#endif

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
    void updateOctaveMode(int value);
    void updateOctaveLow(int value);
    void updateOctaveHigh(int value);
    void updateRandomVelocityAmp(int value);
    void updateRandomTickAmp(int value);
    void updateRandomLengthAmp(int value);
    void updateAttackTime(int value);
    void updateReleaseTime(int value);
    void checkIfRandomSet();
    void checkIfEnvelopeSet();
    void updateText(const QString& newtext);
    void updateRepeatPattern(int);
    void selectPatternPreset(int);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void openTextEditWindow(bool on);
    void storeCurrentPattern();
    void removeCurrentPattern();
    void setRandomVisible(bool on);
    void setEnvelopeVisible(bool on);

    void updateChIn(int value);
    void updateEnableRestartByKbd(bool on);
    void updateEnableTrigByKbd(bool on);
    void updateTrigLegato(bool on);
    void updateIndexIn(int value);
    void updateRangeIn(int value);

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
* Suppresses output of Arp data.
*
* It calls
* MidiArp::setMuted and ArpScreen::setMuted
* @param on Set to True for muting this module
*
*/
    void setMuted(bool on);
/*!
* @brief Slot for the ArpWidget::deferChanges action.
*
* Sets a flag in the midi worker causing parameter changes to become
* active/inactive only at pattern end.
*
* @param on Set to True for deferring parameter changes to pattern end
*/
    void updateDeferChanges(bool on);

#ifdef APPBUILD
    void handleController(int ccnumber, int channel, int value);
/*!
 * @brief Updates the ArpScreen and other GUI elements with data from
 * the MidiSeq instance.
 *
 * It is called by Engine::updateDisplay(), which itself is
 * connected to the MTimer::timeout event. It runs in the MTimer thread.
 * It reads the waveform data and other settings from the MidiArp instance
 * and sets GUI cursor, wave display and other elements accordingly. This
 * way, no memory allocations are done within the jack run thread, for
 * instance by MIDI controllers, since the Qt widgets are not called directly.
 * This function also checks whether parameter chages from ParStore are
 * pending and causes them to get transferred if so.
 */
    void updateDisplay();
#endif

    int getGrooveIndex() { return midiWorker->getGrooveIndex(); }
};

#endif
