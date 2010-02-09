/*
 *      midilfo.h
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

#ifndef MIDILFO_H
#define MIDILFO_H

#include <QObject>
#include <QString>
#include <QVector>
#include <alsa/asoundlib.h>
#include <main.h>

    struct LfoSample {
        int value;
        int tick;
        bool muted;
    };    

class MidiLfo : public QObject  {
    
  Q_OBJECT

  private:
    double queueTempo;
    int lastMouseLoc;
    int frameptr;
    int clip(int value, int min, int max, bool *outOfRange);
    QVector<LfoSample> lfoData;
     
  public:
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool hold, isMuted;
    int freq, amp, offs, ccnumber;
    int size, res, waveFormIndex;
    int cwmin;
    QVector<LfoSample> customWave;
    QVector<bool> muteMask;
           
  public:
    MidiLfo();
    ~MidiLfo();
    void getData(QVector<LfoSample> *lfoData);  
    void getNextFrame(QVector<LfoSample> *p_lfoData);
    
  public slots:  
    void updateFrequency(int);
    void updateAmplitude(int);
    void updateOffset(int);
    void updateCustomWaveOffset(int);
    void updateQueueTempo(int);
    void muteLfo(bool); //set mute
    void updateWaveForm(int val);
    void setCustomWavePoint(double, double, bool);
    void toggleMutePoint(double);
    void resizeAll();
    void copyToCustom();
    void resetFramePtr();
};
                              
#endif
