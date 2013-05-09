/*!
 * @file qmidiarp_lfo_lv2.h
 * @brief Headers for the MidiLfo LV2 plugin class
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2013 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef QMIDIARP_LFO_LV2_H
#define QMIDIARP_LFO_LV2_H

#include "midilfo.h"
#include "globstore.h"
#include "lfowidget.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#define QMIDIARP_LFO_LV2_URI "https://git.code.sf.net/p/qmidiarp/code"
#define QMIDIARP_LFO_LV2_PREFIX QMIDIARP_LFO_LV2_URI "#"


class qmidiarp_lfo_lv2 : public MidiLfo
{
public:

        qmidiarp_lfo_lv2(double sample_rate, const LV2_Feature *const *host_features);

        ~qmidiarp_lfo_lv2();

        enum PortIndex {
                MidiIn = 0,
                MidiOut = 1,
                AMPLITUDE = 2,
                OFFSET = 3,
                RESOLUTION = 4,
                SIZE = 5,
                FREQUENCY = 6,
                CH_OUT = 7,
                CH_IN = 8
        };

        void connect_port(uint32_t port, void *data);

        void run(uint32_t nframes);

        void activate();
        void deactivate();

private:

        float *val[9];
        uint32_t eventID;
        uint64_t curFrame;
        int inLfoFrame;
        double sampleRate;
        void updateParams();

        LV2_Event_Buffer *inEventBuffer;
        LV2_Event_Buffer *outEventBuffer;
};

#endif
