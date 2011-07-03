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
}

MidiLfo::~MidiLfo(){
}

void MidiLfo::setMuted(bool on)
{
    isMuted = on;
}

void MidiLfo::getNextFrame(QVector<Sample> *p_data)
{
    //this function is called by seqdriver and returns one sample
    //if res <= LFO_FRAMELIMIT. If res > LFO_FRAMELIMIT, a frame is output
    //The FRAMELIMIT avoids excessive cursor updating

    QVector<Sample> frame;
    Sample sample;
    int step = TPQN / res;
    int npoints = size * res;
    int lt, l1;
    int framelimit;
    int framesize;
    int index;

    frame.clear();

    if (isRecording) framelimit = 32; else framelimit = LFO_FRAMELIMIT;
    framesize = res / framelimit;
    l1 = 0;
    lt = 0;

    do {
        index = (l1 + frameptr) % npoints;
        sample = data.at(index);
        if (isRecording) {
            if (!framesize) {
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
        frame.append(sample);
        lt+=step;
        l1++;
    } while ((l1 < framesize) & (l1 < npoints));

    lastSampleValue = recValue;

    sample.value = -1;
    sample.tick = lt;
    frame.append(sample);
    emit nextStep(frameptr);

    frameptr += l1;
    frameptr %= npoints;

    *p_data = frame;
}

void MidiLfo::getData(QVector<Sample> *p_data)
{
    //this function returns the full LFO wave

    Sample sample;
    int l1 = 0;
    int lt = 0;
    int step = TPQN / res;
    int val = 0;
    int tempval;
    bool cl = false;
    int npoints = size * res;
    //res: number of events per beat
    //size: size of waveform in beats

    data.clear();

    switch(waveFormIndex) {
        case 0: //sine
            for (l1 = 0; l1 < npoints; l1++) {
                sample.value = clip((-cos((double)(l1 * 6.28 /
                res * freq / 32)) + 1) * amp / 2 + offs, 0, 127, &cl);
                sample.tick = lt;
                sample.muted = muteMask.at(l1);
                data.append(sample);
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
                data.append(sample);
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
                data.append(sample);
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
                data.append(sample);
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
                data.append(sample);
                lt+=step;
            }
        break;
        case 5: //custom
            lt = step * customWave.count();
            data = customWave;
        break;
        default:
        break;
    }
    sample.value = -1;
    sample.tick = lt;
    data.append(sample);
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
    int step = TPQN / res;
    Sample sample;

    frameptr%=(res * size);
    os = customWave.count();
    customWave.resize(size * res);
    muteMask.resize(size * res);
    for (l1 = 0; l1 < customWave.count(); l1++) {
        if (l1 >= os) muteMask.replace(l1, muteMask.at(l1 % os));
        sample = customWave.at(l1 % os);
        sample.tick = lt;
        sample.muted = muteMask.at(l1);
        customWave.replace(l1, sample);
        lt+=step;
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

void MidiLfo::resetFramePtr()
{
    frameptr = 0;
}

void MidiLfo::record(int value)
{
    recValue = value;
    isRecording = true;
}

bool MidiLfo::wantEvent(MidiEvent inEv)
{
    if (!recordMode) return(false);
    if (inEv.channel != chIn) return(false);
    if (inEv.data != ccnumberIn) return(false);
    return(true);
}
