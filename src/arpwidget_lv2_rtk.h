/*!
 * @file arpwidget_lv2_rtk.h
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

#ifndef QMIDIARP_ARPWIDGET_LV2_RTK_H
#define QMIDIARP_ARPWIDGET_LV2_RTK_H

#include "lv2_common.h"

#define QMIDIARP_ARP_LV2_URI QMIDIARP_LV2_URI "/arp"
#define QMIDIARP_ARP_LV2_PREFIX QMIDIARP_ARP_LV2_URI "#"
#define QMIDIARP_ARP_LV2UI_URI QMIDIARP_ARP_LV2_PREFIX "ui"
#define QMIDIARP_ARP_LV2UI_X11_URI QMIDIARP_ARP_LV2_PREFIX "ui_x11"

#define ARPSCR_MIN_W   250
#define ARPSCR_MIN_H   120
#define ARPSCR_VMARG    22
#define ARPSCR_HMARG    26

#define CSR_MIN_W   250
#define CSR_MIN_H     6
#define CSR_VMARG     4
#define CSR_HMARG    ARPSCR_HMARG

#define PATTERNBUFSIZE 256

enum PortIndex {
    MidiIn = 0,
    MidiOut = 1,
    ATTACK = 2,
    RELEASE = 3,
    RANDOM_TICK = 4,
    RANDOM_LEN = 5,
    RANDOM_VEL = 6,
    CH_OUT = 7,
    CH_IN = 8,
    CURSOR_POS = 9, //output
    ENABLE_RESTARTBYKBD = 10,
    ENABLE_TRIGBYKBD = 11,
    MUTE = 12,
    LATCH_MODE = 13,
    OCTAVE_MODE = 14,
    OCTAVE_LOW = 15,
    OCTAVE_HIGH = 16,
    INDEX_IN1 = 17,
    INDEX_IN2 = 18,
    RANGE_IN1 = 19,
    RANGE_IN2 = 20,
    ENABLE_TRIGLEGATO = 21,
    REPEAT_MODE = 22,
    RPATTERNFLAG = 23,
    DEFER = 24,
    PATTERN_PRESET = 25,
    TRANSPORT_MODE = 26,
    TEMPO = 27,
    HOST_TEMPO = 28,
    HOST_POSITION = 29,
    HOST_SPEED = 30
};

#define RTK_URI QMIDIARP_ARP_LV2_URI "#"
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
#define DAWIDTH ARPSCR_MIN_W
#endif

#define DAHEIGHT ARPSCR_MIN_H + CSR_MIN_H + CSR_VMARG
#define DACENTER (rintf(.5 * (DAHEIGHT)))

#define WAVEBUFSIZE 32768


typedef struct {
  LV2_Atom_Forge forge;
  LV2_URID_Map*  map;
  QMidiArpURIs   uris;

  LV2UI_Write_Function write;
  LV2UI_Controller controller;


/* Drawing parameters */
  int currentIndex; // = 0;
  int maxOctave; // = 0;
  int minOctave; // = 0;
  double nSteps; // = 1.0;
  double minStepWidth; // = 1.0;
  int patternMaxIndex; // = 0;


/* Mouse state */
  double mouse_x;
  double mouse_y;
  int mouse_buttons;
  int mouse_pressed;

  bool isMuted;
  
  bool draw_only_cursor;
  bool receivedPatternOnce;
  bool receivePatternFlag;

  char *pattern;
  int patternLen;

// these two replace the Sample vectors (sequence or lfo wave)
// ! when datavalue is negative the point is muted ! 

  // Widgets common to all modules

  RobWidget *in_out_box;
  RobWidget *ctable_notefilter;
  RobTkCBtn *btn_filter;
  RobTkLbl  *lbl_filter_range, *lbl_filter_index;
  RobTkSpin *spb_index_in0, *spb_index_in1, *spb_range_in0, *spb_range_in1;

  RobTkSep *sep_inout[4];
  RobTkSelect *ch_in, *ch_out;
  RobTkLbl  *lbl_inbox, *lbl_outbox, *lbl_ch_in, *lbl_ch_out;
  RobTkCBtn *btn_mute, *btn_restartkbd, *btn_triglegato, *btn_trigkbd, *btn_defer;

  RobTkCBtn *btn_transport;
  RobTkLbl  *lbl_tempo;
  RobTkSpin *spb_tempo;
  RobWidget *hbox, *ctable_spin;
  
  // Arp specific Widgets
  //RobWidget *hbox_bottom;
  RobWidget *hbox_da;
  // RobWidget *hbox_pattern;
  RobWidget *vbox;
  //RobWidget *hbox_stretch;
  RobWidget *ctable_wave;

  RobTkSep *sep[5];
  RobWidget *darea;
  
  RobTkLbl  *lbl_rnd_shift, *lbl_rnd_vel, *lbl_rnd_len, *lbl_attack, *lbl_release;
  RobTkDial  *dial_control[5];
  RobTkLbl   *dial_control_ann[5];
  RobWidget  *dial_control_box[5];
  
  RobTkLbl  *lbl_repeat_mode, *lbl_oct_mode;
  RobTkSelect *sel_repeat_mode, *sel_presets;
  
  RobTkSelect *sel_oct_mode, *sel_oct_low, *sel_oct_high;
  RobTkCBtn *btn_latch, *btn_text_edit, *btn_text_remove, *btn_text_store;
  
  //RobTkLbl *lbl_pattern_text;
  

  bool uiIsUp;
  bool offset_suppress_callback;
  cairo_surface_t *gridnlabels;
  PangoFontDescription *font[4];
  bool     error;

  uint32_t  w_height;

#ifdef LVGL_RESIZEABLE
  uint32_t w_width;
#endif
} QMidiArpArpUI;

#endif
