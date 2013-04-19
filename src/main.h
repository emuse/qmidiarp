/**  @file main.h
 *   @brief Main include file. Defines global Macros.
 *
 *   @section LICENSE
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
#ifndef __MAIN_H
#define __MAIN_H

#include <QXmlStreamReader>

#include "midievent.h"

extern QString global_jack_session_uuid;
/*!
* @brief allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It also advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml reference to QXmlStreamReader containing the open XML stream
*/
extern void skipXmlElement(QXmlStreamReader& xml);

#define CT_FOOTSW       0x40

#define MAX_PORTS         64
#define SEQPOOL         2048
#define JQ_BUFSZ        1024
#define LFO_FRAMELIMIT    16
#define MAXNOTES         128
#define TPQN             192
#define MIDICLK_TPQN      24
#define MAXCHORD          33

#define QMARCNAME ".qmidiarprc"
#define JSFILENAME "js_saved.qmax"

#define COMPACT_STYLE "QLabel { font: 7pt; } \
    QComboBox { font: 7pt; max-height: 15px;} \
    QToolButton { max-height: 20px;} \
    QSpinBox { font: 7pt; max-height: 20px;} \
    QCheckBox { font: 7pt; max-height: 20px;} \
    QGroupBox { font: 7pt; }"

#endif
