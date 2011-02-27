/*!
 * @file midiseq.cpp
 * @brief MIDI worker class for the Seq Module. Implements a monophonic
 * step sequencer as a QObject.
 *
 * The parameters of MidiSeq are controlled by the SeqWidget class.
 * A pointer to MidiSeq is passed to the SeqDriver thread, which calls
 * the MidiSeq::getNextNote member as a function of the position of
 * the ALSA queue. MidiSeq will return a note from its internal
 * MidiSeq::data buffer. The MidiSeq::data buffer is populated by the
 * MidiSeq::getData function at each modification done via
 * the SeqWidget. It is modified by drawing a sequence of notes on the
 * SeqWidget display or by recording incoming notes step by step. In all
 * cases the sequence has resolution, velocity, note length and
 * size attributes and single points can be tagged as muted, which will
 * avoid data output at the corresponding position.
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
#include <cmath>
#include "midiseq.h"


MidiSeq::MidiSeq()
{
    enableNoteIn = true;
    enableVelIn = true;
    currentRecStep = 0;
    chIn = 0;
    queueTempo = 100.0;
    vel = 0;
    transp = 0;
    size = 4;
    res = 4;
    notelength = 74;
    portOut = 0;
    channelOut = 0;
    waveFormIndex = 0;
    isMuted = false;
    int lt = 0;
    int l1 = 0;
    int step = TICKS_PER_QUARTER / res;
    Sample sample;
    sample.value = 60;
    customWave.clear();
    for (l1 = 0; l1 < size * res; l1++) {
            sample.tick = lt;
            sample.muted = false;
            customWave.append(sample);
            lt+=step;
    }
    muteMask.fill(false, size * res);
}

MidiSeq::~MidiSeq(){
}

void MidiSeq::setMuted(bool on)
{
    isMuted = on;
}

bool MidiSeq::wantEvent(MidiEvent inEv) {

    if ((inEv.type != EV_NOTEON) && (inEv.type != EV_CONTROLLER)) return(false);

    if (inEv.channel != chIn) return(false);

    if ((inEv.type == EV_NOTEON) && (inEv.value > 0)) {
        if (!(enableNoteIn)) return(false);
        if ((inEv.data < 36) || (inEv.data >= 84)) return(false);
    }
    return(true);
}

void MidiSeq::getData(QVector<Sample> *p_data)
{
    Sample sample;
    int lt = 0;
    int step = TICKS_PER_QUARTER / res;

    //res: number of events per beat
    //size: size of waveform in beats
    QVector<Sample> data;
    data.clear();

    switch(waveFormIndex) {
        case 0: //custom
            lt = step * customWave.count();
            data = customWave;
        break;
        default:
        break;
    }
    sample.value = -1;
    sample.tick = lt;
    data.append(sample);
    *p_data = data;
}

int MidiSeq::clip(int value, int min, int max, bool *outOfRange)
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

void MidiSeq::updateWaveForm(int val)
{
    waveFormIndex = val;
}

void MidiSeq::updateVelocity(int val)
{
    vel = val;
}

void MidiSeq::updateTranspose(int val)
{
    transp = val;
}

void MidiSeq::recordNote(int val)
{
        setRecordedNote(val);
        currentRecStep++;
        currentRecStep %= (res * size);
}

void MidiSeq::updateQueueTempo(int val)
{
    queueTempo = (double)val;
}

void MidiSeq::setCustomWavePoint(double mouseX, double mouseY)
{
    currentRecStep = mouseX * res * size;
    setRecordedNote(mouseY * 48 + 36);
}

void MidiSeq::setRecordedNote(int note)
{
    Sample sample;

    sample = customWave.at(currentRecStep);
    sample.value = note;
    sample.tick = currentRecStep * TICKS_PER_QUARTER / res;
    customWave.replace(currentRecStep, sample);
}

void MidiSeq::resizeAll()
{
    int lt = 0;
    int l1 = 0;
    int os;
    int step = TICKS_PER_QUARTER / res;
    Sample sample;

    os = customWave.count();
    customWave.resize(size * res);
    muteMask.resize(size * res);
    for (l1 = 0; l1 < customWave.count(); l1++) {
        sample = customWave.at(l1 % os);
        sample.tick = lt;
        sample.muted = muteMask.at(l1);
        customWave.replace(l1, sample);
        lt+=step;
    }
    currentRecStep %= (res * size);
}

void MidiSeq::copyToCustom()
{
    QVector<Sample> data;
    int m;

    data.clear();
    getData(&data);
    m = data.count();
    data.remove(m - 1);
    customWave = data;
}

bool MidiSeq::toggleMutePoint(double mouseX)
{
    Sample sample;
    bool m;
    int loc = mouseX * (res * size);

    m = muteMask.at(loc);
    muteMask.replace(loc, !m);
    sample = customWave.at(loc);
    sample.muted = !m;
    customWave.replace(loc, sample);
    return(!m);
}

void MidiSeq::setMutePoint(double mouseX, bool on)
{
    Sample sample;
    int loc = mouseX * (res * size);

    sample = customWave.at(loc);
    sample.muted = on;
    customWave.replace(loc, sample);
    muteMask.replace(loc, on);
}
