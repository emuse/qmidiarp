/**
 * @file engine.cpp
 * @brief Implementation of the Engine class
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
 *
 */

#include <QString>

#include <iostream>

#include "seqdriver.h"
#include "engine.h"


Engine::Engine(GlobStore *p_globStore, GrooveWidget *p_grooveWidget, int p_portCount, bool p_alsamidi, QWidget *parent) : QThread(parent), modified(false)
{
    ready = false;

    logEventBuffer.resize(128);
    logTickBuffer.resize(128);
    logEventCount = 0;

    midiControl = new MidiControl();
    midiControl->ID = -3;
    midiControl->parentDockID = -3;
    connect(midiControl, SIGNAL(setMidiLearn(int, int, int)),
            this, SLOT(setMidiLearn(int, int, int)));

    globStoreWidget = p_globStore;
    connect(globStoreWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            this, SLOT(setMidiLearn(int, int, int)));

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
        driver = new JackDriver(portCount, this, tr_state_cb, midi_event_received_callback, tick_callback, tempo_callback);
    }
    else {
    // In case of ALSA MIDI with Jack Transport sync, JackDriver is instantiated with 0 ports
    // a pointer to jackSync has to be passed to driver
        jackSync = new JackDriver(0, this, tr_state_cb, midi_event_received_callback, tick_callback, tempo_callback);
        driver = new SeqDriver(jackSync, portCount, this, midi_event_received_callback, tick_callback);
    }

    midiLearnFlag = false;
    midiControllable = true;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    schedDelayTicks = 2;
    status = false;
    sendLogEvents = false;
    useMidiClock = false;
    currentTick = 0;
    requestTick = 0;
    tempo = 120;
    requestedTempo = 120;

    restoreRequest = -1;
    restoreModIx = 0;
    restoreModType = 'X';
    restoreModWindowIndex = -1;
    restoreTick = -1;
    schedRestoreLocation = -1;

    nextMinArpTick = 0;
    nextMinLfoTick = 0;
    nextMinSeqTick = 0;
    resetTicks(0);
    dispTimer = new MTimer();
    connect(dispTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
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

void Engine::renameDock(const QString& name, int parentDockID, int widgetID)
{
    moduleWindow(parentDockID)->setWindowTitle(name);
    if (name.startsWith('S')) seqWidget(widgetID)->parStore->topButton->setText(name);
    if (name.startsWith('L')) lfoWidget(widgetID)->parStore->topButton->setText(name);
    if (name.startsWith('A')) arpWidget(widgetID)->parStore->topButton->setText(name);
    setModified(true);
}


void Engine::updateIDs(int curID)
{
    int l1, tempDockID;
    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->manageBox->ID = l1;
        arpWidget(l1)->midiControl->ID = l1;
        arpWidget(l1)->setProperty("widgetID", l1);
        tempDockID = arpWidget(l1)->manageBox->parentDockID;
        if (tempDockID > curID) {
            arpWidget(l1)->manageBox->parentDockID = tempDockID - 1;
            arpWidget(l1)->midiControl->parentDockID = tempDockID - 1;
        }
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->manageBox->ID = l1;
        lfoWidget(l1)->midiControl->ID = l1;
        lfoWidget(l1)->setProperty("widgetID", l1);
        tempDockID = lfoWidget(l1)->manageBox->parentDockID;
        if (tempDockID > curID) {
            lfoWidget(l1)->manageBox->parentDockID = tempDockID - 1;
            lfoWidget(l1)->midiControl->parentDockID = tempDockID - 1;
        }
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->manageBox->ID = l1;
        seqWidget(l1)->midiControl->ID = l1;
        seqWidget(l1)->setProperty("widgetID", l1);
        tempDockID = seqWidget(l1)->manageBox->parentDockID;
        if (tempDockID > curID) {
            seqWidget(l1)->manageBox->parentDockID = tempDockID - 1;
            seqWidget(l1)->midiControl->parentDockID = tempDockID - 1;
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
    if (!moduleWindowCount()) return false;

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

    return (modified || arpmodified || lfomodified || seqmodified
            || globStoreWidget->isModified());
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

    globStoreWidget->setModified(m);
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
    int l1;
    if (!moduleWindowCount()) return;
    if (!on) {
        for (l1 = 0; l1 < midiArpCount(); l1++) {
            midiArp(l1)->clearNoteBuffer();
        }
    }
    status = on;
    driver->setTransportStatus(on);
    if (on) {
        resetTicks(driver->getCurrentTick());
        driver->requestEchoAt(0);
    }
}

void Engine::tick_callback(void * context, bool echo_from_trig)
{
  ((Engine *)context)->echoCallback(echo_from_trig);
}

void Engine::echoCallback(bool echo_from_trig)
{
    int l1, l2;
    int tick = driver->getCurrentTick();
    int note_tick = 0;
    int length;
    int outport;
    int frameptr;
    int percent;
    bool isNew;
    bool restoreFlag = (restoreRequest >= 0);
    MidiEvent outEv;

    currentTick = tick;

        //~ printf("       tick %d     ",tick);
        //~ printf("nextMinLfoTick %d  ",nextMinLfoTick);
        //~ printf("nextMinSeqTick %d  ",nextMinSeqTick);
        //~ printf("nextMinArpTick %d  \n",nextMinArpTick);

    //LFO data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextMinLfoTick) && (midiLfoCount())) {
        for (l1 = 0; l1 < midiLfoCount(); l1++) {
            if ((echo_from_trig && midiLfo(l1)->gotKbdTrig)
                    || (!midiLfo(l1)->gotKbdTrig && !echo_from_trig)) {
                if ((tick + 8) >= midiLfo(l1)->nextTick) {
                    outEv.type = EV_CONTROLLER;
                    outEv.data = midiLfo(l1)->ccnumber;
                    outEv.channel = midiLfo(l1)->channelOut;
                    frameptr = midiLfo(l1)->getFramePtr();
                    lfoWidget(l1)->cursor->updatePosition(frameptr);
                    midiLfo(l1)->getNextFrame(tick);
                    outport = midiLfo(l1)->portOut;
                    l2 = 0;
                    while (midiLfo(l1)->frame.at(l2).value > -1) {
                        if (!midiLfo(l1)->frame.at(l2).muted && !midiLfo(l1)->isMuted) {
                            outEv.value = midiLfo(l1)->frame.at(l2).value;
                            driver->sendMidiEvent(outEv, midiLfo(l1)->frame.at(l2).tick
                                , outport);
                        }
                        l2++;
                    }
                    frameptr = midiLfo(l1)->getFramePtr();
                    percent = frameptr * 100 / (midiLfo(l1)->nPoints);
                    lfoWidget(l1)->parStore->ndc->updatePercent(percent);
                    if ((restoreModType == 'L') && (l1 == restoreModIx)
                            && (!globStoreWidget->timeModeBox->currentIndex())) {
                        globStoreWidget->indicator->updatePercent(percent);
                        if (!frameptr && restoreFlag) {
                            restoreTick = midiLfo(l1)->frame.at(l2).tick;
                            restoreFlag = false;
                        }
                    }
                }
            }
            if (!l1)
                nextMinLfoTick = midiLfo(l1)->nextTick - schedDelayTicks;
            else if (midiLfo(l1)->nextTick < nextMinLfoTick + schedDelayTicks)
                nextMinLfoTick = midiLfo(l1)->nextTick - schedDelayTicks;
        }
        driver->requestEchoAt(nextMinLfoTick, 0);
    }

    //Seq notes data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextMinSeqTick) && (midiSeqCount())) {
        for (l1 = 0; l1 < midiSeqCount(); l1++) {
            if ((echo_from_trig && midiSeq(l1)->gotKbdTrig)
                    || (!midiSeq(l1)->gotKbdTrig && !echo_from_trig)) {
                if ((tick + 8) >= midiSeq(l1)->nextTick) {
                    outEv.type = EV_NOTEON;
                    outEv.value = midiSeq(l1)->vel;
                    outEv.channel = midiSeq(l1)->channelOut;
                    seqWidget(l1)->cursor->updatePosition(midiSeq(l1)->getCurrentIndex());

                    midiSeq(l1)->getNextNote(&seqSample, tick);
                    length = midiSeq(l1)->notelength;
                    outport = midiSeq(l1)->portOut;
                    if ((!midiSeq(l1)->isMuted) && (!seqSample.muted)) {
                        outEv.data = seqSample.value;
                        driver->sendMidiEvent(outEv, seqSample.tick, outport, length);
                    }
                        frameptr = midiSeq(l1)->getCurrentIndex();
                        percent = frameptr * 100 / (midiSeq(l1)->nPoints);
                        seqWidget(l1)->parStore->ndc->updatePercent(percent);
                    if ((restoreModType == 'S') && (l1 == restoreModIx)
                          && (!globStoreWidget->timeModeBox->currentIndex())) {
                        globStoreWidget->indicator->updatePercent(percent);
                        if (!frameptr && restoreFlag) {
                            restoreTick = midiSeq(l1)->nextTick;
                            restoreFlag = false;
                        }
                    }
                }
            }
            if (!l1)
                nextMinSeqTick = midiSeq(l1)->nextTick - schedDelayTicks;
            else if (midiSeq(l1)->nextTick < nextMinSeqTick + schedDelayTicks)
                nextMinSeqTick = midiSeq(l1)->nextTick - schedDelayTicks;
        }
        driver->requestEchoAt(nextMinSeqTick, 0);
    }

    //Arp Note queueing
    if ((tick + 8) >= nextMinArpTick) {
        for (l1 = 0; l1 < midiArpCount(); l1++) {
            if ((echo_from_trig && midiArp(l1)->gotKbdTrig)
                    || (!midiArp(l1)->gotKbdTrig && !echo_from_trig)) {
                if ((tick + 8) >= midiArp(l1)->nextTick) {
                    outEv.type = EV_NOTEON;
                    outEv.channel = midiArp(l1)->channelOut;
                    midiArp(l1)->newRandomValues();
                    midiArp(l1)->prepareCurrentNote(tick + schedDelayTicks);
                    note_tick = midiArp(l1)->returnTick;
                    length = midiArp(l1)->returnLength * 4;
                    outport = midiArp(l1)->portOut;
                    isNew = midiArp(l1)->returnIsNew;
                    arpWidget(l1)->screen->updateScreen(midiArp(l1)->getGrooveIndex());
                    if (!midiArp(l1)->returnNote.isEmpty()) {
                        if (isNew && midiArp(l1)->returnVelocity.at(0)) {
                            l2 = 0;
                            while(midiArp(l1)->returnNote.at(l2) >= 0) {
                                outEv.data = midiArp(l1)->returnNote.at(l2);
                                outEv.value = midiArp(l1)->returnVelocity.at(l2);
                                driver->sendMidiEvent(outEv, note_tick, outport, length);
                                l2++;
                            }
                        }
                    }
                    frameptr = midiArp(l1)->getGrooveIndex() - 1;
                    if (midiArp(l1)->nPoints)
                        percent = frameptr * 100 / (midiArp(l1)->nPoints);
                    else
                        percent = 0;
                    arpWidget(l1)->parStore->ndc->updatePercent(percent);
                    if ((restoreModType == 'A') && (l1 == restoreModIx)
                            && (!globStoreWidget->timeModeBox->currentIndex())) {
                        globStoreWidget->indicator->updatePercent(percent);
                        if (!frameptr && restoreFlag) {
                            restoreTick = note_tick;
                            restoreFlag = false;
                        }
                    }
                }
            }
            if (!l1)
                nextMinArpTick = midiArp(l1)->nextTick - schedDelayTicks;
            else if (midiArp(l1)->nextTick < nextMinArpTick + schedDelayTicks)
                nextMinArpTick = midiArp(l1)->nextTick - schedDelayTicks;
        }

        if (0 > nextMinArpTick) nextMinArpTick = 0;
        if (midiArpCount()) driver->requestEchoAt(nextMinArpTick, 0);
    }

    if (restoreFlag && (globStoreWidget->timeModeBox->currentIndex())) {
        percent = 100 * (currentTick - requestTick) / (restoreTick - requestTick);
        globStoreWidget->indicator->updatePercent(percent);
    }

    if ((restoreTick > -1)
        && (!midiArpCount() || (nextMinArpTick + schedDelayTicks >= restoreTick))
        && (!midiLfoCount() || (nextMinLfoTick + schedDelayTicks >= restoreTick))
        && (!midiSeqCount() || (nextMinSeqTick + schedDelayTicks >= restoreTick))) {
        restoreTick = -1;
        restoreFlag = false;
        // TODO: At term the following should be restore() instead of
        // schedRestore(). restore is not yet fit for realtime. But
        // testing seems positive this way.
        schedRestore(restoreRequest);
    }
}

bool Engine::midi_event_received_callback(void * context, MidiEvent ev)
{
  return ((Engine *)context)->eventCallback(ev);
}

bool Engine::eventCallback(MidiEvent inEv)
{
    bool unmatched;
    bool no_collision = false;
    int l1;
    unmatched = true;
    int tick = driver->getCurrentTick();

    if (sendLogEvents) {
        logEventBuffer.replace(logEventCount, inEv);
        logTickBuffer.replace(logEventCount, tick);
        logEventCount++;
    }

    if (useMidiClock){
        if (inEv.type == EV_START) {
            setStatus(true);
            return(false);
        }
        if (inEv.type == EV_STOP) {
            setStatus(false);
            return(false);
        }
    }

    for (l1 = 0; l1 < midiLfoCount(); l1++) {
        unmatched = midiLfo(l1)->handleEvent(inEv, tick);
        if (midiLfo(l1)->gotKbdTrig) {
            nextMinLfoTick = midiLfo(l1)->nextTick;
            no_collision = driver->requestEchoAt(nextMinLfoTick, true);
            if (!no_collision) midiLfo(l1)->gotKbdTrig = false;
        }
    }
    for (l1 = 0; l1 < midiSeqCount(); l1++) {
        unmatched = midiSeq(l1)->handleEvent(inEv, tick);
        if (midiSeq(l1)->gotKbdTrig) {
            nextMinSeqTick = midiSeq(l1)->nextTick;
            no_collision = driver->requestEchoAt(nextMinSeqTick, true);
            if (!no_collision) midiSeq(l1)->gotKbdTrig = false;
        }
    }
    for (l1 = 0; l1 < midiArpCount(); l1++) {
        unmatched = midiArp(l1)->handleEvent(inEv, tick, 1);
        if (midiArp(l1)->gotKbdTrig) {
            nextMinArpTick = midiArp(l1)->nextTick;
            no_collision = driver->requestEchoAt(nextMinArpTick, true);
            if (!no_collision) midiArp(l1)->gotKbdTrig = false;
        }
    }

    if (inEv.type == EV_CONTROLLER) {
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

void Engine::showAllIOPanels(bool on)
{
    int l1;

    for (l1 = 0; l1 < arpWidgetCount(); l1++)
        arpWidget(l1)->hideInOutBoxAction->setChecked(on);
    for (l1 = 0; l1 < lfoWidgetCount(); l1++)
        lfoWidget(l1)->hideInOutBoxAction->setChecked(on);
    for (l1 = 0; l1 < seqWidgetCount(); l1++)
        seqWidget(l1)->hideInOutBoxAction->setChecked(on);
}

void Engine::sendController(int ccnumber, int channel, int value)
{
    int l1;

    handleController(ccnumber, channel, value);
    grooveWidget->handleController(ccnumber, channel, value);
    globStoreWidget->handleController(ccnumber, channel, value);

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
        grooveWidget->midiControl->requestAppendMidiCC(midiLearnID,
                ccnumber, channel, -100, 100);
        midiLearnFlag = false;
        return;
        }

    if (midiLearnWindowID == -2) {
        globStoreWidget->midiControl->requestAppendMidiCC(midiLearnID,
                ccnumber, channel, 0, 127);
        midiLearnFlag = false;
        return;
        }

    if (midiLearnWindowID == -3) {
        midiControl->requestAppendMidiCC(midiLearnID,
                ccnumber, channel, 0, 127);
        midiLearnFlag = false;
        return;
        }

    int min = (midiLearnID) ? 0 : 127; //if control is toggle min=max

    if (midiLearnWindowID == 1) {
        arpWidget(midiLearnModuleID)->midiControl->requestAppendMidiCC(midiLearnID,
                ccnumber, channel, min, 127);
    }
    if (midiLearnWindowID == 2) {
        lfoWidget(midiLearnModuleID)->midiControl->requestAppendMidiCC(midiLearnID,
                ccnumber, channel, min, 127);
    }
    if (midiLearnWindowID == 3) {
        seqWidget(midiLearnModuleID)->midiControl->requestAppendMidiCC(midiLearnID,
                ccnumber, channel, min, 127);
    }

    midiLearnFlag = false;
}

void Engine::handleController(int ccnumber, int channel, int value)
{
    if (!midiControl->ccList.count()) return;

    MidiCC midiCC = midiControl->ccList.at(0);
    int sval, min, max;

    if ((ccnumber != midiCC.ccnumber) || (channel != midiCC.channel)) return;

    min = midiCC.min;
    max = midiCC.max;
    sval = min + ((double)value * (max - min) / 127);
    requestedTempo = sval;
}

void Engine::resetTicks(int curtick)
{
    int l1;

    for (l1 = 0; l1 < midiArpCount(); l1++) {
        midiArp(l1)->foldReleaseTicks(driver->trStartingTick - curtick);
        midiArp(l1)->setNextTick(curtick);
        if (!l1) nextMinArpTick = midiArp(l1)->nextTick;
        if (midiArp(l1)->nextTick < nextMinArpTick)
            nextMinArpTick=midiArp(l1)->nextTick;
    }

    for (l1 = 0; l1 < midiLfoCount(); l1++) {
        midiLfo(l1)->setNextTick(curtick);
        if (!l1) nextMinLfoTick = midiLfo(l1)->nextTick;
        if (midiLfo(l1)->nextTick < nextMinLfoTick)
            nextMinLfoTick=midiLfo(l1)->nextTick;
    }

    for (l1 = 0; l1 < midiSeqCount(); l1++) {
        midiSeq(l1)->setNextTick(curtick);
        if (!l1) nextMinSeqTick = midiSeq(l1)->nextTick;
        if (midiSeq(l1)->nextTick < nextMinSeqTick)
            nextMinSeqTick=midiSeq(l1)->nextTick;
    }
}

void Engine::setMidiControllable(bool on)
{
    midiControllable = on;
    modified = true;
}

void Engine::setUseMidiClock(bool on)
{
    setStatus(false);
    driver->setUseMidiClock(on);
    useMidiClock = on;
    modified = true;
}

void Engine::setUseJackTransport(bool on)
{
    driver->setUseJackTransport(on);
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
        if (midiLearnWindowID >= 0) {
            QChar test = moduleWindow(midiLearnWindowID)->objectName().at(0);
            if (test == 'A') midiLearnWindowID = 1;
            if (test == 'L') midiLearnWindowID = 2;
            if (test == 'S') midiLearnWindowID = 3;
        }
        midiLearnModuleID = moduleID;
        midiLearnID = controlID;
    }
}

void Engine::setTempo(double bpm)
{
    driver->requestTempo(bpm);
    requestedTempo = bpm;

    tempo=bpm;
    modified = true;
}

void Engine::tempo_callback(double bpm, void *context)
{
    ((Engine *)context)->requestedTempo = bpm;
}

void Engine::setSendLogEvents(bool on)
{
    sendLogEvents = on;
    modified = true;
}

void Engine::tr_state_cb(bool on, void *context)
{
    if  (((Engine  *)context)->ready) {
        if (((Engine  *)context)->driver->useJackSync) {
           ((Engine  *)context)->setStatus(on);
        }
    }
}

void Engine::store(int ix)
{
    int l1;

    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->storeParams(ix);
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->storeParams(ix);
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->storeParams(ix);
    }
}

void Engine::requestRestore(int ix)
{
    if (status == false) {
        restore(ix);
        return;
    }

    restoreRequest = ix;

    globStoreWidget->setDispState(ix, 2);
    if (globStoreWidget->timeModeBox->currentIndex()) {
        requestTick = currentTick;
        restoreTick = TPQN * (2 + globStoreWidget->switchAtBeatBox
            ->currentIndex() + currentTick / TPQN);
    }
}

void Engine::schedRestore(int ix)
{
    schedRestoreLocation = ix;
}

void Engine::restore(int ix)
{
    int l1;
    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->restoreParams(ix);
        arpWidget(l1)->parStore->oldRestoreRequest = ix;
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->restoreParams(ix);
        lfoWidget(l1)->parStore->oldRestoreRequest = ix;
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->restoreParams(ix);
        seqWidget(l1)->parStore->oldRestoreRequest = ix;
    }
    restoreRequest = -1;

    globStoreWidget->requestDispState(ix, 1);
}

void Engine::removeParStores(int ix)
{
    int l1;
    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->parStore->removeLocation(ix);
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->parStore->removeLocation(ix);
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->parStore->removeLocation(ix);
    }
}

void Engine::updateGlobRestoreTimeModule(int windowIndex)
{
    restoreModType = globStoreWidget->timeModuleBox
                        ->itemText(windowIndex).at(0);

    restoreModIx = moduleWindowList.at(windowIndex)->widget()
                        ->property("widgetID").toInt();
}

void Engine::updateDisplay()
{
    int l1;
    // The following is a test whether the display update loop is fast
    // enough to restore the parameters in time. This cannot be done in
    // realtime currently, since it requires reworking the restore routines
    // entirely. Testing seems positive, but TODO.

    if (schedRestoreLocation >= 0) {
        restore(schedRestoreLocation);
        schedRestoreLocation = -1;
    }

    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->updateDisplay();
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->updateDisplay();
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->updateDisplay();
    }

    globStoreWidget->updateDisplay();
    grooveWidget->updateDisplay();
    midiControl->update();

    if ((sendLogEvents) && (logEventCount)) {
        for (l1 = 0; l1 < logEventCount; l1++) {
            emit midiEventReceived(logEventBuffer.at(l1), logTickBuffer.at(l1));
        }
        logEventCount = 0;
    }

    if (requestedTempo != tempo) {
        tempo = requestedTempo;
        emit tempoUpdated(tempo);
    }
}

MTimer::MTimer()
{
    start();
}

void MTimer::run() {

    while(true) {
        // set fixed to 5000us
        usleep(5000);
        emit timeout();
    }
}
