/*!
 * @file midiseq.cpp
 * @brief Implements the MidiSeq MIDI worker class for the Seq Module.
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
#include <cmath>
#include "midiseq.h"


MidiSeq::MidiSeq()
{
    enableNoteIn = true;
    enableNoteOff = false;
    enableVelIn = true;
    recordMode = false;
    trigByKbd = false;
    restartByKbd = false;
    trigLegato = false;
    enableLoop = true;
    gotKbdTrig = false;
    currentRecStep = 0;
    seqFinished = false;
    reverse = false;
    pingpong = false;
    backward = false;
    reflect = false;
    restartFlag = false;
    curLoopMode = 0;
    noteCount = 0;
    loopMarker = 0;

    nOctaves = 4;
    baseOctave = 3;

    chIn = 0;
    queueTempo = 100.0;
    vel = 0;
    velDefer = 0;
    transp = 0;
    transpDefer = 0;
    size = 4;
    res = 4;
    nPoints = 16;
    maxNPoints = 16;
    notelength = 74;
    notelengthDefer = 74;
    portOut = 0;
    channelOut = 0;
    currentIndex = 0;
    nextTick = 0;
    grooveTick = 0;
    newGrooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    isMuted = false;
    isMutedDefer = false;
    deferChanges = false;
    parChangesPending = false;
    lastMute = false;
    dataChanged = false;
    needsGUIUpdate = false;

    int lt = 0;
    int l1 = 0;
    int step = TPQN / res;
    Sample sample;
    sample.value = 60;
    customWave.resize(2048);
    muteMask.resize(2048);
    for (l1 = 0; l1 < 2048; l1++) {
            sample.tick = lt;
            sample.muted = false;
            customWave.replace(l1, sample);
            lt+=step;
    }
    muteMask.fill(false, 2048);
}

MidiSeq::~MidiSeq(){
}

void MidiSeq::setMuted(bool on)
{
    isMutedDefer = on;
    if (deferChanges) {
        parChangesPending = true;
    }
    else isMuted = on;

    needsGUIUpdate = false;
}

bool MidiSeq::handleEvent(MidiEvent inEv, int tick)
{
    if (inEv.type != EV_NOTEON) return(true);
    if (inEv.channel != chIn) return(true);
    if ((inEv.data < 36) || (inEv.data >= 84)) return(true);

    if (inEv.value) {
        /*This is a NOTE ON event*/
        if (recordMode) {
            recordNote(inEv.data);
            return(false);
        }
        if (enableNoteIn) {
            updateTranspose(inEv.data - 60);
            needsGUIUpdate = true;
        }
        if (restartByKbd && (!noteCount || trigLegato)) restartFlag = true;
        if (enableVelIn) {
            updateVelocity(inEv.value);
            needsGUIUpdate = true;
        }
        seqFinished = false;
        noteCount++;
        if (trigByKbd && ((noteCount == 1) || trigLegato)) {
            nextTick = tick + 2; //schedDelayTicks;
            gotKbdTrig = true;
        }
    }
    else {
        /*This is a NOTE OFF event*/
        if (enableNoteOff && (noteCount == 1)) seqFinished = true;
        if (noteCount) noteCount--;
    }

    return(false);
}

void MidiSeq::getNextNote(Sample *p_sample, int tick)
{
    const int frame_nticks = TPQN / res;
    Sample sample;
    int cur_grv_sft;

    gotKbdTrig = false;
    if (restartFlag) setCurrentIndex(0);
    if (!currentIndex) grooveTick = newGrooveTick;

    sample = customWave.at(currentIndex);
    advancePatternIndex();

    if (nextTick < (tick - frame_nticks)) nextTick = tick;

    sample.value+=transp;
    sample.tick = nextTick;


    cur_grv_sft = 0.01 * (grooveTick * (frame_nticks - 1));

    /* pairwise application of new groove shift */
    if (!(currentIndex % 2)) {
        cur_grv_sft = -cur_grv_sft;
        grooveTick = newGrooveTick;
    }
    nextTick += frame_nticks + cur_grv_sft;

    if (!trigByKbd && !(currentIndex % 2)) {
        /* round-up to current resolution (quantize) */
        nextTick/=frame_nticks;
        nextTick*=frame_nticks;
    }

    if (seqFinished) {
        sample.muted = true;
        currentIndex = 0;
    }
    *p_sample = sample;
}

void MidiSeq::advancePatternIndex()
{
    const int npoints = res * size;
    int pivot = abs(loopMarker);
    reflect = pingpong;

    if (curLoopMode == 6) {
        if (pivot)
            currentIndex = rand() % pivot;
        else
            currentIndex = rand() % npoints;
        return;
    }

    if (reverse) {
        if (!pivot) pivot = npoints;
        if (currentIndex == pivot - 1) applyPendingParChanges();
        currentIndex--;
        if (currentIndex == -1) {
            if (!enableLoop) seqFinished = true;
            if (reflect  || !backward) {
                reverse = false;
                currentIndex = 0;
            }
            else currentIndex = pivot - 1;
        }
        else if (currentIndex == pivot - 1) {
            if (!enableLoop) seqFinished = true;
            if (loopMarker < 0) reflect = true;
            if (loopMarker > 0) reflect = false;
            if (reflect) {
                reverse = false;
                currentIndex = pivot;
            }
            else currentIndex = npoints - 1;
        }
    }
    else {
        if (!currentIndex) applyPendingParChanges();
        currentIndex++;
        if (currentIndex == npoints) {
            if (!enableLoop) seqFinished = true;

            if (reflect || backward) {
                reverse = true;
                currentIndex = npoints - 1;
            }
            else currentIndex = pivot;
        }
        else if ((currentIndex == pivot)) {
            if (!pivot) pivot = npoints;
            if (!enableLoop) seqFinished = true;
            if (loopMarker > 0) reflect = true;
            if (loopMarker < 0) reflect = false;
            if (reflect) {
                reverse = true;
                currentIndex = pivot - 1;
            }
            else currentIndex = 0;
        }
    }
}

void MidiSeq::getData(QVector<Sample> *p_data)
{
    Sample sample;
    int lt = 0;
    const int step = TPQN / res;
    const int npoints = res * size;

    QVector<Sample> data;

    lt = step * npoints;
    data = customWave.mid(0, npoints);
    sample.value = -1;
    sample.tick = lt;
    data.append(sample);
    *p_data = data;
}

int MidiSeq::clip(int value, int min, int max, bool *outOfRange)
{
    int tmp = value;

    *outOfRange = false;
    if (tmp > max) {
        tmp = max;
        *outOfRange = true;
    } else if (tmp < min) {
        tmp = min;
        *outOfRange = true;
    }
    return(tmp);
}

void MidiSeq::updateResolution(int val)
{
    res = val;
    resizeAll();
}

void MidiSeq::updateSize(int val)
{
    size = val;
    resizeAll();
}

void MidiSeq::updateLoop(int val)
{
    backward = val&1;
    pingpong = val&2;
    enableLoop = !(val&4);
    curLoopMode = val;
}

void MidiSeq::updateNoteLength(int val)
{
    notelengthDefer = val;
    if (deferChanges) {
        parChangesPending = true;
    }
    else notelength = val;
}

void MidiSeq::updateVelocity(int val)
{
    velDefer = val;
    if (deferChanges) {
        parChangesPending = true;
    }
    else vel = val;
}

void MidiSeq::updateTranspose(int val)
{
    transpDefer = val;
    if (deferChanges) {
        parChangesPending = true;
    }
    else transp = val;
}

void MidiSeq::recordNote(int val)
{
        setRecordedNote(val);
        currentRecStep++;
        currentRecStep %= (res * size);
        dataChanged = true;
}

void MidiSeq::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

int MidiSeq::setCustomWavePoint(double mouseX, double mouseY)
{
    currentRecStep = mouseX * res * size;
    setRecordedNote(12 * (mouseY * nOctaves + baseOctave));
    return (currentRecStep);
}

int MidiSeq::mouseEvent(double mouseX, double mouseY, int buttons, int pressed)
{
    int ix = 0;

    if ((mouseY < 0) && (pressed != 2)) {
        if (mouseX < 0) mouseX = 0;
        if (buttons == 2) mouseX = - mouseX;
        setLoopMarkerMouse(mouseX);
        return (0);
    }

    if ((mouseX > 1) || (mouseX < 0) || (mouseY > 1) || (mouseY < 0)) return (0);

    if (buttons == 2) {
        if (pressed == 1) {
            lastMute = toggleMutePoint(mouseX);
            ix = lastMute;
        }
        else if (pressed == 0)
            ix = setMutePoint(mouseX, lastMute);
    }
    else if (pressed != 2) {
        ix = setCustomWavePoint(mouseX, mouseY);
    }
    dataChanged = true;

    return (ix);
}

void MidiSeq::setLoopMarkerMouse(double mouseX)
{
    const int npoints = res * size;
    if (mouseX > 0) setLoopMarker(mouseX * (double)npoints + .5);
    else setLoopMarker(mouseX * (double)npoints - .5);
}

void MidiSeq::setLoopMarker(int ix)
{
    const int npoints = res * size;
    loopMarker = ix;
    if (abs(loopMarker) >= npoints) loopMarker = 0;
    if (!loopMarker) nPoints = npoints;
    else nPoints = abs(loopMarker);
}

void MidiSeq::setRecordMode(int on)
{
    recordMode = on;
}

void MidiSeq::setRecordedNote(int note)
{
    Sample sample;

    sample = customWave.at(currentRecStep);
    sample.value = note;
    sample.tick = currentRecStep * TPQN / res;
    customWave.replace(currentRecStep, sample);
}

void MidiSeq::resizeAll()
{
    int lt = 0;
    int l1 = 0;
    const int step = TPQN / res;
    const int npoints = res * size;
    Sample sample;

    currentIndex%=npoints;
    currentRecStep%=npoints;

    if (maxNPoints < npoints) {
        for (l1 = 0; l1 < npoints; l1++) {
            if (l1 >= maxNPoints)
                muteMask.replace(l1, muteMask.at(l1 % maxNPoints));
            sample = customWave.at(l1 % maxNPoints);
            sample.tick = lt;
            sample.muted = muteMask.at(l1);
            customWave.replace(l1, sample);
            lt+=step;
        }
        maxNPoints = npoints;
    }

    if (!loopMarker) nPoints = npoints;
    if (abs(loopMarker) >= npoints) loopMarker = 0;
    dataChanged = true;
}

bool MidiSeq::toggleMutePoint(double mouseX)
{
    Sample sample;
    bool m;
    int loc = mouseX * (res * size);

    m = muteMask.at(loc);
    muteMask.replace(loc, !m);
    sample = customWave.at(loc);
    sample.muted = !m;
    customWave.replace(loc, sample);
    return(!m);
}

int MidiSeq::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

    sample = customWave.at(loc);
    sample.muted = on;
    customWave.replace(loc, sample);
    muteMask.replace(loc, on);
    return (loc);
}

void MidiSeq::setCurrentIndex(int ix)
{
    currentIndex=ix;

    if (!ix) {
        seqFinished = (enableNoteOff && !noteCount);
        restartFlag = false;
        if (backward) {
            reverse = true;
            if (loopMarker) currentIndex = abs(loopMarker) - 1;
            else currentIndex = res * size - 1;
        }
        else reverse = false;

        reflect = pingpong;
    }
}

void MidiSeq::newGrooveValues(int p_grooveTick, int p_grooveVelocity,
        int p_grooveLength)
{
    // grooveTick is only updated on pair steps to keep quantization
    // newGrooveTick stores the GUI value temporarily
    newGrooveTick = p_grooveTick;
    grooveVelocity = p_grooveVelocity;
    grooveLength = p_grooveLength;
}

void MidiSeq::applyPendingParChanges()
{
    if (!parChangesPending) return;

    int olddefer = deferChanges;
    deferChanges = false;

    setMuted(isMutedDefer);
    updateNoteLength(notelengthDefer);
    updateVelocity(velDefer);
    updateTranspose(transpDefer);

    deferChanges = olddefer;
    parChangesPending = false;
    needsGUIUpdate = true;

}

void MidiSeq::setNextTick(int tick)
{
    int tickres = TPQN/res;
    int pos = (tick/tickres) % nPoints;

    reverse = false;
    if (pingpong || (loopMarker > 0)) reverse = (((tick/tickres) / nPoints) % 2);

    if (backward) reverse = !reverse;
    if (reverse) pos = nPoints - pos;

    setCurrentIndex(pos);
    nextTick = (tick/tickres) * tickres;
}
