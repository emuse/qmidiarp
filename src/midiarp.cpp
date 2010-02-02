#include <cstdlib>
#include <cstdio>
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
    attack_time = 0.0;
    release_time = 0.0;
    queueTempo = 100.0;
    sustain = false;
    sustainBufferList.clear();
}

MidiArp::~MidiArp(){
    wait();
}

bool MidiArp::isArp(snd_seq_event_t *evIn) {

    if ((evIn->type != SND_SEQ_EVENT_NOTEON)
            && (evIn->type != SND_SEQ_EVENT_NOTEOFF)
            && (evIn->type != SND_SEQ_EVENT_CONTROLLER)) {
        return(false);
    }
    if ((evIn->data.control.channel < chIn)
            || (evIn->data.control.channel > chIn)) {
        return(false);
    }
    if ((evIn->type == SND_SEQ_EVENT_NOTEON) 
            || (evIn->type == SND_SEQ_EVENT_NOTEOFF)) {
        if (((evIn->data.note.note < indexIn[0])
                    || (evIn->data.note.note > indexIn[1])) 
                || ((evIn->data.note.velocity < rangeIn[0])
                    || (evIn->data.note.velocity > rangeIn[1]))) {
            return(false);
        }
    }
    return(true);
}

void MidiArp::addNote(int note, int velocity, int tick)
{
    mutex.lock();
    int bufPtr, newBufPtr, l1, l2, l3;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    //note = evIn->data.note.note;
    if (!noteCount || (note > notes[bufPtr][0][noteCount - 1])) {
        notes[bufPtr][0][noteCount] = note;
        notes[bufPtr][1][noteCount] = velocity;
        notes[bufPtr][2][noteCount] = tick;
        notes[bufPtr][3][noteCount] = 0;
        noteCount++;
    } 
    else {
        l1 = 0;
        while (note > notes[bufPtr][0][l1]) {
            l1++;
        }
        for (l3 = 0; l3 < 4; l3++) {
            for (l2 = noteCount; l2 > l1; l2--) {
                notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 - 1];
            }  
        }
        notes[bufPtr][0][l1] = note;
        notes[bufPtr][1][l1] = velocity;
        notes[bufPtr][2][l1] = tick;
        notes[bufPtr][3][l1] = 0;
        noteCount++;
    } 
    if (repeatPatternThroughChord == 2) noteOfs = noteCount - 1;
    newBufPtr = noteBufPtr;
    noteBufPtr = bufPtr;
    for (l3 = 0; l3 < 4; l3++) {
        for (l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
        }  
    }
    mutex.unlock();
}

void MidiArp::muteArp(bool on)
{
    isMuted = on;
}

void MidiArp::removeNote(int note, int tick, int keep_rel)
{
    int bufPtr, newBufPtr, l1, l2, l3;

    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    //note = evIn->data.note.note;
    if (!noteCount) {
        return;
    }
    if (sustain) {
        sustainBufferList.append(note);
        return;
    }

    if ((!keep_rel) || (!release_time)) {   
        //definitely remove from buffer
        if (note == notes[bufPtr][0][noteCount - 1]) {
            noteCount--; 
            if (repeatPatternThroughChord == 2) noteOfs = noteCount - 1;
        } 
        else {
            l1 = 0;
            while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) l1++;
                for (l3 = 0; l3 < 4; l3++) {
                    for (l2 = l1; l2 < noteCount - 1; l2++) {
                        notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 + 1];
                    }
                }
                noteCount--;
        }
    } 
    else {
        //mark as released but keep with note off time tick
        l1 = 0;
        while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) l1++;
        while ((l1 < noteCount) && (notes[bufPtr][3][l1])) l1++;
        if (note == notes[bufPtr][0][l1]) {
            notes[bufPtr][3][l1] = 1;
            notes[bufPtr][2][l1] = tick;
        }
    }
        newBufPtr = noteBufPtr;
        noteBufPtr = bufPtr;
        for (l3 = 0; l3 < 4; l3++) {
            for (l2 = 0; l2 < noteCount; l2++) {
                notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
            }
        }
}

void MidiArp::removeNote(int *noteptr, int tick, int keep_rel)
{
    int bufPtr, newBufPtr, l1, l2, l3, note ;
    note = *noteptr;
    int tickmark = tick;
    // modify buffer that is not accessed by arpeggio output
    bufPtr = (noteBufPtr) ? 0 : 1;
    if (!noteCount) {
        return;
    }
    if (!keep_rel || (!release_time)) {
        //definitely remove from buffer, do NOT check for doubles
        if (note == notes[bufPtr][0][noteCount - 1]) {
                noteCount--;
                if ((repeatPatternThroughChord == 2) && (noteOfs)) noteOfs--;
        } 
        else {
            l1 = 0;
            while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) {
                l1++;
            }
            while ((l1 < noteCount) && (!notes[bufPtr][3][l1]) 
                    && (tickmark == -1)) {
                //additional forward in buffer to look for release marked note
                // tickmark is -1 only if removeNote called after release
                l1++;
            }
            
            if (note == notes[bufPtr][0][l1]) {
                for (l3 = 0; l3 < 4; l3++) {
                    for (l2 = l1; l2 < noteCount - 1; l2++) {
                        notes[bufPtr][l3][l2] = notes[bufPtr][l3][l2 + 1];
                    }  
                }  
                for (l2 = l1; l2 < noteCount - 1; l2++) {
                    old_attackfn[l2] = old_attackfn[l2 + 1];
                } 
                noteCount--;
            }
        }
    } 
    else {
        //mark as released but keep with note off time tick
        l1 = 0;
        while ((l1 < noteCount) && (note > notes[bufPtr][0][l1])) l1++;
        while ((l1 < noteCount) && (notes[bufPtr][3][l1])) l1++;
        if (note == notes[bufPtr][0][l1]) {
            notes[bufPtr][3][l1] = 1;
            notes[bufPtr][2][l1] = tickmark;
        }
    }
    newBufPtr = noteBufPtr;
    noteBufPtr = bufPtr;
    for (l3 = 0; l3 < 4; l3++) {
        for (l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
        }  
    }
}

void MidiArp::getNote(int *tick, int note[],
        int velocity[], int *length)
{ 
    QChar c;
    int l1, tmpIndex[MAXCHORD], chordIndex, grooveTmp;
    bool gotCC, outOfRange, pause;
    double attackfn, releasefn;
    
    *tick = arpTick + clip(tempo * 0.25 * (double)randomTick, 0,
            1000, &outOfRange);

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
    
    if (noteCount) do {
        noteIndex[l1] = (noteCount) ? tmpIndex[l1] % noteCount : 0; 
        note[l1] = clip(notes[noteBufPtr][0][noteIndex[l1]]
                + octave * 12, 0, 127, &outOfRange);
        grooveTmp = (grooveIndex % 2) ? -grooveVelocity : grooveVelocity;
        
        if ((release_time > 0) && (notes[noteBufPtr][3][noteIndex[l1]])) {
            releasefn = 1.0 - (double)(arpTick 
                    - notes[noteBufPtr][2][noteIndex[l1]])
                    / release_time / (double)TICKS_PER_QUARTER 
                    * 60 / queueTempo;
                    
            if (releasefn < 0.0) releasefn = 0.0;
        } 
        else releasefn = 1.0;

        if (attack_time > 0) {
            if (!notes[noteBufPtr][3][noteIndex[l1]]) {
                attackfn = (double)(arpTick 
                    - notes[noteBufPtr][2][noteIndex[l1]])
                    / attack_time / (double)TICKS_PER_QUARTER 
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
    tempo = 1.0;
    len = 0.5;  
    vel = 0.8;  
    octave = 0; 
}

void MidiArp::getNextNote(int askedTick)
{ 
    int l1 = 0;
    returnNote.clear();
    returnVelocity.clear();

    askedTick = nextNoteTick;
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

void MidiArp::getCurrentNote(int askedTick)
{
    currentTick = askedTick;
    start(Priority(6));
    wait();
}

void MidiArp::run()
{
    int l1 = 0;
    mutex.lock();
    updateNotes(currentTick);
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

void MidiArp::updateNotes(int currentTick)
{
    int l1 = 0;
    
    //allow 8 ticks of tolerance for echo tick for external sync
    if ((currentTick + 8) >= currentNoteTick) {
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

void MidiArp::foldReleaseTicks(int currentTick)
{
    int bufPtr, newBufPtr, l2, l3;
    
    mutex.lock();
    bufPtr = (noteBufPtr) ? 0 : 1;
    
    for (l2 = 0; l2 < noteCount; l2++) {
            notes[bufPtr][2][l2] -= currentTick;
    }
    
    newBufPtr = noteBufPtr;
    noteBufPtr = bufPtr;
    
    for (l3 = 0; l3 < 4; l3++) {
        for (l2 = 0; l2 < noteCount; l2++) {
            notes[newBufPtr][l3][l2] = notes[bufPtr][l3][l2];
        }  
    }
    mutex.unlock();
}

void MidiArp::initArpTick(int currentTick)
{
    mutex.lock();
    arpTick = currentTick;
    lastArpTick = arpTick;
    currentVelocity[0] = 0;
    currentNoteTick = 0;
    nextNoteTick  = 0;
    nextVelocity[0] = 0;
    noteIndex[0] = -1;
    patternIndex = 0;
    mutex.unlock();
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

void MidiArp::clearNoteBuffer()
{
    noteCount = 0;
}

void MidiArp::setSustain(bool on, int sustick)
{
    int l1 = 0;
    sustain = on;
    if (!sustain) {
        for (l1 = 0; l1 < sustainBufferList.count(); l1++) {
            int buf = sustainBufferList.at(l1);
            removeNote(&buf, sustick, 1);
        }  
        sustainBufferList.clear();
    }
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
