/* -*- Mode: C++ ; c-basic-offset: 4 -*- */
/*!
 * @file driverbase.h
 * @brief Base class for drivers
 *
 * @section LICENSE
 *
 *      Copyright 2011 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef DRIVERBASE_H__9383DA6E_DCDB_4840_86DA_6A36E87653D2__INCLUDED
#define DRIVERBASE_H__9383DA6E_DCDB_4840_86DA_6A36E87653D2__INCLUDED

#include <QThread>

class DriverBase : public QThread
{
public:

    bool useMidiClock, useJackSync;
    bool forwardUnmatched, queueStatus;
    int portUnmatched;

    virtual void resetTick(unsigned int tick = 0)
    {
        m_current_tick = tick;
    }

    virtual unsigned int getCurrentTick() const
    {
        return m_current_tick;
    }

    virtual void setNextTick(unsigned int next_tick)
    {
        if (next_tick > m_current_tick)
        {
            m_next_tick = next_tick;
        }
    }

    virtual void setTpm(uint64_t ticks_per_minute)
    {
        m_tpm = ticks_per_minute;
    }

    virtual void setUseJackTransport(bool on)
    {
        useJackSync = on;
    }

    virtual void setUseMidiClock(bool on)
    {
        useMidiClock = on;
    }

    virtual void setForwardUnmatched(bool on)
    {
        forwardUnmatched = on;
    }

    virtual void setPortUnmatched(int id)
    {
        portUnmatched = id;
    }
    virtual void setTempo(int bpm)
    {
        tempo = bpm;
        internalTempo = bpm;
    }

    // duration is in ticks and is valid only for note on events
    virtual void sendMidiEvent(MidiEvent ev, int n_tick, unsigned int outport, unsigned int duration = 0) = 0;
    virtual bool requestEchoAt(int echoTick, bool echo_from_trig = 0) = 0;


    virtual void setTransportStatus(bool run) = 0;
    virtual int getClientId() = 0;

protected:
    DriverBase(
        void * callback_context,
        bool (* midi_event_received_callback)(void * context, MidiEvent ev),
        void (* tick_callback)(void * context, bool echo_from_trig),
        uint64_t backend_rate)
        : m_midi_event_received_callback(midi_event_received_callback)
        , m_tick_callback(tick_callback)
        , m_callback_context(callback_context)
        , m_backend_rate(backend_rate)
        , m_current_tick(0)
        , m_next_tick(0)
        , m_tpm(0)
    {
    }

    uint64_t tickToBackendOffset(unsigned int tick)
    {
        return (uint64_t)tick * m_backend_rate / m_tpm;
    }

    unsigned int backendOffsetToTick(uint64_t backend_offset)
    {
        return backend_offset * m_tpm / m_backend_rate;
    }

    uint64_t getCurrentTickBackendOffset()
    {
        return tickToBackendOffset(m_current_tick);
    }

    uint64_t getNextTickBackendOffset()
    {
        return tickToBackendOffset(m_next_tick);
    }

    bool midi_event_received(MidiEvent ev)
    {
        return m_midi_event_received_callback(m_callback_context, ev);
    }

    void tick_callback(bool echo_from_trig)
    {
        m_tick_callback(m_callback_context, echo_from_trig);
    }

    bool (* m_midi_event_received_callback)(void * context, MidiEvent ev);
    void (* m_tick_callback)(void * context, bool echo_from_trig);
    void * m_callback_context;
    uint64_t m_backend_rate;    // samples(?) per minute (granularity)
    unsigned int m_current_tick;
    unsigned int m_next_tick;
    uint64_t m_tpm;             // ticks per minute
    int tempo, internalTempo;
    int portCount;
};

#endif // #ifndef DRIVERBASE_H__9383DA6E_DCDB_4840_86DA_6A36E87653D2__INCLUDED
