/*!
 * @file seqdriver.h
 * @brief Header for the SeqDriver class
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QWidget>
#include <QThread>
#include <alsa/asoundlib.h>

#include "jackdriver.h"
#include "midiarp.h"
#include "midilfo.h"
#include "midiseq.h"
#include "main.h"
#include "driverbase.h"

/*! @brief ALSA sequencer backend QThread class. Also creates JackDriver
 *
 * SeqDriver is created by Engine at the moment of program start. Its
 * constructor registers ALSA seq input port and the requested number of
 * output ports. It is called with a portless JackDriver instance.
 * The SeqDriver::run() thread is the ALSA sequencer "callback" process
 * handling all incoming and outgoing sequencer events.
 * When the SeqDriver::setTransportStatus() member is called with True argument,
 * a so called "echo event" is scheduled with zero time. Echo events go back
 * to the callback process and allow output and reception of sequencer
 * events depending on the ALSA queue timing. Depending on the event types,
 * the Engine and its modules are called in series and return their
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

        double tickToDelta(int tick);
        int deltaToTick (double curtime);
        double aTimeToDelta(snd_seq_real_time_t* atime);
        const snd_seq_real_time_t* deltaToATime(double curtime);
        snd_seq_remove_events_t *remove_ev;
        void calcClockRatio();

        void initTempo();
        bool callJack(int portcount);

        JackDriver *jackSync;
        jack_position_t jPos;

        int midiTick;
        int lastRatioTick;
        int lastSchedTick;
        int jackOffsetTick;

        double clockRatio;         /* duration of one tick, in nanoseconds; based on current tempo */
        snd_seq_real_time_t delta, realTime;
        snd_seq_real_time_t tmpTime;


    public:
        void sendMidiEvent(MidiEvent ev, int n_tick, unsigned int outport, unsigned int duration = 0);
        bool requestEchoAt(int echoTick, bool echo_from_trig = 0);

    public:
        SeqDriver(
            JackDriver *p_jackSync,
            int p_portCount,
            void * callback_context,
            bool (* midi_event_received_callback)(void * context, MidiEvent ev),
            void (* tick_callback)(void * context, bool echo_from_trig));
        ~SeqDriver();
        void getTime();
        int getClientId(); /** overloaded over DriverBase */
        void run();

   public slots:
        void setTransportStatus(bool run); /** is pure in DriverBase */
        void setTempo(int bpm); /** overloaded over DriverBase */
        void setUseMidiClock(bool on); /** overloaded over DriverBase */
        void setUseJackTransport(bool on); /** overloaded over DriverBase */
};

#endif
