/*!
 * @file midilfo.cpp
 * @brief Implements the MidiLfo MIDI worker class for the LFO Module.
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
#include <cmath>
#include "midilfo.h"


MidiLfo::MidiLfo()
{
    enableNoteOff = false;
    trigByKbd = false;
    restartByKbd = false;
    enableLoop = true;
    curLoopMode = 0;
    seqFinished = false;
    restartFlag = false;
    noteCount = 0;

    queueTempo = 100.0;
    amp = 0;
    offs = 0;
    freq = 32;
    size = 1;
    res = 16;
    old_res = 0;
    ccnumber = 74;
    portOut = 0;
    channelOut = 0;
    chIn = 0;
    ccnumberIn = 74;
    waveFormIndex = 0;
    isMuted = false;
    recordMode = false;
    isRecording = false;
    reverse = false;
    pingpong = false;
    recValue = 0;
    int l1 = 0;
    int lt = 0;
    int step = TPQN / res;
    Sample sample;
    sample.value = 63;
    customWave.clear();
    cwmin = 0;
    for (l1 = 0; l1 < size * res; l1++) {
            sample.tick = lt;
            sample.muted = false;
            customWave.append(sample);
            lt+=step;
    }
    muteMask.fill(false, size * res);
    data.clear();
    lastMouseLoc = 0;
    lastMouseY = 0;
    frameptr = 0;
    nextTick = 0;
    grooveTick = 0;
    newGrooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
}

MidiLfo::~MidiLfo(){
}

void MidiLfo::setMuted(bool on)
{
    isMuted = on;
}

void MidiLfo::getNextFrame(QVector<Sample> *p_data, int tick)
{
    //this function is called by seqdriver and returns one sample
    //if res <= LFO_FRAMELIMIT. If res > LFO_FRAMELIMIT, a frame is output
    //The FRAMELIMIT avoids excessive cursor updating

    QVector<Sample> frame;
    Sample sample;
    const int step = TPQN / res;
    const int npoints = size * res;
    int lt, l1;
    int framelimit;
    int framesize;
    int index;

    frame.clear();

    if (isRecording) framelimit = 32; else framelimit = LFO_FRAMELIMIT;
    framesize = res / framelimit;
    if (!framesize) framesize = 1;

    if (restartFlag) setFramePtr(0);
    if (!frameptr) grooveTick = newGrooveTick;

    l1 = 0;
    lt = nextTick;
    do {
        if (reverse) {
            index = (framesize - 1 - l1 + frameptr) % npoints;
        }
        else {
            index = (l1 + frameptr) % npoints;
        }
        sample = data.at(index);
        if (isRecording) {
            if (framesize < 2) {
                sample.value = recValue;
            }
            else {
            /** We do linear interpolation of points within frames if
             * framesize is > 0 to get a smooth recording at high resolutions
             * interpolation is linear between lastSampleValue and current recValue
             * */
                sample.value = lastSampleValue
                            + (double)(recValue - lastSampleValue) / res * framelimit
                            * ((double)l1 + .5);
            }
            customWave.replace(index, sample);
        }
        sample.tick = lt;
        if (seqFinished) sample.muted = true;
        frame.append(sample);
        lt+=step;
        l1++;
    } while ((l1 < framesize) & (l1 < npoints));


    if (!seqFinished) emit nextStep(frameptr);

    if (reverse) {
        frameptr-=l1;
        if (frameptr < 0) {
            if (!enableLoop) seqFinished = true;
            frameptr = npoints - l1;
            if (pingpong) {
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
            if (pingpong) {
                reverse = true;
                frameptr = npoints - l1;
            }
        }
    }

    int cur_grv_sft = 0.01 * (grooveTick * step);
    /** pairwise application of new groove shift */
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
    frame.append(sample);

    if (!trigByKbd && !(frameptr % 2) && !grooveTick) {
        /** round-up to current resolution (quantize) */
        nextTick/= (step * framesize);
        nextTick*= (step * framesize);
    }

    *p_data = frame;
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
    //res: number of events per beat
    //size: size of waveform in beats

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
    reverse = val&1;
    pingpong = val&2;
    enableLoop = !(val&4);
    curLoopMode = val;
}

void MidiLfo::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiLfo::setCustomWavePoint(double mouseX, double mouseY, bool newpt)
{
    Sample sample;
    int loc = mouseX * (res * size);
    int Y = mouseY * 128;

    if (newpt) {
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
}

void MidiLfo::resizeAll()
{
    int lt = 0;
    int l1 = 0;
    int os;
    const int step = TPQN / res;
    const int npoints = res * size;
    Sample sample;

    frameptr%=npoints;

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
}

void MidiLfo::copyToCustom()
{
    int m;

    m = data.count();
    data.remove(m - 1);
    customWave = data;
}

void MidiLfo::updateCustomWaveOffset(int cwoffs)
{
    Sample sample;
    const int count = customWave.count();
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

void MidiLfo::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

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

void MidiLfo::record(int value)
{
    recValue = value;
    isRecording = true;
}

bool MidiLfo::wantTrigByKbd()
{
    bool on = (trigByKbd && (noteCount == 1));
    return(on);
}

bool MidiLfo::wantEvent(MidiEvent inEv)
{
    if (!recordMode && (inEv.type == EV_CONTROLLER)) return(false);
    if (inEv.channel != chIn) return(false);
    if ((inEv.type == EV_CONTROLLER) && (inEv.data != ccnumberIn)) return(false);
    return(true);
}

void MidiLfo::handleNote(int note, int velocity, int tick)
{
    (void)note;
    (void)tick;

    if (velocity) {
        /**This is a NOTE ON event*/
        if (restartByKbd && !noteCount) restartFlag = true;
        seqFinished = false;
        noteCount++;
    }
    else {
        /**This is a NOTE OFF event*/
        if (enableNoteOff && (noteCount == 1)) seqFinished = true;
        if (noteCount) noteCount--;
    }
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

