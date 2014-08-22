/*!
 * @file lfowidget.h
 * @brief Member definitions for the LfoWidget GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2014 <qmidiarp-devel@lists.sourceforge.net>
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

#ifdef APPBUILD
#include "globstore.h"
#include "midicontrol.h"
#include "managebox.h"
#include "parstore.h"
#endif

#include "midilfo.h"
#include "cursor.h"
#include "slider.h"
#include "lfoscreen.h"

/*!
 * @brief GUI class associated with and controlling a MidiLfo worker

 * It controls the MidiLfo, which is transferred as a pointer
 * when LfoWidget is instantiated. It is embedded in a DockWidget on
 * MainWindow level. It can read its parameter set from an XML stream
 * by calling its readData() function. It instantiates an LfoScreen
 * and interacts with it.
 *
 *
*/
class LfoWidget : public QWidget
{
    Q_OBJECT

    MidiLfo *midiWorker;
#ifdef APPBUILD
    GlobStore *globStore;
#endif
    bool modified;              /**< Is set to True if unsaved parameter modifications exist */
    bool dataChanged;
    bool needsGUIUpdate;
/*!
* @brief populates the LfoWidget::waveForms list with
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
#ifdef APPBUILD
/*!
 * @brief Constructor for LfoWidget. It creates the GUI and an LfoScreen
 * instance.
 *
 * @param *p_midiWorker Pointer to the associated MidiLfo object
 * @param *p_globStore Pointer to the GlobStore widget
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param inOutVisible If set to True, the module will show its In-Out panel
 * @param name Name string of the module
 * @param parent The parent widget of this module, i.e. MainWindow
 */
    LfoWidget(MidiLfo *p_midiWorker, GlobStore *p_globStore,
            int portCount, bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true,
            const QString& name = "", QWidget* parent=0);

    ParStore *parStore;
    MidiControl *midiControl;
    ManageBox *manageBox;
#else
    LfoWidget(
            bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true,
            QWidget* parent=0);
#endif

    ~LfoWidget();
    LfoScreen *screen;
    Cursor *cursor;
    QVector<Sample> data;
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
* @brief Setter for the LfoWidget::channelOut spinbox setting the output
* channel of this module.
* @param value Number of the output channel to send data to
*
*/
    void setChannelOut(int value);
/*!
* @brief Setter for the LfoWidget::portOut spinbox setting the output
* port of this module.
* @param value Number of the output port to send data to
*
*/
    void setPortOut(int value);

#ifdef APPBUILD
/*!
* @brief returns the MidiLfo instance associated with this GUI
* Widget.
* @return MidiLfo instance associated with this GUI
*/
    MidiLfo *getMidiWorker();

/*!
* @brief reads all parameters of this LFO from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief writes all parameters of this LFO to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief copies all LFO module GUI parameters from
* fromWidget
*
* @param fromWidget pointer to the LfoWidget parameters are to be taken from
*/
    void copyParamsFrom(LfoWidget *fromWidget);
/*!
* @brief Accessor for LfoWidget::modified.
* @return True if unsaved parameter modifications exist
*
*/
    bool isModified();
/*!
* @brief sets LfoWidget::modified.
* @param m Set to True if unsaved parameter modifications appear
*
*/
    void setModified(bool);
    void skipXmlElement(QXmlStreamReader& xml);
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

/* SIGNALS */
  signals:
/*! @brief Currently not in use. */
    void patternChanged();
/*! @brief Is re-emitted when a LfoScreen::mouseEvent() is received
 * and no pointer to a MidiWorker is present (LV2 build). */
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
* updates the LfoScreen of this module.
* @param val Offset (0 ... 127) of the waveform.
*
*/
    void updateOffs(int val);

/*!
* @brief Slot for the LfoScreen::mouseEvent signal.
* 
* Mutes or sets a wave point when the mouse is pressed or
* released or moved with held buttons.
* The mouse events are generated by the LfoScreen.
* It calls MidiLfo::mouseEvent()
* or emits the mouseSig() signal if no pointer to a MIDI worker was
* transferred (LV2 build).
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
* @brief Slot for the LfoScreen::mouseWheel signal. changes the
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
* calls the midi worker
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
* suppresses output of LFO data.
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

    void storeParams(int ix, bool empty = false);
    void restoreParams(int ix);

#ifdef APPBUILD
    void handleController(int ccnumber, int channel, int value);
/*!
 * @brief Updates the LfoScreen and other GUI elements with data from
 * the MidiSeq instance.
 *
 * It is called by Engine::updateDisplay(), which itself is
 * connected to the MTimer::timeout event. It runs in the MTimer thread.
 * It reads the waveform data and other settings from the MidiLfo instance
 * and sets GUI cursor, wave display and other elements accordingly. This
 * way, no memory allocations are done within the jack run thread, for
 * instance by MIDI controllers, since the Qt widgets are not called directly.
 * This function also checks whether parameter chages from ParStore are
 * pending and causes them to get transferred if so.
 */
    void updateDisplay();
#endif
    int getFramePtr() { return midiWorker->getFramePtr(); }
    int getNextTick() { return midiWorker->nextTick; }
    bool getReverse() { return midiWorker->reverse; }
};

#endif
