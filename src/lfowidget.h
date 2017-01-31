/*!
 * @file lfowidget.h
 * @brief Member definitions for the LfoWidget GUI class.
 *
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

#ifndef LFOWIDGET_H
#define LFOWIDGET_H

#include "midilfo.h"
#include "inoutbox.h"
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
class LfoWidget : public InOutBox
{
    Q_OBJECT

    MidiLfo *midiLfo;
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
 * @param p_midiLfo Associated MidiLfo Object
 * @param p_globStore The Application-wide globStore widget
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param inOutVisible Add the module with visible InOutBox or not
 * @param name The name of the module preceded by its type (Arp: , etc...)
 */
    LfoWidget(MidiLfo *p_midiLfo, GlobStore *p_globStore,
            int portCount, bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true,
            const QString& name = "");

#else
    LfoWidget(
            bool compactStyle,
            bool mutedAdd = false, bool inOutVisible = true);
#endif

    LfoScreen *screen;
    Cursor *cursor;
    QVector<Sample> data;
    QStringList waveForms;
    QComboBox *resBox, *sizeBox;
    QComboBox *loopBox;
    Slider *frequency, *amplitude, *offset;
    QAction *recordAction;
    QAction *flipWaveVerticalAction;
    QComboBox *waveFormBox, *freqBox;

    QVector<Sample> getCustomWave();
    QVector<bool> getMuteMask();
    int resBoxIndex;
    int sizeBoxIndex;
    int freqBoxIndex;
    int waveFormBoxIndex;


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

    void doStoreParams(int ix);
    void doRestoreParams(int ix);
    void updateDisplay();
    void handleController(int ccnumber, int channel, int value);
    void updateCursorPos() { cursor->updatePosition(getFramePtr()); }
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
    void setRecord(bool on);

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
* @brief Slot for the LfoWidget::flipWaveVerticalAction().
*
* If waveform is custom it calls the midi worker
* MidiLfo::flipWaveVertical() function, which flips the MidiLfo::customWave
* about its mid value.
*
*/
    void updateFlipWaveVertical();

    bool getReverse() { return midiLfo->reverse; }
};

#endif
