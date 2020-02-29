/*!
 * @file midiseq.cpp
 * @brief Implements the MidiSeq MIDI worker class for the Seq Module.
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
 *
 */
#include <cmath>
#include <cstdint>
#include "midiseq.h"


MidiSeq::MidiSeq()
{
    recordMode = false;
    currentRecStep = 0;
    loopMarker = 0;

    nOctaves = 4;
    baseOctave = 3;

    vel = 0;
    velDefer = 0;
    transp = 0;
    transpDefer = 0;
    size = 4;
    res = 4;
    maxNPoints = 16;
    notelength = 180;
    notelengthDefer = 180;
    lastMute = false;
    lastMouseLoc = 0;
    lastMouseY = 0;

    customWave.resize(2048);
    muteMask.resize(2048);
    data.reserve(2048);
    
    int lt = 0;
    int l1 = 0;
    int step = TPQN / res;
    Sample sample;
    sample.value = 60;
    for (l1 = 0; l1 < 2048; l1++) {
        sample.tick = lt;
        sample.muted = false;
        customWave[l1] = sample;
        data[l1] = sample;
        muteMask[l1] = false;
        lt+=step;
    }
    returnNote = sample;
}

bool MidiSeq::handleEvent(MidiEvent inEv, int tick)
{
    if (inEv.type != EV_NOTEON) return(true);
    if (inEv.channel != chIn && chIn != OMNI) return(true);
    if ((inEv.data < 36) || (inEv.data >= 84)) return(true);
    
    if (recordMode && inEv.value) {
        recordNote(inEv.data);
        return(false);
    }
    
    if (((inEv.data < indexIn[0]) || (inEv.data > indexIn[1]))
        || ((inEv.value < rangeIn[0]) || (inEv.value > rangeIn[1]))) {
        return(true);
    }

    if (inEv.value) {
        /*This is a NOTE ON event*/
        if (enableNoteIn) {
            updateTranspose(inEv.data - 60);
            needsGUIUpdate = true;
        }
        if (enableVelIn) {
            updateVelocity(inEv.value);
            needsGUIUpdate = true;
        }
        if (restartByKbd && (!noteCount || trigLegato)) {
            restartFlag = true;
            seqFinished = false;
        }
        noteCount++;
        if (trigByKbd && ((noteCount == 1) || trigLegato)) {
            nextTick = tick + 2; //schedDelayTicks;
            gotKbdTrig = true;
            seqFinished = false;
        }
    }
    else {
        /*This is a NOTE OFF event*/
        if (enableNoteOff && (noteCount == 1)) seqFinished = true;
        if (noteCount) noteCount--;
    }

    return(false);
}

void MidiSeq::getNextFrame(int tick)
{
    const int frame_nticks = TPQN / res;
    Sample sample;
    int cur_grv_sft;

    gotKbdTrig = false;
    if (restartFlag) setFramePtr(0);
    if (!framePtr) grooveTick = newGrooveTick;

    sample = customWave[framePtr];
    advancePatternIndex();

    if (nextTick < (tick - frame_nticks)) nextTick = tick;

    sample.value+=transp;
    sample.tick = nextTick;


    cur_grv_sft = 0.01 * (grooveTick * (frame_nticks - 1));

    /* pairwise application of new groove shift */
    if (!(framePtr % 2)) {
        cur_grv_sft = -cur_grv_sft;
        grooveTick = newGrooveTick;
    }
    nextTick += frame_nticks + cur_grv_sft;

    if (!trigByKbd && !(framePtr % 2)) {
        /* round-up to current resolution (quantize) */
        nextTick/=frame_nticks;
        nextTick*=frame_nticks;
    }

    if (seqFinished) {
        sample.muted = true;
        framePtr = 0;
    }
    returnNote = sample;
}

void MidiSeq::advancePatternIndex()
{
    const int npoints = res * size;
    int pivot = abs(loopMarker);
    reflect = pingpong;

    if (curLoopMode == 6) {
        if (pivot)
            framePtr = rand() % pivot;
        else
            framePtr = rand() % npoints;
        return;
    }

    if (reverse) {
        if (!pivot) pivot = npoints;
        if (framePtr == pivot - 1) applyPendingParChanges();
        framePtr--;
        if (framePtr == -1) {
            if (!enableLoop) seqFinished = true;
            if (reflect  || !backward) {
                reverse = false;
                framePtr = 0;
            }
            else framePtr = pivot - 1;
        }
        else if (framePtr == pivot - 1) {
            if (!enableLoop) seqFinished = true;
            if (loopMarker < 0) reflect = true;
            if (loopMarker > 0) reflect = false;
            if (reflect) {
                reverse = false;
                framePtr = pivot;
            }
            else framePtr = npoints - 1;
        }
    }
    else {
        if (!framePtr) applyPendingParChanges();
        framePtr++;
        if (framePtr == npoints) {
            if (!enableLoop) seqFinished = true;

            if (reflect || backward) {
                reverse = true;
                framePtr = npoints - 1;
            }
            else framePtr = pivot;
        }
        else if ((framePtr == pivot)) {
            if (!pivot) pivot = npoints;
            if (!enableLoop) seqFinished = true;
            if (loopMarker > 0) reflect = true;
            if (loopMarker < 0) reflect = false;
            if (reflect) {
                reverse = true;
                framePtr = pivot - 1;
            }
            else framePtr = 0;
        }
    }
}

void MidiSeq::getData(std::vector<Sample> * p_data)
{
    Sample sample;
    int lt = 0;
    int l1 = 0;
    const int step = TPQN / res;
    const int npoints = res * size;

    data.resize(npoints);

    lt = step * npoints;
    for (l1 = 0; l1 < npoints; l1++) data[l1] = customWave[l1];
    sample.value = -1;
    sample.tick = lt;
    sample.muted = false;
    data.push_back(sample);
    
    *p_data = data;
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
    if (seqFinished) {
        seqFinished = false;
        setFramePtr(0);
    }
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

int MidiSeq::setCustomWavePoint(double mouseX, double mouseY)
{
    currentRecStep = mouseX * res * size;
    setRecordedNote(12 * (mouseY * nOctaves + baseOctave));
    return (currentRecStep);
}

int MidiSeq::mouseEvent(double mouseX, double mouseY, int buttons, int pressed)
{
    int ix = 0;
    bool mute = false;
    int Y = 12 * (mouseY * nOctaves + baseOctave);

    if ((mouseY < 0) && (pressed != 2)) {
        if (mouseX < 0) mouseX = 0;
        if (buttons == 2) mouseX = - mouseX;
        setLoopMarkerMouse(mouseX);
        return (0);
    }

    if ((mouseX > 1) || (mouseX < 0) || (mouseY > 1) || (mouseY < 0)) return (0);

    if (buttons == 2) {
        if (pressed == 1) {
            mute = lastMute;
            lastMute = toggleMutePoint(mouseX);
            mute = (mute != lastMute);
            ix = mouseX * (res * size);
        }
        else if (pressed == 0)
            ix = setMutePoint(mouseX, lastMute);
    }
    else if (pressed != 2) {
        ix = setCustomWavePoint(mouseX, mouseY);
    }
    
    if ( (ix != lastMouseLoc) || (lastMouseY != Y) || (mute) ) dataChanged = true;

    lastMouseLoc = ix;
    lastMouseY = Y;
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

void MidiSeq::updateDispVert(int mode)
{
    switch (mode) {
        case 0:
            nOctaves = 4;
            baseOctave = 3;
        break;
        case 1:
            nOctaves = 2;
            baseOctave = 5;
        break;
        case 2:
            nOctaves = 2;
            baseOctave = 4;
        break;
        case 3:
            nOctaves = 2;
            baseOctave = 3;
        break;
        default:
            nOctaves = 4;
            baseOctave = 3;
    }
}

void MidiSeq::setRecordMode(int on)
{
    recordMode = on;
}

void MidiSeq::setRecordedNote(int note)
{
    Sample sample;

    sample = customWave[currentRecStep];
    sample.value = note;
    sample.tick = currentRecStep * TPQN / res;
    customWave[currentRecStep] = sample;
}

void MidiSeq::resizeAll()
{
    const int step = TPQN / res;
    const int npoints = res * size;
    Sample sample;

    framePtr%=npoints;
    currentRecStep%=npoints;

    if (maxNPoints < npoints) {
        int lt = 0;
        for (int l1 = 0; l1 < npoints; l1++) {
            if (l1 >= maxNPoints)
                muteMask[l1] = muteMask[l1 % maxNPoints];
            sample = customWave[l1 % maxNPoints];
            sample.tick = lt;
            sample.muted = muteMask[l1];
            customWave[l1] = sample;
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

    m = muteMask[loc];
    muteMask[loc] = !m;
    sample = customWave[loc];
    sample.muted = !m;
    customWave[loc] = sample;
    return(!m);
}

int MidiSeq::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

    sample = customWave[loc];
    sample.muted = on;
    customWave[loc] = sample;
    muteMask[loc] = on;
    return (loc);
}

void MidiSeq::setFramePtr(int ix)
{
    framePtr=ix;

    if (!ix) {
        seqFinished = (enableNoteOff && !noteCount);
        restartFlag = false;
        if (backward) {
            reverse = true;
            if (loopMarker) framePtr = abs(loopMarker) - 1;
            else framePtr = res * size - 1;
        }
        else reverse = false;

        reflect = pingpong;
    }
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

    setFramePtr(pos);
    nextTick = (tick/tickres) * tickres;
}
