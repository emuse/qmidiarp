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
#include <QList>
#include <alsa/asoundlib.h>
#include <main.h>
#include <QString>

	struct LfoSample {
		int lfoValue;
		int lfoTick;
	};    

class MidiLfo : public QObject  {
    
  Q_OBJECT

  private:
	double queueTempo;
	
  public:

  private:
    int clip(int value, int min, int max, bool *outOfRange);
    
  public:
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool hold, isMuted;
    int lfoFreq, lfoAmp, lfoOffs, lfoCCnumber;
	int lfoSize, lfoRes, waveFormIndex;
	
  signals:
    void toggleMute();
           
  public:
	MidiLfo();
    ~MidiLfo();
    void getData(QList<LfoSample> *lfoData);  
	
  public slots:  
    void updateFrequency(int);
    void updateAmplitude(int);
    void updateOffset(int);
    void updateQueueTempo(int);
    void muteLfo(bool); //set mute
    void muteLfo(); //toggle mute
    void updateWaveForm(int val);

};
                              
#endif
