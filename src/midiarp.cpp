/*!
 * @file midiarp.cpp
 * @brief Implements the MidiArp MIDI worker class for the Arpeggiator Module.
 *
 *
 *      Copyright 2009 - 2017 <qmidiarp-devel@lists.sourceforge.net>
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
#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include "midiarp.h"

/*
#include "lockfree.h"

// this class must be implemented in such way
// so that its copy (= operator) is realtime safe
class MidiEngine
{
public:
    int foo;
};
*/

MidiArp::MidiArp()
{
    /*
    // this simulates access from the two threads
    // lock congestion is simulated by attempt to update
    // when LockedData destructor is not called
    {
        LockFreeStore<MidiEngine> store; // store for the two instances of MidiEngine
        const MidiEngine * rdata(store); // pointer to the read-only store, to be used from the seq/jack driver thread

        // write to the writable store
        {
            LockedData<MidiEngine> ldata(store); // lock the writable store, destructor will unlock it
            ldata->foo = 1;                       // write access the writable store (lock obtained)
            qWarning("gui: %d", ldata->foo);      // read access to the writable store (lock obtained)
        } // ldata gets out of the scope and its destructor is called. this results in unlock of the writable store

        // attempt update the read-only store and report whether the update succeeded
        // should succeed because the writable store is not locked
        qWarning("update %s", store.try_lockfree_update() ? "complete" : "not complete (gui thread locked)");
        // print the data in the read-only store
        qWarning("seq: %d", rdata->foo);

        // write to the writable store
        {
            LockedData<MidiEngine> ldata(store);
            ldata->foo = 2;
            qWarning("gui: %d", ldata->foo);

            // attempt update the read-only store and report whether the update succeeded
            // should fail because the writable store is locked
            qWarning("update %s", store.try_lockfree_update() ? "complete" : "not complete (gui thread locked)");
            qWarning("seq: %d (data locked by gui)", rdata->foo);
        }

        // print the data in the read-only store before the update
        qWarning("seq: %d (before update)", rdata->foo);
        // attempt update the read-only store and report whether the update succeeded
        // should succeed because the writable store is not locked
        qWarning("update %s", store.try_lockfree_update() ? "complete" : "not complete (gui thread locked)");
        // print the data in the read-only store after the update
        qWarning("seq: %d (after update)", rdata->foo);
    }
*/
    noteBufPtr = 0;
    releaseNoteCount = 0;
    purgeReleaseFlag = false;
    stepWidth = 1.0;     // stepWidth relative to global queue stepWidth
    minStepWidth = 1.0;
    maxOctave = 0;
    minOctave = 0;
    
    octMode = 0;
    octLow = 0;
    octHigh = 0;
    octOfs = 0;
    octIncr = 0;

    nSteps = 1.0;
    len = 0.5;       // note length
    vel = 0.8;  // velocity relative to global velocity
    noteIndex[0] = 0;
    patternIndex = 0;
    patternLen = 0;
    semitone = 0;
    patternMaxIndex = 0;
    noteOfs = 0;
    arpTick = 0;
    returnTick = 0;
    chordMode = false;
    randomTick = 0;
    randomVelocity = 0;
    randomLength = 0;
    randomTickAmp = 0;
    randomVelocityAmp = 0;
    randomLengthAmp = 0;
    hasNewNotes = false;
    repeatPatternThroughChord = 1;
    attack_time = 0.0;
    release_time = 0.0;
    sustain = false;
    sustainBufferCount = 0;
    latch_mode = false;
    latchBufferCount = 0;
    lastLatchTick = 0;
    trigDelayTicks = 4;
}

bool MidiArp::handleEvent(MidiEvent inEv, int tick, int keep_rel)
{
    if (inEv.channel != chIn && chIn != OMNI) return(true);
    if ((inEv.type == EV_CONTROLLER) && 
        ((inEv.data == CT_ALLNOTESOFF) || (inEv.data == CT_ALLSOUNDOFF))) {
        clearNoteBuffer();
        return(true); // In case we receive all notes off we still forward
    }
    if ((inEv.type == EV_CONTROLLER) && (inEv.data == CT_FOOTSW)) {
        setSustain((inEv.value == 127), tick);
        return(false);
    }

    if (inEv.type != EV_NOTEON) return(true);
    if (((inEv.data < indexIn[0]) || (inEv.data > indexIn[1]))
        || ((inEv.value < rangeIn[0]) || (inEv.value > rangeIn[1]))) {
        return(true);
    }

    if (inEv.value) {
        // This is a NOTE ON event
        if (!getPressedNoteCount() || trigLegato) {
            purgeLatchBuffer(tick);
            if (restartByKbd) restartFlag = true;
            // if we have been triggered, remove pending release notes
            if (trigByKbd && release_time > 0) purgeReleaseNotes(noteBufPtr);
        }
        
        addNote(inEv.data, inEv.value, tick);
        
        if (repeatPatternThroughChord == 2) noteOfs = noteCount - 1;

        if ((trigByKbd && (getPressedNoteCount() == 1))
                    || trigLegato) {
            initArpTick(tick + trigDelayTicks);
            gotKbdTrig = true;
        }
    }
    else {
        // This is a NOTE OFF event

        if (!noteCount) {
            return(false);
        }
        if (sustain) {
            if (sustainBufferCount == MAXNOTES - 1) purgeSustainBuffer(tick);
            sustainBuffer[sustainBufferCount] = inEv.data;
            sustainBufferCount++;
            return(false);
        }

        if (latch_mode && keep_rel) {
            if (latchBufferCount == MAXNOTES - 1) purgeLatchBuffer(tick);
            latchBuffer[latchBufferCount] = inEv.data;
            latchBufferCount++;
            if (latchBufferCount != noteCount) {
                if ((uint32_t)tick > (uint32_t)(lastLatchTick + 30) 
                    && (latchBufferCount > 1)) purgeLatchBuffer(tick);
                lastLatchTick = tick;
            }
            return(false);
        }
        
        releaseNote(inEv.data, tick, keep_rel);
    }
    
    return(false);
}

void MidiArp::addNote(int note, int vel, int tick)
{
        // modify buffer that is not accessed by arpeggio output
    int bufPtr = (noteBufPtr) ? 0 : 1;
    int index = 0;

    if (!noteCount || (note > notes[bufPtr][0][noteCount - 1])
            || (repeatPatternThroughChord == 4) )
        index = noteCount;
    else {
        while (index < MAXNOTES && note > notes[bufPtr][0][index]) index++;

        for (int l3 = 0; l3 < 4; l3++) {
            for (int l2 = noteCount; l2 > index; l2--) {
                notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 - 1];
            }
        }
    }
    notes[bufPtr][0][index] = note;
    notes[bufPtr][1][index] = vel;
    notes[bufPtr][2][index] = tick;
    notes[bufPtr][3][index] = 0;
    noteCount++;

    copyNoteBuffer();
}

void MidiArp::releaseNote(int note, int tick, bool keep_rel)
{
    // modify buffer that is not accessed by arpeggio output
    int bufPtr = (noteBufPtr) ? 0 : 1;
    if ((!keep_rel) || (!release_time)) {
        //definitely remove from buffer
        if (note == notes[bufPtr][0][noteCount - 1]
                && (repeatPatternThroughChord != 4)) {
            //note is on top of buffer: only decrement noteCount
            noteCount--;
            if (repeatPatternThroughChord == 2) noteOfs = noteCount - 1;
        }
        else {
            //note is not on top: take out the note and pull down all above
            int index = 0;
            while ((index < MAXNOTES) && (index < noteCount) 
                && (note != notes[bufPtr][0][index])) index++;
            deleteNoteAt(index, bufPtr);
        }
    }
    else tagAsReleased(note, tick, bufPtr);
    
    copyNoteBuffer();
}

void MidiArp::removeNote(int *noteptr, int tick, int keep_rel)
{
    int bufPtr, note ;
    note = *noteptr;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    if (!noteCount) {
        return;
    }
    if (!keep_rel || (!release_time)) {
        // definitely remove from buffer, do NOT check for doubles
        if (note == notes[bufPtr][0][noteCount - 1]
                && (repeatPatternThroughChord != 4)) {
            // note is on top of buffer: only decrement noteCount
            noteCount--;
            if (tick == -1) releaseNoteCount--;
            if ((repeatPatternThroughChord == 2) && (noteOfs)) noteOfs--;
        }
        else {
            // note is not on top: take out the note and pull down all above
            int index = 0;
            if (tick != -1) {
                while ((index < noteCount) 
                        && (note != notes[bufPtr][0][index])) index++;
            }
            else {
                while ((index < noteCount) 
                        && ((note != notes[bufPtr][0][index])
                        || (!notes[bufPtr][3][index]))) index++;
            }

            
            if (note == notes[bufPtr][0][index]) {
                deleteNoteAt(index, bufPtr);
                if (tick == -1) releaseNoteCount--;
                for (int l2 = index; l2 < noteCount; l2++) {
                    old_attackfn[l2] = old_attackfn[l2 + 1];
                }
            }
        }
    }
    else tagAsReleased(note, tick, bufPtr);

    copyNoteBuffer();
}

void MidiArp::deleteNoteAt(int index, int bufPtr)
{
    for (int l3 = 0; l3 < 4; l3++) {
        for (int l2 = index; l2 < noteCount - 1; l2++) {
            notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 + 1];
        }
    }
    noteCount--;
}

void MidiArp::tagAsReleased(int note, int tick, int bufPtr)
{
    //mark as released but keep with note off time tick
    int l1 = 0;
    while ((l1 < noteCount) 
        && ((note != notes[bufPtr][0][l1]) || (notes[bufPtr][3][l1]))) {
        l1++;
    }
    if (note == notes[bufPtr][0][l1]) {
        notes[bufPtr][3][l1] = 1;
        notes[bufPtr][2][l1] = tick;
        releaseNoteCount++;
    }
}

void MidiArp::copyNoteBuffer()
{
    int newBufPtr = noteBufPtr;
    noteBufPtr++;
    noteBufPtr%=2;

    for (int l2 = 0; l2 < noteCount; l2++) {
        for (int l3 = 0; l3 < 4; l3++) {
            notes[newBufPtr][l3][l2] = notes[noteBufPtr][l3][l2];
        }
    }
}

void MidiArp::getNote(int *tick, int note[], int velocity[], int *length)
{
    char c;
    int l1, tmpIndex[MAXCHORD], chordIndex, grooveTmp;
    int current_octave = 0;
    bool outOfRange = false;
    bool gotCC, pause;


    chordIndex = 0;
    tmpIndex[0] = 0;
    tmpIndex[1] = -1;
    gotCC = false;
    pause = false;
    
    if (purgeReleaseFlag) {
        purgeLatchBuffer(arpTick);
        purgeReleaseNotes(noteBufPtr);
        purgeReleaseFlag = false;
    }
    if (restartFlag) advancePatternIndex(true);

    if (!patternIndex) initLoop();

    framePtr++;
    if (framePtr >= nPoints) framePtr = 0;

    chordSemitone[0] = semitone;
    do {
        if (patternLen)
            c = (pattern.at(patternIndex));
        else
            c = ' ';

        if (c != ' ') {
            if (isdigit(c) || (c == 'p')) {
                tmpIndex[chordIndex] = c - '0' + noteOfs;
                if ((chordIndex < MAXNOTES - 1) && chordMode) {
                    chordIndex++;
                    chordSemitone[chordIndex] = semitone;
                }
                gotCC = false;
                pause = (c == 'p');
            }
            else {
                gotCC = true;

                switch(c) {
                    case '(':
                        chordMode = true;
                        break;
                    case ')':
                        // mark end of chord
                        tmpIndex[chordIndex] = -1;
                        chordMode = false;
                        gotCC = false;
                        break;
                    case 't':
                        semitone++;
                        break;
                    case 'g':
                        semitone--;
                        break;
                    case '+':
                        semitone+=12;
                        break;
                    case '-':
                        semitone-=12;
                        break;
                    case '=':
                        semitone = 0;
                        break;
                    case '>':
                        stepWidth *= .5;
                        break;
                    case '<':
                        stepWidth *= 2.0;
                        break;
                    case '.':
                        stepWidth = 1.0;
                        break;
                    case '/':
                        vel += 0.2;
                        break;
                    case '\\':
                        vel -= 0.2;
                        break;
                    case 'd':
                        len *= 2.0;
                        break;
                    case 'h':
                        len *= .5;
                        break;
                }
                chordSemitone[chordIndex] = semitone;
            }
        }
        current_octave = octOfs;
    } while (advancePatternIndex(false) && (gotCC || chordMode || c == ' '));

    l1 = 0;
    if (noteCount) do {
        noteIndex[l1] = (noteCount) ? tmpIndex[l1] % noteCount : 0;
        note[l1] = clip(notes[noteBufPtr][0][noteIndex[l1]] + current_octave * 12
                + chordSemitone[l1], 0, 127, &outOfRange);
        if (outOfRange) checkOctaveAtEdge(false);

        grooveTmp = (framePtr % 2) ? grooveVelocity : -grooveVelocity;
        
        double releasefn = 0;
        if ((release_time > 0) && (notes[noteBufPtr][3][noteIndex[l1]])) {
            releasefn = 1.0 - (double)(arpTick
                    - notes[noteBufPtr][2][noteIndex[l1]])
                    / (release_time * (double)TPQN * 2);

            if (releasefn < 0.0) releasefn = 0.0;
        }
        else releasefn = 1.0;
        
        double attackfn = 0;
        if (attack_time > 0) {
            if (!notes[noteBufPtr][3][noteIndex[l1]]) {
                attackfn = (double)(arpTick
                    - notes[noteBufPtr][2][noteIndex[l1]])
                    / (attack_time * (double)TPQN * 2);

                if (attackfn > 1.0) attackfn = 1.0;
                old_attackfn[noteIndex[l1]] = attackfn;
            }
            else attackfn = old_attackfn[noteIndex[l1]];
        }
        else attackfn = 1.0;

        velocity[l1] = clip((double)notes[noteBufPtr][1][noteIndex[l1]]
                * vel * (1.0 + 0.005 * (double)(randomVelocity + grooveTmp))
                * releasefn * attackfn, 0, 127, &outOfRange);

        if ((release_time > 0.) && (notes[noteBufPtr][3][noteIndex[l1]]) && (!velocity[l1])) {
            removeNote(&notes[noteBufPtr][0][noteIndex[l1]], -1, 0);
        }
        else {
            l1++;
        }
    } while (  (l1 < MAXCHORD - 1)
            && (tmpIndex[l1] >= 0)
            && ((l1 < noteCount) || (tmpIndex[l1] == 0))
            && (noteCount));

    note[l1] = -1; // mark end of array
    grooveTmp = (framePtr % 2) ? grooveLength : -grooveLength;
    *length = clip(len * stepWidth * (double)TPQN
            * (1.0 + 0.005 * (double)(randomLength + grooveTmp)), 2,
            1000000,  &outOfRange);

    if (!framePtr) grooveTick = newGrooveTick;
    grooveTmp = TPQN * stepWidth * grooveTick * 0.01;
    /* pairwise application of new groove shift */
    if (!(framePtr % 2)) {
        grooveTmp = -grooveTmp;
        grooveTick = newGrooveTick;
    }
    arpTick += stepWidth * TPQN + grooveTmp;

    if (!trigByKbd && !framePtr && !grooveTick) {
        /* round-up to current resolution (quantize) */
        arpTick/= (TPQN * minStepWidth);
        arpTick*= (TPQN * minStepWidth);
    }

    *tick = arpTick + clip(stepWidth * 0.25 * (double)randomTick, 0,
            1000, &outOfRange);

    if (!(patternLen && noteCount) || pause || isMuted) {
        velocity[0] = 0;
    }
}

void MidiArp::checkOctaveAtEdge(bool reset)
{
    if (!octMode) return;
    if (!octHigh && !octLow) {
        octOfs = 0;
        return;
    }
    
    if (reset) {
        octOfs = octLow;
        if (octMode == 2) {
            octOfs = octHigh;
            octIncr = -1;
        }
        else {
            octOfs = octLow;
            octIncr = 1;
        }
        return;
    }
    if (octOfs > octHigh) {
        if (octMode == 3){
            octIncr = - octIncr;
            octOfs--;
            octOfs--;
        }
        else {
            octOfs = octLow;
        }
    }
    if (octOfs < octLow) {
        if (octMode == 3) {
            octIncr = - octIncr;
            octOfs++;
            octOfs++;
        }
        else {
            octOfs = octHigh;
        }
    }
}

bool MidiArp::advancePatternIndex(bool reset)
{
    if (patternLen) {
        patternIndex++;
    }

    if ((patternIndex >= patternLen) || reset) {
        patternIndex = 0;
        restartFlag = false;
        applyPendingParChanges();

        switch (repeatPatternThroughChord) {
            case 1:
            case 4:
                noteOfs++;
                if ((noteCount - 1 < patternMaxIndex + noteOfs) || reset) {
                    noteOfs = 0;
                    octOfs+=octIncr;
                    checkOctaveAtEdge(reset);
                }
                break;
            case 2:
                noteOfs--;
                if ((noteCount -1 < patternMaxIndex) ||
                    (noteOfs < patternMaxIndex) || reset) {
                    noteOfs = noteCount - 1;
                    octOfs+=octIncr;
                    checkOctaveAtEdge(reset);
                }
                break;
            case 3:
                if (noteCount) noteOfs = rand() % noteCount;
                break;
            default:
                noteOfs = 0;
        }
        return(false);
    }
    return(true);
}

void MidiArp::initLoop()
{
    stepWidth = 1.0;
    len = 0.5;
    vel = 0.8;
    semitone = 0;
    framePtr = 0;
}

void MidiArp::getNextFrame(int askedTick)
{
    gotKbdTrig = false;
    
    newRandomValues();

    int l1 = 0;
    //allow 8 ticks of tolerance for echo tick for external sync
    if ((askedTick + 8) >= nextTick) {
        returnTick = nextTick;
        getNote(&nextTick, nextNote, nextVelocity, &nextLength);
        while ((l1 < MAXCHORD - 1) && (nextNote[l1] >= 0)) {
            returnNote[l1] = nextNote[l1];
            returnVelocity[l1] = nextVelocity[l1];
            l1++;
        }
        returnLength = nextLength;
        hasNewNotes = true;
    }
    else hasNewNotes = false;
    
    returnNote[l1] = -1; // mark end of chord
}

void MidiArp::foldReleaseTicks(int tick)
{
    int bufPtr, l2;

    bufPtr = (noteBufPtr) ? 0 : 1;

    if (tick <= 0) {
        purgeReleaseNotes(bufPtr);
        return;
    }

    for (l2 = 0; l2 < noteCount; l2++) {
            notes[bufPtr][2][l2] -= tick;
    }

    copyNoteBuffer();
    lastLatchTick -= tick;    
}

void MidiArp::initArpTick(int tick)
{
    arpTick = tick;
    returnVelocity[0] = 0;
    nextTick  = tick;
    nextVelocity[0] = 0;
    noteIndex[0] = -1;
    patternIndex = 0;
    framePtr = 0;
}

std::string MidiArp::stripPattern(const std::string& p_pattern)
{
    std::string p = p_pattern;
    patternLen = 0;
    if (!p.length()) return (p);

    char c = p[p.length() - 1];
    while (!isdigit(c) && (c != 'p') && (c != ')')) {
        p = p.substr(0, p.length() - 1);
        if (p.length() < 1) break;
        c = p[p.length() - 1];
    }

    patternLen = p.length();

    return (p);
}


void MidiArp::updatePattern(const std::string& p_pattern)
{
    int l1;
    char c;

    pattern = p_pattern;
    patternMaxIndex = 0;
    minStepWidth = 1.0;
    minOctave = 0;
    maxOctave = 0;

    double stepwd = 1.0;
    double nsteps = 0.;
    int chordindex = 0;
    bool chordmd = false;
    int oct = 0;
    int npoints = 0;

    pattern = stripPattern(pattern);
    // determine some useful properties of the arp pattern,
    // number of octaves, step width and number of steps in beats and
    // number of points

    for (l1 = 0; l1 < patternLen; l1++) {
        c = pattern[l1];

        if (isdigit(c)) {
            if (!chordindex) {
                nsteps += stepwd;
                npoints++;
                if (chordmd) chordindex++;
            }
            if (isdigit(c) && (c  - '0' > patternMaxIndex))
                patternMaxIndex = c - '0';
        }
        switch(c) {
            case '(':
                chordmd = true;
                chordindex = 0;
                break;

            case ')':
                chordmd = false;
                chordindex = 0;
                break;

            case '>':
                stepwd *= .5;
                if (stepwd < minStepWidth)
                    minStepWidth *= .5;
                break;

            case '<':
                stepwd *= 2.0;
                break;

            case '.':
                stepwd = 1.0;
                break;

            case 'p':
                if (!chordmd) {
                    nsteps += stepwd;
               	    npoints++;
                }
                break;

            case '+':
                oct++;
                if (oct > maxOctave)
                    maxOctave++;
                break;

            case '-':
                oct--;
                if (oct < minOctave)
                    minOctave--;
                break;

            case '=':
                oct=0;
                break;

            default:
                ;
        }

    }

    patternIndex = 0;
    framePtr = 0;
    noteOfs = 0;
    nSteps = nsteps;
    nPoints = npoints;
}

void MidiArp::newRandomValues()
{
    randomTick = (double)randomTickAmp * (0.5 - (double)rand()
            / (double)RAND_MAX);
    randomVelocity = (double)randomVelocityAmp * (0.5 - (double)rand()
            / (double)RAND_MAX);
    randomLength = (double)randomLengthAmp * (0.5 - (double)rand()
            / (double)RAND_MAX);
}

void MidiArp::updateRandomTickAmp(int val)
{
    randomTickAmp = val;
}

void MidiArp::updateOctaveMode(int val)
{
    octMode = val;
    octOfs = 0;
    
    switch (val) {
        case 0: 
            octIncr = 0;
        break;

        case 1:
            octIncr = 1;
        break;

        case 2:
            octIncr = -1;
        break;

        case 3:
            octIncr = 1;
        break;
    }
}

void MidiArp::updateRandomVelocityAmp(int val)
{
    randomVelocityAmp = val;
}

void MidiArp::updateRandomLengthAmp(int val)
{
    randomLengthAmp = val;
}

void MidiArp::updateAttackTime(int val)
{
    attack_time = (double)val;
}

void MidiArp::updateReleaseTime(int val)
{
    if (release_time > 0 && val == 0) purgeReleaseFlag = true;

    release_time = (double)val;
}

void MidiArp::clearNoteBuffer()
{
    noteCount = 0;
    latchBufferCount = 0;
    releaseNoteCount = 0;
}

int MidiArp::getPressedNoteCount()
{
    int c = noteCount - latchBufferCount - releaseNoteCount;
    return(c);
}

void MidiArp::setSustain(bool on, int sustick)
{
    sustain = on;
    if (!sustain) {
        purgeSustainBuffer(sustick);
        if (latch_mode) purgeLatchBuffer(sustick);
    }
}

void MidiArp::purgeSustainBuffer(int sustick)
{
    for (int l1 = 0; l1 < sustainBufferCount; l1++) {
        int buf = sustainBuffer[l1];
        removeNote(&buf, sustick, 1);
    }
    sustainBufferCount = 0;
}

void MidiArp::setLatchMode(bool on)
{
    latch_mode = on;
    if (!latch_mode) purgeLatchBuffer(arpTick);
}

void MidiArp::purgeLatchBuffer(int latchtick)
{
    for (int l1 = 0; l1 < latchBufferCount; l1++) {
        int buf = latchBuffer[l1];
        removeNote(&buf, latchtick, 1);
    }
    latchBufferCount = 0;
}

void MidiArp::purgeReleaseNotes(int bufptr)
{
    for (int l1 = noteCount - 1; l1 >= 0; l1--) {
        if (notes[bufptr][3][l1])
            deleteNoteAt(l1, bufptr);
            releaseNoteCount--;
    }
}

void MidiArp::applyPendingParChanges()
{
    if (!parChangesPending) return;

    int olddefer = deferChanges;
    deferChanges = false;
    setMuted(isMutedDefer);
    deferChanges = olddefer;
    parChangesPending = false;
    needsGUIUpdate = true;
}

void MidiArp::setNextTick(int tick)
{
    if (nSteps == 0) return;

    returnTick = tick / (int)(nSteps*TPQN) * (int)(nSteps*TPQN);
    patternIndex = 0;
    framePtr = 0;
    arpTick = returnTick;
    nextTick = returnTick;
}
