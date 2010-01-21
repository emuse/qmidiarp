#ifndef SEQDRIVER_H
#define SEQDRIVER_H

#include <QWidget>
#include <QSocketNotifier>
#include <alsa/asoundlib.h>

#include "jacksync.h"
#include "midiarp.h"
#include "midilfo.h"
#include "midiseq.h"
#include "main.h"

class SeqDriver : public QWidget {

    Q_OBJECT

    private:
        int portCount;
        QList<MidiArp *> *midiArpList; 
        QList<MidiLfo *> *midiLfoList; 
        QList<MidiSeq *> *midiSeqList; 
        QSocketNotifier *seqNotifier;
        snd_seq_t *seq_handle;
        int clientid;
        int portid_out[MAX_PORTS];
        int lfoMinPacketSize, lfoPacketSize[20];
        int seqMinPacketSize, seqPacketSize[20];
        int portid_in;
        int queue_id;
        bool startQueue;
        bool modified;
        bool midi_controllable;
        snd_seq_tick_time_t tick, nextEchoTick, jack_offset_tick;
        int lastLfoTick[20], nextLfoTick;
        int lastSeqTick[20], nextSeqTick;
        int tempo, internal_tempo;
        QVector<LfoSample> lfoData;
        QVector<SeqSample> seqData;

        void initSeqNotifier();
        const snd_seq_real_time_t *tickToDelta(int tick);
        snd_seq_real_time_t addAlsaTimes(snd_seq_real_time_t time1
                        , snd_seq_real_time_t time2);
        int deltaToTick (snd_seq_real_time_t curtime);
        void calcMidiRatio();

        JackSync *jackSync;
        jack_position_t jpos;

        int midiTick;
        double m_ratio;
        snd_seq_real_time_t delta, real_time, jack_offset;
        
    public:
        bool forwardUnmatched, runQueueIfArp, runArp;
        int portUnmatched;
        int midiclock_tpb;
        int grooveTick, grooveVelocity, grooveLength;
        bool use_midiclock, use_jacksync;

    public:
        SeqDriver(QList<MidiArp*> *p_midiArpList, 
                QList<MidiLfo *> *p_midiLfoList,
                QList<MidiSeq *> *p_midiSeqList, QWidget* parent=0);
        ~SeqDriver();
        void registerPorts(int num);
        int getPortCount();
        void initArpQueue();
        snd_seq_real_time_t get_time();
        void setQueueStatus(bool run);
        bool isModified();
        void setModified(bool);
        int getAlsaClientId();

   signals:
        void midiEvent(snd_seq_event_t *ev);
        void controlEvent(int ccnumber, int channel, int value);
        void nextStep(snd_seq_tick_time_t tick);
        void jackShutdown(bool); //boolean is passed to main toolbar 
                                //jackSync button

   public slots:
        void procEvents(int fd);
        void setForwardUnmatched(bool on);
        void setPortUnmatched(int id);
        void setQueueTempo(int bpm);
        void runQueue(bool);
        void setGrooveTick(int);
        void setGrooveVelocity(int);
        void setGrooveLength(int);
        void sendGroove();
        void setUseMidiClock(bool on);
        void updateMIDItpb(int midiTpb);
        void setMidiControllable(bool on);
        void setUseJackTransport(bool on);
        void jackShutdown();
};

#endif
