/*!
 * @file seqdriver.h
 * @brief Header for the SeqDriver class
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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
 
#ifndef SEQDRIVER_H
#define SEQDRIVER_H

#include "config.h"

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>

#include "jackdriver.h"
#include "driverbase.h"

/*! @brief ALSA sequencer backend QThread class.
 *
 * SeqDriver is created by Engine at the moment of program start. Its
 * constructor registers ALSA seq input port and the requested number of
 * output ports. It is called with a portless JackDriver instance for
 * connection to JACK Transport.
 * The SeqDriver::run() thread is the ALSA sequencer "callback" process
 * handling all incoming and outgoing sequencer events.
 * When the SeqDriver::setTransportStatus() function is called with True
 * argument, a so called "echo event" is scheduled to ALSA with zero time.
 * ALSA will send the echo events back
 * to the process and call Engine::echoCallback() regularly to query for
 * events to be scheduled. Incoming MIDI events from ALSA are transferred
 * to the Engine::eventCallback(). Engine will use the sendMidiEvent()
 * function to schedule events. After the event output, a new echo
 * event is requested for the next MIDI event to be output, which will
 * again call the SeqDriver::run() thread, and so on.
 * In order to provide accurate synchronization with external sources
 * such as Jack Transport or an incoming ALSA MIDI clock,
 * SeqDriver works with snd_seq_real_time timing information when it
 * communicates with the ALSA queue. Internally, the real time information
 * is rescaled to a simpler tick-based timing, which is currently 192 tpqn
 * using the deltaToTick() and tickToDelta() functions.
 */
class SeqDriver : public DriverBase {

    Q_OBJECT

    private:
        snd_seq_t *seq_handle;
        int clientid;
        int portid_out[MAX_PORTS];
        int portid_in;
        int queue_id;
        bool startQueue;
        bool threadAbort;

        double tickToDelta(uint64_t tick);
        uint64_t deltaToTick (double curtime);
        double aTimeToDelta(snd_seq_real_time_t* atime);
        const snd_seq_real_time_t* deltaToATime(double curtime);
        snd_seq_remove_events_t *remove_ev;
        void calcMidiClockTempo(double realtime);
        void sendMidiClock();
        void initTempo();
        bool callJack(int portcount, const QString & clientname=PACKAGE);

        JackDriver *jackSync;
        jack_position_t jPos;

        uint64_t midiTick;
        uint64_t lastRatioTick;
        uint64_t midiTempoRefreshTick;
        uint64_t nextMidiClockTick;
        uint64_t clockStartOffsetTick;
        uint64_t lastSchedTick;
        uint64_t tempoChangeTick;
        uint64_t tempoChangeFrame;

        double tempoChangeTime;
        snd_seq_real_time_t atime;


    public:
        void sendMidiEvent(MidiEvent ev, uint64_t n_tick, unsigned int outport, unsigned int duration = 0);
        bool requestEchoAt(uint64_t echoTick, bool echo_from_trig = 0);

    public:
        SeqDriver(
            JackDriver *p_jackSync,
            int p_portCount,
            void * callback_context,
            bool (* midi_event_received_callback)(void * context, MidiEvent ev),
            void (* tick_callback)(void * context, bool echo_from_trig));
        ~SeqDriver();
        double getCurrentTime();
        void calcCurrentTick(double time); /** calculate m_current_tick based on realTime */
        void requestTempo(double bpm);
        void setTempo(double bpm);
        int getClientId();
        void run();

   public slots:
        void setTransportStatus(bool run);
        void setUseMidiClock(bool on);
        void setUseJackTransport(bool on);
};

#endif
#endif
