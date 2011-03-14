/*!
 * @file seqdriver.cpp
 * @brief Implementation of the SeqDriver class
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
#include <cstdio>
#include <QString>
#include <alsa/asoundlib.h>

#include "seqdriver.h"
#include "config.h"


SeqDriver::SeqDriver(QList<MidiArp *> *p_midiArpList,
        QList<MidiLfo *> *p_midiLfoList, QList<MidiSeq *> *p_midiSeqList,
        int p_portCount, QWidget *parent)
    : QThread(parent), modified(false)
{
    int err;
    char buf[16];
    int l1;

    /** Register ALSA client */
    err = snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) {
        qWarning("Error opening ALSA sequencer (%s).", snd_strerror(err));
        exit(1);
    }

    snd_seq_set_client_name(seq_handle, PACKAGE);
    clientid = snd_seq_client_id(seq_handle);

    /** Register ALSA input port */
    portid_in = snd_seq_create_simple_port(seq_handle, "in",
                    SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                    SND_SEQ_PORT_TYPE_APPLICATION);
    if (portid_in < 0) {
        qWarning("Error creating sequencer port (%s).",
                snd_strerror(portid_in));
        exit(1);
    }

    /** Setup ALSA sequencer queue */
    snd_seq_set_client_pool_output(seq_handle, SEQPOOL);
    queue_id = snd_seq_alloc_queue(seq_handle);

    /** Register ALSA output ports */
    portCount = p_portCount;
    for (l1 = 0; l1 < portCount; l1++) {
        snprintf(buf, sizeof(buf), "out %d", l1 + 1);
        portid_out[l1] = snd_seq_create_simple_port(seq_handle, buf,
                SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
                SND_SEQ_PORT_TYPE_APPLICATION);
        if (portid_out[l1] < 0) {
            qWarning("Error creating sequencer port (%s).",
                    snd_strerror(portid_out[l1]));
            exit(1);
        }
    }

    midiArpList = p_midiArpList;
    midiLfoList = p_midiLfoList;
    midiSeqList = p_midiSeqList;

    portUnmatched = 0;
    forwardUnmatched = false;

    runArp = false;
    startQueue = false;
    use_jacksync = false;
    use_midiclock = false;
    midi_controllable = true;

    internal_tempo = 100;
    schedDelayTicks = 2;

    resetTicks();

    gotKbdTrig = false;
    threadAbort = false;
    start(Priority(6));
}

SeqDriver::~SeqDriver(){

    if (use_jacksync) setUseJackTransport(false);

    threadAbort = true;
    wait();

}

void SeqDriver::run()
{
    snd_seq_event_t *evIn;
    bool unmatched;
    bool fallback = false;
    MidiEvent inEv;
    int pollR = 0;

    int nfds;
    struct pollfd *pfds;

    nfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfds = (struct pollfd *) alloca(nfds * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfds, nfds, POLLIN);

    while ((poll >= 0) && (!threadAbort)) {

        pollR = poll(pfds, nfds, 200);
        while (pollR > 0) {

            snd_seq_event_input(seq_handle, &evIn);

            inEv.type = evIn->type;
            inEv.data = evIn->data.note.note;

            if ((inEv.type == EV_CLOCK)&& use_midiclock) {
                midiTick++;
                tick = midiTick*TICKS_PER_QUARTER/MIDICLK_TPQ;
                if ((tick > nextLfoTick) && (midiLfoList->count())) {
                    fallback = true;
                }
                if ((tick > nextSeqTick) && (midiSeqList->count())) {
                    fallback = true;
                }
            }

            if (((inEv.type == EV_ECHO) || startQueue || fallback) && runArp) {
                fallback = false;
                real_time = evIn->time.time;
                handleEcho(inEv);
            }
            else {
                unmatched = true;
                inEv.channel = evIn->data.control.channel;

                if ((inEv.type == EV_NOTEON) || (inEv.type == EV_NOTEOFF)) {
                    real_time = evIn->time.time;
                    inEv.data = evIn->data.note.note;
                    if (inEv.type == EV_NOTEON) {
                        inEv.value = evIn->data.note.velocity;
                    }
                    else {
                        inEv.value = 0;
                        inEv.type = EV_NOTEON;
                    }
                }
                else inEv.value = evIn->data.control.value;

                if (inEv.type == EV_CONTROLLER) {
                    inEv.data = evIn->data.control.param;
                }

                unmatched = (handleEvent(inEv) && unmatched);

                if (forwardUnmatched && unmatched) {
                    snd_seq_ev_set_subs(evIn);
                    snd_seq_ev_set_direct(evIn);
                    snd_seq_ev_set_source(evIn, portid_out[portUnmatched]);
                    snd_seq_event_output_direct(seq_handle, evIn);
                }

                emit midiEvent(inEv.type, inEv.data, inEv.channel, inEv.value);
            }
            if (!runArp) tick = 0; //some events still come in after queue stop
            pollR = snd_seq_event_input_pending(seq_handle, 0);
        }
    }
}

void SeqDriver::handleEcho(MidiEvent inEv)
{
    int l1, l2;
    QVector<int> note, velocity;
    int note_tick = 0;
    int length;
    int outport;
    int seqtransp;
    bool isNew;
    MidiEvent outEv;
    int tmp_tick = 0;

    note.clear();
    velocity.clear();

    if (use_midiclock) {
        tick = midiTick*TICKS_PER_QUARTER/MIDICLK_TPQ;
        calcClockRatio();
    }
    else if (use_jacksync) {
        if (jackSync->isRunning()) {

            if (!jackSync->get_state())
                setQueueStatus(false);

            jpos = jackSync->get_pos();
            if (jpos.beats_per_minute > 0)
                tempo = jpos.beats_per_minute;

            tick = (double)jpos.frame * TICKS_PER_QUARTER
                    / jpos.frame_rate * tempo / 60.
                    - jack_offset_tick;
            calcClockRatio();
        }
    }
    else {
        m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
        tick = deltaToTick(aTimeToDelta(&real_time));
    }


        //~ printf("       tick %d     ",tick);
        //~ printf("nextLfoTick %d  ",nextLfoTick);
        //~ printf("nextSeqTick %d  ",nextSeqTick);
        //~ printf("nextArpTick %d  \n",nextArpTick);
        //~ printf("midiTick %d   ",midiTick);
        //~ printf("m_ratio %f  ",m_ratio);
        //~ printf("Jack Beat %d\n", jpos.beat);
        //~ printf("Jack Frame %d \n ", (int)jpos.frame);
        //~ printf("Jack BBT offset %d\n", (int)jpos.bbt_offset);
    if (tick < 0) return;
    startQueue = false;

    //LFO data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextLfoTick) && (midiLfoList->count())) {
        l2 = 0;
        for (l1 = 0; l1 < midiLfoList->count(); l1++) {
            if ((tick + 8) >= (lastLfoTick[l1] + lfoPacketSize[l1])) {
                outEv.type = EV_CONTROLLER;
                outEv.data = midiLfoList->at(l1)->ccnumber;
                outEv.channel = midiLfoList->at(l1)->channelOut;
                midiLfoList->at(l1)->getNextFrame(&lfoData);
                outport = midiLfoList->at(l1)->portOut;
                if (!midiLfoList->at(l1)->isMuted) {
                    l2 = 0;
                    while (lfoData.at(l2).value > -1) {
                        if (!lfoData.at(l2).muted) {
                            outEv.value = lfoData.at(l2).value;
                            tmp_tick = lfoData.at(l2).tick + nextLfoTick;
                            /** round-up to current resolution */
                            tmp_tick/= lfoData.last().tick;
                            tmp_tick*= lfoData.last().tick;
                            schedEvent(outEv, tmp_tick, outport);
                        }
                        l2++;
                    }
                }
                lfoPacketSize[l1] = lfoData.last().tick;
                lastLfoTick[l1] = tmp_tick;
                if (!l1)
                    lfoMinPacketSize = lfoPacketSize[l1];
                else if (lfoPacketSize[l1] < lfoMinPacketSize)
                    lfoMinPacketSize = lfoPacketSize[l1];
            }
        }

        nextLfoTick += lfoMinPacketSize;
        requestEchoAt(nextLfoTick);
    }

    //Seq notes data request and queueing
    //add 8 ticks to startoff condition to cope with initial sync imperfections
    if (((tick + 8) >= nextSeqTick) && (midiSeqList->count())) {
        l2 = 0;
        for (l1 = 0; l1 < midiSeqList->count(); l1++) {
            if ((tick + 8) >= (lastSeqTick[l1] + seqPacketSize[l1])) {
                outEv.type = EV_NOTEON;
                outEv.value = midiSeqList->at(l1)->vel;
                outEv.channel = midiSeqList->at(l1)->channelOut;
                midiSeqList->at(l1)->getNextNote(&seqSample);
                lastSeqTick[l1]+=seqPacketSize[l1];
                seqPacketSize[l1] = TICKS_PER_QUARTER / midiSeqList->at(l1)->res;
                /** round-up to current resolution */
                lastSeqTick[l1]/=seqPacketSize[l1];
                lastSeqTick[l1]*=seqPacketSize[l1];
                length = midiSeqList->at(l1)->notelength;
                seqtransp = midiSeqList->at(l1)->transp;
                outport = midiSeqList->at(l1)->portOut;
                if (!midiSeqList->at(l1)->isMuted) {
                    if (!seqSample.muted) {
                        outEv.data = seqSample.value + seqtransp;
                        schedEvent(outEv, lastSeqTick[l1], outport, length);
                    }
                }
                if (!l1)
                    seqMinPacketSize = seqPacketSize[l1];
                else if (seqPacketSize[l1] < seqMinPacketSize)
                    seqMinPacketSize = seqPacketSize[l1];
            }
        }

        nextSeqTick += seqMinPacketSize;
        requestEchoAt(nextSeqTick);
    }

    //Arp Note queueing
    if ((tick + 8) >= nextArpTick) {
        for (l1 = 0; l1 < midiArpList->count(); l1++) {
            if ((gotKbdTrig && (inEv.data == 2) && midiArpList->at(l1)->wantTrigByKbd())
                    || (!gotKbdTrig && (inEv.data == 0))) {
                gotKbdTrig = false;
                if (tick + schedDelayTicks >= nextNoteTick[l1]) {
                    outEv.type = EV_NOTEON;
                    outEv.channel = midiArpList->at(l1)->channelOut;
                    midiArpList->at(l1)->newRandomValues();
                    midiArpList->at(l1)->updateQueueTempo(tempo);
                    midiArpList->at(l1)->prepareCurrentNote(tick);
                    note = midiArpList->at(l1)->returnNote;
                    velocity = midiArpList->at(l1)->returnVelocity;
                    note_tick = midiArpList->at(l1)->returnTick;
                    length = midiArpList->at(l1)->returnLength * 4;
                    outport = midiArpList->at(l1)->portOut;
                    isNew = midiArpList->at(l1)->returnIsNew;
                    if (!velocity.isEmpty()) {
                        if (isNew && velocity.at(0)) {
                            l2 = 0;
                            while(note.at(l2) >= 0) {
                                outEv.data = note.at(l2);
                                outEv.value = velocity.at(l2);
                                schedEvent(outEv, note_tick, outport, length);
                                l2++;
                            }
                        }
                    }
                    nextNoteTick[l1] = midiArpList->at(l1)->getNextNoteTick();
                }
            }
            if (!l1)
                nextArpTick = nextNoteTick[l1] - schedDelayTicks;
            else if (nextNoteTick[l1] < nextArpTick + schedDelayTicks)
                nextArpTick = nextNoteTick[l1] - schedDelayTicks;
        }

        if (0 > nextArpTick) nextArpTick = 0;
        requestEchoAt(nextArpTick, 0);
    }
}

bool SeqDriver::handleEvent(MidiEvent inEv)
{
    bool unmatched;
    int l1;
    unmatched = true;

    if (inEv.type == EV_CONTROLLER) {

        if (inEv.data == CT_FOOTSW) {
            for (l1 = 0; l1 < midiArpList->count(); l1++) {
                if (midiArpList->at(l1)->wantEvent(inEv)) {
                    midiArpList->at(l1)->setSustain((inEv.value == 127), tick);
                    unmatched = false;
                }
            }
        }
        else {
            //Does any LFO want to record this?
            for (l1 = 0; l1 < midiLfoList->count(); l1++) {
                if (midiLfoList->at(l1)->wantEvent(inEv)) {
                    midiLfoList->at(l1)->record(inEv.value);
                    unmatched = false;
                }
            }
            if (midi_controllable) {
                emit controlEvent(inEv.data, inEv.channel, inEv.value);
                unmatched = false;
            }
        }
        return unmatched;
    }

    if (inEv.type == EV_NOTEON) {

        for (l1 = 0; l1 < midiSeqList->count(); l1++) {
            if (midiSeqList->at(l1)->wantEvent(inEv)) {
                unmatched = false;
                if (inEv.value > 0) emit noteEvent(inEv.data, inEv.value);
            }
        }
        for (l1 = 0; l1 < midiArpList->count(); l1++) {
            if (midiArpList->at(l1)->wantEvent(inEv)) {
                unmatched = false;
                if (inEv.value > 0) {

                    get_time();
                    tick = deltaToTick(aTimeToDelta(&tmptime));
                    midiArpList->at(l1)->handleNoteOn(inEv.data, inEv.value, tick);

                    if (midiArpList->at(l1)->wantTrigByKbd()) {
                        nextArpTick = tick;
                        nextNoteTick[l1] = nextArpTick + schedDelayTicks;
                        gotKbdTrig = true;
                        requestEchoAt(nextArpTick, 2);
                    }
                }
                else {
                    midiArpList->at(l1)->handleNoteOff(inEv.data, tick, 1);
                }
            }
        }
        return unmatched;
    }

    if (use_midiclock){
        if (inEv.type == EV_START) {
            setQueueStatus(true);
        }
        if (inEv.type == EV_STOP) {
            setQueueStatus(false);
        }
        return unmatched;
    }
    return unmatched;
}

void SeqDriver::resetTicks()
{
    int l1;

    for (l1 = 0; l1 < midiArpList->count(); l1++) {
        midiArpList->at(l1)->foldReleaseTicks(tick);
        midiArpList->at(l1)->initArpTick(0);
    }
    for (l1 = 0; l1 < midiLfoList->count(); l1++) {
        midiLfoList->at(l1)->resetFramePtr();
    }
    for (l1 = 0; l1 < midiSeqList->count(); l1++) {
        midiSeqList->at(l1)->setCurrentIndex(0);
    }
    for (l1 = 0; l1 < 20; l1++) {
        nextNoteTick[l1] = 0;
        lastLfoTick[l1] = 0;
        lfoPacketSize[l1] = 0;
        lastSeqTick[l1] = 0;
        seqPacketSize[l1] = 0;
    }
    nextLfoTick = 0;
    nextSeqTick = 0;
    nextArpTick = 0;
    lastSchedTick = 0;
    jack_offset_tick = 0;

    if (use_midiclock) {
        midiTick = 0;
    }
    else if (use_jacksync) {
        if (jackSync->isRunning()) {
            jpos = jackSync->get_pos();
            // qtractor for instance doesn't set tempo atm...
            if (jpos.beats_per_minute > 0)
                tempo = jpos.beats_per_minute;
            else
                tempo = internal_tempo;
            jack_offset_tick = (double)jpos.frame * TICKS_PER_QUARTER
                    / jpos.frame_rate * tempo / 60;
            m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
        }
    }
    else {
        tempo = internal_tempo;
        m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
    }

    tick = 0;
}

void SeqDriver::schedEvent(MidiEvent outEv, int n_tick, int outport, int length)
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);

    ev.type = outEv.type;
    if (ev.type == EV_NOTEON) {
        snd_seq_ev_set_note(&ev, 0, outEv.data, outEv.value, length);
    }
    else if (ev.type == EV_CONTROLLER) {
        ev.data.control.param = outEv.data;
        ev.data.control.value = outEv.value;
    }

    ev.data.control.channel = outEv.channel;
    snd_seq_ev_schedule_real(&ev, queue_id, 0, deltaToATime(tickToDelta(n_tick)));
    snd_seq_ev_set_subs(&ev);
    snd_seq_ev_set_source(&ev, portid_out[outport]);
    snd_seq_event_output_direct(seq_handle, &ev);
}

bool SeqDriver::requestEchoAt(int echoTick, int infotag)
{
    if ((echoTick == lastSchedTick) && (echoTick)) return false;

    lastSchedTick = echoTick;

    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    ev.type = SND_SEQ_EVENT_ECHO;
    ev.data.note.note = infotag;
    snd_seq_ev_schedule_real(&ev, queue_id,  0, deltaToATime(tickToDelta(echoTick)));
    snd_seq_ev_set_dest(&ev, clientid, portid_in);
    snd_seq_event_output_direct(seq_handle, &ev);
    return true;
}

void SeqDriver::setForwardUnmatched(bool on)
{
    forwardUnmatched = on;
    modified = true;
}

void SeqDriver::setPortUnmatched(int id)
{
    portUnmatched = id;
    modified = true;
}

void SeqDriver::setQueueTempo(int bpm)
{
    tempo = bpm;
    internal_tempo = bpm;
    m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
}

void SeqDriver::get_time()
{
    snd_seq_queue_status_t *status;

    snd_seq_queue_status_malloc(&status);
    snd_seq_get_queue_status(seq_handle, queue_id, status);

    const snd_seq_real_time_t* current_time =
        snd_seq_queue_status_get_real_time(status);
    tmptime = *current_time;
    snd_seq_queue_status_free(status);
}

void SeqDriver::setQueueStatus(bool run)
{
    if (run) {

        runArp = true;
        startQueue = true;

        resetTicks();

        snd_seq_start_queue(seq_handle, queue_id, NULL);
        snd_seq_drain_output(seq_handle);

        requestEchoAt(0);

        qWarning("Alsa Queue started");
    }
    else {
        runArp = false;
        //    snd_seq_drop_output(seq_handle);
        for (int l1 = 0; l1 < midiArpList->count(); l1++) {
            midiArpList->at(l1)->clearNoteBuffer();
        }
        snd_seq_remove_events_t *remove_ev;

        snd_seq_remove_events_malloc(&remove_ev);
        snd_seq_remove_events_set_queue(remove_ev, queue_id);
        snd_seq_remove_events_set_condition(remove_ev,
                SND_SEQ_REMOVE_OUTPUT | SND_SEQ_REMOVE_IGNORE_OFF);
        snd_seq_remove_events(seq_handle, remove_ev);
        snd_seq_remove_events_free(remove_ev);

        snd_seq_stop_queue(seq_handle, queue_id, NULL);

        tick = 0;

        qWarning("Alsa Queue stopped");
    }
}

void SeqDriver::setUseJackTransport(bool on)
{
    if (on) {
        jackSync = new JackSync();
        jackSync->setParent(this);
        connect(jackSync, SIGNAL(j_tr_state(bool)),
                this, SLOT(setQueueStatus(bool)));
        connect(jackSync, SIGNAL(j_shutdown()),
                this, SLOT(jackShutdown()));
        use_jacksync = true;

        if (jackSync->initJack()) {
            emit jackShutdown(false);
            return;
        }
        else if (jackSync->activateJack()) {
            emit jackShutdown(false);
            return;
        }
    }
    else {
        if (use_jacksync) {
            jackSync->deactivateJack();
            delete jackSync;
            use_jacksync = false;
        }
    }
}

void SeqDriver::jackShutdown()
{
    setQueueStatus(false);
    emit jackShutdown(false);
}

void SeqDriver::setUseMidiClock(bool on)
{
    m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
    setQueueStatus(false);
    use_midiclock = on;
}

void SeqDriver::setModified(bool m)
{
    modified = m;
}

bool SeqDriver::isModified()
{
    return modified;
}

double SeqDriver::tickToDelta(int tick)
{
    return (double)m_ratio * tick;
}

int SeqDriver::deltaToTick(double curtime)
{
    return (int)(curtime / m_ratio + .5);
}

double SeqDriver::aTimeToDelta(snd_seq_real_time_t* atime)
{
    return (double)atime->tv_sec * 1e9 + (double)atime->tv_nsec;
}

const snd_seq_real_time_t* SeqDriver::deltaToATime(double curtime)
{
    delta.tv_sec = (int)(curtime * 1e-9);
    delta.tv_nsec = (long)(curtime - (double)delta.tv_sec * 1e9);
    return &delta;
}

void SeqDriver::calcClockRatio()
{
    double old_m_ratio = m_ratio;

    if (tick > 0) {
        m_ratio = aTimeToDelta(&real_time)/tick;
    }
    if ((m_ratio == 0) || (m_ratio > 60e9 / tempo)) {
        m_ratio = old_m_ratio;
    }
}

int SeqDriver::getAlsaClientId()
{
    return clientid;
}

void SeqDriver::setMidiControllable(bool on)
{
    midi_controllable = on;
    modified = true;
}
