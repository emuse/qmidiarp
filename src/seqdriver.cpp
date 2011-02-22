#include <cstdio>
#include <QString>
#include <alsa/asoundlib.h>

#include "seqdriver.h"
#include "config.h"


SeqDriver::SeqDriver(QList<MidiArp *> *p_midiArpList,
        QList<MidiLfo *> *p_midiLfoList, QList<MidiSeq *> *p_midiSeqList,
        QWidget *parent)
    : QThread(parent), modified(false)
{
    int err;

    err = snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0);
    if (err < 0) {
        qWarning("Error opening ALSA sequencer (%s).", snd_strerror(err));
        exit(1);  }
        snd_seq_set_client_name(seq_handle, PACKAGE);
        clientid = snd_seq_client_id(seq_handle);
        portid_in = snd_seq_create_simple_port(seq_handle, "in",
                        SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
                        SND_SEQ_PORT_TYPE_APPLICATION);
        if (portid_in < 0) {
            qWarning("Error creating sequencer port (%s).",
                    snd_strerror(portid_in));
            exit(1);
        }
    snd_seq_set_client_pool_output(seq_handle, SEQPOOL);

    initArpQueue();

    midiArpList = p_midiArpList;
    midiLfoList = p_midiLfoList;
    midiSeqList = p_midiSeqList;

    portCount = 0;
    portUnmatched = 0;

    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;

    forwardUnmatched = false;
    runArp = false;
    startQueue = false;
    runQueueIfArp = true;
    use_jacksync = false;
    use_midiclock = false;
    midi_controllable = true;
    tempo = 100;
    internal_tempo = 100;
    schedDelayTicks = 2;
    m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
    int l1 = 0;
    for (l1 = 0; l1 < 20; l1++) lastLfoTick[l1] = 0;
    for (l1 = 0; l1 < 20; l1++) lastSeqTick[l1] = 0;
    for (l1 = 0; l1 < 20; l1++) nextNoteTick[l1] = 0;
    midiTick = 0;
    nextLfoTick = 0;
    nextSeqTick = 0;
    nextEchoTick = 0;
    lastKbdTick = 0;
    gotKbdTrig = false;
    threadAbort = false;
}

SeqDriver::~SeqDriver(){

    if (use_jacksync) setUseJackTransport(false);

    threadAbort = true;
    wait();

}

void SeqDriver::registerPorts(int num)
{
    int l1;
    char buf[16];

    portCount = num;
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
    start(Priority(6));
}

int SeqDriver::getPortCount()
{
    return(portCount);
}

void SeqDriver::run()
{
    int l1, l2;
    QVector<int> note, velocity;
    int noteTick = 0;
    int length, ccnumber;
    int lfoccnumber, lfochannel, lfoport;
    int seqlength, seqchannel, seqport, seqtransp, seqvel;
    snd_seq_event_t *evIn, evOut;
    bool unmatched, foundEcho, isNew;
    bool fallback = false;
    int pollR = 0;
    note.clear();
    velocity.clear();

    int nfds;
    struct pollfd *pfds;

    nfds = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
    pfds = (struct pollfd *) alloca(nfds * sizeof(struct pollfd));
    snd_seq_poll_descriptors(seq_handle, pfds, nfds, POLLIN);

    while ((poll >= 0) && (!threadAbort)) {

        pollR = poll(pfds, nfds, 200);
        while (pollR > 0) {

            snd_seq_event_input(seq_handle, &evIn);

            if (use_midiclock && (evIn->type == SND_SEQ_EVENT_CLOCK)) {
                midiTick++;
                tick = midiTick*TICKS_PER_QUARTER/MIDICLK_TPQ;
                if (((int)tick > nextLfoTick) && (midiLfoList->count())) {
                    fallback = true;
                }
                if (((int)tick > nextSeqTick) && (midiSeqList->count())) {
                    fallback = true;
                }
            }

            if (runArp && ((evIn->type == SND_SEQ_EVENT_ECHO) || startQueue || fallback)) {

                fallback = false;

                real_time = evIn->time.time;

                if (use_midiclock) {
                    tick = midiTick*TICKS_PER_QUARTER/MIDICLK_TPQ;
                    calcMidiRatio();
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
                        calcMidiRatio();
                    }
                }
                else {
                    m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
                    tick = deltaToTick(evIn->time.time);
                }


                    //~ printf("       tick %d     \n",tick);
                    //~ printf("nextLfoTick %d  ",nextLfoTick);
                    //~ printf("nextSeqTick %d  ",nextSeqTick);
                    //~ printf("nextEchoTick %d  \n",nextEchoTick);
                    //~ printf("midiTick %d   ",midiTick);
                    //~ printf("m_ratio %f  ",m_ratio);
                    //~ printf("Jack Beat %d\n", jpos.beat);
                    //~ printf("Jack Frame %d \n ", (int)jpos.frame);
                    //~ printf("Jack BBT offset %d\n", (int)jpos.bbt_offset);
                if (tick < 0) break;
                startQueue = false;
                foundEcho = false;

                //LFO data request and queueing
                //add 8 ticks to startoff condition to cope with initial sync imperfections
                if (((tick + 8) >= nextLfoTick) && (midiLfoList->count())) {
                    l2 = 0;
                    for (l1 = 0; l1 < midiLfoList->count(); l1++) {
                        if ((tick + 8) >= (lastLfoTick[l1] + lfoPacketSize[l1])) {
                            midiLfoList->at(l1)->getNextFrame(&lfoData);
                            lfoccnumber = midiLfoList->at(l1)->ccnumber;
                            lfochannel = midiLfoList->at(l1)->channelOut;
                            lfoport = midiLfoList->at(l1)->portOut;
                            if (!midiLfoList->at(l1)->isMuted) {
                                l2 = 0;
                                while (lfoData.at(l2).value > -1) {
                                    if (!lfoData.at(l2).muted) {
                                        snd_seq_ev_clear(&evOut);
                                        snd_seq_ev_set_controller(&evOut, 0,
                                                lfoccnumber,
                                                lfoData.at(l2).value);
                                        evOut.data.control.channel = lfochannel;
                                        snd_seq_ev_schedule_real(&evOut, queue_id, 0,
                                                tickToDelta(lfoData.at(l2).tick + nextLfoTick));
                                        snd_seq_ev_set_subs(&evOut);
                                        snd_seq_ev_set_source(&evOut,
                                                portid_out[lfoport]);
                                        snd_seq_event_output_direct(seq_handle, &evOut);
                                    }
                                    l2++;
                                }
                            }
                            lastLfoTick[l1] += lfoPacketSize[l1];
                            lfoPacketSize[l1] = lfoData.last().tick;
                            if (!l1)
                                lfoMinPacketSize = lfoPacketSize[l1];
                            else if (lfoPacketSize[l1] < lfoMinPacketSize)
                                lfoMinPacketSize = lfoPacketSize[l1];
                        }
                    }
                    nextLfoTick += lfoMinPacketSize;

                    // next echo request for LFO
                    snd_seq_ev_clear(evIn);
                    evIn->type = SND_SEQ_EVENT_ECHO;
                    snd_seq_ev_schedule_real(evIn, queue_id,  0,
                            tickToDelta(nextLfoTick));
                    snd_seq_ev_set_dest(evIn, clientid, portid_in);
                    snd_seq_event_output_direct(seq_handle, evIn);
                }

                //Seq notes data request and queueing
                //add 8 ticks to startoff condition to cope with initial sync imperfections
                if (((tick + 8) >= nextSeqTick) && (midiSeqList->count())) {
                    l2 = 0;
                    for (l1 = 0; l1 < midiSeqList->count(); l1++) {
                        if ((tick + 8) >= (lastSeqTick[l1] + seqPacketSize[l1])) {
                            midiSeqList->at(l1)->getData(&seqData);
                            seqlength = midiSeqList->at(l1)->notelength;
                            seqtransp = midiSeqList->at(l1)->transp;
                            seqvel = midiSeqList->at(l1)->vel;
                            seqchannel = midiSeqList->at(l1)->channelOut;
                            seqport = midiSeqList->at(l1)->portOut;
                            if (!midiSeqList->at(l1)->isMuted) {
                                l2 = 0;
                                while (seqData.at(l2).value > -1) {
                                    if (!seqData.at(l2).muted) {
                                        snd_seq_ev_clear(&evOut);
                                        snd_seq_ev_set_note(&evOut, 0,
                                                seqData.at(l2).value+seqtransp,
                                                seqvel, seqlength);
                                        evOut.data.control.channel = seqchannel;
                                        snd_seq_ev_schedule_real(&evOut, queue_id, 0,
                                                tickToDelta(seqData.at(l2).tick + nextSeqTick));
                                        snd_seq_ev_set_subs(&evOut);
                                        snd_seq_ev_set_source(&evOut,
                                                portid_out[seqport]);
                                        snd_seq_event_output_direct(seq_handle, &evOut);
                                    }
                                    l2++;
                                }
                            }
                            lastSeqTick[l1] += seqPacketSize[l1];
                            seqPacketSize[l1] = seqData.last().tick;
                            if (!l1)
                                seqMinPacketSize = seqPacketSize[l1];
                            else if (seqPacketSize[l1] < seqMinPacketSize)
                                seqMinPacketSize = seqPacketSize[l1];
                        }
                    }
                    nextSeqTick += seqMinPacketSize;

                    // next echo request for Seq
                    if (((nextSeqTick != nextLfoTick) || (!nextLfoTick))) {
                        snd_seq_ev_clear(evIn);
                        evIn->type = SND_SEQ_EVENT_ECHO;
                        snd_seq_ev_schedule_real(evIn, queue_id,  0,
                                tickToDelta(nextSeqTick));
                        snd_seq_ev_set_dest(evIn, clientid, portid_in);
                        snd_seq_event_output_direct(seq_handle, evIn);
                    }
                }

                //Arp Note queueing
                if ((tick + 8) >= nextEchoTick) {
                    for (l1 = 0; l1 < midiArpList->count(); l1++) {
                        if (((evIn->data.note.note == 0) && gotKbdTrig
                                && midiArpList->at(l1)->wantTrigByKbd())
                                || (!gotKbdTrig && (evIn->data.note.note == 0))) {
                            gotKbdTrig = false;
                            if (tick + schedDelayTicks >= nextNoteTick[l1]) {
                                midiArpList->at(l1)->newRandomValues();
                                midiArpList->at(l1)->updateQueueTempo(tempo);
                                midiArpList->at(l1)->prepareCurrentNote(tick);
                                note = midiArpList->at(l1)->returnNote;
                                nextNoteTick[l1] = midiArpList->at(l1)->getNextNoteTick();
                                velocity = midiArpList->at(l1)->returnVelocity;
                                noteTick = midiArpList->at(l1)->returnTick;
                                length = midiArpList->at(l1)->returnLength;
                                isNew = midiArpList->at(l1)->returnIsNew;
                                if (!velocity.isEmpty()) {
                                    if (isNew && velocity.at(0)) {
                                        l2 = 0;
                                        while(note.at(l2) >= 0) {
                                            snd_seq_ev_clear(&evOut);
                                            snd_seq_ev_set_note(&evOut, 0, note.at(l2),
                                                    velocity.at(l2), (int)length*4);
                                            evOut.data.control.channel =
                                                midiArpList->at(l1)->channelOut;
                                            snd_seq_ev_schedule_real(&evOut, queue_id, 0,
                                                    tickToDelta(noteTick));
                                            snd_seq_ev_set_subs(&evOut);
                                            snd_seq_ev_set_source(&evOut,
                                                    portid_out[midiArpList->at(l1)->portOut]);
                                            snd_seq_event_output_direct(seq_handle, &evOut);
                                            l2++;
                                        }
                                    }
                                }
                            }
                        }
                        if (!l1)
                            nextEchoTick = nextNoteTick[l1] - schedDelayTicks;
                        else if (nextNoteTick[l1] < nextEchoTick + schedDelayTicks)
                            nextEchoTick = nextNoteTick[l1] - schedDelayTicks;
                    }
                    //next echo request for Arp
                    if (0 > nextEchoTick) nextEchoTick = 0;
                    if (((nextEchoTick != nextLfoTick) || (!nextLfoTick)) &&
                     ((nextEchoTick != nextSeqTick) || (!nextSeqTick))) {
                        snd_seq_ev_clear(evIn);
                        evIn->type = SND_SEQ_EVENT_ECHO;
                        evIn->data.note.note = 0;
                        snd_seq_ev_schedule_real(evIn, queue_id,  0,
                                tickToDelta(nextEchoTick));
                        snd_seq_ev_set_dest(evIn, clientid, portid_in);
                        snd_seq_event_output_direct(seq_handle, evIn);
                    }
                }
            }
            else {
                emit midiEvent(evIn);
                unmatched = true;

                if (evIn->type == SND_SEQ_EVENT_CONTROLLER) {
                    ccnumber = (int)evIn->data.control.param;
                    if (ccnumber == 64) {
                        //Sustain Footswitch has changed
                        for (l1 = 0; l1 < midiArpList->count(); l1++) {
                            if (midiArpList->at(l1)->isArp(evIn)) {
                                midiArpList->at(l1)->setSustain(
                                        (evIn->data.control.value == 127), tick);
                                unmatched = false;
                            }
                        }
                    }
                    else {
                        if (midi_controllable) {
                            emit controlEvent(ccnumber, evIn->data.control.channel,
                                                evIn->data.control.value);
                            unmatched = false;
                        }
                    }
                }

                if ((evIn->type == SND_SEQ_EVENT_NOTEON)
                        || (evIn->type == SND_SEQ_EVENT_NOTEOFF)) {
                    get_time();
                    tick = deltaToTick(tmptime);
                    for (l1 = 0; l1 < midiSeqList->count(); l1++) {
                        if (midiSeqList->at(l1)->isSeq(evIn)) {
                            unmatched = false;
                            if ((evIn->type == SND_SEQ_EVENT_NOTEON)
                                    && (evIn->data.note.velocity > 0)) {
                                emit noteEvent(evIn->data.note.note,
                                        evIn->data.note.velocity);
                            }
                        }
                    }
                    for (l1 = 0; l1 < midiArpList->count(); l1++) {
                        if (midiArpList->at(l1)->isArp(evIn)) {
                            unmatched = false;
                            if ((evIn->type == SND_SEQ_EVENT_NOTEON)
                                    && (evIn->data.note.velocity > 0)) {

                                midiArpList->at(l1)->handleNoteOn(evIn->data.note.note,
                                                evIn->data.note.velocity, tick);

                                if (midiArpList->at(l1)->wantTrigByKbd()) {
                                    nextEchoTick = tick;
                                    nextNoteTick[l1] = nextEchoTick + schedDelayTicks;
                                    gotKbdTrig = true;
                                }
                            }
                            else {
                                midiArpList->at(l1)->handleNoteOff(evIn->data.note.note, tick, 1);
                            }
                        }
                    }
                    // place echo request if in keyboard trigger mode
                    if (gotKbdTrig
                     && ((nextEchoTick != nextLfoTick) || (!nextLfoTick))
                     && ((nextEchoTick != nextSeqTick) || (!nextSeqTick))
                     && (nextEchoTick != lastKbdTick)) {
                        lastKbdTick = nextEchoTick;
                        snd_seq_ev_clear(evIn);
                        evIn->type = SND_SEQ_EVENT_ECHO;
                        evIn->data.note.note = 0;
                        snd_seq_ev_schedule_real(evIn, queue_id,  0,
                                tickToDelta(nextEchoTick));
                        snd_seq_ev_set_dest(evIn, clientid, portid_in);
                        snd_seq_event_output_direct(seq_handle, evIn);
                    }
                }
                if (use_midiclock){
                    if (evIn->type == SND_SEQ_EVENT_START) {
                        setQueueStatus(true);
                    }
                    if (evIn->type == SND_SEQ_EVENT_STOP) {
                        setQueueStatus(false);
                    }
                }

                if (forwardUnmatched && unmatched) {
                    snd_seq_ev_set_subs(evIn);
                    snd_seq_ev_set_direct(evIn);
                    snd_seq_ev_set_source(evIn, portid_out[portUnmatched]);
                    snd_seq_event_output_direct(seq_handle, evIn);
                }
            }
            if (!runArp) tick = 0; //some events still come in after queue stop
            pollR = snd_seq_event_input_pending(seq_handle, 0);
        }
    }
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

void SeqDriver::initArpQueue()
{
    queue_id = snd_seq_alloc_queue(seq_handle);
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

void SeqDriver::runQueue(bool on)
{
    runQueueIfArp = on;
    setQueueStatus(on);
}

void SeqDriver::setQueueStatus(bool run)
{
    int l1;
    snd_seq_event_t evOut;

    if (run) {
        for (l1 = 0; l1 < midiArpList->count(); l1++) {
            midiArpList->at(l1)->foldReleaseTicks(tick);
        }
        for (l1 = 0; l1 < midiLfoList->count(); l1++) {
            midiLfoList->at(l1)->resetFramePtr();
        }
        runArp = true;
        startQueue = true;
        snd_seq_start_queue(seq_handle, queue_id, NULL);
        snd_seq_drain_output(seq_handle);
        snd_seq_ev_clear(&evOut);
        for (l1 = 0; l1 < 20; l1++) {
            nextNoteTick[l1] = 0;
            lastLfoTick[l1] = 0;
            lfoPacketSize[l1] = 0;
            lastSeqTick[l1] = 0;
            seqPacketSize[l1] = 0;
        }
        nextLfoTick = 0;
        nextSeqTick = 0;
        nextEchoTick = 0;
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
        evOut.type = SND_SEQ_EVENT_NOTEOFF;
        evOut.data.note.note = 0;
        evOut.data.note.velocity = 0;
        snd_seq_ev_schedule_real(&evOut, queue_id,  0, tickToDelta(0));
        snd_seq_ev_set_dest(&evOut, clientid, portid_in);
        snd_seq_event_output_direct(seq_handle, &evOut);

        for (l1 = 0; l1 < midiArpList->count(); l1++) {
            midiArpList->at(l1)->initArpTick(tick);
        }

        qWarning("Alsa Queue started");

    }
    else {
        runArp = false;
        //    snd_seq_drop_output(seq_handle);
        for (l1 = 0; l1 < midiArpList->count(); l1++) {
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

void SeqDriver::setGrooveTick(int val)
{
    int l1;

    grooveTick = val;
    for (l1 = 0; l1 < midiArpList->count(); l1++) {
        midiArpList->at(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
    modified = true;
}

void SeqDriver::setGrooveVelocity(int val)
{
    int l1;

    grooveVelocity = val;
    for (l1 = 0; l1 < midiArpList->count(); l1++) {
        midiArpList->at(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
    modified = true;
}

void SeqDriver::setGrooveLength(int val)
{
    int l1;

    grooveLength = val;
    for (l1 = 0; l1 < midiArpList->count(); l1++) {
        midiArpList->at(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
    modified = true;
}

void SeqDriver::sendGroove()
{
    int l1;

    for (l1 = 0; l1 < midiArpList->count(); l1++) {
        midiArpList->at(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
}

void SeqDriver::setUseJackTransport(bool on)
{
    if (on) {
        jackSync = new JackSync();
        jackSync->setParent(this);
        connect(jackSync, SIGNAL(j_tr_state(bool)),
                this, SLOT(runQueue(bool)));
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
    runQueue(false);
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

const snd_seq_real_time_t* SeqDriver::tickToDelta(int tick)
{
    double tmp =  m_ratio * tick;
    delta.tv_sec = (int)(tmp * 1e-9);
    delta.tv_nsec = (long)(tmp - (double)delta.tv_sec * 1e9);
    return &delta;
}

int SeqDriver::deltaToTick(snd_seq_real_time_t curtime)
{
    double alsatick;
    alsatick = (double)curtime.tv_sec * 1e9 / m_ratio
        + (double)curtime.tv_nsec / m_ratio;
    return (int)(alsatick + .5);
}

void SeqDriver::calcMidiRatio()
{
    double old_m_ratio = m_ratio;

    if (tick > 0) {
        m_ratio = ((double)real_time.tv_sec * 1e9
                + (double)real_time.tv_nsec)/tick;
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
