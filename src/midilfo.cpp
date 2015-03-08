/*!
 * @file midilfo.cpp
 * @brief Implements the MidiLfo MIDI worker class for the LFO Module.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2015 <qmidiarp-devel@lists.sourceforge.net>
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
#include "midilfo.h"


MidiLfo::MidiLfo()
{
    enableNoteOff = false;
    trigByKbd = false;
    gotKbdTrig = false;
    restartByKbd = false;
    trigLegato = false;
    enableLoop = true;
    curLoopMode = 0;
    seqFinished = false;
    restartFlag = false;
    noteCount = 0;

    queueTempo = 100.0;
    amp = 64;
    offs = 0;
    freq = 8;
    size = 4;
    res = 4;
    nPoints = 16;
    maxNPoints = 16;
    old_res = 0;
    ccnumber = 74;
    portOut = 0;
    channelOut = 0;
    chIn = 0;
    ccnumberIn = 74;
    waveFormIndex = 0;
    isMuted = false;
    isMutedDefer = false;
    deferChanges = false;
    parChangesPending = false;
    recordMode = false;
    isRecording = false;
    reverse = false;
    pingpong = false;
    backward = false;
    reflect = false;
    recValue = 0;
    int l1 = 0;
    int lt = 0;
    int step = TPQN / res;
    cwmin = 0;

    customWave.resize(8192);
    Sample sample;
    sample.value = 63;
    sample.tick = 0;
    for (l1 = 0; l1 < size * res; l1++) {
        sample.tick = lt;
        sample.muted = false;
        customWave.replace(l1, sample);
        lt+=step;
    }
    muteMask.fill(false, 8192);
    data.clear();
    frame.resize(32);
    frame.fill(sample);
    updateWaveForm(waveFormIndex);
    getData(&data);
    lastMouseLoc = 0;
    lastMouseY = 0;
    frameptr = 0;
    frameSize = 1;
    nextTick = 0;
    grooveTick = 0;
    newGrooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;

    lastMute = false;
    dataChanged = false;
    needsGUIUpdate = false;
}

MidiLfo::~MidiLfo(){
}

void MidiLfo::setMuted(bool on)
{
    isMutedDefer = on;
    if (deferChanges) {
        parChangesPending = true;
    }
    else isMuted = on;
    needsGUIUpdate = false;
}

void MidiLfo::getNextFrame(int tick)
{
    //this function is called by engine and returns one sample
    //if res <= LFO_FRAMELIMIT. If res > LFO_FRAMELIMIT, a frame is output
    //The FRAMELIMIT avoids excessive cursor updating

    if (frameptr >= data.size()) return;
    
    Sample sample;
    const int step = TPQN / res;
    const int npoints = size * res;
    int lt, l1;
    int framelimit;
    int index;

    gotKbdTrig = false;

    if (isRecording) framelimit = 32; else framelimit = LFO_FRAMELIMIT;
    frameSize = res / framelimit;
    if (!frameSize) frameSize = 1;

    if (restartFlag) setFramePtr(0);
    if (!frameptr) grooveTick = newGrooveTick;

    l1 = 0;
    lt = nextTick;
    do {
        if (reverse) {
            index = (frameSize - 1 - l1 + frameptr) % npoints;
        }
        else {
            index = (l1 + frameptr) % npoints;
        }
        sample = data.at(index);

        if (isRecording) {
            if (frameSize < 2) {
                sample.value = recValue;
            }
            else {
            /* We do linear interpolation of points within frames if
             * frameSize is > 0 to get a smooth recording at high resolutions
             * interpolation is linear between lastSampleValue and current recValue
             */
                sample.value = lastSampleValue
                            + (double)(recValue - lastSampleValue) / res * framelimit
                            * ((double)l1 + .5);
            }
            customWave.replace(index, sample);
            dataChanged = true;
        }
        sample.tick = lt;
        if (seqFinished) sample.muted = true;
        frame.replace(l1, sample);
        lt+=step;
        l1++;
    } while ((l1 < frameSize) && (l1 < npoints));


    reflect = pingpong;

    if ((!frameptr && !reverse)
        || (frameptr == npoints - l1 && reverse)) applyPendingParChanges();

    if (curLoopMode == 6) {
        frameptr = (rand() % npoints) / l1;
        frameptr *= l1;
    }
    else {
        if (reverse) {
            frameptr-=l1;
            if (frameptr < 0) {
                if (!enableLoop) seqFinished = true;
                frameptr = npoints - l1;
                if (reflect  || !backward) {
                    reverse = false;
                    frameptr = 0;
                }
            }
        }
        else {
            frameptr+=l1;
            if (frameptr >= npoints) {
                if (!enableLoop) seqFinished = true;
                frameptr = 0;
                if (reflect || backward) {
                    reverse = true;
                    frameptr = npoints - l1;
                }
            }
        }
    }
    int cur_grv_sft = 0.01 * (grooveTick * (step - 1));
    /* pairwise application of new groove shift */
    if (!(frameptr % 2)) {
        cur_grv_sft = -cur_grv_sft;
        grooveTick = newGrooveTick;
    }
    if (res > 16) cur_grv_sft = 0;

    lastSampleValue = recValue;

    nextTick = lt + cur_grv_sft;
    if (nextTick < (tick - lt)) nextTick = tick;
    sample.value = -1;
    sample.tick = nextTick;
    frame.replace(l1, sample);

    if (!trigByKbd && !(frameptr % 2) && !grooveTick) {
        /* round-up to current resolution (quantize) */
        nextTick/= (step * frameSize);
        nextTick*= (step * frameSize);
    }

    if (seqFinished) frameptr = 0;

}

void MidiLfo::getData(QVector<Sample> *p_data)
{
    //this function returns the full LFO wave

    Sample sample;
    const int step = TPQN / res;
    const int npoints = size * res;
    int l1 = 0;
    int lt = 0;
    int val = 0;
    int tempval;
    bool cl = false;
    QVector<Sample> tmpdata;

    tmpdata.clear();

    switch(waveFormIndex) {
        case 0: //sine
            for (l1 = 0; l1 < npoints; l1++) {
                sample.value = clip((-cos((double)(l1 * 6.28 /
                res * freq / 32)) + 1) * amp / 2 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt += step;
            }
        break;
        case 1: //sawtooth up
            val = 0;
            for (l1 = 0; l1 < npoints; l1++) {
                sample.value = clip(val * amp / res / 32
                + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt += step;
                val += freq;
                val %= res * 32;
            }
        break;
        case 2: //triangle
            val = 0;
            for (l1 = 0; l1 < npoints; l1++) {
                tempval = val - res * 16;
                if (tempval < 0 ) tempval = -tempval;
                sample.value = clip((res * 16 - tempval) * amp
                        / res / 16 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt += step;
                val += freq;
                val %= res * 32;
            }
        break;
        case 3: //sawtooth down
            val = 0;
            for (l1 = 0; l1 < npoints; l1++) {
                sample.value = clip((res * 32 - val)
                        * amp / res / 32 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt+=step;
                val += freq;
                val %= res * 32;
            }
        break;
        case 4: //square
            for (l1 = 0; l1 < npoints; l1++) {
                sample.value = clip(amp * ((l1 * freq / 16
                        / res) % 2 == 0) + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                tmpdata.append(sample);
                lt+=step;
            }
        break;
        case 5: //custom
            lt = step * npoints;
            tmpdata = customWave.mid(0, npoints);
        break;
        default:
        break;
    }
    sample.value = -1;
    sample.tick = lt;
    tmpdata.append(sample);
    data = tmpdata;
    *p_data = data;
}

int MidiLfo::clip(int value, int min, int max, bool *outOfRange)
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

void MidiLfo::updateWaveForm(int val)
{
    waveFormIndex = val;
}

void MidiLfo::updateFrequency(int val)
{
    freq = val;
}

void MidiLfo::updateAmplitude(int val)
{
    amp = val;
}

void MidiLfo::updateOffset(int val)
{
    if (isRecording) return;
    if (waveFormIndex == 5) updateCustomWaveOffset(val);
    offs = val;
}

void MidiLfo::updateResolution(int val)
{
    res = val;
    resizeAll();
}

void MidiLfo::updateSize(int val)
{
    size = val;
    resizeAll();
}

void MidiLfo::updateLoop(int val)
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

void MidiLfo::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

int MidiLfo::setCustomWavePoint(double mouseX, double mouseY, bool newpt)
{
    Sample sample;
    int loc = mouseX * (res * size);
    int Y = mouseY * 128;

    if (newpt || (lastMouseLoc >= (res * size))) {
    // the mouse was just clicked so we can directly set the point
        lastMouseLoc = loc;
        lastMouseY = Y;
    }

    if (loc == lastMouseLoc) lastMouseY = Y;

    do {
    //if the mouse was moved, we interpolate potentially missing points after
    //the last mouse position
        if (loc > lastMouseLoc) {
            lastMouseY += (double)(lastMouseY - Y) / (lastMouseLoc - loc) + .5;
            lastMouseLoc++;
        }
        if (loc < lastMouseLoc) {
            lastMouseY -= (double)(lastMouseY - Y) / (lastMouseLoc - loc) - .5;
            lastMouseLoc--;
        }
        sample = customWave.at(lastMouseLoc);
        sample.value = lastMouseY;
        customWave.replace(lastMouseLoc, sample);
    } while (lastMouseLoc != loc);
    return (loc);
}

int MidiLfo::mouseEvent(double mouseX, double mouseY, int buttons, int pressed)
{
    int ix = 0;
    if (buttons == 2) {
        if (pressed == 1) {
            lastMute = toggleMutePoint(mouseX);
            ix = lastMute;
        }
        else if (pressed == 0)
            ix = setMutePoint(mouseX, lastMute);
    }
    else if (pressed != 2) {
        if (waveFormIndex < 5) {
            copyToCustom();
        }
        ix = setCustomWavePoint(mouseX, mouseY, pressed);
        newCustomOffset();
    }
    dataChanged = true;
    return (ix);
}

void MidiLfo::resizeAll()
{
    int lt = 0;
    int l1 = 0;
    const int step = TPQN / res;
    const int npoints = res * size;
    Sample sample;

    frameptr%=npoints;

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
    nPoints = npoints;
    dataChanged = true;
}

void MidiLfo::copyToCustom()
{
    updateWaveForm(5);
    for (int l1 = 0; l1 < nPoints; l1++)
        customWave.replace(l1, data.at(l1));

}

void MidiLfo::newCustomOffset()
{
    int min = 127;
    int value;
    const int npoints = res * size;
    for (int l1 = 0; l1 < npoints; l1++) {
        value = customWave.at(l1).value;
        if (value < min) min = value;
    }
    cwmin = min;
}

void MidiLfo::updateCustomWaveOffset(int cwoffs)
{
    Sample sample;
    const int count = res * size;
    int l1 = 0;
    bool cl = false;

    while ((!cl) && (l1 < count)) {
        sample.value = clip(customWave.at(l1).value + cwoffs - cwmin,
                            0, 127, &cl);
        l1++;
        }

    if (cl) return;

    for (l1 = 0; l1 < count; l1++) {
        sample = customWave.at(l1);
        sample.value += cwoffs - cwmin;
        customWave.replace(l1, sample);
    }
    cwmin = cwoffs;
}

bool MidiLfo::toggleMutePoint(double mouseX)
{
    Sample sample;
    bool m;
    int loc = mouseX * (res * size);

    m = muteMask.at(loc);
    muteMask.replace(loc, !m);
    if (waveFormIndex == 5) {
        sample = customWave.at(loc);
        sample.muted = !m;
        customWave.replace(loc, sample);
    }
    lastMouseLoc = loc;
    return(!m);
}

int MidiLfo::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

    if (lastMouseLoc >= (res * size)) lastMouseLoc = loc;

    do {
        if (waveFormIndex == 5) {
            sample = customWave.at(lastMouseLoc);
            sample.muted = on;
            customWave.replace(lastMouseLoc, sample);
        }
        muteMask.replace(lastMouseLoc, on);
        if (loc > lastMouseLoc) lastMouseLoc++;
        if (loc < lastMouseLoc) lastMouseLoc--;
    } while (lastMouseLoc != loc);

    return (loc);
}

void MidiLfo::setFramePtr(int idx)
{
    frameptr = idx;
    if (!idx) {
        reverse = curLoopMode&1;
        seqFinished = (enableNoteOff && !noteCount);
        restartFlag = false;
        if (reverse) frameptr = res * size - 1;
    }
}

void MidiLfo::setRecordMode(bool on)
{
    if (!on) {
        isRecording = false;
        newCustomOffset();
        dataChanged = true;
    }
    recordMode = on;
}

void MidiLfo::record(int value)
{
    recValue = value;
    isRecording = true;
}

bool MidiLfo::handleEvent(MidiEvent inEv, int tick)
{

    if (!recordMode && (inEv.type == EV_CONTROLLER)) return(true);
    if (inEv.channel != chIn) return(true);
    if ((inEv.type == EV_CONTROLLER) && (inEv.data != ccnumberIn)) return(true);

    if ((inEv.type == EV_CONTROLLER) && recordMode) {
        record(inEv.value);
        return (false);
    }
    if (inEv.type != EV_NOTEON) return (true);

    if (inEv.value) {
        /*This is a NOTE ON event*/
        if (restartByKbd && (!noteCount || trigLegato)) restartFlag = true;
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

void MidiLfo::newGrooveValues(int p_grooveTick, int p_grooveVelocity,
        int p_grooveLength)
{
    // grooveTick is only updated on pair steps to keep quantization
    // newGrooveTick stores the GUI value temporarily
    newGrooveTick = p_grooveTick;
    grooveVelocity = p_grooveVelocity;
    grooveLength = p_grooveLength;
}

void MidiLfo::applyPendingParChanges()
{
    if (!parChangesPending) return;

    int olddefer = deferChanges;
    deferChanges = false;
    setMuted(isMutedDefer);
    deferChanges = olddefer;
    parChangesPending = false;
    needsGUIUpdate = true;
}

void MidiLfo::setNextTick(int tick)
{
    int tickres = TPQN/res;
    int pos = (tick/tickres) % nPoints;

    reverse = false;
    if (pingpong) reverse = (((tick/tickres) / nPoints) % 2);

    if (backward) reverse = !reverse;
    if (reverse) pos = nPoints - pos;

    setFramePtr(pos);
    nextTick = (tick/tickres) * tickres;
}
