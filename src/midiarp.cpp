/*!
 * @file midiarp.cpp
 * @brief Implements the MidiArp MIDI worker class for the Arpeggiator Module.
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
#include <cstdlib>
#include <cstdio>
#include <QString>
#include "midiarp.h"


MidiArp::MidiArp()
{
    for (int l1 = 0; l1 < 2; l1++) {
        rangeIn[l1] = (l1) ? 127 : 0;
        indexIn[l1] = (l1) ? 127 : 0;
    }
    chIn = 0;
    portOut = 0;
    channelOut = 0;
    noteBufPtr = 0;
    noteCount = 0;
    restartByKbd = false;
    trigByKbd = false;
    stepWidth = 1.0;     // stepWidth relative to global queue stepWidth
    len = 0.5;       // note length
    vel = 0.8;  // velocity relative to global velocity
    noteIndex[0] = 0;
    patternIndex = 0;
    grooveIndex = 0;
    patternLen = 0;
    octave = 0;
    newCurrent = false;
    newNext = false;
    currentNoteTick = 0;
    nextNoteTick = 0;
    patternMaxIndex = 0;
    noteOfs = 0;
    arpTick = 0;
    chordMode = false;
    randomTick = 0;
    randomVelocity = 0;
    randomLength = 0;
    randomTickAmp = 0;
    randomVelocityAmp = 0;
    randomLengthAmp = 0;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
    repeatPatternThroughChord = 1;
    isMuted = false;
    attack_time = 0.0;
    release_time = 0.0;
    queueTempo = 100.0;
    sustain = false;
    sustainBuffer.clear();
    latch_mode = false;
    latchBuffer.clear();
    latchTimer = new QTimer(this);
    latchTimer->setSingleShot(true);
    connect(latchTimer, SIGNAL(timeout()), this, SLOT(purgeLatchBuffer()));
}

MidiArp::~MidiArp(){
    wait();
}

void MidiArp::setMuted(bool on)
{
    isMuted = on;
}

bool MidiArp::wantEvent(MidiEvent event) {

    if (event.channel != chIn) return(false);
    if ((event.type == EV_CONTROLLER) && (event.data != CT_FOOTSW)) return(false);
    if (event.type == EV_NOTEON) {
        if (((event.data < indexIn[0]) || (event.data > indexIn[1]))
            || ((event.value < rangeIn[0]) || (event.value > rangeIn[1]))) {
            return(false);
        }
    }
    return(true);
}

void MidiArp::handleNoteOn(int note, int velocity, int tick)
{
    mutex.lock();
    int bufPtr, index;

    if (!getPressedNoteCount()) {
        purgeLatchBuffer();
        if (restartByKbd) advancePatternIndex(true);
        if (trigByKbd) initArpTick(tick);
    }
    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;

    if (!noteCount || (note > notes[bufPtr][0][noteCount - 1]))
        index = noteCount;
    else {
        index = 0;
        while (note > notes[bufPtr][0][index]) index++;

        for (int l3 = 0; l3 < 4; l3++) {
            for (int l2 = noteCount; l2 > index; l2--) {
                notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 - 1];
            }
        }
    }
    notes[bufPtr][0][index] = note;
    notes[bufPtr][1][index] = velocity;
    notes[bufPtr][2][index] = tick;
    notes[bufPtr][3][index] = 0;
    noteCount++;

    if (repeatPatternThroughChord == 2) noteOfs = noteCount - 1;
    copyNoteBuffer();
    mutex.unlock();
}

void MidiArp::handleNoteOff(int note, int tick, int keep_rel)
{
    int bufPtr, index;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    if (!noteCount) {
        return;
    }
    if (sustain) {
        sustainBuffer.append(note);
        return;
    }

    if (latch_mode) {
        latchBuffer.append(note);
        if (latchBuffer.count() == noteCount) {
            latchTimer->stop();
        }
        else {
            latchTimer->start(200);
        }
        return;
    }

    if ((!keep_rel) || (!release_time)) {
        //definitely remove from buffer
        if (note == notes[bufPtr][0][noteCount - 1]) {
            //note is on top of buffer: only decrement noteCount
            noteCount--;
            if (repeatPatternThroughChord == 2) noteOfs = noteCount - 1;
        }
        else {
            //note is not on top: take out the note and pull down all above
            index = 0;
            while ((index < noteCount) && (note > notes[bufPtr][0][index])) index++;
            deleteNoteAt(index, bufPtr);
        }
    }
    else tagAsReleased(note, tick, bufPtr);

    copyNoteBuffer();
}

void MidiArp::removeNote(int *noteptr, int tick, int keep_rel)
{
    int bufPtr, index, note ;
    note = *noteptr;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    if (!noteCount) {
        return;
    }
    if (!keep_rel || (!release_time)) {
        // definitely remove from buffer, do NOT check for doubles
        if (note == notes[bufPtr][0][noteCount - 1]) {
            // note is on top of buffer: only decrement noteCount
            noteCount--;
            if ((repeatPatternThroughChord == 2) && (noteOfs)) noteOfs--;
        }
        else {
            // note is not on top: take out the note and pull down all above
            index = 0;
            while ((index < noteCount) && (note > notes[bufPtr][0][index])) index++;

            // additional forward in buffer to look for release marked note
            // tick is -1 only if removeNote called after release
            while ((index < noteCount) && (!notes[bufPtr][3][index])
                    && (tick == -1)) {
                index++;
            }

            if (note == notes[bufPtr][0][index]) {
                deleteNoteAt(index, bufPtr);
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
    while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) l1++;
    while ((l1 < noteCount) && (notes[bufPtr][3][l1])) l1++;
    if (note == notes[bufPtr][0][l1]) {
        notes[bufPtr][3][l1] = 1;
        notes[bufPtr][2][l1] = tick;
    }
}

void MidiArp::copyNoteBuffer()
{
    int newBufPtr = noteBufPtr;
    noteBufPtr++;
    noteBufPtr%=2;

    for (int l3 = 0; l3 < 4; l3++) {
        for (int l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[noteBufPtr][l3][l2];
        }
    }
}

void MidiArp::getNote(int *tick, int note[], int velocity[], int *length)
{
    QChar c;
    int l1, tmpIndex[MAXCHORD], chordIndex, grooveTmp;
    bool gotCC, outOfRange, pause;
    double attackfn, releasefn;


    chordIndex = 0;
    tmpIndex[0] = 0;
    tmpIndex[1] = -1;
    gotCC = false;
    pause = false;


    if (!patternIndex) initLoop();
    do {
        if (patternLen)
            c = (pattern.at(patternIndex));
        else
            c = ' ';

        if (c != ' ') {
            if (c.isDigit() || (c == 'p')) {
                tmpIndex[chordIndex] = c.digitValue() + noteOfs;
                if (chordMode) {
                    chordIndex++;
                }
                gotCC = false;
                pause = (c == 'p');
            }
            else {
                gotCC = true;

                switch(c.toAscii()) {
                    case '(':
                        chordMode = true;
                        break;
                    case ')':
                        // mark end of chord
                        tmpIndex[chordIndex] = -1;
                        chordMode = false;
                        gotCC = false;
                        break;
                    case '+':
                        octave++;
                        break;
                    case '-':
                        octave--;
                        break;
                    case '=':
                        octave=0;
                        break;
                    case '>':
                        stepWidth /= 2.0;
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
                        len *= 2;
                        break;
                    case 'h':
                        len /= 2;
                        break;
                }
            }
        }
    } while (advancePatternIndex(false) && (gotCC || chordMode));

    l1 = 0;

    if (noteCount) do {
        noteIndex[l1] = (noteCount) ? tmpIndex[l1] % noteCount : 0;
        note[l1] = clip(notes[noteBufPtr][0][noteIndex[l1]]
                + octave * 12, 0, 127, &outOfRange);
        grooveTmp = (grooveIndex % 2) ? -grooveVelocity : grooveVelocity;

        if ((release_time > 0) && (notes[noteBufPtr][3][noteIndex[l1]])) {
            releasefn = 1.0 - (double)(arpTick
                    - notes[noteBufPtr][2][noteIndex[l1]])
                    / release_time / (double)TPQN
                    * 60 / queueTempo;

            if (releasefn < 0.0) releasefn = 0.0;
        }
        else releasefn = 1.0;

        if (attack_time > 0) {
            if (!notes[noteBufPtr][3][noteIndex[l1]]) {
                attackfn = (double)(arpTick
                    - notes[noteBufPtr][2][noteIndex[l1]])
                    / attack_time / (double)TPQN
                    * 60 / queueTempo;

                if (attackfn > 1.0) attackfn = 1.0;
                old_attackfn[noteIndex[l1]] = attackfn;
            }
            else attackfn = old_attackfn[noteIndex[l1]];
        }
        else attackfn = 1.0;

        velocity[l1] = clip((double)notes[noteBufPtr][1][noteIndex[l1]]
                * vel * (1.0 + 0.005 * (double)(randomVelocity + grooveTmp))
                * releasefn * attackfn, 0, 127, &outOfRange);

        if ((notes[noteBufPtr][3][noteIndex[l1]]) && (!velocity[l1]))

            removeNote(&notes[noteBufPtr][0][noteIndex[l1]], -1, 0);

        else

            l1++;

    } while ((tmpIndex[l1] >= 0) && (l1 < MAXCHORD - 1) && (l1 < noteCount));

    note[l1] = -1; // mark end of array
    grooveTmp = (grooveIndex % 2) ? -grooveLength : grooveLength;
    *length = clip(len * stepWidth * (double)TPQN
            * (1.0 + 0.005 * (double)(randomLength + grooveTmp)), 2,
            1000000,  &outOfRange);
    grooveTmp = (grooveIndex % 2) ? -grooveTick : grooveTick;
    arpTick += stepWidth * (double)TPQN
        * (1.0 + 0.005 * (double)grooveTmp);
    *tick = arpTick + clip(stepWidth * 0.25 * (double)randomTick, 0,
            1000, &outOfRange);

    if (!(patternLen && noteCount) || pause || isMuted) {
        velocity[0] = 0;
    }
    grooveIndex++;
    emit nextStep(grooveIndex);
}

bool MidiArp::advancePatternIndex(bool reset)
{
    if (patternLen) {
        patternIndex++;
    }
    if ((patternIndex >= patternLen) || reset) {
        patternIndex = 0;
        switch (repeatPatternThroughChord) {
            case 1:
                noteOfs++;
                if (noteCount - 1 < patternMaxIndex + noteOfs) {
                    noteOfs = 0;
                }
                break;
            case 2:
                noteOfs--;
                if ((noteCount -1 < patternMaxIndex) ||
                    (noteOfs < patternMaxIndex)) {
                    noteOfs = noteCount - 1;
                }
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
    octave = 0;
    grooveIndex = 0;
}

void MidiArp::prepareNextNote(int askedTick)
{
    int l1 = 0;
    returnNote.clear();
    returnVelocity.clear();

    nextNoteTick = askedTick;
    while ((nextNote[l1] >= 0) && (l1 < MAXCHORD - 1)) {
        returnNote.append(nextNote[l1]);
        returnVelocity.append(nextVelocity[l1]);
        l1++;
    }
    returnNote.append(-1); // mark end of chord
    returnLength = nextLength;
    returnIsNew = newNext;
    newNext = false;
}

int MidiArp::getNextNoteTick()
{
    return(nextNoteTick);
}

bool MidiArp::wantTrigByKbd()
{
    bool on = ((getPressedNoteCount() == 1) && trigByKbd);
    return(on);
}

void MidiArp::prepareCurrentNote(int askedTick)
{
    currentTick = askedTick;
    start(Priority(6));
    wait();
}

void MidiArp::run()
{
    int l1 = 0;
    mutex.lock();
    updateNotes();
    returnTick = currentNoteTick;
    returnNote.clear();
    returnVelocity.clear();

    while ((currentNote[l1] >= 0) && (l1 < MAXCHORD - 1)) {
        returnNote.append(currentNote[l1]);
        returnVelocity.append(currentVelocity[l1]);
        l1++;
    }
    returnNote.append(-1); // mark end of chord
    returnLength = currentLength;
    returnIsNew = newCurrent;
    newCurrent = false;
    mutex.unlock();
}

void MidiArp::updateNotes()
{
    int l1 = 0;

    //allow 8 ticks of tolerance for echo tick for external sync
    if ((currentTick + 8) >= currentNoteTick) {
        currentNoteTick = nextNoteTick;
        getNote(&nextNoteTick, nextNote, nextVelocity, &nextLength);
        while ((nextNote[l1] >= 0) && (l1 < MAXCHORD - 1)) {
            currentNote[l1] = nextNote[l1];
            currentVelocity[l1] = nextVelocity[l1];
            l1++;
        }
        currentNote[l1] = -1; // mark end of chord
        currentLength = nextLength;
        newCurrent = true;
        newNext = true;
    }
}

void MidiArp::foldReleaseTicks(int tick)
{
    int bufPtr, l2;

    mutex.lock();
    bufPtr = (noteBufPtr) ? 0 : 1;

    for (l2 = 0; l2 < noteCount; l2++) {
            notes[bufPtr][2][l2] -= tick;
    }

    copyNoteBuffer();
    mutex.unlock();
}

void MidiArp::initArpTick(int tick)
{
    arpTick = tick;
    currentVelocity[0] = 0;
    currentNoteTick = tick;
    nextNoteTick  = tick;
    nextVelocity[0] = 0;
    noteIndex[0] = -1;
    patternIndex = 0;
    grooveIndex = 0;
}

void MidiArp::updatePattern(const QString& p_pattern)
{
    mutex.lock();
    int l1;
    QChar c;

    pattern = p_pattern;
    patternLen = pattern.length();
    patternMaxIndex = 0;
    if (patternLen)
    {
        c = (pattern.at(patternLen - 1));
        while (!c.isDigit() && (c != 'p') && (c != ')'))
        {
            pattern = pattern.left(patternLen - 1);
            patternLen--;
            if (patternLen < 1) break;
            c = (pattern.at(patternLen - 1));
        }
    }

    for (l1 = 0; l1 < patternLen; l1++) {
        c = pattern.at(l1);
        if (c.isDigit() && (c.digitValue() > patternMaxIndex)) {
            patternMaxIndex = c.digitValue();
        }
    }
    patternIndex = 0;
    grooveIndex = 0;
    noteOfs = 0;
    mutex.unlock();
}

int MidiArp::clip(int value, int min, int max, bool *outOfRange)
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

void MidiArp::newRandomValues()
{
    mutex.lock();
    randomTick = (double)randomTickAmp * (0.5 - (double)random()
            / (double)RAND_MAX);
    randomVelocity = (double)randomVelocityAmp * (0.5 - (double)random()
            / (double)RAND_MAX);
    randomLength = (double)randomLengthAmp * (0.5 - (double)random()
            / (double)RAND_MAX);
    mutex.unlock();
}

void MidiArp::updateRandomTickAmp(int val)
{
    randomTickAmp = val;
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
    release_time = (double)val;
}

void MidiArp::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiArp::updateTriggerMode(int val)
{
    switch (val) {
        case 0:
            trigByKbd = false;
            restartByKbd = false;
        break;
        case 1:
            trigByKbd = false;
            restartByKbd = true;
        break;
        case 2:
            trigByKbd = true;
            restartByKbd = true;
        break;
        default:
            trigByKbd = false;
            restartByKbd = false;
    }
}

void MidiArp::clearNoteBuffer()
{
    noteCount = 0;
    latchBuffer.clear();
}

int MidiArp::getPressedNoteCount()
{
    int c = noteCount - latchBuffer.count();
    return(c);
}

void MidiArp::setSustain(bool on, int sustick)
{
    sustain = on;
    if (!sustain) {
        purgeSustainBuffer(sustick);
        if (latch_mode) purgeLatchBuffer();
    }
}

void MidiArp::purgeSustainBuffer(int sustick)
{
    for (int l1 = 0; l1 < sustainBuffer.count(); l1++) {
        int buf = sustainBuffer.at(l1);
        removeNote(&buf, sustick, 1);
    }
    sustainBuffer.clear();
}

void MidiArp::setLatchMode(bool on)
{
    latch_mode = on;
    if (!latch_mode) purgeLatchBuffer();
}

void MidiArp::purgeLatchBuffer()
{
    for (int l1 = 0; l1 < latchBuffer.count(); l1++) {
        int buf = latchBuffer.at(l1);
        removeNote(&buf, arpTick, 1);
    }
    latchBuffer.clear();
}

void MidiArp::newGrooveValues(int p_grooveTick, int p_grooveVelocity,
        int p_grooveLength)
{
    mutex.lock();
    grooveTick = p_grooveTick;
    grooveVelocity = p_grooveVelocity;
    grooveLength = p_grooveLength;
    mutex.unlock();
}
