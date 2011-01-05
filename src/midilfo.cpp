/*
 *      midilfo.cpp
 *      
 *      Copyright 2009 <qmidiarp-devel@lists.sourceforge.net>
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
 */

#include <cmath>
#include "midilfo.h"


MidiLfo::MidiLfo()
{
    queueTempo = 100.0;
    amp = 0;
    offs = 0;
    freq = 4;
    size = 1;
    res = 16;
    ccnumber = 74;
    portOut = 0;
    channelOut = 0;
    waveFormIndex = 0;
    isMuted = false;
    int l1 = 0;
    int lt = 0;
    int step = TICKS_PER_QUARTER / res;
    LfoSample lfoSample;
    lfoSample.value = 63;
    customWave.clear();
    cwmin = 0;
    for (l1 = 0; l1 < size * res; l1++) {
            lfoSample.tick = lt;
            lfoSample.muted = false;
            customWave.append(lfoSample);
            lt+=step;
    }
    muteMask.fill(false, size * res);
    lfoData.clear();
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

void MidiLfo::getNextFrame(QVector<LfoSample> *p_lfoData)
{
    //this function is called by seqdriver and returns a frame of
    //maximum LFO_FRAMESIZE points
    
    QVector<LfoSample> lfoFrame;
    LfoSample lfoSample;
    int step = TICKS_PER_QUARTER / res;
    int npoints = size * res;
    int lt, l1;
    
    lfoFrame.clear();
    lt = 0;
    l1 = 0;
    
    while ((l1 < LFO_FRAMESIZE) && (l1 < npoints)) {
        lfoSample = lfoData.at((l1 + frameptr) % npoints);
        lfoSample.tick = lt;
        lfoFrame.append(lfoSample);
        lt+=step;
        l1++;
    }
    lfoSample.value = -1;
    lfoSample.tick = lt;
    lfoFrame.append(lfoSample);    
   
    frameptr += l1;
    frameptr %= npoints;
    
    *p_lfoData = lfoFrame;
}

void MidiLfo::getData(QVector<LfoSample> *p_lfoData)
{ 
    //this function returns the full LFO wave
    
    LfoSample lfoSample;
    int l1 = 0;
    int lt = 0;
    int step = TICKS_PER_QUARTER / res;
    int val = 0;
    int tempval;
    bool cl = false;
    int npoints = size * res;
    //res: number of events per beat
    //size: size of waveform in beats
    
    lfoData.clear();
    
    switch(waveFormIndex) {
        case 0: //sine
            for (l1 = 0; l1 < npoints; l1++) {
                lfoSample.value = clip((-cos((double)(l1 * 6.28 / 
                res * freq / 4)) + 1) * amp / 2 + offs, 0, 127, &cl);
                lfoSample.tick = lt;
                lfoSample.muted = muteMask.at(l1);
                lfoData.append(lfoSample);
                lt += step;
            }
        break;
        case 1: //sawtooth up
            val = 0;
            for (l1 = 0; l1 < npoints; l1++) {
                lfoSample.value = clip(val * amp / res / 4 
                + offs, 0, 127, &cl);
                lfoSample.tick = lt;
                lfoSample.muted = muteMask.at(l1);
                lfoData.append(lfoSample);
                lt += step;
                val += freq;
                val %= res * 4;
            }
        break;
        case 2: //triangle
            val = 0;
            for (l1 = 0; l1 < npoints; l1++) {
                tempval = val - res * 2;
                if (tempval < 0 ) tempval = -tempval;
                lfoSample.value = clip((res * 2 - tempval) * amp 
                        / res / 2 + offs, 0, 127, &cl);
                lfoSample.tick = lt;
                lfoSample.muted = muteMask.at(l1);
                lfoData.append(lfoSample);
                lt += step;
                val += freq;
                val %= res * 4;
            }
        break;
        case 3: //sawtooth down
            val = 0;
            for (l1 = 0; l1 < npoints; l1++) {
                lfoSample.value = clip((res * 4 - val) 
                        * amp / res / 4 + offs, 0, 127, &cl);
                lfoSample.tick = lt;
                lfoSample.muted = muteMask.at(l1);
                lfoData.append(lfoSample);
                lt+=step;
                val += freq;
                val %= res * 4;
            }
        break;
        case 4: //square
            for (l1 = 0; l1 < npoints; l1++) {
                lfoSample.value = clip(amp * ((l1 * freq / 2 
                        / res) % 2 == 0) + offs, 0, 127, &cl);
                lfoSample.tick = lt;
                lfoSample.muted = muteMask.at(l1);
                lfoData.append(lfoSample);
                lt+=step;
            }
        break;
        case 5: //custom
            lt = step * customWave.count();
            lfoData = customWave;
        break;
        default:
        break;
    }
    lfoSample.value = -1;
    lfoSample.tick = lt;
    lfoData.append(lfoSample);    
    *p_lfoData = lfoData;
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
    offs = val;
}

void MidiLfo::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiLfo::setCustomWavePoint(double mouseX, double mouseY, bool newpt)
{
    LfoSample lfoSample;
    int loc = mouseX * (res * size);
    int Y = mouseY * 128;
    
    if (newpt) {
        lastMouseLoc = loc;
        lastMouseY = Y;
    }
    
    if (loc == lastMouseLoc) lastMouseY = Y;
    
    do {
        if (loc > lastMouseLoc) {
            lastMouseY += (double)(lastMouseY - Y) / (lastMouseLoc - loc) + .5;
            lastMouseLoc++;
        } 
        if (loc < lastMouseLoc) {
            lastMouseY -= (double)(lastMouseY - Y) / (lastMouseLoc - loc) - .5;
            lastMouseLoc--;
        }
        lfoSample = customWave.at(lastMouseLoc);
        lfoSample.value = lastMouseY;
        customWave.replace(lastMouseLoc, lfoSample);
    } while (lastMouseLoc != loc);
}

void MidiLfo::resizeAll()
{
    int lt = 0;
    int l1 = 0;
    int os;
    int step = TICKS_PER_QUARTER / res;
    LfoSample lfoSample;
    
    os = customWave.count();
    customWave.resize(size * res);
    muteMask.resize(size * res);
    for (l1 = 0; l1 < customWave.count(); l1++) {
        lfoSample = customWave.at(l1 % os);
        lfoSample.tick = lt;
        lfoSample.muted = muteMask.at(l1);
        customWave.replace(l1, lfoSample);
        lt+=step;
    }
}

void MidiLfo::copyToCustom()
{
    int m;
    
    m = lfoData.count();
    lfoData.remove(m - 1);
    customWave = lfoData;
}

void MidiLfo::updateCustomWaveOffset(int cwoffs)
{
    LfoSample lfoSample;
    const int count = customWave.count();
    int l1 = 0;
    bool cl = false;
    
    while ((!cl) && (l1 < count)) {
        lfoSample.value = clip(customWave.at(l1).value + cwoffs - cwmin,
                            0, 127, &cl);
        l1++;
        }
        
    if (cl) return;
    
    for (l1 = 0; l1 < count; l1++) {
        lfoSample = customWave.at(l1);
        lfoSample.value += cwoffs - cwmin;
        customWave.replace(l1, lfoSample);
    }
    cwmin = cwoffs;
}

bool MidiLfo::toggleMutePoint(double mouseX)
{
    LfoSample lfoSample;
    bool m;
    int loc = mouseX * (res * size);
    
    m = muteMask.at(loc);
    muteMask.replace(loc, !m);
    if (waveFormIndex == 5) {
        lfoSample = customWave.at(loc);
        lfoSample.muted = !m;
        customWave.replace(loc, lfoSample);
    }
    lastMouseLoc = loc;
    return(!m);
}

void MidiLfo::setMutePoint(double mouseX, bool on)
{
    LfoSample lfoSample;
    int loc = mouseX * (res * size);
    
    do {
        if (waveFormIndex == 5) {
            lfoSample = customWave.at(lastMouseLoc);
            lfoSample.muted = on;
            customWave.replace(lastMouseLoc, lfoSample);
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
