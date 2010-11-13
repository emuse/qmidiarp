/*
 *      midiseq.h
 *      
 *      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
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

#ifndef MIDISEQ_H
#define MIDISEQ_H

#include <QObject>
#include <QString>
#include <QVector>
#include <alsa/asoundlib.h>
#include <main.h>

    struct SeqSample {
        int value;
        int tick;
        bool muted;
    };    

class MidiSeq : public QObject  {
    
  Q_OBJECT

  private:
    double queueTempo;
    int lastMouseLoc;
    int clip(int value, int min, int max, bool *outOfRange);
     
  public:
    int chIn;       // Channel of input events
    bool enableNoteIn; // Index input/output (for Controller events)
    bool enableVelIn; // Parameter that is mapped, [0] low, [1] high boundary
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool hold, isMuted;
    int vel, transp, notelength;
    int size, res, waveFormIndex;
    int currentRecStep;
    QVector<SeqSample> customWave;
    QVector<bool> muteMask;
               
  public:
    MidiSeq();
    ~MidiSeq();
    bool isSeq(snd_seq_event_t *evIn);
    void getData(QVector<SeqSample> *seqData);  
    bool toggleMutePoint(double);
    
  public slots:  
    void updateVelocity(int);
    void updateTranspose(int);
    void updateQueueTempo(int);
    void recordNote(int note);
    void setMuted(bool); //set mute
    void updateWaveForm(int val);
    void setCustomWavePoint(double, double);
    void setMutePoint(double, bool);
    void resizeAll();
    void copyToCustom();
    void setRecordedNote(int note);
};
                              
#endif
