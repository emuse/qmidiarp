/*!
 * @file seqdriver.h
 * @brief Header for the SeqDriver class
 */
#ifndef SEQDRIVER_H
#define SEQDRIVER_H

#include <QWidget>
#include <QThread>
#include <alsa/asoundlib.h>

#include "jacksync.h"
#include "midiarp.h"
#include "midilfo.h"
#include "midiseq.h"
#include "main.h"
#include "driverbase.h"

/*! @brief ALSA sequencer backend QThread class. Also creates JackSync
 *
 * SeqDriver is created by ArpData at the moment of program start. Its
 * constructor registers ALSA seq input port and the requested number of
 * output ports. I also creates a JackSync instance whose ports are only
 * created when the SeqDriver::setUseJackTransport member is called.
 * Pointers to the MIDI workers MidiLfo, MidiSeq are passed to SeqDriver
 * as arguments.
 * The SeqDriver::run() thread is the ALSA sequencer "callback" process
 * handling all incoming and outgoing sequencer events.
 * When the SeqDriver::setQueueStatus() member is called with True argument,
 * a so called "echo event" is scheduled with zero time. Echo events go back
 * to the callback process and allow output and reception of sequencer
 * events depending on the ALSA queue timing. Depending on the event types,
 * the MIDI worker interfaces are called in series and return their
 * data to be output to the queue. After the data output, a new echo
 * event is requested for the next MIDI event to be output, which will
 * again call the SeqDriver::run() thread, and so on.
 * In order to provide accurate synchronization with external sources
 * such as Jack Transport or an incoming ALSA MIDI clock,
 * SeqDriver works with snd_seq_real_time timing information when it
 * communicates with the ALSA queue. Internally, the real time information
 * is rescaled to a simpler tick-based timing, which is currently 192 tpqn
 * using the SeqDriver::deltaToTick and SeqDriver::tickToDelta functions.
 */
class SeqDriver : public QThread, public DriverBase {

    Q_OBJECT

    private:
        int portCount;
        snd_seq_t *seq_handle;
        int clientid;
        int portid_out[MAX_PORTS];
        int portid_in;
        int queue_id;
        bool startQueue;
        bool modified;
        bool threadAbort;

        double tickToDelta(int tick);
        int deltaToTick (double curtime);
        double aTimeToDelta(snd_seq_real_time_t* atime);
        const snd_seq_real_time_t* deltaToATime(double curtime);
        void calcClockRatio();

        void initTempo();

        void sendMidiEvent(MidiEvent ev, unsigned int outport, unsigned int duration = 0);

        JackSync *jackSync;
        jack_position_t jPos;

        int midiTick;
        int lastSchedTick;
        int jackOffsetTick;
        int tempo, internalTempo;

        double clockRatio;         /* duration of one tick, in nanoseconds; based on current tempo */
        snd_seq_real_time_t delta, realTime;
        snd_seq_real_time_t tmpTime;

    public:
        bool forwardUnmatched, queueStatus;
        int portUnmatched;
        bool useMidiClock, useJackSync;
        void sendMidiEvent(MidiEvent outEv, int n_tick, int outport, int length = 0);
        bool requestEchoAt(int echoTick, int infotag = 1);

    public:
        SeqDriver(
            int p_portCount,
            QWidget* parent,
            void * callback_context,
            void (* midi_event_received_callback)(void * context, MidiEvent ev),
            void (* tick_callback)(void * context, MidiEvent ev));
        ~SeqDriver();
        void getTime();
        bool isModified();
        void setModified(bool);
        int getAlsaClientId();
        void run();

   signals:
        void jackShutdown(bool); //boolean is passed to main toolbar
                                //jackSync button

   public slots:
        void setForwardUnmatched(bool on);
        void setPortUnmatched(int id);
        void setQueueStatus(bool run);
        void setTempo(int bpm);
        void setUseMidiClock(bool on);
        void setUseJackTransport(bool on);
        void jackShutdown();
};

#endif
