/*!
 * @file lfowidget.h
 * @brief Member definitions for the LfoWidget GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2013 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef LFOWIDGET_H
#define LFOWIDGET_H

#include <QAction>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QPushButton>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "globstore.h"
#include "midilfo.h"
#include "slider.h"
#include "lfoscreen.h"
#include "midicontrol.h"
#include "managebox.h"
#include "parstore.h"
#include "cursor.h"

/*! This array holds the currently available frequency values.
 */
const int lfoFreqValues[14] = {1, 2, 4, 8, 16, 24, 32, 64, 96, 128, 160, 192, 224, 256};

/*! @brief This array holds the currently available LFO size values.
 */
const int lfoSizeValues[12] = {1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 24, 32};

/*!
 * @brief GUI class associated with and controlling a MidiLfo worker

 * It controls the MidiLfo LFO and
 * is created alongwith each MidiLfo and embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData member. It manages a LfoWidget::ccList
 * for each
 * instance for MIDI controllers attributed through the MIDILearn
 * context menu. It instantiates an LfoScreen and interacts with it.
 *
 *
*/
class LfoWidget : public QWidget
{
    Q_OBJECT

    MidiLfo *midiWorker;
    GlobStore *globStore;
    bool modified;              /**< Is set to True if unsaved parameter modifications exist */
    bool dataChanged;
    bool needsGUIUpdate;
/*!
* @brief This function populates the LfoWidget::waveForms list with
* waveform names.
*
* @par Currently they are
*   - 0 Sine
*   - 1 Saw up
*   - 2 Triangle
*   - 3 Saw Down
*   - 4 Square
*   - 5 Custom
*
*/
    void loadWaveForms();

/* PUBLIC MEMBERS */
  public:
/*!
 * @brief Constructor for LfoWidget. It creates the GUI and an LfoScreen
 * instance.
 *
 * @param p_midiWorker Associated MidiLfo Object
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param parent The parent widget of this module, i.e. MainWindow
 */
    LfoWidget(MidiLfo *p_midiWorker, GlobStore *p_globStore,
            int portCount, bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true,
            const QString& name = "", QWidget* parent=0);
    ~LfoWidget();

    QVector<Sample> data;
    ParStore *parStore;
    MidiControl *midiControl;
    LfoScreen *screen;
    Cursor *cursor;
    ManageBox *manageBox;

    QStringList waveForms;
    QComboBox *chIn;
    QSpinBox  *ccnumberInBox;
    QSpinBox  *ccnumberBox;
    QComboBox *channelOut, *portOut;
    QComboBox *resBox, *sizeBox;
    QComboBox *loopBox;
    QWidget *inOutBox;
    QCheckBox *enableNoteOff;
    QCheckBox *enableRestartByKbd;
    QCheckBox *enableTrigByKbd;
    QCheckBox *enableTrigLegato;
    Slider *frequency, *amplitude, *offset;
    QAction *recordAction;
    QAction* hideInOutBoxAction;
    QAction *muteOutAction;
    QAction *deferChangesAction;
    QToolButton *muteOut;
    QComboBox *waveFormBox, *freqBox;

    void setChIn(int value);
    QVector<Sample> getCustomWave();
    int resBoxIndex;
    int sizeBoxIndex;
    int freqBoxIndex;
    int waveFormBoxIndex;

/*!
* @brief This function returns the MidiLfo instance associated with this GUI
* Widget.
* @return MidiLfo instance associated with this GUI
*/
    MidiLfo *getMidiWorker();

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
/*!
* @brief This function copies all LFO module GUI parameters from
* fromWidget
*
* @param fromWidget pointer to the LfoWidget parameters are to be taken from
*/
    void copyParamsFrom(LfoWidget *fromWidget);
/*!
* @brief Settor for the LfoWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void setChannelOut(int value);
/*!
* @brief Settor for the LfoWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void setPortOut(int value);
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
    void skipXmlElement(QXmlStreamReader& xml);

/* SIGNALS */
  signals:
/*! @brief Currently not in use. */
    void patternChanged();
/*! @brief Is re-emitted when received from screen and no MidiWorker present. */
    void mouseSig(double, double, int, int);

/* PUBLIC SLOTS */
  public slots:
    void updateChIn(int value);
    void updateCcnumberIn(int value);
    void updateScreen(int value);
    void setRecord(bool on);
    void updateEnableNoteOff(bool on);
    void updateEnableRestartByKbd(bool on);
    void updateEnableTrigByKbd(bool on);
    void updateTrigLegato(bool on);
    void setInOutBoxVisible(bool on);

    void updateDisplay();

/*!
* @brief Slot for the LfoWidget::waveFormBox combobox setting the waveform
* of this module.
* @param val Waveform index to choose as present in LfoWidget::loadWaveForms.
*
*/
    void updateWaveForm(int);
/*!
* @brief Slot for the LfoWidget::resBox combobox. Sets the resolution
* of the LFO.
*
* It sets MidiLfo::res and updates the LfoScreen of this module.
* @param val Resolution index from lfoResValues to set.
*
*/
    void updateRes(int);
/*!
* @brief Slot for the LfoWidget::sizeBox combobox. Sets the waveform size
* of the LFO.
*
* It sets MidiLfo::size and updates the LfoScreen of this module.
* @param val Size (number of bars) of the waveform.
*
*/
    void updateSize(int);
/*!
* @brief Slot for the LfoWidget::loopBox combobox. Sets the loop mode
* of the LFO.
*
* It sets MidiLfo::reverse, MidiLfo::pingpong and MidiLfo::enableLoop
* @param val Combination index ranging from 0 to 5
*
*/
    void updateLoop(int);
/*!
* @brief Slot for the LfoWidget::ccnumberBox spinbox setting the output
* controller CC number of this module.
* @param val CC number to send data to
*
*/
    void updateCcnumber(int val);
/*!
* @brief Slot for the LfoWidget::freqBox combobox. Sets the frequency
* of the LFO.
*
* It sets MidiLfo::freq for this and updates the LfoScreen of this module.
* @param val Frequency index from lfoFreqValues to set.
*
*/
    void updateFreq(int val);
/*!
* @brief Slot for the LfoWidget::amplitude slider. Sets the amplitude
* of the waveform for this LFO.
*
* It sets MidiLfo::amp and updates the LfoScreen of this module.
* @param val Amplitude (0 ... 127) of the calculated waveform.
*
*/
    void updateAmp(int val);
/*!
* @brief Slot for the LfoWidget::offset slider. Sets the offset
* of the waveform for this LFO.
*
* This function updates the LfoScreen of this module.
* @param val Offset (0 ... 127) of the waveform.
*
*/
    void updateOffs(int val);

/*!
* @brief Slot for the LfoScreen::mouseMoved signal. This function
* mutes or sets a wave point when the mouse is moved with held buttons.
*
* The mouse events are generated by the LfoScreen.
* It sets a MidiLfo::setCustomWavePoint and calls LfoWidget::newCustomOffset
* if the left button is held while moving. It sets a MidiLfo::setMutePoint
* if the right button is held while moving.
*
* @param mouseX Normalized mouse position on LfoScreen in X
* direction (0.0 ... 1.0)
* @param mouseY Normalized mouse position on LfoScreen in Y
* direction (0.0 ... 1.0)
* @param buttons 1 for left mouse button, 2 for right mouse button
* @param pressed 0 for mouse moved, 1 for mouse pressed, 2 for mouse released
*
*/
    void mouseEvent(double, double, int, int pressed);
/*!
* @brief Slot for the LfoScreen::mouseWheel signal. This function changes the
* offset by the steps the mouse wheel was turned.
*
* The mouse events are generated by the LfoScreen.
* @param step number of steps and direction the offset will be changed by
*
*/
    void mouseWheel(int);
/*!
* @brief Slot currently not in use.
*
* This function calls the midi worker
* MidiLfo::copyToCustom, which copies the current waveform to the custom
* waveform buffer
*
* It switches the waveForm combobox to index 5 and calls
* LfoWidget::updateWaveForm.
*
*/
    void copyToCustom();

/*!
* @brief Slot for the LfoWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void updateChannelOut(int value);
/*!
* @brief Slot for the LfoWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void updatePortOut(int value);
/*!
* @brief Slot for the LfoWidget::muteOut checkbox.
* This function suppresses output of LFO data.
*
* It calls
* MidiLfo::setMuted and LfoScreen::setMuted
* @param on Set to True for muting this module
*
*/
    void setMuted(bool on);
/*!
* @brief Slot for the LfoWidget::deferChanges action.
*
* Sets a flag in the midi worker causing parameter changes to become
* active/inactive only at pattern end.
*
* @param on Set to True for deferring parameter changes to pattern end
*/
    void updateDeferChanges(bool on);
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

    int getFramePtr() { return midiWorker->getFramePtr(); }
    int getNextTick() { return midiWorker->nextTick; }
    bool getReverse() { return midiWorker->reverse; }
    void handleController(int ccnumber, int channel, int value);
};

#endif
