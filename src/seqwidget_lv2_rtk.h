/*!
 * @file seqwidget_lv2_rtk.h
 * @brief Headers for the RobTk-based LV2 GUI for the QMidiArp Seq plugin.
 *
 *
 *      Copyright 2009 - 2024 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef QMIDIARP_SEQWIDGET_LV2_RTK_H
#define QMIDIARP_SEQWIDGET_LV2_RTK_H

#include "lv2_common.h"

#define QMIDIARP_SEQ_LV2_URI QMIDIARP_LV2_URI "/seq"
#define QMIDIARP_SEQ_LV2_PREFIX QMIDIARP_SEQ_LV2_URI "#"
#define QMIDIARP_SEQ_LV2UI_URI QMIDIARP_SEQ_LV2_PREFIX "ui"
#define QMIDIARP_SEQ_LV2UI_X11_URI QMIDIARP_SEQ_LV2_PREFIX "ui_x11"

#define SEQSCR_MIN_W      640
#define SEQSCR_MIN_H      246 //246
#define SEQSCR_VMARG_TOP  14
#define SEQSCR_VMARG_BOT  26
#define SEQSCR_HMARG      20

#define CSR_MIN_W   250
#define CSR_MIN_H     6
#define CSR_VMARG    10
#define CSR_HMARG    20

enum PortIndex {
        MidiIn = 0,
        MidiOut = 1,
        VELOCITY = 2,
        NOTELENGTH = 3,
        RESOLUTION = 4,
        SIZE = 5,
        TRANSPOSE = 6,
        CH_OUT = 7,
        CH_IN = 8,
        CURSOR_POS = 9, //output
        LOOPMARKER = 10,
        LOOPMODE = 11,
        MUTE = 12,
        MOUSEX = 13,
        MOUSEY = 14,
        MOUSEBUTTON = 15,
        MOUSEPRESSED = 16,
        ENABLE_NOTEIN = 17,
        ENABLE_VELIN = 18,
        ENABLE_NOTEOFF = 19,
        ENABLE_RESTARTBYKBD = 20,
        ENABLE_TRIGBYKBD = 21,
        ENABLE_TRIGLEGATO = 22,
        INDEX_IN1 = 23,
        INDEX_IN2 = 24,
        RANGE_IN1 = 25,
        RANGE_IN2 = 26,
        RECORD = 27,
        DEFER = 28,
        CURR_RECSTEP = 29, //output
        TRANSPORT_MODE = 30,
        TEMPO = 31,
        HOST_TEMPO = 32,
        HOST_POSITION = 33,
        HOST_SPEED = 34,
        DISPLAY_ZOOM = 35
};

#define RTK_URI QMIDIARP_SEQ_LV2_URI "#"
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
#define DAWIDTH SEQSCR_MIN_W
#endif

#define DAHEIGHT SEQSCR_MIN_H + CSR_MIN_H + CSR_VMARG
#define DACENTER (rintf(.5 * (DAHEIGHT)))

#define WAVEBUFSIZE 32768


typedef struct {
  LV2_Atom_Forge forge;
  LV2_URID_Map*  map;
  QMidiArpURIs     uris;

  LV2UI_Write_Function write;
  LV2UI_Controller controller;


/* Drawing parameters */
  int baseOctave; // = 3;
  int nOctaves; //= 4;
  int currentRecStep; // = 0;
  int loopMarker; // = 0;
  int currentIndex; // = 0;
  int dispVert; // = 0;
  int mouseY; // = 0;
  int res; // = 4;
  int size; // = 4;

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
  RobTkCBtn *btn_notein, *btn_velin, *btn_noteoff;

  RobTkCBtn *btn_transport;
  RobTkLbl  *lbl_tempo;
  RobTkSpin *spb_tempo;
  RobWidget *hbox, *ctable_spin;
  
  // Seq specific Widgets
  RobWidget *hbox_bottom;
  RobWidget *hbox_da;
  RobWidget *vbox, *ctable2;
  RobWidget *hbox_stretch;
  RobWidget *hbox_disp_vert;

  RobTkSep *sep[3];
  RobWidget *darea;
  
  RobTkLbl  *lbl_velocity, *lbl_notelength, *lbl_transpose;
  RobTkDial  *dial_control[3];
  RobTkLbl   *dial_control_ann[3];
  RobWidget  *dial_control_box[3];

  RobTkRadioGrp *disp_rbtn_group;
  RobTkRBtn *rbtn_disp_vert[4];
  
  RobTkLbl  *lbl_size, *lbl_res;
  RobTkSelect *res_box, *size_box;
  
  RobTkSelect *loop_mode;
  RobTkCBtn *btn_record, *btn_defer;


  bool uiIsUp;
  cairo_surface_t *gridnlabels;
  PangoFontDescription *font[4];
  bool     error;

  uint32_t  w_height;

#ifdef LVGL_RESIZEABLE
  uint32_t w_width;
#endif
} QMidiArpSeqUI;

#endif
