/*
 *      midilfo.h
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
 */

#ifndef MIDILFO_H
#define MIDILFO_H

#include <QObject>
#include <QString>
#include <QVector>

#include <main.h>

#ifndef SAMPLE_H
#define SAMPLE_H

/*! @brief Structure holding elements of a MIDI note or controller representing
 * one point of a waveform
 */
    struct Sample {
        int value;
        int tick;
        bool muted;
    };
#endif

class MidiLfo : public QObject  {

  Q_OBJECT

  private:
    double queueTempo;
    int lastMouseLoc, lastMouseY;
    int frameptr;
    int clip(int value, int min, int max, bool *outOfRange);
    QVector<Sample> data;

  public:
    int portOut;    // Output port (ALSA Sequencer)
    int channelOut;
    bool hold, isMuted;
    int freq, amp, offs, ccnumber;
    int size, res, waveFormIndex;
    int cwmin;
    QVector<Sample> customWave;
    QVector<bool> muteMask;

  public:
    MidiLfo();
    ~MidiLfo();
    void getData(QVector<Sample> *data);
    void getNextFrame(QVector<Sample> *p_data);
    bool toggleMutePoint(double);

  public slots:
    void updateFrequency(int);
    void updateAmplitude(int);
    void updateOffset(int);
    void updateCustomWaveOffset(int);
    void updateQueueTempo(int);
    void setMuted(bool); //set mute
    void updateWaveForm(int val);
    void setCustomWavePoint(double, double, bool);
    void setMutePoint(double, bool);
    void resizeAll();
    void copyToCustom();
    void resetFramePtr();
};

#endif
