#ifndef MIDIARP_H
#define MIDIARP_H

#include <qobject.h>
#include <alsa/asoundlib.h>
#include <main.h>

class MidiArp : public QObject  {
    
  Q_OBJECT
    
  private:
    void updateNotes(snd_seq_tick_time_t currentTick);  
    void getNote(snd_seq_tick_time_t *tick, int note[], int velocity[], int *length);
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
    
  private:
    void initLoop();  
    int clip(int value, int min, int max, bool *outOfRange);
    
  public:
    int chIn;       // Channel of input events
    int indexIn[2]; // Index input/output (for Controller events)
    int rangeIn[2]; // Parameter that is mapped, [0] low, [1] high boundary
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    int notes[2][2][MAXNOTES]; // Buffer Index, Note/Velocity, Data Index
    int noteBufPtr, noteCount, patternLen, patternMaxIndex, noteOfs;
    bool hold;
    int repeatPatternThroughChord;
    double tempo, len, vel;
    int octave, noteIndex[MAXCHORD], patternIndex;
    int randomVelocity, randomTick, randomLength;
    int randomTickAmp, randomVelocityAmp, randomLengthAmp;
    QString pattern;
          
  public:
    MidiArp();
    ~MidiArp();
    bool isArp(snd_seq_event_t *evIn);   // Check if evIn is in the input range of the map
    void addNote(snd_seq_event_t *evIn); // Add input Note for Arpeggio
    void removeNote(snd_seq_event_t *evIn); // Remove input Note from Arpeggio
    void getCurrentNote(snd_seq_tick_time_t currentTick, snd_seq_tick_time_t *tick, int note[], int velocity[], int *length, bool *isNew);
    void getNextNote(snd_seq_tick_time_t currentTick, snd_seq_tick_time_t *tick, int note[], int velocity[], int *length, bool *isNew);
    void initArpTick(snd_seq_tick_time_t currentTick);
    void newRandomValues();
    void newGrooveValues(int p_grooveTick, int p_grooveVelocity, int p_grooveLength);
    
  public slots:  
    void updatePattern(QString);
    void updateRandomTickAmp(int);
    void updateRandomVelocityAmp(int);
    void updateRandomLengthAmp(int);

};
                              
#endif
