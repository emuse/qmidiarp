/*!
 * @file lfowidget_lv2_rtk.h
 * @brief Headers for the RobTk-based LV2 GUI for the QMidiArp Lfo plugin.
 *
 *
 *      Copyright 2009 - 2023 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      The structure as well as some lines of code and functions were 
 *      taken from sisco lv2 by Robin Garaeus: https://github.com/x42/sisco.lv2 
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

#ifndef QMIDIARP_LFOWIDGET_LV2_RTK_H
#define QMIDIARP_LFOWIDGET_LV2_RTK_H

#include "lv2_common.h"

#define QMIDIARP_LFO_LV2_URI QMIDIARP_LV2_URI "/lfo"
#define QMIDIARP_LFO_LV2_PREFIX QMIDIARP_LFO_LV2_URI "#"
#define QMIDIARP_LFO_LV2UI_URI QMIDIARP_LFO_LV2_PREFIX "ui"
#define QMIDIARP_LFO_LV2UI_X11_URI QMIDIARP_LFO_LV2_PREFIX "ui_x11"

#define LFOSCR_MIN_W   250
#define LFOSCR_MIN_H   120
#define LFOSCR_VMARG    12
#define LFOSCR_HMARG    26

#define CSR_MIN_W   250
#define CSR_MIN_H     6
#define CSR_VMARG     4
#define CSR_HMARG    LFOSCR_HMARG

enum PortIndex {
            MidiIn = 0,
            MidiOut = 1,
            AMPLITUDE = 2,
            OFFSET = 3,
            RESOLUTION = 4,
            SIZE = 5,
            FREQUENCY = 6,
            CH_OUT = 7,
            CH_IN = 8,
            CURSOR_POS = 9, //output
            WAVEFORM = 10,
            LOOPMODE = 11,
            MUTE = 12,
            MOUSEX = 13,
            MOUSEY = 14,
            MOUSEBUTTON = 15,
            MOUSEPRESSED = 16,
            CC_OUT = 17,
            CC_IN = 18,
            INDEX_IN1 = 19,
            INDEX_IN2 = 20,
            RANGE_IN1 = 21,
            RANGE_IN2 = 22,
            ENABLE_NOTEOFF = 23,
            ENABLE_RESTARTBYKBD = 24,
            ENABLE_TRIGBYKBD = 25,
            ENABLE_TRIGLEGATO = 26,
            RECORD = 27,
            DEFER = 28,
            PHASE = 29,
            TRANSPORT_MODE = 30,
            TEMPO = 31,
            WaveOut = 32,
            HOST_TEMPO = 33,
            HOST_POSITION = 34,
            HOST_SPEED = 35
        };

#define RTK_URI QMIDIARP_LFO_LV2_URI "#"
#define RTK_GUI "ui"

#ifndef MIN
#define MIN(A,B) ( (A) < (B) ? (A) : (B) )
#endif
#ifndef MAX
#define MAX(A,B) ( (A) > (B) ? (A) : (B) )
#endif

#ifdef HAVE_LV2_1_8
#define x_forge_object lv2_atom_forge_object
#else
#define x_forge_object lv2_atom_forge_blank
#endif

/* drawing area size */

#ifdef LVGL_RESIZEABLE
#define DAWIDTH  (ui->w_width)
#else
#define DAWIDTH LFOSCR_MIN_W
#endif

#define DAHEIGHT LFOSCR_MIN_H + CSR_MIN_H + CSR_VMARG
#define DACENTER (rintf(.5 * (DAHEIGHT)))

#define WAVEBUFSIZE 32768


typedef struct {
  LV2_Atom_Forge forge;
  LV2_URID_Map*  map;
  QMidiArpURIs     uris;

  LV2UI_Write_Function write;
  LV2UI_Controller controller;


/* Drawing parameters */
  int currentRecStep; // = 0;
  int loopMarker; // = 0;
  int currentIndex; // = 0;
  int mouseY; // = 0;
  int res; // = 4;
  int size; // = 4;

  bool copiedToCustomFlag;
  int xMax;

/* Mouse state */
  double mouse_x;
  double mouse_y;
  int mouse_buttons;
  int mouse_pressed;

  bool recordMode;
  bool isMuted;
  
  bool draw_only_cursor;

// these two replace the Sample vectors (sequence or lfo wave)
// ! when datavalue is negative the point is muted ! 

  uint64_t datavalues[WAVEBUFSIZE];
  uint64_t dataticks[WAVEBUFSIZE];
  int data_count;

  // Widgets common to all modules

  RobWidget *in_out_box;
  RobWidget *ctable_notefilter;
  RobTkCBtn *btn_filter;
  RobTkLbl  *lbl_filter_range, *lbl_filter_index;
  RobTkSpin *spb_index_in0, *spb_index_in1, *spb_range_in0, *spb_range_in1;

  RobTkSep *sep_inout[4];
  RobTkSelect *ch_in, *ch_out;
  RobTkLbl  *lbl_inbox, *lbl_outbox, *lbl_ch_in, *lbl_ch_out;
  RobTkCBtn *btn_mute, *btn_restartkbd, *btn_triglegato, *btn_trigkbd;
  RobTkCBtn *btn_noteoff;

  RobTkCBtn *btn_transport;
  RobTkLbl  *lbl_tempo;
  RobTkSpin *spb_tempo;
  RobWidget *hbox, *ctable_spin;
  
  // Lfo specific Widgets
  RobWidget *hbox_bottom;
  RobWidget *hbox_da;
  RobWidget *vbox, *ctable2;
  RobWidget *hbox_stretch;
  RobWidget *ctable_wave;

  RobTkSep *sep[5];
  RobWidget *darea;
  
  RobTkSelect *cc_in, *cc_out;
  RobTkLbl *lbl_cc_in, *lbl_cc_out;

  RobTkLbl    *lbl_freq;
  RobTkSelect *sel_freq;
  RobTkPBtn   *btn_flip;
  
  RobTkLbl  *lbl_amplitude, *lbl_offset, *lbl_phase;
  RobTkDial  *dial_control[3];
  RobTkLbl   *dial_control_ann[3];
  RobWidget  *dial_control_box[3];
  
  RobTkLbl  *lbl_size, *lbl_res;
  RobTkSelect *res_box, *size_box;
  
  RobTkSelect *loop_mode;
  RobTkSelect *waveform;
  RobTkCBtn *btn_record, *btn_defer;


  bool uiIsUp;
  bool offset_suppress_callback;
  cairo_surface_t *gridnlabels;
  PangoFontDescription *font[4];
  bool     error;

  uint32_t  w_height;

#ifdef LVGL_RESIZEABLE
  uint32_t w_width;
#endif
} QMidiArpLfoUI;

#endif
