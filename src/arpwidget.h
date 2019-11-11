/*!
 * @file arpwidget.h
 * @brief Member definitions for the ArpWidget GUI class.
 *
 *
 *      Copyright 2009 - 2019 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QLineEdit>

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

    MidiArp *midiArp;

    QGroupBox *randomBox, *envelopeBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QToolButton *latchModeButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;
    int patternPresetBoxIndex;

    void loadPatternPresets();
/* PUBLIC MEMBERS */
  public:
#ifdef APPBUILD
/*!
 * @brief Constructor for ArpWidget. It creates the GUI and an ArpScreen
 * instance.
 *
 * @param p_midiArp Associated MidiArp Object
 * @param p_globStore The Application-wide globStore widget
 * @param portCount Number of available MIDI output ports
 * @param compactStyle If set to True, Widget will use reduced spacing and small fonts
 * @param mutedAdd If set to True, the module will be added in muted state
 * @param inOutVisible Add the module with visible InOutBox or not
 * @param name The name of the module preceded by its type (Arp: , etc...)
 */
    ArpWidget(MidiArp *p_midiArp, GlobStore *p_globStore, Prefs *p_prefs,
            bool inOutVisible = true,
            const QString& name = "");

#else
    ArpWidget();
#endif

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

    void doStoreParams(int ix);
    void doRestoreParams(int ix);
    void updateDisplay();
    void handleController(int ccnumber, int channel, int value);
#endif

    void updateCursorPos() { screen->updateCursor(midiArp->getFramePtr()); }
    
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


 /*! @brief Slot for ArpWidget::latchModeAction.
  * Will cause notes remaining in MidiArp::latchBuffer until new
  * stakato note received */
    void setLatchMode(bool);
};

#endif
