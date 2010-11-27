#ifndef MIDIARP_H
#define MIDIARP_H

#include <QMutex>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QVector>
#include <alsa/asoundlib.h>
#include <main.h>


class MidiArp : public QThread  {
    
  Q_OBJECT
    
  private:
    int nextNote[MAXCHORD], nextVelocity[MAXCHORD];
    int currentNoteTick, nextNoteTick, currentTick, arpTick;
    int currentNote[MAXCHORD], currentVelocity[MAXCHORD];
    int currentLength, nextLength;
    bool newCurrent, newNext, chordMode;
    int grooveTick, grooveVelocity, grooveLength, grooveIndex;
    int randomTick, randomVelocity, randomLength;
    double queueTempo;

    QVector<int> sustainBuffer;
    QVector<int> latchBuffer;
    QTimer *latchTimer;

    bool sustain, latch_mode;
    int octave, noteIndex[MAXCHORD], patternIndex;
    int notes[2][4][MAXNOTES]; // Buffer Index, Note/Velocity/On-offTick/releaseMark, Data Index
    double old_attackfn[MAXNOTES];
    int noteBufPtr, noteCount, patternLen, patternMaxIndex, noteOfs;

    QMutex mutex;
    void initLoop();  
    int clip(int value, int min, int max, bool *outOfRange);
    void updateNotes(int currentTick);  
    void getNote(int *tick, int note[], int velocity[], int *length);
    bool advancePatternIndex(bool reset);
   
  public:
    int chIn;       // Channel of input events
    int indexIn[2]; // Index input/output (for Controller events)
    int rangeIn[2]; // Parameter that is mapped, [0] low, [1] high boundary
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool hold, isMuted;
    int repeatPatternThroughChord;
    double tempo, len, vel;
    double attack_time, release_time; 
    int randomTickAmp, randomVelocityAmp, randomLengthAmp;
    QString pattern;
    QVector<int> returnNote, returnVelocity;
    int returnTick, returnIsNew, returnLength;
           
  public:
    MidiArp();
    ~MidiArp();
    bool isArp(snd_seq_event_t *evIn);   // Check if evIn is in the input range of the arp
    void addNote(int note, int velocity, int tick); // Add input Note for Arpeggio
    void handleNoteOff(int note, int tick, int keep_rel); // Remove input Note from Arpeggio
    void removeNote(int *noteptr, int tick, int keep_rel); // Remove input Note from Arpeggio
    void getCurrentNote(int askedTick);
    void getNextNote(int askedTick);
    int getNextNoteTick();
    void initArpTick(int currentTick);
    void foldReleaseTicks(int currentTick);
    void newRandomValues();
    void newGrooveValues(int p_grooveTick, int p_grooveVelocity,
            int p_grooveLength);
    void run();
    
  signals:
    void nextStep(int patternIndex);

  public slots:  
    void updatePattern(const QString&);
    void updateRandomTickAmp(int);
    void updateRandomVelocityAmp(int);
    void updateRandomLengthAmp(int);
    void updateAttackTime(int);
    void updateQueueTempo(int);
    void updateReleaseTime(int);
    void setMuted(bool); //set mute
    void setSustain(bool, int); //set sustain
    void setLatchMode(bool); //set latch mode
    void purgeSustainBuffer(int sustick);
    void purgeLatchBuffer();
    void clearNoteBuffer();
};
                              
#endif
