/*!
 * @file midiseq.cpp
 * @brief Implements the MidiSeq MIDI worker class for the Seq Module.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
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
    transp = 0;
    size = 4;
    res = 4;
    nPoints = 16;
    maxNPoints = 16;
    notelength = 74;
    portOut = 0;
    channelOut = 0;
    waveFormIndex = 0;
    currentIndex = 0;
    nextTick = 0;
    grooveTick = 0;
    newGrooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    isMuted = false;
    int lt = 0;
    int l1 = 0;
    int step = TPQN / res;
    Sample sample;
    sample.value = 60;
    customWave.reserve(512);
    customWave.clear();
    for (l1 = 0; l1 < size * res; l1++) {
            sample.tick = lt;
            sample.muted = false;
            customWave.append(sample);
            lt+=step;
    }
    muteMask.fill(false, size * res);
}

MidiSeq::~MidiSeq(){
}

void MidiSeq::setMuted(bool on)
{
    isMuted = on;
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
        if (enableNoteIn) updateTranspose(inEv.data - 60);
        if (restartByKbd && (!noteCount || trigLegato)) restartFlag = true;
        if (enableVelIn) updateVelocity(inEv.value);
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

    if (reverse) {
        currentIndex--;
        if (!pivot) pivot = npoints;
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
    data.clear();

    switch(waveFormIndex) {
        case 0: //custom
            lt = step * npoints;
            data = customWave.mid(0, npoints);
        break;
        default:
        break;
    }
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

void MidiSeq::updateWaveForm(int val)
{
    waveFormIndex = val;
}

void MidiSeq::updateLoop(int val)
{
    backward = val&1;
    pingpong = val&2;
    enableLoop = !(val&4);
    curLoopMode = val;
}

void MidiSeq::updateVelocity(int val)
{
    vel = val;
}

void MidiSeq::updateTranspose(int val)
{
    transp = val;
}

void MidiSeq::recordNote(int val)
{
        setRecordedNote(val);
        currentRecStep++;
        currentRecStep %= (res * size);
}

void MidiSeq::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiSeq::setCustomWavePoint(double mouseX, double mouseY)
{
    currentRecStep = mouseX * res * size;
    setRecordedNote(12 * (mouseY * nOctaves + baseOctave));
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
    int os;
    const int step = TPQN / res;
    const int npoints = res * size;
    Sample sample;

    currentIndex%=npoints;
    currentRecStep%=npoints;

    os = customWave.count();
    if (os < npoints) {
        customWave.resize(npoints);
        muteMask.resize(npoints);
        for (l1 = 0; l1 < npoints; l1++) {
            if (l1 >= os) muteMask.replace(l1, muteMask.at(l1 % os));
            sample = customWave.at(l1 % os);
            sample.tick = lt;
            sample.muted = muteMask.at(l1);
            customWave.replace(l1, sample);
            lt+=step;
        }
    }

    if (npoints > maxNPoints) maxNPoints = npoints;
    if (!loopMarker) nPoints = npoints;
}

void MidiSeq::copyToCustom()
{
    QVector<Sample> data;
    int m;

    data.clear();
    getData(&data);
    m = data.count();
    data.remove(m - 1);
    customWave = data;
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

void MidiSeq::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

    sample = customWave.at(loc);
    sample.muted = on;
    customWave.replace(loc, sample);
    muteMask.replace(loc, on);
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
