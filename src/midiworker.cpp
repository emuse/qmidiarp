/*!
 * @file midiworker.cpp
 * @brief Implements the MIDI worker module base class.
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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
#include "midiworker.h"


MidiWorker::MidiWorker()
{
    enableNoteIn = true;
    enableNoteOff = false;
    enableVelIn = true;
    trigByKbd = false;
    gotKbdTrig = false;
    restartByKbd = false;
    trigLegato = false;
    for (int l1 = 0; l1 < 2; l1++) {
        rangeIn[l1] = (l1) ? 127 : 0;
        indexIn[l1] = (l1) ? 127 : 0;
    }
    
    queueTempo = 100.0;
    ccnumber = 74;
    portOut = 0;
    channelOut = 0;
    chIn = OMNI;
    ccnumberIn = 74;
    isMuted = false;
    isMutedDefer = false;
    deferChanges = false;
    reverse = false;
    pingpong = false;
    backward = false;
    reflect = false;
    seqFinished = false;
    restartFlag = false;
    triggerMode = 0;
    enableLoop = true;
    curLoopMode = 0;
    noteCount = 0;
    nextTick = 0;

    returnLength = 0;

    grooveTick = 0;
    newGrooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    framePtr = 0;
    nRepetitions = 1;
    currentRepetition = 0;
    nPoints = 1;

    dataChanged = false;
    needsGUIUpdate = false;
    parChangesPending = false;

}

void MidiWorker::setMuted(bool on)
{
    isMutedDefer = on;
    if (deferChanges) {
        parChangesPending = true;
    }
    else isMuted = on;

    needsGUIUpdate = false;
}

int MidiWorker::clip(int value, int min, int max, bool *outOfRange)
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
