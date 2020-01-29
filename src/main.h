/**  @file main.h
 *   @brief Main include file. Defines global Macros.
 *
 *   @section LICENSE
 *
 *      Copyright 2009 - 2019 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef __MAIN_H
#define __MAIN_H

#include "midievent.h"

#define CT_FOOTSW       0x40
#define CT_ALLNOTESOFF  0x7B
#define CT_ALLSOUNDOFF  0x78

#define MAX_PORTS         64
#define SEQPOOL         2048
#define JQ_BUFSZ        1024
#define LFO_FRAMELIMIT    16
#define MAXNOTES         128
#define TPQN             192
#define MIDICLK_TPQN      24
#define MAXCHORD          33
#define OMNI              16

#define QMARCNAME ".qmidiarprc"
#define JSFILENAME "js_saved.qmax"

#define COMPACT_STYLE "QLabel { font: 7pt; } \
    QComboBox { font: 7pt; max-height: 15px;} \
    QToolButton { font: 8pt; max-height: 15px;} \
    QSpinBox { font: 7pt; max-height: 20px;} \
    QCheckBox { font: 7pt; max-height: 20px;} \
    QGroupBox { font: 7pt; }"


/*! This array holds the currently available frequency values.
 */
const int lfoFreqValues[14] = {1, 2, 4, 8, 16, 24, 32, 64, 96, 128, 160, 192, 224, 256};

/*! @brief This array holds the currently available LFO size values.
 */
const int lfoSizeValues[18] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 24, 32};

/*! @brief This array holds the currently available LFO resolution values.
 */
const int lfoResValues[9] = {1, 2, 4, 8, 16, 32, 64, 96, 192};

/*! @brief This array holds the currently available Seq resolution values.
 */
const int seqResValues[5] = {1, 2, 4, 8, 16};

/*! @brief This array holds the currently available Seq size values.
 */
const int seqSizeValues[18] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 24, 32};
#endif
