/**
 * @file engine.h
 * @brief Header file for the Engine class
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

#ifndef ENGINE_H
#define ENGINE_H

#include <QThread>
#include <QDockWidget>
#include <QList>
#include "seqdriver.h"
#include "jackdriver.h"
#include "midiarp.h"
#include "arpwidget.h"
#include "midilfo.h"
#include "lfowidget.h"
#include "midiseq.h"
#include "seqwidget.h"
#include "groovewidget.h"

/*!
 * @brief Core Engine Thread. Instantiates SeqDriver and JackDriver.
 *
 * For each module type there is a QList for each of
 * its components (for example MidiArp and ArpWidget). In parallel there is
 * a common list for all modules containing their DockWidgets.
 * Engine also instantiates the MIDI Driver backend and processes MIDI
 * events coming in and going out. It dispatches incoming events to the
 * worker modules and schedules resulting events back to the driver.
 * Controller events are dispatched to the modules as requiered by their
 * MIDI Learn MidiCCList.
 *
 */
class Engine : public QThread  {

  Q_OBJECT

  private:
    QList<MidiArp *> midiArpList;
    QList<ArpWidget *> arpWidgetList;
    QList<QDockWidget *> moduleWindowList;
    QList<MidiLfo *> midiLfoList;
    QList<LfoWidget *> lfoWidgetList;
    QList<MidiSeq *> midiSeqList;
    QList<SeqWidget *> seqWidgetList;
    int portCount;
    bool modified;
    int midiLearnID, midiLearnWindowID, midiLearnModuleID;
    bool midiLearnFlag;

    //From SeqDriver
    bool gotArpKbdTrig;
    bool gotSeqKbdTrig;
    int  schedDelayTicks;
    int nextMinLfoTick;
    int nextMinSeqTick;
    int nextMinArpTick;
    QVector<Sample> lfoData;
    Sample seqSample;
    bool sendLogEvents;

    static bool midi_event_received_callback(void * context, MidiEvent ev);
    static void tick_callback(void * context, bool echo_from_trig);
    static void tr_state_cb(bool tr_state, void * context);

  public:
    int grooveTick, grooveVelocity, grooveLength;
    bool midiControllable;
    bool status;
    bool ready;
    GrooveWidget *grooveWidget;
    JackDriver *jackSync;
    DriverBase *driver;

  public:
    Engine(GrooveWidget *p_grooveWidget, int p_portCount, bool p_alsamidi, QWidget* parent=0);
    ~Engine();
    int getPortCount();
    bool isModified();


    void addModuleWindow(QDockWidget *moduleWindow);
    void removeModuleWindow(QDockWidget *moduleWindow);
    QDockWidget *moduleWindow(int index);
    int moduleWindowCount();
    void updateIDs(int curID);

    void addMidiArp(MidiArp *midiArp);
    void addArpWidget(ArpWidget *arpWidget);
    void removeMidiArp(MidiArp *midiArp);
    void removeArpWidget(ArpWidget *arpWidget);
    int midiArpCount();
    int arpWidgetCount();
    MidiArp *midiArp(int index);
    ArpWidget *arpWidget(int index);

    void addMidiLfo(MidiLfo *midiLfo);
    void addLfoWidget(LfoWidget *lfoWidget);
    void removeMidiLfo(MidiLfo *midiLfo);
    void removeLfoWidget(LfoWidget *lfoWidget);
    int midiLfoCount();
    int lfoWidgetCount();
    MidiLfo *midiLfo(int index);
    LfoWidget *lfoWidget(int index);

    void addMidiSeq(MidiSeq *midiSeq);
    void addSeqWidget(SeqWidget *seqWidget);
    void removeMidiSeq(MidiSeq *midiSeq);
    void removeSeqWidget(SeqWidget *seqWidget);
    int midiSeqCount();
    int seqWidgetCount();
    MidiSeq *midiSeq(int index);
    SeqWidget *seqWidget(int index);
    int getClientId();
    void setTempo(int bpm);

  signals:
/**
 * @brief This signal is connected to the LogWidget::appendEvent() slot
 *
 * @param ev MidiEvent received by Engine
 * @param tick Set to the tick value at which the event was received
 */
    void midiEventReceived(MidiEvent ev, int tick);

  public slots:
    void setStatus(bool);
/**
 * @brief This function is used to set the modified flag, which is queried before
 * loading a new session file or quitting qmidiarp.
 *
 * @param m Set to True if parameter modifications are present
 */
    void setModified(bool);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void handleController(int ccnumber, int channel, int value);
    void setMidiLearn(int moduleWindowID, int moduleID, int controlID);
    void setMidiControllable(bool on);
    void setCompactStyle(bool on);
    void setGrooveTick(int grooveTick);
    void setGrooveVelocity(int grooveVelocity);
    void setGrooveLength(int grooveLength);
    void sendGroove();
    void setSendLogEvents(bool on);
    bool eventCallback(MidiEvent inEv);
    void echoCallback(bool echo_from_trig);
    void resetTicks(int curtick);
};

#endif
