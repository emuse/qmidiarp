/**
 * @file arpdata.h
 * @brief Managing class for created module components in lists. Instantiates SeqDriver.
 *
 * For each module component and type there is a QList (for example MidiArp
 * and ArpWidget). In parallel there is
 * a common list for all modules containing their DockWidgets.
 * ArpData also instantiates the SeqDriver MIDI backend and handles MIDI
 * controller events through signaling by seqDriver. Controllers are
 * dispatched to the modules as requiered by their MIDI Learn
 * MidiCCList.
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

#ifndef ARPDATA_H
#define ARPDATA_H

#include <QWidget>
#include <QDockWidget>
#include <QList>
#include "seqdriver.h"
#include "midiarp.h"
#include "arpwidget.h"
#include "midilfo.h"
#include "lfowidget.h"
#include "midiseq.h"
#include "seqwidget.h"

/*!
 * @brief Manages created module components in lists. Instantiates SeqDriver.
 *
 * For each module type there is a QList for each of
 * its components (for example MidiArp and ArpWidget). In parallel there is
 * a common list for all modules containing their DockWidgets.
 * ArpData also instantiates the SeqDriver MIDI backend and handles MIDI
 * controller events through signaling by SeqDriver. Controllers are
 * dispatched to the modules as requiered by their MIDI Learn
 * MidiCCList.
 *
 */
class ArpData : public QWidget  {

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
    int mute_ccnumber, midiLearnID, midiLearnWindowID, midiLearnModuleID;
    bool midi_mutable, midiLearnFlag;

  public:
    SeqDriver *seqDriver;

    ArpData(QWidget* parent=0);
    ~ArpData();
    void registerPorts(int num);
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
    int getAlsaClientId();

  public slots:
    void runQueue(bool);
/**
 * This function is used to set the modified flag, which is queried before
 * loading a new session file or quitting qmidiarp.
 *
 * @param m Set to True if parameter modifications are present
 */
    void setModified(bool);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void handleController(int ccnumber, int channel, int value);
    void setMidiLearn(int moduleWindowID, int moduleID, int controlID);
    void setCompactStyle(bool on);
};

#endif
