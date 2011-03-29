/**
 * @file arpdata.cpp
 * @brief Managing class for created module components in lists. Instantiates SeqDriver.
 *
 * For each module component and type there is a QList (for example MidiArp
 * and ArpWidget). In parallel there is
 * a common list for all modules containing their DockWidgets.
 * ArpData also instantiates the SeqDriver MIDI backend and handles MIDI
 * controller events through signaling by seqDriver. Controllers are
 * dispatched to the modules as requiered by their MIDI Learn
 * MidiCCList.
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
 *
 */

#include <QString>

#include "seqdriver.h"
#include "arpdata.h"


ArpData::ArpData(int p_portCount, QWidget *parent) : QWidget(parent), modified(false)
{
    portCount = p_portCount;

    seqDriver = new SeqDriver(portCount, this, this, midi_event_received_callback, tick_callback);
    /* TODO: This is a temporary lazy replacement for a proper callback
     *       to be installed at term
     * */
    qRegisterMetaType<MidiEvent>("MidiEvent");
    connect(seqDriver, SIGNAL(handleEvent(MidiEvent, int)),
            this, SLOT(eventCallback(MidiEvent, int)));
    connect(seqDriver, SIGNAL(handleEcho(MidiEvent, int)),
            this, SLOT(echoCallback(MidiEvent, int)));


    midiLearnFlag = false;
    midiControllable = true;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    gotArpKbdTrig = false;
    gotSeqKbdTrig = false;
    schedDelayTicks = 2;
    transportStatus = false;

    resetTicks(0);
}

ArpData::~ArpData(){
}
//Arp handling
void ArpData::addMidiArp(MidiArp *midiArp)
{
    midiArpList.append(midiArp);
}

void ArpData::addArpWidget(ArpWidget *arpWidget)
{
    arpWidgetList.append(arpWidget);
    modified = true;
}

void ArpData::removeMidiArp(MidiArp *midiArp)
{
    if (transportStatus && (moduleWindowCount() < 1)) {
        setTransportStatus(false);
    }
    int i = midiArpList.indexOf(midiArp);
    if (i != -1)
        delete midiArpList.takeAt(i);
}

void ArpData::removeArpWidget(ArpWidget *arpWidget)
{
    removeMidiArp(arpWidget->getMidiWorker());
    arpWidgetList.removeOne(arpWidget);
    modified = true;
}

void ArpData::updatePatternPresets(const QString& n, const QString& p, int index)
{
    int l1;
    for (l1 = 0; l1 < midiArpCount(); l1++) {
        arpWidgetList.at(l1)->updatePatternPresets(n, p, index);
    }
}

int ArpData::midiArpCount()
{
    return(midiArpList.count());
}

int ArpData::arpWidgetCount()
{
    return(arpWidgetList.count());
}

MidiArp *ArpData::midiArp(int index)
{
    return(midiArpList.at(index));
}

ArpWidget *ArpData::arpWidget(int index)
{
    return(arpWidgetList.at(index));
}


void ArpData::setGrooveTick(int val)
{
    grooveTick = val;
    sendGroove();
    modified = true;
}

void ArpData::setGrooveVelocity(int val)
{
    grooveVelocity = val;
    sendGroove();
    modified = true;
}

void ArpData::setGrooveLength(int val)
{
    grooveLength = val;
    sendGroove();
    modified = true;
}

void ArpData::sendGroove()
{
    for (int l1 = 0; l1 < midiArpList.count(); l1++) {
        midiArp(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
}

//LFO handling

void ArpData::addMidiLfo(MidiLfo *midiLfo)
{
    midiLfoList.append(midiLfo);
}

void ArpData::addLfoWidget(LfoWidget *lfoWidget)
{
    lfoWidgetList.append(lfoWidget);
    modified = true;
}

void ArpData::removeMidiLfo(MidiLfo *midiLfo)
{
    if (transportStatus && (moduleWindowCount() < 1)) {
        setTransportStatus(false);
    }
    int i = midiLfoList.indexOf(midiLfo);
    if (i != -1)
        delete midiLfoList.takeAt(i);
}

void ArpData::removeLfoWidget(LfoWidget *lfoWidget)
{
    removeMidiLfo(lfoWidget->getMidiWorker());
    lfoWidgetList.removeOne(lfoWidget);
    modified = true;
}

int ArpData::midiLfoCount()
{
    return(midiLfoList.count());
}

int ArpData::lfoWidgetCount()
{
    return(lfoWidgetList.count());
}

MidiLfo *ArpData::midiLfo(int index)
{
    return(midiLfoList.at(index));
}

LfoWidget *ArpData::lfoWidget(int index)
{
    return(lfoWidgetList.at(index));
}

//SEQ handling

void ArpData::addMidiSeq(MidiSeq *midiSeq)
{
    midiSeqList.append(midiSeq);
}

void ArpData::addSeqWidget(SeqWidget *seqWidget)
{
    seqWidgetList.append(seqWidget);
    modified = true;
}

void ArpData::removeMidiSeq(MidiSeq *midiSeq)
{
    if (transportStatus && (moduleWindowCount() < 1)) {
        setTransportStatus(false);
    }
    int i = midiSeqList.indexOf(midiSeq);
    if (i != -1)
        delete midiSeqList.takeAt(i);
}

void ArpData::removeSeqWidget(SeqWidget *seqWidget)
{
    removeMidiSeq(seqWidget->getMidiWorker());
    seqWidgetList.removeOne(seqWidget);
    modified = true;
}

int ArpData::midiSeqCount()
{
    return(midiSeqList.count());
}

int ArpData::seqWidgetCount()
{
    return(seqWidgetList.count());
}

MidiSeq *ArpData::midiSeq(int index)
{
    return(midiSeqList.at(index));
}

SeqWidget *ArpData::seqWidget(int index)
{
    return(seqWidgetList.at(index));
}

//module Window handling (dockWidgets)

void ArpData::addModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.append(moduleWindow);
    modified = true;
}

void ArpData::removeModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.removeOne(moduleWindow);
    delete moduleWindow;
    modified = true;
}

QDockWidget *ArpData::moduleWindow(int index)
{
    return(moduleWindowList.at(index));
}

int ArpData::moduleWindowCount()
{
    return(moduleWindowList.count());
}

void ArpData::updateIDs(int curID)
{
    int l1, tempDockID;
    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->ID = l1;
        tempDockID = arpWidget(l1)->parentDockID;
        if (tempDockID > curID) {
            arpWidget(l1)->parentDockID = tempDockID - 1;
            }
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->ID = l1;
        tempDockID = lfoWidget(l1)->parentDockID;
        if (tempDockID > curID) {
            lfoWidget(l1)->parentDockID = tempDockID - 1;
        }
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->ID = l1;
        tempDockID = seqWidget(l1)->parentDockID;
        if (tempDockID > curID) {
            seqWidget(l1)->parentDockID = tempDockID - 1;
        }
    }
}

//general

void ArpData::setCompactStyle(bool on)
{
    int l1;
    if (on) {
        for (l1 = 0; l1 < moduleWindowCount(); l1++)
            moduleWindow(l1)->setStyleSheet(COMPACT_STYLE);
    }
    else {
        for (l1 = 0; l1 < moduleWindowCount(); l1++)
            moduleWindow(l1)->setStyleSheet("");
    }
}

bool ArpData::isModified()
{
    bool arpmodified = false;
    bool lfomodified = false;
    bool seqmodified = false;

    for (int l1 = 0; l1 < arpWidgetCount(); l1++)
        if (arpWidget(l1)->isModified()) {
            arpmodified = true;
            break;
        }
    for (int l1 = 0; l1 < lfoWidgetCount(); l1++)
        if (lfoWidget(l1)->isModified()) {
            lfomodified = true;
            break;
        }

    for (int l1 = 0; l1 < seqWidgetCount(); l1++)
        if (seqWidget(l1)->isModified()) {
            seqmodified = true;
            break;
        }

    return modified || seqDriver->isModified()
                    || arpmodified || lfomodified || seqmodified;
}

void ArpData::setModified(bool m)
{
    modified = m;
    seqDriver->setModified(m);

    for (int l1 = 0; l1 < arpWidgetCount(); l1++)
        arpWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < lfoWidgetCount(); l1++)
        lfoWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < seqWidgetCount(); l1++)
        seqWidget(l1)->setModified(m);
}

/* All following functions are the core engine to be placed into
 * Nedko's new DriverBase class. They need to be made realtime-safe.
 * They currently call different seqDriver backend functions, which
 * can eventually (hopefully) get a jackDriver equivalent, so that
 * switching between the driver backends can be done from here.
 * Note that these function have been in the SeqDriver thread before, and
 * that they are now in the main (GUI) thread. This is why there are
 * dropouts when moving windows.
 */

int ArpData::getPortCount()
{
    return(portCount);
}

int ArpData::getClientId()
{
    return seqDriver->getAlsaClientId();
}

void ArpData::setTransportStatus(bool on)
{
    if (moduleWindowCount()) {
        if (!on) {
            for (int l1 = 0; l1 < midiArpCount(); l1++) {
                midiArp(l1)->clearNoteBuffer();
            }
        }
        transportStatus = on;
        resetTicks(0);
        seqDriver->setQueueStatus(on);
    }
}

void ArpData::tick_callback(void * context)
{
  // ((ArpData *)context)->echoCallback()
}

void ArpData::echoCallback(MidiEvent inEv, int tick)
{
    int l1, l2;
    QVector<int> note, velocity;
    int note_tick = 0;
    int length;
    int outport;
    int seqtransp;
    bool isNew;
    MidiEvent outEv;
    int frame_nticks = 0;

    note.clear();
    velocity.clear();



        //~ printf("       tick %d     ",tick);
        //~ printf("nextMinLfoTick %d  ",nextMinLfoTick);
        //~ printf("nextMinSeqTick %d  ",nextMinSeqTick);
        //~ printf("nextMinArpTick %d  \n",nextMinArpTick);
        //~ printf("midiTick %d   ",midiTick);
        //~ printf("clockRatio %f  ",clockRatio);
        //~ printf("Jack Beat %d\n", jpos.beat);
        //~ printf("Jack Frame %d \n ", (int)jpos.frame);
        //~ printf("Jack BBT offset %d\n", (int)jpos.bbt_offset);

    //LFO data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextMinLfoTick) && (midiLfoCount())) {
        for (l1 = 0; l1 < midiLfoCount(); l1++) {
            if ((tick + 8) >= nextLfoTick[l1]) {
                outEv.type = EV_CONTROLLER;
                outEv.data = midiLfo(l1)->ccnumber;
                outEv.channel = midiLfo(l1)->channelOut;
                midiLfo(l1)->getNextFrame(&lfoData);
                frame_nticks = lfoData.last().tick;
                outport = midiLfo(l1)->portOut;
                if (!midiLfo(l1)->isMuted) {
                    l2 = 0;
                    while (lfoData.at(l2).value > -1) {
                        if (!lfoData.at(l2).muted) {
                            outEv.value = lfoData.at(l2).value;
                            seqDriver->sendMidiEvent(outEv, nextLfoTick[l1] + lfoData.at(l2).tick
                                , outport);
                        }
                        l2++;
                    }
                }
                nextLfoTick[l1] += frame_nticks;
                /** round-up to current resolution (quantize) */
                nextLfoTick[l1]/= frame_nticks;
                nextLfoTick[l1]*= frame_nticks;
            }
            if (!l1)
                nextMinLfoTick = nextLfoTick[l1];
            else if (nextLfoTick[l1] < nextMinLfoTick)
                nextMinLfoTick = nextLfoTick[l1];
        }
        seqDriver->requestEchoAt(nextMinLfoTick);
    }

    //Seq notes data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextMinSeqTick) && (midiSeqCount())) {
        for (l1 = 0; l1 < midiSeqCount(); l1++) {
            if ((gotSeqKbdTrig && (inEv.data == 2) && midiSeq(l1)->wantTrigByKbd())
                    || (!gotSeqKbdTrig && (inEv.data == 0))) {
                gotSeqKbdTrig = false;
                if ((tick + 8) >= nextSeqTick[l1]) {
                    outEv.type = EV_NOTEON;
                    outEv.value = midiSeq(l1)->vel;
                    outEv.channel = midiSeq(l1)->channelOut;
                    midiSeq(l1)->getNextNote(&seqSample);
                    frame_nticks = TPQN / midiSeq(l1)->res;
                    length = midiSeq(l1)->notelength;
                    seqtransp = midiSeq(l1)->transp;
                    outport = midiSeq(l1)->portOut;
                    if (!midiSeq(l1)->isMuted) {
                        if (!seqSample.muted) {
                            outEv.data = seqSample.value + seqtransp;
                            seqDriver->sendMidiEvent(outEv, nextSeqTick[l1], outport, length);
                        }
                    }
                    nextSeqTick[l1]+=frame_nticks;
                    if (!midiSeq(l1)->trigByKbd) {
                        /** round-up to current resolution (quantize) */
                        nextSeqTick[l1]/=frame_nticks;
                        nextSeqTick[l1]*=frame_nticks;
                    }
                }
            }
            if (!l1)
                nextMinSeqTick = nextSeqTick[l1];
            else if (nextSeqTick[l1] < nextMinSeqTick)
                nextMinSeqTick = nextSeqTick[l1];
        }
        seqDriver->requestEchoAt(nextMinSeqTick, 0);
    }

    //Arp Note queueing
    if ((tick + 8) >= nextMinArpTick) {
        for (l1 = 0; l1 < midiArpCount(); l1++) {
            if ((gotArpKbdTrig && (inEv.data == 2) && midiArp(l1)->wantTrigByKbd())
                    || (!gotArpKbdTrig && (inEv.data == 0))) {
                gotArpKbdTrig = false;
                if (tick + schedDelayTicks >= nextArpTick[l1]) {
                    outEv.type = EV_NOTEON;
                    outEv.channel = midiArp(l1)->channelOut;
                    midiArp(l1)->newRandomValues();
                    midiArp(l1)->updateQueueTempo(tempo);
                    midiArp(l1)->prepareCurrentNote(tick);
                    note = midiArp(l1)->returnNote;
                    velocity = midiArp(l1)->returnVelocity;
                    note_tick = midiArp(l1)->returnTick;
                    length = midiArp(l1)->returnLength * 4;
                    outport = midiArp(l1)->portOut;
                    isNew = midiArp(l1)->returnIsNew;
                    if (!velocity.isEmpty()) {
                        if (isNew && velocity.at(0)) {
                            l2 = 0;
                            while(note.at(l2) >= 0) {
                                outEv.data = note.at(l2);
                                outEv.value = velocity.at(l2);
                                seqDriver->sendMidiEvent(outEv, note_tick, outport, length);
                                l2++;
                            }
                        }
                    }
                    nextArpTick[l1] = midiArp(l1)->getNextNoteTick();
                }
            }
            if (!l1)
                nextMinArpTick = nextArpTick[l1] - schedDelayTicks;
            else if (nextArpTick[l1] < nextMinArpTick + schedDelayTicks)
                nextMinArpTick = nextArpTick[l1] - schedDelayTicks;
        }

        if (0 > nextMinArpTick) nextMinArpTick = 0;
        seqDriver->requestEchoAt(nextMinArpTick, 0);
    }
}

void ArpData::midi_event_received_callback(void * context, MidiEvent ev)
{
  // ((ArpData *)context)->eventCallback(ev, seqDriver->getCurrentTick())
}

bool ArpData::eventCallback(MidiEvent inEv, int tick)
{
    bool unmatched;
    int l1;
    unmatched = true;

    if (inEv.type == EV_CONTROLLER) {

        if (inEv.data == CT_FOOTSW) {
            for (l1 = 0; l1 < midiArpCount(); l1++) {
                if (midiArp(l1)->wantEvent(inEv)) {
                    midiArp(l1)->setSustain((inEv.value == 127), tick);
                    unmatched = false;
                }
            }
        }
        else {
            //Does any LFO want to record this?
            for (l1 = 0; l1 < midiLfoCount(); l1++) {
                if (midiLfo(l1)->wantEvent(inEv)) {
                    midiLfo(l1)->record(inEv.value);
                    unmatched = false;
                }
            }
            if (midiControllable) {
                handleController(inEv.data, inEv.channel, inEv.value);
                unmatched = false;
            }
        }
        return unmatched;
    }

    if (inEv.type == EV_NOTEON) {

        for (l1 = 0; l1 < midiSeqCount(); l1++) {
            if (midiSeq(l1)->wantEvent(inEv)) {
                unmatched = false;

                midiSeq(l1)->handleNote(inEv.data, inEv.value, tick);

                if (inEv.value && midiSeq(l1)->wantTrigByKbd()) {
                    nextMinSeqTick = tick;
                    nextSeqTick[l1] = nextMinSeqTick + 2;
                    gotSeqKbdTrig = true;
                    seqDriver->requestEchoAt(nextMinSeqTick, 2);
                }
            }
        }
        for (l1 = 0; l1 < midiArpCount(); l1++) {
            if (midiArp(l1)->wantEvent(inEv)) {
                unmatched = false;
                if (inEv.value) {

                    midiArp(l1)->handleNoteOn(inEv.data, inEv.value, tick);

                    if (midiArp(l1)->wantTrigByKbd()) {
                        nextMinArpTick = tick;
                        nextArpTick[l1] = nextMinArpTick + 2;
                        gotArpKbdTrig = true;
                        seqDriver->requestEchoAt(nextMinArpTick, 2);
                    }
                }
                else {
                    midiArp(l1)->handleNoteOff(inEv.data, tick, 1);
                }
            }
        }
        return unmatched;
    }

    return unmatched;
}

void ArpData::handleController(int ccnumber, int channel, int value)
{
    bool m;
    int min, max, sval;
    QVector<MidiCC> cclist;
    if (!midiLearnFlag) {
        for (int l1 = 0; l1 < arpWidgetCount(); l1++) {
            cclist = arpWidget(l1)->ccList;
            for (int l2 = 0; l2 < cclist.count(); l2++) {
                min = cclist.at(l2).min;
                max = cclist.at(l2).max;

                if ((ccnumber == cclist.at(l2).ccnumber) &&
                    (channel == cclist.at(l2).channel)) {
                    switch (cclist.at(l2).ID) {
                        case 0: if (min == max) {
                                    if (value == max) {
                                        m = arpWidget(l1)->muteOut->isChecked();
                                        arpWidget(l1)->muteOut->setChecked(!m);
                                        return;
                                    }
                        case 1:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                arpWidget(l1)->selectPatternPreset(sval);
                                return;
                        break;
                                }
                                else {
                                    if (value == max) {
                                        arpWidget(l1)->muteOut->setChecked(false);
                                    }
                                    if (value == min) {
                                        arpWidget(l1)->muteOut->setChecked(true);
                                    }
                                }
                        break;
                        default:
                        break;
                    }
                }
            }
        }

        for (int l1 = 0; l1 < lfoWidgetCount(); l1++) {
            cclist = lfoWidget(l1)->ccList;
            for (int l2 = 0; l2 < cclist.count(); l2++) {
                min = cclist.at(l2).min;
                max = cclist.at(l2).max;
                if ((ccnumber == cclist.at(l2).ccnumber) &&
                    (channel == cclist.at(l2).channel)) {
                    switch (cclist.at(l2).ID) {
                        case 0: if (min == max) {
                                    if (value == max) {
                                        m = lfoWidget(l1)->muteOut->isChecked();
                                        lfoWidget(l1)->muteOut->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        lfoWidget(l1)->muteOut->setChecked(false);
                                    }
                                    if (value == min) {
                                        lfoWidget(l1)->muteOut->setChecked(true);
                                    }
                                }
                        break;

                        case 1:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->amplitude->setValue(sval);
                                return;
                        break;

                        case 2:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->offset->setValue(sval);
                                return;
                        break;
                        case 3:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->waveFormBox->setCurrentIndex(sval);
                                lfoWidget(l1)->updateWaveForm(sval);
                                return;
                        break;
                        case 4:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->freqBox->setCurrentIndex(sval);
                                lfoWidget(l1)->updateFreq(sval);
                                return;
                        break;
                        case 5: if (min == max) {
                                    if (value == max) {
                                        m = lfoWidget(l1)->recordAction->isChecked();
                                        lfoWidget(l1)->recordAction->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        lfoWidget(l1)->recordAction->setChecked(true);
                                    }
                                    if (value == min) {
                                        lfoWidget(l1)->recordAction->setChecked(false);
                                    }
                                }
                        break;

                        default:
                        break;
                    }
                }
            }
        }

        for (int l1 = 0; l1 < seqWidgetCount(); l1++) {
            cclist = seqWidget(l1)->ccList;
            for (int l2 = 0; l2 < cclist.count(); l2++) {
                min = cclist.at(l2).min;
                max = cclist.at(l2).max;
                if ((ccnumber == cclist.at(l2).ccnumber) &&
                    (channel == cclist.at(l2).channel)) {
                    switch (cclist.at(l2).ID) {
                        case 0: if (min == max) {
                                    if (value == max) {
                                        m = seqWidget(l1)->muteOut->isChecked();
                                        seqWidget(l1)->muteOut->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        seqWidget(l1)->muteOut->setChecked(false);
                                    }
                                    if (value == min) {
                                        seqWidget(l1)->muteOut->setChecked(true);
                                    }
                                }
                        break;

                        case 1:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                seqWidget(l1)->velocity->setValue(sval);
                                return;
                        break;

                        case 2:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                seqWidget(l1)->notelength->setValue(sval);
                                return;
                        break;

                        case 3: if (min == max) {
                                    if (value == max) {
                                        m = seqWidget(l1)->recordAction->isChecked();
                                        seqWidget(l1)->recordAction->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        seqWidget(l1)->recordAction->setChecked(true);
                                    }
                                    if (value == min) {
                                        seqWidget(l1)->recordAction->setChecked(false);
                                    }
                                }
                        break;

                        default:
                        break;
                    }
                }
            }
        }
    }
    else {
        int min = (midiLearnID) ? 0 : 127; //if control is toggle min=max
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Arp")) {
            arpWidget(midiLearnModuleID)->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("LFO")) {
            lfoWidget(midiLearnModuleID)->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Seq")) {
            seqWidget(midiLearnModuleID)->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }

        midiLearnFlag = false;
    }
}

void ArpData::resetTicks(int curtick)
{
    int l1;

    for (l1 = 0; l1 < midiArpCount(); l1++) {
        midiArp(l1)->foldReleaseTicks(curtick);
        midiArp(l1)->initArpTick(0);
    }
    for (l1 = 0; l1 < midiLfoCount(); l1++) {
        midiLfo(l1)->resetFramePtr();
    }
    for (l1 = 0; l1 < midiSeqCount(); l1++) {
        midiSeq(l1)->setCurrentIndex(0);
    }
    for (l1 = 0; l1 < 20; l1++) {
        nextArpTick[l1] = 0;
        nextLfoTick[l1] = 0;
        nextSeqTick[l1] = 0;
    }
    nextMinLfoTick = 0;
    nextMinSeqTick = 0;
    nextMinArpTick = 0;
}

void ArpData::setMidiControllable(bool on)
{
    midiControllable = on;
    modified = true;
}

void ArpData::setMidiLearn(int moduleWindowID, int moduleID, int controlID)
{
    if (0 > controlID) {
        midiLearnFlag = false;
        return;
    }
    else {
        midiLearnFlag = true;
        midiLearnWindowID = moduleWindowID;
        midiLearnModuleID = moduleID;
        midiLearnID = controlID;
    }
}

void ArpData::setTempo(int bpm)
{
    tempo = bpm;
    seqDriver->setTempo(bpm);
    modified = true;
}
