#ifndef MIDIARP_H
#define MIDIARP_H

#include <QObject>
#include <QString>
#include <QVector>
#include <alsa/asoundlib.h>
#include <main.h>


class MidiArp : public QObject  {
    
  Q_OBJECT
    
  private:
    void updateNotes(snd_seq_tick_time_t currentTick);  
    void getNote(snd_seq_tick_time_t *tick, int note[], int velocity[],
            int *length);
    bool advancePatternIndex();
    snd_seq_tick_time_t nextNoteTick;
    int nextNote[MAXCHORD], nextVelocity[MAXCHORD];
    int nextLength;
    snd_seq_tick_time_t currentNoteTick;
    int currentNote[MAXCHORD], currentVelocity[MAXCHORD];
    int currentLength;
    bool newCurrent, newNext, chordMode;
    snd_seq_tick_time_t arpTick, lastArpTick;
    int grooveTick, grooveVelocity, grooveLength, grooveIndex;
    double queueTempo;
    QVector<int> sustainBufferList;
    
  private:
    void initLoop();  
    int clip(int value, int min, int max, bool *outOfRange);
    int sustain;
    int randomVelocity, randomTick, randomLength;
    int octave, noteIndex[MAXCHORD], patternIndex;
    int notes[2][4][MAXNOTES]; // Buffer Index, Note/Velocity/On-offTick/releaseMark, Data Index
    double old_attackfn[MAXNOTES];
    int noteBufPtr, noteCount, patternLen, patternMaxIndex, noteOfs;
    
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
           
  public:
    MidiArp();
    ~MidiArp();
    bool isArp(snd_seq_event_t *evIn);   // Check if evIn is in the input range of the arp
    void addNote(snd_seq_event_t *evIn, int tick); // Add input Note for Arpeggio
    void removeNote(snd_seq_event_t *evIn, int tick, int keep_rel); // Remove input Note from Arpeggio
    void removeNote(int *noteptr, int tick, int keep_rel); // Remove input Note from Arpeggio
    void getCurrentNote(snd_seq_tick_time_t currentTick,
            snd_seq_tick_time_t *tick, int note[], int velocity[],
            int *length, bool *isNew);
    void getNextNote(snd_seq_tick_time_t *tick, int note[], int velocity[],
            int *length, bool *isNew);
    void initArpTick(snd_seq_tick_time_t currentTick);
    void newRandomValues();
    void newGrooveValues(int p_grooveTick, int p_grooveVelocity,
            int p_grooveLength);
    
  public slots:  
    void updatePattern(const QString&);
    void updateRandomTickAmp(int);
    void updateRandomVelocityAmp(int);
    void updateRandomLengthAmp(int);
    void updateAttackTime(int);
    void updateQueueTempo(int);
    void updateReleaseTime(int);
    void muteArp(bool); //set mute
    void setSustain(bool, int); //set sustain
    void clearNoteBuffer();
};
                              
#endif
