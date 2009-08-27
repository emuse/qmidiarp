#include <stdlib.h>
#include <stdio.h>
#include <QString>
#include <alsa/asoundlib.h>
#include "midiarp.h"


MidiArp::MidiArp()
{
    int l1;

    for (l1 = 0; l1 < 2; l1++) {
        rangeIn[l1] = (l1) ? 127 : 0;
        indexIn[l1] = (l1) ? 127 : 0;
    }  
    chIn = 0;
    portOut = 0;
    channelOut = 0;
    noteBufPtr = 0;
    noteCount = 0;
    hold = false;
    tempo = 1.0;     // tempo relative to global queue tempo
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
    lastArpTick = 0;
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
}

MidiArp::~MidiArp(){
}

bool MidiArp::isArp(snd_seq_event_t *evIn) {

    if ((evIn->type != SND_SEQ_EVENT_NOTEON)
            && (evIn->type != SND_SEQ_EVENT_NOTEOFF)) {
        return(false);
    }  
    if ((evIn->data.control.channel < chIn)
            || (evIn->data.control.channel > chIn)) {
        return(false);
    }       
    if (((evIn->data.note.note < indexIn[0])
                || (evIn->data.note.note > indexIn[1])) 
            || ((evIn->data.note.velocity < rangeIn[0])
                || (evIn->data.note.velocity > rangeIn[1]))) {
        return(false);
    }  

    return(true);
}

void MidiArp::addNote(snd_seq_event_t *evIn)
{
    int bufPtr, newBufPtr, l1, l2, l3, note;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    note = evIn->data.note.note;
    if (!noteCount || (note >= notes[bufPtr][0][noteCount - 1])) {
        notes[bufPtr][0][noteCount] = note;
        notes[bufPtr][1][noteCount] = evIn->data.note.velocity;
        noteCount++;
    } else {
        l1 = 0;
        while (note > notes[bufPtr][0][l1]) {
            l1++;
        }
        for (l3 = 0; l3 < 2; l3++) {
            for (l2 = noteCount; l2 > l1; l2--) {
                notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 - 1];
            }  
        }
        notes[bufPtr][0][l1] = note;
        notes[bufPtr][1][l1] = evIn->data.note.velocity;
        noteCount++;
    }  
    newBufPtr = noteBufPtr;
    noteBufPtr = bufPtr;
    for (l3 = 0; l3 < 2; l3++) {
        for (l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
        }  
    }
}

void MidiArp::muteArp(bool on)
{
    isMuted = on;
}

void MidiArp::removeNote(snd_seq_event_t *evIn)
{
    int bufPtr, newBufPtr, l1, l2, l3, note;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    note = evIn->data.note.note;
    if (!noteCount) {
        return;
    }
    if (note == notes[bufPtr][0][noteCount - 1]) {
        do {
            noteCount--;
        } while (note == notes[bufPtr][0][noteCount - 1]);
    } else {
        l1 = 0;
        while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) {
            l1++;
        }
        while (note == notes[bufPtr][0][l1]) {
            for (l3 = 0; l3 < 2; l3++) {
                for (l2 = l1; l2 < noteCount - 1; l2++) {
                    notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 + 1];
                }  
            }  
            noteCount--;
        }
    }  
    newBufPtr = noteBufPtr;
    noteBufPtr = bufPtr;
    for (l3 = 0; l3 < 2; l3++) {
        for (l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
        }  
    }
}

void MidiArp::removeNote(int *noteptr)
{
    int bufPtr, newBufPtr, l1, l2, l3, note ;
    note = *noteptr;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    //note = evIn->data.note.note;
    if (!noteCount) {
        return;
    }
    if (note == notes[bufPtr][0][noteCount - 1]) {
        do {
            noteCount--;
        } while (note == notes[bufPtr][0][noteCount - 1]);
    } else {
        l1 = 0;
        while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) {
            l1++;
        }
        while (note == notes[bufPtr][0][l1]) {
            for (l3 = 0; l3 < 2; l3++) {
                for (l2 = l1; l2 < noteCount - 1; l2++) {
                    notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 + 1];
                }  
            }  
            noteCount--;
        }
    }  
    newBufPtr = noteBufPtr;
    noteBufPtr = bufPtr;
    for (l3 = 0; l3 < 2; l3++) {
        for (l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
        }  
    }
}

void MidiArp::getNote(snd_seq_tick_time_t *tick, int note[],
        int velocity[], int *length)
{ 
    QChar c;
    int l1, tmpIndex[MAXCHORD], chordIndex, grooveTmp;
    bool gotCC, outOfRange, pause;

    *tick = arpTick + clip(tempo * 0.25 * (double)randomTick, 0,
            1000, &outOfRange);

    chordIndex = 0;
    tmpIndex[0] = 0;
    tmpIndex[1] = -1;
    gotCC = false;
    pause = false;

    if (!patternIndex) {
        initLoop();
    }

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
            } else {
                gotCC = true;

                switch(c.toAscii()) { 
                    case '(':
                        chordMode = true;
                        break;
                    case ')':
                        chordMode = false;
                        // mark end of chord
                        tmpIndex[chordIndex] = -1;
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
                        tempo /= 2.0;
                        break;
                    case '<':
                        tempo *= 2.0;
                        break;
                    case '.':
                        tempo = 1.0;
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
    } while (advancePatternIndex() && (gotCC || chordMode));  
    l1 = 0;
    do {
        noteIndex[l1] = (noteCount) ? tmpIndex[l1] % noteCount : 0; 
        note[l1] = clip(notes[noteBufPtr][0][noteIndex[l1]]
                + octave * 12, 0, 127, &outOfRange);
        grooveTmp = (grooveIndex % 2) ? -grooveVelocity : grooveVelocity;
        velocity[l1] = clip((double)notes[noteBufPtr][1][noteIndex[l1]]
                * vel * (1.0 + 0.005 * (double)(randomVelocity + grooveTmp)),
                0, 127, &outOfRange);
        l1++;
    } while ((tmpIndex[l1] >= 0) && (l1 < MAXCHORD - 1) && (l1 < noteCount));

    note[l1] = -1; // mark end of array
    grooveTmp = (grooveIndex % 2) ? -grooveLength : grooveLength;
    *length = clip(len * tempo * (double)TICKS_PER_QUARTER
            * (1.0 + 0.005 * (double)(randomLength + grooveTmp)), 2,
            1000000,  &outOfRange);
    lastArpTick = arpTick;
    grooveTmp = (grooveIndex % 2) ? -grooveTick : grooveTick;
    arpTick += tempo * (double)TICKS_PER_QUARTER
        * (1.0 + 0.005 * (double)grooveTmp);

    if (!(patternLen && noteCount) || pause || isMuted) {
        velocity[0] = 0;
    }  
    grooveIndex++;
}

bool MidiArp::advancePatternIndex()
{
    if (patternLen) {
        patternIndex++;
    }
    if (patternIndex >= patternLen) {
        patternIndex = 0;
        grooveIndex = 0;
        switch (repeatPatternThroughChord) {
            case 1:
                noteOfs++;
                if (noteCount-1 < patternMaxIndex + noteOfs) {
                    noteOfs = 0;
                }
                break;
            case 2:
                noteOfs--;
                if (noteOfs < 0) {
                    noteOfs = noteCount-1 - patternMaxIndex;
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
    tempo = 1.0;
    len = 0.5;  
    vel = 0.8;  
    octave = 0; 
}

void MidiArp::getNextNote(snd_seq_tick_time_t currentTick,
        snd_seq_tick_time_t *tick, int note[], int velocity[],
        int *length, bool *isNew)
{ 
    int l1 = 0;

    *tick = nextNoteTick;
    while ((nextNote[l1] >= 0) && (l1 < MAXCHORD - 1)) { 
        note[l1] = nextNote[l1];
        velocity[l1] = nextVelocity[l1];
        l1++;
    }  
    note[l1] = -1; // mark end of chord
    *length = nextLength;
    *isNew = newNext;
    newNext = false;
}

void MidiArp::getCurrentNote(snd_seq_tick_time_t currentTick,
        snd_seq_tick_time_t *tick, int note[], int velocity[],
        int *length, bool *isNew)
{
    int l1 = 0;

    updateNotes(currentTick);
    *tick = currentNoteTick;

    while ((currentNote[l1] >= 0) && (l1 < MAXCHORD - 1)) { 
        note[l1] = currentNote[l1];
        velocity[l1] = currentVelocity[l1];
        l1++;
    }  
    note[l1] = -1; // mark end of chord
    *length = currentLength;
    *isNew = newCurrent;
    newCurrent = false;
}

void MidiArp::updateNotes(snd_seq_tick_time_t currentTick)
{
    int l1 = 0;

    if (currentTick >= currentNoteTick) {
        currentNoteTick = nextNoteTick;
        while ((nextNote[l1] >= 0) && (l1 < MAXCHORD - 1)) { 
            currentNote[l1] = nextNote[l1];
            currentVelocity[l1] = nextVelocity[l1];
            l1++;
        }  
        currentNote[l1] = -1; // mark end of chord
        currentLength = nextLength;
        newCurrent = true;
        getNote(&nextNoteTick, nextNote, nextVelocity, &nextLength);
        newNext = true;
    } 
}

void MidiArp::initArpTick(snd_seq_tick_time_t currentTick)
{
    arpTick = currentTick;
    lastArpTick = arpTick;
    currentVelocity[0] = 0;
    currentNoteTick = 0;
    nextNoteTick  = 0;
    nextVelocity[0] = 0;
    noteIndex[0] = -1;
    patternIndex = 0;
}

void MidiArp::updatePattern(const QString& p_pattern, ArpScreen *arpScreen)
{
    int l1;
    QChar c;

    pattern = p_pattern;
    patternLen = pattern.length();
    patternMaxIndex = 0;
    for (l1 = 0; l1 < patternLen; l1++) {
        c = pattern.at(l1);
        if (c.isDigit() && (c.digitValue() > patternMaxIndex)) {
            patternMaxIndex = c.digitValue();
        }
    }  
    noteOfs = 0;
    arpScreen->updateArpScreen(pattern);
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
    randomTick = (double)randomTickAmp * (0.5 - (double)random()
            / (double)RAND_MAX);
    randomVelocity = (double)randomVelocityAmp * (0.5 - (double)random()
            / (double)RAND_MAX);
    randomLength = (double)randomLengthAmp * (0.5 - (double)random()
            / (double)RAND_MAX);
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

void MidiArp::newGrooveValues(int p_grooveTick, int p_grooveVelocity,
        int p_grooveLength)
{
    grooveTick = p_grooveTick;
    grooveVelocity = p_grooveVelocity;
    grooveLength = p_grooveLength;
}    

