/**
 * @file engine.cpp
 * @brief Implementation of the Engine class
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
 *
 */

#include <QString>

#include "seqdriver.h"
#include "engine.h"


Engine::Engine(GrooveWidget *p_grooveWidget, int p_portCount, bool p_alsamidi, QWidget *parent) : QThread(parent), modified(false)
{
    ready = false;
    grooveWidget = p_grooveWidget;
    connect(grooveWidget, SIGNAL(newGrooveTick(int)),
            this, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)),
            this, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)),
            this, SLOT(setGrooveLength(int)));
    connect(grooveWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            this, SLOT(setMidiLearn(int, int, int)));
    portCount = p_portCount;

    if (!p_alsamidi) {
        driver = new JackDriver(portCount, this, tr_state_cb, midi_event_received_callback, tick_callback);
    }
    else {
    // In case of ALSA MIDI with Jack Transport sync, JackDriver is instantiated with 0 ports
    // a pointer to jackSync has to be passed to driver
        jackSync = new JackDriver(0,  this, tr_state_cb, midi_event_received_callback, tick_callback);
        driver = new SeqDriver(jackSync, portCount, this, midi_event_received_callback, tick_callback);
    }

    midiLearnFlag = false;
    midiControllable = true;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    gotArpKbdTrig = false;
    gotSeqKbdTrig = false;
    gotLfoKbdTrig = false;
    schedDelayTicks = 2;
    status = false;
    sendLogEvents = false;

    resetTicks(0);
    ready = true;
}

Engine::~Engine()
{
    delete driver;
}

//Arp handling
void Engine::addMidiArp(MidiArp *midiArp)
{
    midiArpList.append(midiArp);
}

void Engine::addArpWidget(ArpWidget *arpWidget)
{
    arpWidgetList.append(arpWidget);
    modified = true;
}

void Engine::removeMidiArp(MidiArp *midiArp)
{
    if (status && (moduleWindowCount() < 1)) {
        setStatus(false);
    }
    int i = midiArpList.indexOf(midiArp);
    if (i != -1)
        delete midiArpList.takeAt(i);
}

void Engine::removeArpWidget(ArpWidget *arpWidget)
{
    removeMidiArp(arpWidget->getMidiWorker());
    arpWidgetList.removeOne(arpWidget);
    modified = true;
}

void Engine::updatePatternPresets(const QString& n, const QString& p, int index)
{
    int l1;
    for (l1 = 0; l1 < midiArpCount(); l1++) {
        arpWidgetList.at(l1)->updatePatternPresets(n, p, index);
    }
}

int Engine::midiArpCount()
{
    return(midiArpList.count());
}

int Engine::arpWidgetCount()
{
    return(arpWidgetList.count());
}

MidiArp *Engine::midiArp(int index)
{
    return(midiArpList.at(index));
}

ArpWidget *Engine::arpWidget(int index)
{
    return(arpWidgetList.at(index));
}


void Engine::setGrooveTick(int val)
{
    grooveTick = val;
    sendGroove();
    modified = true;
}

void Engine::setGrooveVelocity(int val)
{
    grooveVelocity = val;
    sendGroove();
    modified = true;
}

void Engine::setGrooveLength(int val)
{
    grooveLength = val;
    sendGroove();
    modified = true;
}

void Engine::sendGroove()
{
    int l1;

    for (l1 = 0; l1 < midiArpList.count(); l1++) {
        midiArp(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
        arpWidget(l1)->screen->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
    for (l1 = 0; l1 < midiSeqList.count(); l1++) {
        midiSeq(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
        seqWidget(l1)->screen->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
    for (l1 = 0; l1 < midiLfoList.count(); l1++) {
        midiLfo(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
        lfoWidget(l1)->screen->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
}

//LFO handling

void Engine::addMidiLfo(MidiLfo *midiLfo)
{
    midiLfoList.append(midiLfo);
}

void Engine::addLfoWidget(LfoWidget *lfoWidget)
{
    lfoWidgetList.append(lfoWidget);
    modified = true;
}

void Engine::removeMidiLfo(MidiLfo *midiLfo)
{
    if (status && (moduleWindowCount() < 1)) {
        setStatus(false);
    }
    int i = midiLfoList.indexOf(midiLfo);
    if (i != -1)
        delete midiLfoList.takeAt(i);
}

void Engine::removeLfoWidget(LfoWidget *lfoWidget)
{
    removeMidiLfo(lfoWidget->getMidiWorker());
    lfoWidgetList.removeOne(lfoWidget);
    modified = true;
}

int Engine::midiLfoCount()
{
    return(midiLfoList.count());
}

int Engine::lfoWidgetCount()
{
    return(lfoWidgetList.count());
}

MidiLfo *Engine::midiLfo(int index)
{
    return(midiLfoList.at(index));
}

LfoWidget *Engine::lfoWidget(int index)
{
    return(lfoWidgetList.at(index));
}

//SEQ handling

void Engine::addMidiSeq(MidiSeq *midiSeq)
{
    midiSeqList.append(midiSeq);
}

void Engine::addSeqWidget(SeqWidget *seqWidget)
{
    seqWidgetList.append(seqWidget);
    modified = true;
}

void Engine::removeMidiSeq(MidiSeq *midiSeq)
{
    if (status && (moduleWindowCount() < 1)) {
        setStatus(false);
    }
    int i = midiSeqList.indexOf(midiSeq);
    if (i != -1)
        delete midiSeqList.takeAt(i);
}

void Engine::removeSeqWidget(SeqWidget *seqWidget)
{
    removeMidiSeq(seqWidget->getMidiWorker());
    seqWidgetList.removeOne(seqWidget);
    modified = true;
}

int Engine::midiSeqCount()
{
    return(midiSeqList.count());
}

int Engine::seqWidgetCount()
{
    return(seqWidgetList.count());
}

MidiSeq *Engine::midiSeq(int index)
{
    return(midiSeqList.at(index));
}

SeqWidget *Engine::seqWidget(int index)
{
    return(seqWidgetList.at(index));
}

//module Window handling (dockWidgets)

void Engine::addModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.append(moduleWindow);
    modified = true;
}

void Engine::removeModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.removeOne(moduleWindow);
    delete moduleWindow;
    modified = true;
}

QDockWidget *Engine::moduleWindow(int index)
{
    return(moduleWindowList.at(index));
}

int Engine::moduleWindowCount()
{
    return(moduleWindowList.count());
}

void Engine::updateIDs(int curID)
{
    int l1, tempDockID;
    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->manageBox->ID = l1;
        tempDockID = arpWidget(l1)->manageBox->parentDockID;
        if (tempDockID > curID) {
            arpWidget(l1)->manageBox->parentDockID = tempDockID - 1;
            }
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->manageBox->ID = l1;
        tempDockID = lfoWidget(l1)->manageBox->parentDockID;
        if (tempDockID > curID) {
            lfoWidget(l1)->manageBox->parentDockID = tempDockID - 1;
        }
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->manageBox->ID = l1;
        tempDockID = seqWidget(l1)->manageBox->parentDockID;
        if (tempDockID > curID) {
            seqWidget(l1)->manageBox->parentDockID = tempDockID - 1;
        }
    }
}

//general

void Engine::setCompactStyle(bool on)
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

bool Engine::isModified()
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

    return modified || arpmodified || lfomodified || seqmodified;
}

void Engine::setModified(bool m)
{
    modified = m;

    for (int l1 = 0; l1 < arpWidgetCount(); l1++)
        arpWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < lfoWidgetCount(); l1++)
        lfoWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < seqWidgetCount(); l1++)
        seqWidget(l1)->setModified(m);
}

/* All following functions are the core engine of QMidiArp. They need to
 * be made realtime-safe.
 * They currently call different driver backend functions, which
 * can eventually (hopefully) get a jackDriver equivalent, so that
 * switching between the driver backends can be done from here.
 */

int Engine::getPortCount()
{
    return(portCount);
}

int Engine::getClientId()
{
    return driver->getClientId();
}

void Engine::setStatus(bool on)
{
    if (moduleWindowCount()) {
        if (!on) {
            for (int l1 = 0; l1 < midiArpCount(); l1++) {
                midiArp(l1)->clearNoteBuffer();
            }
        }
        status = on;
        resetTicks(0);
        driver->setTransportStatus(on);
    }
}

void Engine::tick_callback(void * context, bool echo_from_trig)
{
  ((Engine *)context)->echoCallback(echo_from_trig);
}

void Engine::echoCallback(bool echo_from_trig)
{
    int l1, l2;
    QVector<int> note, velocity;
    int tick = driver->getCurrentTick();
    int note_tick = 0;
    int length;
    int outport;
    bool isNew;
    MidiEvent outEv;

    note.clear();
    velocity.clear();

        //~ printf("       tick %d     ",tick);
        //~ printf("nextMinLfoTick %d  ",nextMinLfoTick);
        //~ printf("nextMinSeqTick %d  ",nextMinSeqTick);
        //~ printf("nextMinArpTick %d  \n",nextMinArpTick);

    //LFO data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextMinLfoTick) && (midiLfoCount())) {
        for (l1 = 0; l1 < midiLfoCount(); l1++) {
            if ((gotLfoKbdTrig && echo_from_trig && midiLfo(l1)->wantTrigByKbd())
                    || (!gotLfoKbdTrig && !echo_from_trig)) {
                gotLfoKbdTrig = false;
                if ((tick + 8) >= midiLfo(l1)->nextTick) {
                    outEv.type = EV_CONTROLLER;
                    outEv.data = midiLfo(l1)->ccnumber;
                    outEv.channel = midiLfo(l1)->channelOut;
                    midiLfo(l1)->getNextFrame(&lfoData, tick);
                    outport = midiLfo(l1)->portOut;
                    if (!midiLfo(l1)->isMuted) {
                        l2 = 0;
                        while (lfoData.at(l2).value > -1) {
                            if (!lfoData.at(l2).muted) {
                                outEv.value = lfoData.at(l2).value;
                                driver->sendMidiEvent(outEv, nextMinLfoTick + lfoData.at(l2).tick
                                    , outport);
                            }
                            l2++;
                        }
                    }
                }
            }
            if (!l1)
                nextMinLfoTick = midiLfo(l1)->nextTick;
            else if (midiLfo(l1)->nextTick < nextMinLfoTick)
                nextMinLfoTick = midiLfo(l1)->nextTick;
        }
        driver->requestEchoAt(nextMinLfoTick, 0);
    }

    //Seq notes data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextMinSeqTick) && (midiSeqCount())) {
        for (l1 = 0; l1 < midiSeqCount(); l1++) {
            if ((gotSeqKbdTrig && echo_from_trig && midiSeq(l1)->wantTrigByKbd())
                    || (!gotSeqKbdTrig && !echo_from_trig)) {
                gotSeqKbdTrig = false;
                if ((tick + 8) >= midiSeq(l1)->nextTick) {
                    outEv.type = EV_NOTEON;
                    outEv.value = midiSeq(l1)->vel;
                    outEv.channel = midiSeq(l1)->channelOut;
                    midiSeq(l1)->getNextNote(&seqSample, tick);
                    length = midiSeq(l1)->notelength;
                    outport = midiSeq(l1)->portOut;
                    if (!midiSeq(l1)->isMuted) {
                        if (!seqSample.muted) {
                            outEv.data = seqSample.value;
                            driver->sendMidiEvent(outEv, seqSample.tick, outport, length);
                        }
                    }
                }
            }
            if (!l1)
                nextMinSeqTick = midiSeq(l1)->nextTick;
            else if (midiSeq(l1)->nextTick < nextMinSeqTick)
                nextMinSeqTick = midiSeq(l1)->nextTick;
        }
        driver->requestEchoAt(nextMinSeqTick, 0);
    }

    //Arp Note queueing
    if ((tick + 8) >= nextMinArpTick) {
        for (l1 = 0; l1 < midiArpCount(); l1++) {
            if ((gotArpKbdTrig && echo_from_trig && midiArp(l1)->wantTrigByKbd())
                    || (!gotArpKbdTrig && !echo_from_trig)) {
                gotArpKbdTrig = false;
                if ((tick + 8) >= midiArp(l1)->nextTick) {
                    outEv.type = EV_NOTEON;
                    outEv.channel = midiArp(l1)->channelOut;
                    midiArp(l1)->newRandomValues();
                    midiArp(l1)->prepareCurrentNote(tick + schedDelayTicks);
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
                                driver->sendMidiEvent(outEv, note_tick, outport, length);
                                l2++;
                            }
                        }
                    }
                    midiArp(l1)->nextTick = midiArp(l1)->getNextNoteTick();
                }
            }
            if (!l1)
                nextMinArpTick = midiArp(l1)->nextTick - schedDelayTicks;
            else if (midiArp(l1)->nextTick < nextMinArpTick + schedDelayTicks)
                nextMinArpTick = midiArp(l1)->nextTick - schedDelayTicks;
        }

        if (0 > nextMinArpTick) nextMinArpTick = 0;
        driver->requestEchoAt(nextMinArpTick, 0);
    }
}

bool Engine::midi_event_received_callback(void * context, MidiEvent ev)
{
  return ((Engine *)context)->eventCallback(ev);
}

bool Engine::eventCallback(MidiEvent inEv)
{
    bool unmatched;
    int l1;
    unmatched = true;
    int tick = driver->getCurrentTick();

    /* Does this cost time or other problems? The signal is sent to the LogWidget.*/
    if (sendLogEvents) emit midiEventReceived(inEv, tick);

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
                if (!midiLearnFlag)
                    sendController(inEv.data, inEv.channel, inEv.value);
                else
                    learnController(inEv.data, inEv.channel);
                unmatched = false;
            }
        }
        return unmatched;
    }

    if (inEv.type == EV_NOTEON) {
        for (l1 = 0; l1 < midiLfoCount(); l1++) {
            if (midiLfo(l1)->wantEvent(inEv)) {
                unmatched = false;

                midiLfo(l1)->handleNote(inEv.data, inEv.value, tick);

                if (inEv.value && midiLfo(l1)->wantTrigByKbd()) {
                    nextMinLfoTick = tick;
                    midiLfo(l1)->nextTick = nextMinLfoTick + schedDelayTicks;
                    gotLfoKbdTrig = true;
                    driver->requestEchoAt(nextMinLfoTick, true);
                }
            }
        }
        for (l1 = 0; l1 < midiSeqCount(); l1++) {
            if (midiSeq(l1)->wantEvent(inEv)) {
                unmatched = false;

                midiSeq(l1)->handleNote(inEv.data, inEv.value, tick);

                if (inEv.value && midiSeq(l1)->wantTrigByKbd()) {
                    nextMinSeqTick = tick;
                    midiSeq(l1)->nextTick = nextMinSeqTick + schedDelayTicks;
                    gotSeqKbdTrig = true;
                    driver->requestEchoAt(nextMinSeqTick, true);
                }
            }
        }
        for (l1 = 0; l1 < midiArpCount(); l1++) {
            if (midiArp(l1)->wantEvent(inEv)) {
                unmatched = false;

                midiArp(l1)->handleNote(inEv.data, inEv.value, tick, 1);

                if (inEv.value && midiArp(l1)->wantTrigByKbd()) {
                    nextMinArpTick = tick;
                    midiArp(l1)->nextTick = nextMinArpTick + schedDelayTicks;
                    gotArpKbdTrig = true;
                    driver->requestEchoAt(nextMinArpTick, true);
                }
            }
        }
        return unmatched;
    }
    return unmatched;
}

void Engine::sendController(int ccnumber, int channel, int value)
{
    int l1;

    grooveWidget->handleController(ccnumber, channel, value);

    for (l1 = 0; l1 < arpWidgetCount(); l1++)
        arpWidget(l1)->handleController(ccnumber, channel, value);
    for (l1 = 0; l1 < lfoWidgetCount(); l1++)
        lfoWidget(l1)->handleController(ccnumber, channel, value);
    for (l1 = 0; l1 < seqWidgetCount(); l1++)
        seqWidget(l1)->handleController(ccnumber, channel, value);
}

void Engine::learnController(int ccnumber, int channel)
{
    if (midiLearnWindowID == -1) {
        grooveWidget->midiControl->appendMidiCC(midiLearnID,
                ccnumber, channel, -100, 100);
        midiLearnFlag = false;
        return;
        }
    int min = (midiLearnID) ? 0 : 127; //if control is toggle min=max
    if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Arp")) {
        arpWidget(midiLearnModuleID)->midiControl->appendMidiCC(midiLearnID,
                ccnumber, channel, min, 127);
    }
    if (moduleWindow(midiLearnWindowID)->objectName().startsWith("LFO")) {
        lfoWidget(midiLearnModuleID)->midiControl->appendMidiCC(midiLearnID,
                ccnumber, channel, min, 127);
    }
    if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Seq")) {
        seqWidget(midiLearnModuleID)->midiControl->appendMidiCC(midiLearnID,
                ccnumber, channel, min, 127);
    }

    midiLearnFlag = false;
}

void Engine::resetTicks(int curtick)
{
    int l1;

    for (l1 = 0; l1 < midiArpCount(); l1++) {
        midiArp(l1)->foldReleaseTicks(curtick);
        midiArp(l1)->initArpTick(0);
        midiArp(l1)->nextTick = 0;
    }
    for (l1 = 0; l1 < midiLfoCount(); l1++) {
        midiLfo(l1)->setFramePtr(0);
        midiLfo(l1)->nextTick = 0;
    }
    for (l1 = 0; l1 < midiSeqCount(); l1++) {
        midiSeq(l1)->setCurrentIndex(0);
        midiSeq(l1)->nextTick = 0;
    }
    nextMinLfoTick = 0;
    nextMinSeqTick = 0;
    nextMinArpTick = 0;
}

void Engine::setMidiControllable(bool on)
{
    midiControllable = on;
    modified = true;
}

void Engine::setMidiLearn(int moduleWindowID, int moduleID, int controlID)
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

void Engine::setTempo(int bpm)
{
    driver->setTempo(bpm);
    modified = true;
}

void Engine::setSendLogEvents(bool on)
{
    sendLogEvents = on;
}

void Engine::tr_state_cb(bool on, void *context)
{
    if  (((Engine  *)context)->ready) {
        if (((Engine  *)context)->driver->useJackSync) {
           ((Engine  *)context)->setStatus(on);
        }
    }
}
