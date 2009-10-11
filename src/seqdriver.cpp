#include <cstdio>
#include <QString>
#include <QSocketNotifier>
#include <alsa/asoundlib.h>


#include "seqdriver.h"
#include "config.h"


SeqDriver::SeqDriver(QList<MidiArp *> *p_midiArpList, 
        QList<MidiLfo *> *p_midiLfoList, QWidget *parent)
    : QWidget(parent), modified(false)
{
    int err;

    midiArpList = p_midiArpList;
    midiLfoList = p_midiLfoList; 
    portCount = 0;
    forwardUnmatched = false;
    portUnmatched = 0;
    mute_cnumber = 37;
    midi_mutable = false;

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
            qWarning("Error creating sequencer port(%s).",
                    snd_strerror(portid_in));
            exit(1);
        }
        snd_seq_set_client_pool_output(seq_handle, SEQPOOL);                                     
        tempo = 100;
        grooveTick = 0;
        firstArpTick = 0;
        grooveVelocity = 0;
        grooveLength = 0;
        runArp = false;
        startQueue = false;
        runQueueIfArp = true;
        initArpQueue();
        setQueueTempo(100);
        midiTick = 0;
        use_midiclock = false;
        midiclock_tpb = 96;
        m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
        int l1 = 0;
        for (l1 = 0; l1 < 20; l1++) lastLfoTick[l1] = 0;
        nextLfoTick = 0;
        nextEchoTick = 0;
}

SeqDriver::~SeqDriver(){
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
    initSeqNotifier();
}

void SeqDriver::initSeqNotifier()
{
    int alsaEventFd = 0;

    struct pollfd pfd[1];
    snd_seq_poll_descriptors(seq_handle, pfd, 1, POLLIN);
    alsaEventFd = pfd[0].fd;
    seqNotifier = new QSocketNotifier(alsaEventFd, QSocketNotifier::Read);
    QObject::connect(seqNotifier, SIGNAL(activated(int)),
            this, SLOT(procEvents(int)));
}


int SeqDriver::getPortCount()
{
    return(portCount);
}

void SeqDriver::procEvents(int)
{
    int l1, l2;
    int note[MAXCHORD], velocity[MAXCHORD];
    int length, ccnumber;
    int lfoccnumber, lfochannel, lfoport;
    snd_seq_event_t *evIn, evOut;
    snd_seq_tick_time_t noteTick;
    bool unmatched, foundEcho, isNew;

    do {
        snd_seq_event_input(seq_handle, &evIn);

        if (use_midiclock && (evIn->type == SND_SEQ_EVENT_CLOCK)) {
            midiTick += 4;
        }

        if (runArp && ((evIn->type == SND_SEQ_EVENT_ECHO) || startQueue)) 
        {
            real_time = evIn->time.time;
            if (use_midiclock) {
                tick = midiTick*TICKS_PER_QUARTER/midiclock_tpb;
                calcMidiRatio();
            }
            else {
                m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
                tick = deltaToTick(evIn->time.time);
            }

            //                printf("       tick %d     ",tick);
            //                printf("nextLfoTick %d\n",nextLfoTick);
            //                printf("midiTick %d   ",midiTick);
            //                printf("m_ratio %f\n",m_ratio);
            emit nextStep((tick-firstArpTick));
            startQueue = false;
            foundEcho = false;

            //LFO data request and queueing
            if (((int)tick >= nextLfoTick) && (midiLfoList->count())) {
                l2 = 0;
                for (l1 = 0; l1 < midiLfoList->count(); l1++) {
                    if ((int)tick >= (lastLfoTick[l1] + lfoPacketSize[l1])) {
                        midiLfoList->at(l1)->getData(&lfoData);
                        lfoccnumber = midiLfoList->at(l1)->lfoCCnumber;
                        lfochannel = midiLfoList->at(l1)->channelOut;
                        lfoport = midiLfoList->at(l1)->portOut;
                        if (!midiLfoList->at(l1)->isMuted) {
                            l2 = 0;
                            while (lfoData.at(l2).lfoValue > -1) {
                                snd_seq_ev_clear(&evOut);
                                snd_seq_ev_set_controller(&evOut, 0, 
                                        lfoccnumber,
                                        lfoData.at(l2).lfoValue);
                                evOut.data.control.channel = lfochannel;
                                snd_seq_ev_schedule_real(&evOut, queue_id, 0,
                                        tickToDelta(lfoData.at(l2).lfoTick + nextLfoTick));
                                snd_seq_ev_set_subs(&evOut);  
                                snd_seq_ev_set_source(&evOut,
                                        portid_out[lfoport]);
                                snd_seq_event_output_direct(seq_handle, &evOut);
                                l2++;
                            }
                        }
                        lastLfoTick[l1] += lfoPacketSize[l1];
                        lfoPacketSize[l1] = lfoData.last().lfoTick;
                        if (!l1) lfoMinPacketSize = lfoPacketSize[l1]; 
                        else if (lfoPacketSize[l1] < lfoMinPacketSize) 
                            lfoMinPacketSize = lfoPacketSize[l1];
                    }
                }
                nextLfoTick += lfoMinPacketSize;
                nextEchoTick = nextLfoTick;

                // next echo request for LFO
                snd_seq_ev_clear(evIn);
                evIn->type = SND_SEQ_EVENT_ECHO;
                snd_seq_ev_schedule_real(evIn, queue_id,  0,
                        tickToDelta(nextLfoTick));
                snd_seq_ev_set_dest(evIn, clientid, portid_in);
                snd_seq_event_output_direct(seq_handle, evIn);
            }
            //Note queueing
            for (l1 = 0; l1 < midiArpList->count(); l1++) 
            {
                midiArpList->at(l1)->newRandomValues();
                midiArpList->at(l1)->updateQueueTempo(tempo);
                midiArpList->at(l1)->getCurrentNote(tick, &noteTick,
                        note, velocity, &length, &isNew);

                if (isNew && velocity[0]) 
                {
                    l2 = 0;
                    while(note[l2] >= 0) 
                    {
                        snd_seq_ev_clear(&evOut);
                        snd_seq_ev_set_note(&evOut, 0, note[l2],
                                velocity[l2], (int)length*4);
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

                //set Echo tick for next request
                midiArpList->at(l1)->getNextNote(&noteTick, note,
                        velocity, &length, &isNew);
                if (isNew) 
                {
                    if (!foundEcho) 
                    {
                        foundEcho = true;
                        nextEchoTick = noteTick;
                    } else 
                    {
                        if (noteTick < nextEchoTick) 
                        {
                            nextEchoTick = noteTick;
                        }
                    }
                }
            }
            if (((int)nextEchoTick != nextLfoTick) || (!nextLfoTick)){
                snd_seq_ev_clear(evIn);
                evIn->type = SND_SEQ_EVENT_ECHO;
                snd_seq_ev_schedule_real(evIn, queue_id,  0, 
                        tickToDelta(nextEchoTick));
                snd_seq_ev_set_dest(evIn, clientid, portid_in);
                snd_seq_event_output_direct(seq_handle, evIn);
            }

        } else 
        {
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
                if ((evIn->data.control.value == 127) && (ccnumber > (mute_cnumber - 1)) 
                        && midi_mutable	&& (ccnumber < (mute_cnumber + midiArpList->count())))
                {
                    //Mute Toggle Controller received
                    midiArpList->at(ccnumber - mute_cnumber)->muteArp();
                    unmatched = false;
                }
            }

            if ((evIn->type == SND_SEQ_EVENT_NOTEON)
                    || (evIn->type == SND_SEQ_EVENT_NOTEOFF)) {
                for (l1 = 0; l1 < midiArpList->count(); l1++) {
                    if (midiArpList->at(l1)->isArp(evIn)) {
                        unmatched = false;
                        if ((evIn->type == SND_SEQ_EVENT_NOTEON)
                                && (evIn->data.note.velocity > 0)) {
                            midiArpList->at(l1)->addNote(evIn, tick);
                        } else {
                            midiArpList->at(l1)->removeNote(evIn, tick, 1);
                        }
                    }
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

    } while (snd_seq_event_input_pending(seq_handle, 0) > 0);  

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
    m_ratio = 60e9/TICKS_PER_QUARTER/tempo;
}

snd_seq_real_time_t SeqDriver::get_time()
{
    snd_seq_queue_status_t *status;

    snd_seq_queue_status_malloc(&status);
    snd_seq_get_queue_status(seq_handle, queue_id, status);

    const snd_seq_real_time_t* current_time = 
        snd_seq_queue_status_get_real_time(status);
    snd_seq_queue_status_free(status);
    return(*current_time);
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
        runArp = true;
        startQueue = true;
        for (l1 = 0; l1 < 20; l1++) {
            lastLfoTick[l1] = 0;
            lfoPacketSize[l1] = 0;
        }
        nextLfoTick = 0;
        nextEchoTick = 0;
        snd_seq_start_queue(seq_handle, queue_id, NULL);
        snd_seq_drain_output(seq_handle);
        real_time = get_time();
        if (use_midiclock) {
            midiTick = 0;
            tick = 0;
        } else
            tick = deltaToTick(real_time);
        snd_seq_ev_clear(&evOut);
        evOut.type = SND_SEQ_EVENT_NOTEOFF;
        evOut.data.note.note = 0;
        evOut.data.note.velocity = 0;
        snd_seq_ev_schedule_real(&evOut, queue_id,  0, &real_time);
        snd_seq_ev_set_dest(&evOut, clientid, portid_in);
        snd_seq_event_output_direct(seq_handle, &evOut);

        for (l1 = 0; l1 < midiArpList->count(); l1++) {
            midiArpList->at(l1)->initArpTick(tick);
            firstArpTick=(int)tick;
        }

    } else {
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

void SeqDriver::setUseMidiClock(bool on)
{
    runQueue(false);
    use_midiclock = on;
}

void SeqDriver::updateMIDItpb(int midiTpb)
{
    midiclock_tpb = midiTpb;
}

void SeqDriver::updateCnumber(int cnumber)
{
    mute_cnumber = cnumber;
    modified = true;
}

void SeqDriver::setMidiMutable(bool on)
{
    midi_mutable = on;
    modified = true;
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
    return (int)(alsatick +.5);
}

void SeqDriver::calcMidiRatio()
{
    double old_m_ratio = m_ratio;
    if (tick) {
        m_ratio = ((double)real_time.tv_sec * 1e9 
                + (double)real_time.tv_nsec)/tick;
        m_ratio += old_m_ratio;
        m_ratio /= 2;
    }
    if ((m_ratio == 0) || (m_ratio > 60e9 / tempo)) 	
        m_ratio = old_m_ratio;
}

int SeqDriver::getAlsaClientId()
{
        return clientid;
}
