/**
 * @file engine.cpp
 * @brief Implementation of the Engine class
 *
 *
 *      Copyright 2009 - 2023 <qmidiarp-devel@lists.sourceforge.net>
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

#include <iostream>
#include "engine.h"


Engine::Engine(GlobStore *p_globStore, GrooveWidget *p_grooveWidget, 
            int p_portCount, bool p_alsamidi, QWidget *parent) 
            : QObject(parent), modified(false)
{
    ready = false;

    logEventBuffer.resize(128);
    logTickBuffer.resize(128);
    logEventCount = 0;

    midiControl = new MidiControl;
    midiControl->ID = -3;
    connect(midiControl, SIGNAL(setMidiLearn(int, int)),
            this, SLOT(setMidiLearn(int, int)));

    globStoreWidget = p_globStore;
    connect(globStoreWidget->midiControl, SIGNAL(setMidiLearn(int, int)),
            this, SLOT(setMidiLearn(int, int)));

    grooveWidget = p_grooveWidget;
    connect(grooveWidget, SIGNAL(newGrooveTick(int)),
            this, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)),
            this, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)),
            this, SLOT(setGrooveLength(int)));
    connect(grooveWidget->midiControl, SIGNAL(setMidiLearn(int, int)),
            this, SLOT(setMidiLearn(int, int)));
    portCount = p_portCount;

    if (!p_alsamidi) {
        driver = new JackDriver(portCount, this, tr_state_cb, 
                midi_event_received_callback, tick_callback, tempo_callback);
    }
#ifdef HAVE_ALSA
    else {
    // In case of ALSA MIDI with Jack Transport sync, JackDriver is 
    // instantiated with 0 ports
    // a pointer to jackSync has to be passed to driver
        jackSync = new JackDriver(0, this, tr_state_cb, 
                midi_event_received_callback, tick_callback, tempo_callback);
        driver = new SeqDriver(jackSync, portCount, this, 
                midi_event_received_callback, tick_callback);
    }
#endif

    alsaMidi = p_alsamidi;
    alsaSyncTol = 2;
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
    restoreTick = -1;
    schedRestoreLocation = -1;

    nextMinTick = 0;
    resetTicks(0);
    dispTimer = new MTimer();
    connect(dispTimer, SIGNAL(timeout()), this, SLOT(updateDisplay()));
    ready = true;
}

Engine::~Engine()
{
    delete driver;
    delete midiControl;
}

void Engine::updatePatternPresets(const QString& n, const QString& p, int index)
{
    for (int l1 = 0; l1 < midiWorkerCount(); l1++) {
        if (moduleWidget(l1)->name.at(0) == 'A')
            ((ArpWidget *)moduleWidget(l1))->updatePatternPresets(n, p, index);
    }
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

void Engine::sendGroove(int ix)
{
    if (ix  == -1)
        for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
            moduleWidget(l1)->newGrooveValues(grooveTick, grooveVelocity, grooveLength);
        }
    else
        moduleWidget(ix)->newGrooveValues(grooveTick, grooveVelocity, grooveLength);
}

//Module management

MidiWorker *Engine::midiWorker(int index)
{
    if (index == -1) index = midiWorkerList.count() - 1;
    return(midiWorkerList.at(index));
}

void Engine::addMidiWorker(MidiWorker *midiWorker)
{
    midiWorkerList.append(midiWorker);
    modified = true;
}

void Engine::removeMidiWorker(MidiWorker *midiWorker)
{
    if (status && (midiWorkerCount() < 1)) {
        setStatus(false);
    }
    int i = midiWorkerList.indexOf(midiWorker);
    if (i != -1)
        delete midiWorkerList.takeAt(i);
}

int Engine::midiWorkerCount()
{
    return(midiWorkerList.count());
}

ModuleWidget *Engine::moduleWidget(int index)
{
    if (index == -1) index = moduleWidgetList.count() - 1;
    return(moduleWidgetList.at(index));
}

void Engine::addModuleWidget(ModuleWidget *moduleWidget)
{
    addMidiWorker(moduleWidget->midiWorker);
    moduleWidgetList.append(moduleWidget);
    sendGroove(moduleWidgetCount() - 1);
    updateGlobRestoreTimeModule(restoreModIx);

    modified = true;
}

void Engine::removeModuleWidget(ModuleWidget *moduleWidget)
{
    moduleWidgetList.removeOne(moduleWidget);
    removeMidiWorker(moduleWidget->midiWorker);

    delete moduleWidget->parent();
    modified = true;
}

int Engine::moduleWidgetCount(const char mtype)
{
    if (mtype == ' ') return moduleWidgetList.count();
    
    int count = 0;
    for (int l1 = 0; l1 < moduleWidgetList.count(); l1++) {
        if (moduleWidget(l1)->name.at(0) == mtype) count++;
    }
    return count;
}

void Engine::renameDock(const QString& name, int ID)
{
    ( (QDockWidget *)(moduleWidget(ID)->parent()) )->setWindowTitle(name);
    moduleWidget(ID)->parStore->topButton->setText(name);
    setModified(true);
}


void Engine::updateIDs(int curID)
{
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->setID(l1);
        if (l1 > curID) {
            moduleWidget(l1)->setID(l1 - 1);
        }
    }
}

//general

void Engine::setCompactStyle(bool on)
{
    int l1;
    if (on) {
        for (l1 = 0; l1 < moduleWidgetCount(); l1++) {
            moduleWidget(l1)->setStyleSheet(COMPACT_STYLE);
        }
    }
    else {
        for (l1 = 0; l1 < moduleWidgetCount(); l1++) {
            moduleWidget(l1)->setStyleSheet("");
        }
    }
}

bool Engine::isModified()
{
    if (!moduleWidgetCount()) return false;

    bool modmodified = false;

    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        if ( moduleWidget(l1)->isModified() ) {
            modmodified = true;
            break;
        }
    }

    return (modified || modmodified || globStoreWidget->isModified());
}

void Engine::setModified(bool m)
{
    modified = m;

    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->needsGUIUpdate = false;
        moduleWidget(l1)->setModified(m);
    }

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
    if (!moduleWidgetCount()) return;
    if (!on) {
        for (int l1 = 0; l1 < midiWorkerCount(); l1++) {
            midiWorker(l1)->clearNoteBuffer();
        }
    }
    status = on;
    driver->setTransportStatus(on);
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->parStore->engineRunning = on;
    }
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
    int tol = alsaSyncTol;
    int tick = driver->getCurrentTick();
    bool restoreFlag = (restoreRequest >= 0);
    
    currentTick = tick;

        //~ printf("       tick %d     ",tick);
        //~ printf("nextMinTick %d  ",nextMinTick);
    
    //Module data request and queueing
    for (l1 = 0; l1 < moduleWidgetCount(); l1++) {
        if (moduleWidget(l1)->prepareNextFrame(echo_from_trig, tol, tick, 
                                       &restoreTick, &restoreFlag)) {
            
            l2 = 0;
            while (midiWorker(l1)->outFrame[l2].data > -1) {
                if (!midiWorker(l1)->outFrame[l2].muted && !midiWorker(l1)->isMuted) {
                    MidiEvent outEv = mkMidiEvent(
                                        midiWorker(l1)->eventType, 
                                        midiWorker(l1)->channelOut, 
                                        midiWorker(l1)->outFrame[l2].data, 
                                        midiWorker(l1)->outFrame[l2].value);
                    driver->sendMidiEvent(outEv, 
                                        midiWorker(l1)->outFrame[l2].tick,
                                        midiWorker(l1)->portOut, 
                                        midiWorker(l1)->returnLength);
                }
                l2++;
            }
        }
    }
    
    //Calculate timing of next echo to be requested (minimum of all modules)
    for (l1 = 0; l1 < midiWorkerCount(); l1++) {
        int64_t nt = midiWorker(l1)->nextTick - schedDelayTicks;
        if (nt < nextMinTick + schedDelayTicks || !l1) nextMinTick = nt;            
    }
    if (nextMinTick < 0) nextMinTick = 0;
    if (moduleWidgetCount()) driver->requestEchoAt(nextMinTick, 0);

    //Update GlobStore master indicator pacman
    if (restoreFlag && (globStoreWidget->timeModeBox->currentIndex())) {
        int percent = 100 * (currentTick - requestTick) / (restoreTick - requestTick);
        globStoreWidget->indicator->updatePercent(percent);
    }

    //Check for parameter restore requests
    if ((restoreTick > -1)
        && (!moduleWidgetCount() || (nextMinTick + schedDelayTicks >= restoreTick))) {
        restoreTick = -1;
        schedRestore(restoreRequest);
    }
}

bool Engine::midi_event_received_callback(void * context, MidiEvent ev)
{
  return ((Engine *)context)->eventCallback(ev);
}

bool Engine::eventCallback(MidiEvent inEv)
{
    bool unmatched = true;
    bool no_collision = false;
    int l1;

    int tick = driver->getCurrentTick();

    if (sendLogEvents) {
        logEventBuffer.replace(logEventCount, inEv);
        logTickBuffer.replace(logEventCount, tick);
        logEventCount++;
    }

    /* from here on we handle Note Off events as Note On / Vel 0 events */
    if (inEv.type == EV_NOTEOFF) {
        inEv.type = EV_NOTEON;
        inEv.value = 0;
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
    if (midiLearnFlag && inEv.type == EV_NOTEON) {   //input range midi learn
        if (midiLearnWindowID > 0) {
            if (midiLearnID == 10) {
                moduleWidget(midiLearnModuleID)->indexIn[0]->setValue(inEv.data);
            }
            else if (midiLearnID == 11) {
                moduleWidget(midiLearnModuleID)->indexIn[1]->setValue(inEv.data);
            }
            midiLearnFlag = false;
        }
    }
    for (l1 = 0; l1 < midiWorkerCount(); l1++) {
        if (status && moduleWidget(l1)->name.at(0) == 'A') {
            unmatched = midiWorker(l1)->handleEvent(inEv, tick, 1);
        }
        else {
            unmatched = midiWorker(l1)->handleEvent(inEv, tick);
        }
        if (midiWorker(l1)->gotKbdTrig) {
            nextMinTick = midiWorker(l1)->nextTick;
            no_collision = driver->requestEchoAt(nextMinTick, true);
            if (!no_collision) midiWorker(l1)->gotKbdTrig = false;
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
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->hideInOutBoxAction->setChecked(on);
    }
}

void Engine::sendController(int ccnumber, int channel, int value)
{
    handleController(ccnumber, channel, value);
    grooveWidget->handleController(ccnumber, channel, value);
    globStoreWidget->handleController(ccnumber, channel, value);

    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->handleController(ccnumber, channel, value);
    }
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
        midiControl->requestAppendMidiCC(midiLearnID, ccnumber, channel, 0, 127);
        midiLearnFlag = false;
        return;
        }

    int min = (midiLearnID) ? 0 : 127; //if control is toggle min=max

    if (midiLearnWindowID > 0) {
        moduleWidget(midiLearnModuleID)->midiControl
            ->requestAppendMidiCC(midiLearnID, ccnumber, channel, min, 127);
    }
    midiLearnFlag = false;
}

void Engine::handleController(int ccnumber, int channel, int value)
{
    if (!midiControl->ccList.count()) return;

    MidiCC midiCC = midiControl->ccList.at(0);
    int sval, min, max;

    if ((ccnumber != midiCC.ccnumber) || (channel != midiCC.channel)) return;
    if ((driver->useJackSync) || (driver->useMidiClock)) return;
    min = midiCC.min;
    max = midiCC.max;
    sval = min + ((double)value * (max - min) / 127);
    requestedTempo = sval;
}

void Engine::resetTicks(int curtick)
{
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        if (status && moduleWidget(l1)->name.at(0) == 'A') {
            midiWorker(l1)->foldReleaseTicks(driver->trStartingTick - curtick);
        }
        midiWorker(l1)->setNextTick(curtick);
        if (curtick == 0) {
            midiWorker(l1)->currentRepetition = 0;
        }
        if (!l1) nextMinTick = midiWorker(l1)->nextTick;
        if (midiWorker(l1)->nextTick < nextMinTick)
            nextMinTick=midiWorker(l1)->nextTick;
    }
}

void Engine::setMidiControllable(bool on)
{
    midiControllable = on;
    modified = true;
}

void Engine::setUseMidiClock(bool on)
{
    if (alsaMidi and on)
        alsaSyncTol = 3000;
    else
        alsaSyncTol = 2;
    
    setStatus(false);
    driver->setUseMidiClock(on);
    useMidiClock = on;
    modified = true;
}

void Engine::setUseJackTransport(bool on)
{
    if (alsaMidi and on)
        alsaSyncTol = 1000;
    else
        alsaSyncTol = 2;
    
    driver->setUseJackTransport(on);
    modified = true;
}

void Engine::setMidiLearn(int moduleWidgetID, int controlID)
{
    if (0 > controlID) {
        midiLearnFlag = false;
        return;
    }
    else {
        midiLearnFlag = true;
        midiLearnWindowID = moduleWidgetID;
        if (midiLearnWindowID >= 0) midiLearnWindowID = 1;
        midiLearnModuleID = moduleWidgetID;
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
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->storeParams(ix);
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
            ->currentIndex()) + currentTick;
    }
}

void Engine::schedRestore(int ix)
{
    schedRestoreLocation = ix;
}

void Engine::restore(int ix)
{
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->parStore->setRestoreRequest(ix, true);
        moduleWidget(l1)->parStore->oldRestoreRequest = ix;
    }

    restoreRequest = -1;

    globStoreWidget->requestDispState(ix, 1);
}

void Engine::removeParStores(int ix)
{
    for (int l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->parStore->removeLocation(ix);
    }
}

void Engine::updateGlobRestoreTimeModule(int windowIndex)
{
    moduleWidget(restoreModIx)->parStore->isRestoreMaster = false;
    moduleWidget(windowIndex)->parStore->isRestoreMaster = true;

    restoreModIx = windowIndex;
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

    for (l1 = 0; l1 < moduleWidgetCount(); l1++) {
        moduleWidget(l1)->updateDisplay();
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
