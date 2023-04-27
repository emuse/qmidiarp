/*!
 * @file arpwidget_lv2_rtk.c
 * @brief Implementation of the RobTk-based LV2 GUI for the QMidiArp Seq plugin.
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

///////////////////////
#define LVGL_RESIZEABLE
///////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#include "arpwidget_lv2_rtk.h"
#include "main.h"

static const float color_tbg[4] = {0.0, 0.0, 0.0, 0.7};

static const float color_blk[4] = {0.0, 0.0, 0.0, 1.0};
static const float color_gry[4] = {0.5, 0.5, 0.5, 1.0};
static const float color_wht[4] = {1.0, 1.0, 1.0, 1.0};


/******************************************************************************
 * Allocate Data structures
 */

LV2UI_Controller     controller;
LV2UI_Write_Function write_Function;
/******************************************************************************
 * Communication with DSP backend -- send/receive settings
 */

void updateParam(LV2UI_Handle handle, int index, float fValue)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;    
    ui->write(ui->controller, index, sizeof(float), 0, &fValue);
}

void receivePattern(LV2UI_Handle handle, LV2_Atom* atom)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;
    const QMidiArpURIs* uris = &ui->uris;

    if ( (atom->type != uris->atom_Blank) 
            && (atom->type != uris->atom_Object)) return;

    ui->receivedPatternOnce = true;

    /* cast the buffer to Atom Object */
    LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
    LV2_Atom *a0 = NULL;
    lv2_atom_object_get(obj, uris->pattern_string, &a0, NULL);
    if (obj->body.otype != uris->pattern_string) return;

    /* handle pattern string as atom body */
    const char* p = (const char*)LV2_ATOM_BODY(a0);
    
    int plen = strlen(p);    
    if (!plen) return;

    ui->pattern[0] = '\0';
    strncat(ui->pattern, p, PATTERNBUFSIZE - 1);
    ui->patternLen = plen;
    //robtk_lbl_set_text(ui->lbl_pattern_text, ui->pattern);
    ui->receivePatternFlag = false;
}


void sendPattern(LV2UI_Handle handle)
{

    QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;
    const QMidiArpURIs* uris = &ui->uris;

    const char* c = ui->pattern;
    uint8_t obj_buf[256];

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&ui->forge, 0);

    // prepare forge buffer and initialize atom-sequence
    lv2_atom_forge_set_buffer(&ui->forge, obj_buf, sizeof(obj_buf));
    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ui->forge, &frame, 1, uris->pattern_string);

    // forge container object of type 'pattern_string'
    lv2_atom_forge_property_head(&ui->forge, uris->pattern_string, 0);
    lv2_atom_forge_string(&ui->forge, c, strlen(c));

    // close-off frame 
    lv2_atom_forge_pop(&ui->forge, &frame);
    ui->write(ui->controller, MidiIn, lv2_atom_total_size(msg), ui->uris.atom_eventTransfer, msg);
}

static void 
ui_state(LV2UI_Handle handle)
{
  (void)handle;
}

/** notfiy backend that UI is closed */
static void ui_disable(LV2UI_Handle handle)
{
  QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;
  uint8_t obj_buf[64];
  lv2_atom_forge_set_buffer(&ui->forge, obj_buf, 64);
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_frame_time(&ui->forge, 0);
  LV2_Atom* msg = (LV2_Atom*)x_forge_object(&ui->forge, &frame, 1, ui->uris.ui_down);
  lv2_atom_forge_pop(&ui->forge, &frame);
  ui->write(ui->controller, 0, lv2_atom_total_size(msg), ui->uris.atom_eventTransfer, msg);
}

/** notify backend that UI is active:
 * request state and enable data-transmission */
static void ui_enable(LV2UI_Handle handle)
{
  QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;
  uint8_t obj_buf[64];
  lv2_atom_forge_set_buffer(&ui->forge, obj_buf, 64);
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_frame_time(&ui->forge, 0);
  LV2_Atom* msg = (LV2_Atom*)x_forge_object(&ui->forge, &frame, 1, ui->uris.ui_up);
  lv2_atom_forge_pop(&ui->forge, &frame);
  ui->write(ui->controller, 0, lv2_atom_total_size(msg), ui->uris.atom_eventTransfer, msg);
}
/******************************************************************************
 * WIDGET CALLBACKS
 */

/* common update functions */

static bool update_trigkbd (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, ENABLE_TRIGBYKBD, robtk_cbtn_get_active(ui->btn_trigkbd));
    return TRUE;
}

static bool update_restartkbd (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, ENABLE_RESTARTBYKBD, robtk_cbtn_get_active(ui->btn_restartkbd));
    return TRUE;

}

static bool update_triglegato (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, ENABLE_TRIGLEGATO, robtk_cbtn_get_active(ui->btn_triglegato));
    return TRUE;
}

static bool update_notefilter (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    if (robtk_cbtn_get_active(ui->btn_filter)) {
      robwidget_show(ui->ctable_notefilter, TRUE);
    }
    else {
      robwidget_hide(ui->ctable_notefilter, TRUE);
    }
    return TRUE;
}

static bool update_index_in0 (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, INDEX_IN1, robtk_spin_get_value(ui->spb_index_in0));
    return TRUE;

}

static bool update_index_in1 (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, INDEX_IN2, robtk_spin_get_value(ui->spb_index_in1));
    return TRUE;

}

static bool update_range_in0 (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, RANGE_IN1, robtk_spin_get_value(ui->spb_range_in0));
    return TRUE;

}

static bool update_range_in1 (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, RANGE_IN2, robtk_spin_get_value(ui->spb_range_in1));
    return TRUE;

}

static bool update_transport (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, TRANSPORT_MODE, robtk_cbtn_get_active(ui->btn_transport));
    return TRUE;
}

static bool update_tempo (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, TEMPO, robtk_spin_get_value(ui->spb_tempo));
    return TRUE;
}


static bool update_ch_in (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, CH_IN, robtk_select_get_item(ui->ch_in));
    return TRUE;
}

static bool update_ch_out (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, CH_OUT, robtk_select_get_item(ui->ch_out));
    return TRUE;
}
/* end common update functions */


static bool update_rnd_shift (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, RANDOM_TICK, robtk_dial_get_value(ui->dial_control[0]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[0]));
    robtk_lbl_set_text(ui->dial_control_ann[0], txt);
    return TRUE;

}

static bool update_rnd_vel (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[1]));
    updateParam(ui, RANDOM_VEL, robtk_dial_get_value(ui->dial_control[1]));
    robtk_lbl_set_text(ui->dial_control_ann[1], txt);
    return TRUE;

}

static bool update_rnd_len (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, RANDOM_LEN, robtk_dial_get_value(ui->dial_control[2]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[2]));
    robtk_lbl_set_text(ui->dial_control_ann[2], txt);
    return TRUE;

}

static bool update_attack (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, ATTACK, robtk_dial_get_value(ui->dial_control[3]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[3]));
    robtk_lbl_set_text(ui->dial_control_ann[3], txt);
    return TRUE;

}

static bool update_release (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, RELEASE, robtk_dial_get_value(ui->dial_control[4]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[4]));
    robtk_lbl_set_text(ui->dial_control_ann[4], txt);
    return TRUE;

}

static bool update_repeat_mode (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    uint8_t val = robtk_select_get_item(ui->sel_repeat_mode);
    updateParam(ui, REPEAT_MODE, val);
    return TRUE;

}

static bool update_oct_mode (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    uint8_t val = robtk_select_get_item(ui->sel_oct_mode);
    updateParam(ui, OCTAVE_MODE, val);
    return TRUE;

}

static bool update_oct_low (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    // here we have negative values : cannot use uint !! //
    int8_t val = robtk_select_get_item(ui->sel_oct_low);
    updateParam(ui, OCTAVE_LOW, val - 3);
    return TRUE;

}

static bool update_oct_high (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    uint8_t val = robtk_select_get_item(ui->sel_oct_high);
    updateParam(ui, OCTAVE_HIGH, val);
    return TRUE;

}

static bool update_mute (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, MUTE, robtk_cbtn_get_active(ui->btn_mute));
    ui->isMuted = robtk_cbtn_get_active(ui->btn_mute);
    queue_draw(ui->darea);
    return TRUE;
}

static bool update_latch (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    
    updateParam(ui, LATCH_MODE, robtk_cbtn_get_active(ui->btn_latch));
    return TRUE;
}

static bool update_defer (RobWidget *widget, void* data)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) data;
    updateParam(ui, DEFER, robtk_cbtn_get_active(ui->btn_defer));
    return TRUE;
}

void updatePattern(LV2UI_Handle handle)
{
    QMidiArpArpUI* ui = (QMidiArpArpUI*) handle;
    
    if (!strlen(ui->pattern)) return;
     
    int patternMaxIndex = 0;
    double minStepWidth = 1.0;
    int minOctave = 0;
    int maxOctave = 0;

    double stepwd = 1.0;
    double nsteps = 0.;
    int chordindex = 0;
    bool chordmd = false;
    int oct = 0;
    int npoints = 0;
    
    // Strip trailing control tokens from end of pattern
    char c = ui->pattern[strlen(ui->pattern) - 1];
    while ( strlen(ui->pattern) 
                && !((int)c > 47) 
                && !((int)c < 58)
                && (c != 'p') 
                && (c != ')') ) {
        ui->pattern[strlen(ui->pattern) - 1] = '\0';
        c = ui->pattern[strlen(ui->pattern) - 1];
    }
    
    int patternLen = strlen(ui->pattern);
    
    if (!ui->receivePatternFlag) sendPattern(handle);

    // determine some useful properties of the arp pattern,
    // number of octaves, step width and number of steps in beats and
    // number of points

    for (int l1 = 0; l1 < patternLen; l1++) {
        const char c = ui->pattern[l1];

        if ((int)c > 47 && (int)c < 58) {
            if (!chordindex) {
                nsteps += stepwd;
                npoints++;
                if (chordmd) chordindex++;
            }
            if (((int)c - 48) > patternMaxIndex)
                patternMaxIndex = (int)c - 48;
        }
        switch(c) {
            case '(':
                chordmd = true;
                chordindex = 0;
                break;

            case ')':
                chordmd = false;
                chordindex = 0;
                break;

            case '>':
                stepwd *= .5;
                if (stepwd < minStepWidth)
                    minStepWidth *= .5;
                break;

            case '<':
                stepwd *= 2.0;
                break;

            case '.':
                stepwd = 1.0;
                break;

            case 'p':
                if (!chordmd) {
                    nsteps += stepwd;
                    npoints++;
                }
                break;

            case '+':
                oct++;
                if (oct > maxOctave)
                    maxOctave++;
                break;

            case '-':
                oct--;
                if (oct < minOctave)
                    minOctave--;
                break;

            case '=':
                oct=0;
                break;

            default:
                ;
        }
    }
    ui->minOctave = minOctave;
    ui->maxOctave = maxOctave;
    ui->minStepWidth = minStepWidth;
    ui->nSteps = nsteps;
    ui->patternMaxIndex = patternMaxIndex;
    ui->patternLen = patternLen;
    queue_draw(ui->darea);
}

/*
static void update_mouse(RobWidget* handle) {  
  QMidiArpArpUI* ui = (QMidiArpArpUI*) GET_HANDLE(handle);

  if (ui->mouse_x < 0.) ui->mouse_x = 0;
  if (ui->mouse_x > 1.) ui->mouse_x = 1;
  if (ui->mouse_y > 1.) ui->mouse_y = 1;
  if (ui->mouse_y < 0.) ui->mouse_y = 0.;

  if (ui->mouse_pressed != 0) {
    updateParam(ui, MOUSEPRESSED, ui->mouse_pressed);
    updateParam(ui, MOUSEBUTTON, ui->mouse_buttons);
    updateParam(ui, MOUSEX, ui->mouse_x);
    updateParam(ui, MOUSEY, ui->mouse_y);
    ui->draw_only_cursor = false;

    if ((ui->mouse_buttons == 1) && (ui->mouse_pressed == 1)
          && robtk_select_get_item(ui->waveform) != 5) {
        robtk_select_set_item(ui->waveform, 5);
    }
  }
    if (!(ui->mouse_pressed > 0) && (ui->mouse_buttons == 1)) updateParam(ui, WAVEFORM, 5);
}

static RobWidget* mouse_down(RobWidget* handle, RobTkBtnEvent *ev) {
  QMidiArpArpUI* ui = (QMidiArpArpUI*) GET_HANDLE(handle);
  
  ui->mouse_pressed = 1;

  if (ev->button == 1) {
    ui->mouse_buttons = 1;
  } 
  else if (ev->button == 3) {
    ui->mouse_buttons = 2;
  } 
  else {
    ui->mouse_buttons = 0;
    return handle;
  }
  update_mouse(handle);
  return NULL;
}

static RobWidget* mouse_up(RobWidget* handle, RobTkBtnEvent *ev) {
  QMidiArpArpUI* ui = (QMidiArpArpUI*) GET_HANDLE(handle);
  ui->mouse_pressed = 2;
  ui->mouse_buttons = 0;
  update_mouse(handle);
  return NULL;
}

static RobWidget* mouse_move(RobWidget* handle, RobTkBtnEvent *ev) {
  QMidiArpArpUI* ui = (QMidiArpArpUI*) GET_HANDLE(handle);
  
  int w = DAWIDTH;
  int h = ARPSCR_MIN_H;

  //printf("UI Mouse move %d1 %d2 \n", ev->x, ev->y);
  
  ui->mouse_x = ((double)ev->x - ARPSCR_HMARG) / (w - 2 * ARPSCR_HMARG);
  ui->mouse_y = 1. - ((double)ev->y - ARPSCR_VMARG) /
                (h - 2 * ARPSCR_VMARG);

  //printf("Relative coords %f1 %f2 \n", ui->mouse_x, ui->mouse_y);

  if (ui->mouse_buttons > 0) {
    update_mouse(handle);
    return handle;
  }
  return handle;
}
*/
/******************************************************************************
 * Pango / Cairo Rendering, Expose
 */

static void render_text(
    cairo_t* cr,
    const char *txt,
    PangoFontDescription *font,
    const float x, const float y,
    const float ang, const int align,
    const float * const col)
{
  int tw, th;
  cairo_save(cr);

  PangoLayout * pl = pango_cairo_create_layout(cr);
  pango_layout_set_font_description(pl, font);
  pango_layout_set_text(pl, txt, -1);
  pango_layout_get_pixel_size(pl, &tw, &th);
  cairo_translate (cr, x, y);
  if (ang != 0) { cairo_rotate (cr, ang); }
  switch(abs(align)) {
    case 1:
      cairo_translate (cr, -tw, -th/2.0);
      break;
    case 2:
      cairo_translate (cr, -tw/2.0 - 0.5, -th/2.0);
      break;
    case 3:
      cairo_translate (cr, -0.5, -th/2.0);
      break;
    case 4:
      cairo_translate (cr, -tw, -th);
      break;
    case 5:
      cairo_translate (cr, -tw/2.0 - 0.5, -th);
      break;
    case 6:
      cairo_translate (cr, -0.5, -th);
      break;
    case 7:
      cairo_translate (cr, -tw, 0);
      break;
    case 8:
      cairo_translate (cr, -tw/2.0 - 0.5, 0);
      break;
    case 9:
      cairo_translate (cr, -0.5, 0);
      break;
    default:
      break;
  }
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  if (align < 0) {
    CairoSetSouerceRGBA(color_tbg);
    cairo_rectangle (cr, 0, 0, tw, th);
    cairo_fill (cr);
  }
  cairo_set_source_rgba (cr, col[0], col[1], col[2], col[3]);
  pango_cairo_layout_path(cr, pl);
  pango_cairo_show_layout(cr, pl);
  g_object_unref(pl);
  cairo_restore(cr);
  cairo_new_path (cr);
}

/* gdk drawing area draw callback
 * -- this runs in gtk's main thread */
static bool expose_event(RobWidget* handle, cairo_t* cr, cairo_rectangle_t *ev)
{
  QMidiArpArpUI* ui = (QMidiArpArpUI*) GET_HANDLE(handle);

  /* limit cairo-drawing to widget */
  cairo_rectangle (cr, 0, 0, DAWIDTH, DAHEIGHT);
  cairo_clip(cr);

  /* limit cairo-drawing to exposed area */
  cairo_rectangle (cr, ev->x, ev->y, ev->width, ev->height);
  cairo_clip(cr);
  cairo_set_source_surface(cr, ui->gridnlabels, 0, 0);
  if (ui->isMuted) {
    cairo_set_source_rgba(cr, 70./256, 70./256, 70./256, 1);
  }
  else {
    cairo_set_source_rgba(cr, 10./256, 50./256, 10./256, 1);
  }
  cairo_paint (cr);

  cairo_save(cr);
  /* limit cairo-drawing to scope-area */
  cairo_rectangle (cr, 0, 0, DAWIDTH, DAHEIGHT);
  cairo_clip(cr);

  cairo_set_line_width(cr, 1.0);

  
  int w = DAWIDTH;
  int h = ARPSCR_MIN_H;
  int notestreak_thick = 2;
  
  // Text colors
  // const float color_txt1[4] = {50./256, 180./256, 130./256, 1};
  const float color_txt2[4] = {50./256, 180./256, 130./256, 1};
  // const float color_txt3[4] = {50./256, 180./256, 130./256, 1};

  //Determine font geometry
  int tx_w, tx_h;
  get_text_geometry("1", ui->font[2], &tx_w, &tx_h);

  //Grid
  double len = ui->nSteps;
  int xscale = (w - 2 * ARPSCR_HMARG) / len;
  int yscale = h - 2 * ARPSCR_VMARG;

  //Beat separators (4 beats)
  for (int l1 = 0; l1 < ui->nSteps + 1; l1++) {
    int ofs;
    if (l1 < 10) {
        ofs = w / len * .5 - 4 + ARPSCR_HMARG;
    } else {
        ofs = w / len * .5 - 6 + ARPSCR_HMARG;
    }
    if ((bool)(l1%4)) {
        cairo_set_source_rgba(cr, 60./256, 180./256, 60./256, 1);
    } else {
        cairo_set_source_rgba(cr, 60./256, 180./256, 150./256, 1);
    }
    int x = l1 * xscale;
    cairo_move_to(cr, ARPSCR_HMARG + x, ARPSCR_VMARG);
    cairo_line_to(cr, ARPSCR_HMARG + x, h-ARPSCR_VMARG);
    cairo_stroke(cr);

    if (l1 < ui->nSteps) {

        //Beat numbers
        char int_str[20];
        sprintf(int_str, "%d", l1+1);
        render_text(cr, int_str, ui->font[1], 
                ofs + x + tx_w / 2, 
                ARPSCR_VMARG - tx_h / 2, 0, 1, color_txt2);

        // Beat divisor separators
        cairo_set_source_rgba(cr, 40./256, 100./256, 40./256, 1);
        for (int l2 = 1; l2 < 1.0/ui->minStepWidth; l2++) {
          int x1 = x + l2 * xscale * ui->minStepWidth;
          if (x1 < xscale*len) {
            cairo_move_to(cr, ARPSCR_HMARG + x1, ARPSCR_VMARG);
            cairo_line_to(cr, ARPSCR_HMARG + x1, h-ARPSCR_VMARG);
            cairo_stroke(cr);
          }
        }
    }
  }

  //Octave separators and numbers
  cairo_set_source_rgba(cr, 40./256, 120./256, 40./256, 1);
  int noctaves = ui->maxOctave - ui->minOctave + 1;
  for (int l1 = 0; l1 < noctaves + 1; l1++) {
    int ypos = yscale * l1 / noctaves + ARPSCR_VMARG;
    cairo_move_to(cr, ARPSCR_HMARG, ypos);
    cairo_line_to(cr, w - ARPSCR_HMARG, ypos);
    cairo_stroke(cr);
    if (l1 < noctaves) {
      char int_str[20];
      sprintf(int_str, "%d", noctaves - l1 + ui->minOctave - 1);
      render_text(cr, int_str, ui->font[1], 
            ARPSCR_HMARG / 2 - 3 + tx_w / 2, 
            yscale * (l1 + 0.5) / noctaves + ARPSCR_VMARG, 0, 1, color_txt2);
    }
  }

  //Draw arpTicks
  double curstep= 0.0;
  int notelen = xscale/8;
  double stepwd = 1.0;
  double vel = 0.8;
  int octave = 0;
  int semitone = 0;
  bool chordmd = false;
  int chordindex = 0;

  int grooveIndex = 0;
  int polyindex = 0;
  int nlines = 0;
  
  for (int l1 = 0; l1 < ui->patternLen; l1++)
  {
      int forward = 0;
      char c = ui->pattern[l1];
      //printf("current c %c  ascii %d\n", c, (int)c);
      if ((int)c > 47 && (int)c < 58) // if c is digit
      {
          nlines = (int)c - 48 + 1;
          if (chordmd && (nlines == 1) && chordindex) {
              polyindex++;
          }
          if (!chordindex)
          {
              if (chordmd) chordindex++;
              curstep += stepwd;
          }
          if (!chordmd) forward = 1;

      }
      else
      {
          switch (c)
          {
              case '(':
                  chordmd = true;
                  chordindex = 0;
                  break;

              case ')':
                  if (chordmd) forward = 1;
                  chordmd = false;
                  chordindex = 0;
                  nlines = 0;
                  break;

              case '>':
                  stepwd *= .5;
                  break;

              case '<':
                  stepwd *= 2.0;
                  break;

              case '.':
                  stepwd = 1.0;
                  break;

              case 'p':
                  nlines = 0;
                  if (!chordmd) {
                      curstep += stepwd;
                      forward = 1;
                  }
                 break;

              case '+':
                  octave++;
                  break;

              case '-':
                  octave--;
                  break;

              case 't':
                  semitone++;
                  break;

              case 'g':
                  semitone--;
                  break;

              case '=':
                  octave=0;
                  semitone=0;
                  break;

              case '/':
                  vel += 0.2;
                  break;

              case '\\':
                  vel -= 0.2;
                  break;

              case 'd':
                  notelen *= 2;
                  break;

              case 'h':
                  notelen *= .5;
                  break;

              default:
                  ;
          }
      }
      if (((int)c > 47 && (int)c < 58) || c == 'p') {
          int octYoffset = (octave - ui->minOctave) * (ui->patternMaxIndex + 1);
          int x = (curstep - stepwd) * xscale ;
          int dx = notelen;
          double v = vel;
          cairo_set_line_width(cr, notestreak_thick);
          int xpos = ARPSCR_HMARG + x + notestreak_thick / 2;

          // Arp ticks
          if (nlines > 0) {
              int ypos = yscale - yscale * (nlines - 1 + octYoffset)
                      / (ui->patternMaxIndex + 1) / noctaves
                      + ARPSCR_VMARG - 3 + notestreak_thick
                      - notestreak_thick * polyindex;
              if (semitone != 0)
                  cairo_set_source_rgba(cr, (50 + 60 * v)/256, (130 + 40 * v)/256, 
                        (abs(100 + 10 * semitone) % 256)/256, 1);
              else
                  cairo_set_source_rgba(cr, (80 + 60 * v)/256, (160 + 40 * v)/256, 
                        (80 + 60 * v)/256, 1);
              cairo_move_to(cr, xpos, ypos);
              cairo_line_to(cr, xpos + dx - notestreak_thick, ypos);
              cairo_stroke(cr);
          }
          // Cursor
          if (grooveIndex == ui->currentIndex) {
              int ypos = h - ARPSCR_VMARG + 2 + notestreak_thick * 2;
              cairo_set_line_width(cr, notestreak_thick * 2);
              cairo_move_to(cr, xpos, ypos);
              cairo_line_to(cr, xpos + dx - notestreak_thick, ypos);
              cairo_stroke(cr);
          }
          cairo_set_line_width(cr, 1.);
      }
      grooveIndex+=forward;
  }
  // Add pattern text for info
  get_text_geometry(ui->pattern, ui->font[2], &tx_w, &tx_h);
  render_text(cr, ui->pattern, ui->font[2], 
      ARPSCR_HMARG + tx_w, 
      h - tx_h / 2 + 5, 0, 1, color_txt2);

  cairo_restore(cr);
  
  return TRUE;
}

/******************************************************************************
 * RobWidget
 */

#ifdef LVGL_RESIZEABLE
static void
size_request(RobWidget* handle, int *w, int *h) {
  (void)handle;
  *w = 320;
  *h = ARPSCR_MIN_H + CSR_MIN_H + CSR_VMARG;
}

static void
size_allocate(RobWidget* handle, int w, int h) {
  QMidiArpArpUI* ui = (QMidiArpArpUI*)GET_HANDLE(handle);
  if ((uint32_t)w == ui->w_width
      && (uint32_t)h == ui->w_height) {
    robwidget_set_size(ui->darea, w, h);
    return;
  }
  ui->w_width = MIN(16384, w );
  ui->w_height = MIN(8192, h);


  robwidget_set_size(ui->darea, w, h);

  cairo_surface_destroy(ui->gridnlabels);
  ui->gridnlabels = NULL;

  cairo_t *cr;

  if (!ui->gridnlabels) {
    ui->gridnlabels = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
    DAWIDTH, DAHEIGHT);
  }
  cr = cairo_create(ui->gridnlabels);
  CairoSetSouerceRGBA(color_blk);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_rectangle (cr, 0, 0, DAWIDTH, DAHEIGHT);
  cairo_fill (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_destroy(cr);

}

#else

static void
size_request(RobWidget* handle, int *w, int *h) {
  QMidiArpArpUI* ui = (QMidiArpArpUI*)GET_HANDLE(handle);
  *w = DAWIDTH;
  //*w = 720;
  *h = ARPSCR_MIN_H + CSR_MIN_H + CSR_VMARG; // 254;
}

#endif

#define robtk_dial_new_narrow(min, max, step) \
  robtk_dial_new_with_size(min, max, step, \
      GSP_WIDTH, GSP_HEIGHT, GSP_CX, GSP_CY, GSP_RADIUS)


static void in_out_box_new(QMidiArpArpUI* ui)
{
// In-Out boxes
  for (int i = 0; i < 4; i++) {
    ui->sep_inout[i] = robtk_sep_new(TRUE);
  }
  robtk_sep_set_linewidth(ui->sep_inout[0], 0);
  robtk_sep_set_linewidth(ui->sep_inout[3], 0);

  ui->lbl_inbox = robtk_lbl_new("Input");

  ui->btn_restartkbd = robtk_cbtn_new("Restart", GBT_LED_LEFT, true);
  ui->btn_trigkbd = robtk_cbtn_new("Trigger", GBT_LED_LEFT, true);
  ui->btn_triglegato = robtk_cbtn_new("Legato", GBT_LED_LEFT, true);

  robtk_cbtn_set_color_on(ui->btn_restartkbd, .2, .8, .6);
  robtk_cbtn_set_color_off(ui->btn_restartkbd, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_trigkbd, .2, .8, .6);
  robtk_cbtn_set_color_off(ui->btn_trigkbd, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_triglegato, .2, .8, .6);
  robtk_cbtn_set_color_off(ui->btn_triglegato, .1, .1, .3);


  robtk_cbtn_set_callback(ui->btn_restartkbd, update_restartkbd, ui);
  robtk_cbtn_set_callback(ui->btn_trigkbd, update_trigkbd, ui);
  robtk_cbtn_set_callback(ui->btn_triglegato, update_triglegato, ui);

  // ChannelIn selectbox (combobox)

  ui->lbl_ch_in = robtk_lbl_new("Ch");
  ui->ch_in = robtk_select_new();
  for (int i = 0; i < 16; i++) {
        char int_str[16];
        sprintf(int_str, "%d", i + 1 );
        robtk_select_add_item(ui->ch_in, i, int_str);
    }
  robtk_select_add_item(ui->ch_in, 16, "OMNI");
    
  robtk_select_set_item(ui->ch_in, 0);
  robtk_select_set_default_item(ui->ch_in, 0);
  robtk_select_set_callback(ui->ch_in, update_ch_in, ui);

  // Note filter button and select boxes
  ui->btn_filter = robtk_cbtn_new("Note Filter", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_filter, .2, .8, .5);
  robtk_cbtn_set_color_off(ui->btn_filter, .1, .1, .3);
  robtk_cbtn_set_callback(ui->btn_filter, update_notefilter, ui);
  
  
  ui->spb_index_in0     = robtk_spin_new(0, 127, 1);
  ui->spb_index_in1     = robtk_spin_new(0, 127, 1);
  ui->spb_range_in0     = robtk_spin_new(0, 127, 1);
  ui->spb_range_in1     = robtk_spin_new(0, 127, 1);

  ui->lbl_filter_index = robtk_lbl_new("Note");  
  robtk_spin_set_default(ui->spb_index_in0, 0);
  robtk_spin_set_default(ui->spb_index_in1, 127);
  robtk_spin_set_default(ui->spb_range_in0, 0);
  robtk_spin_set_default(ui->spb_range_in1, 127);

  ui->lbl_filter_range = robtk_lbl_new("Vel");
  robtk_spin_set_value(ui->spb_index_in0, 0);
  robtk_spin_set_value(ui->spb_index_in1, 127);
  robtk_spin_set_value(ui->spb_range_in0, 0);
  robtk_spin_set_value(ui->spb_range_in1, 127);

  robtk_spin_set_callback(ui->spb_index_in0, update_index_in0, ui);
  robtk_spin_set_callback(ui->spb_index_in1, update_index_in1, ui);
  robtk_spin_set_callback(ui->spb_range_in0, update_range_in0, ui);
  robtk_spin_set_callback(ui->spb_range_in1, update_range_in1, ui);

  ui->ctable_notefilter = rob_table_new(/*rows*/2, /*cols*/ 3, FALSE);

#define TBLFILTERADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable_notefilter, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLFILTERATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable_notefilter, WIDGET, X0, X1, Y0, Y1, 2, 2, XX, XY)

  int row = 0;
  TBLFILTERADD(robtk_lbl_widget(ui->lbl_filter_index), 0, 1, row, row+1);
  TBLFILTERADD(robtk_spin_widget(ui->spb_index_in0), 1, 2, row, row+1);
  TBLFILTERADD(robtk_spin_widget(ui->spb_index_in1), 2, 3, row, row+1);
  row++;
  TBLFILTERADD(robtk_lbl_widget(ui->lbl_filter_range), 0, 1, row, row+1);
  TBLFILTERADD(robtk_spin_widget(ui->spb_range_in0), 1, 2, row, row+1);
  TBLFILTERADD(robtk_spin_widget(ui->spb_range_in1), 2, 3, row, row+1);
  row++;
  TBLFILTERATT(robtk_sep_widget(ui->sep_inout[0]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 
  
  ui->lbl_outbox = robtk_lbl_new("Output");
  
  // Channel Out selectbox (combobox)

  ui->lbl_ch_out = robtk_lbl_new("Ch");
  ui->ch_out = robtk_select_new();
  for (int i = 0; i < 16; i++) {
        char int_str[16];
        sprintf(int_str, "%d", i + 1 );
        robtk_select_add_item(ui->ch_out, i, int_str);
    }
    
  robtk_select_set_item(ui->ch_out, 0);
  robtk_select_set_default_item(ui->ch_out, 0);
  robtk_select_set_callback(ui->ch_out, update_ch_out, ui);

// Transport controls

  // Host / Internal transport
  ui->btn_transport = robtk_cbtn_new("Host transport", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_transport, .2, .8, .5);
  robtk_cbtn_set_color_off(ui->btn_transport, .1, .1, .3);
  robtk_cbtn_set_callback(ui->btn_transport, update_transport, ui);

  robtk_cbtn_set_active(ui->btn_transport, true);
  
  ui->lbl_tempo = robtk_lbl_new("Tempo");
  ui->spb_tempo = robtk_spin_new(5, 200, 1);
  robtk_spin_set_value(ui->spb_tempo, 120);
  robtk_spin_set_callback(ui->spb_tempo, update_tempo, ui);

  /* LAYOUT */

  ui->in_out_box = rob_table_new(/*rows*/13, /*cols*/ 5, FALSE);

#define TBLADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->in_out_box, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->in_out_box, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  row = 0;
  // Input

  TBLADD(robtk_lbl_widget(ui->lbl_inbox), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_restartkbd), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_trigkbd), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_triglegato), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_select_widget(ui->ch_in), 0, 1, row, row+1);
  TBLADD(robtk_lbl_widget(ui->lbl_ch_in), 1, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_filter), 0, 2, row, row+1);
  row++;
  TBLADD(ui->ctable_notefilter, 0, 2, row, row+1);
  row++;
  TBLATT(robtk_sep_widget(ui->sep_inout[1]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 
  row++;
  
  // Output
  TBLADD(robtk_lbl_widget(ui->lbl_outbox), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_select_widget(ui->ch_out), 0, 1, row, row+1);
  TBLADD(robtk_lbl_widget(ui->lbl_ch_out), 1, 2, row, row+1);
  row++;
    TBLATT(robtk_sep_widget(ui->sep_inout[2]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 
  row++;
  // Transport
  TBLADD(robtk_cbtn_widget(ui->btn_transport), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_lbl_widget(ui->lbl_tempo), 0, 1, row, row+1);
  TBLADD(robtk_spin_widget(ui->spb_tempo), 1, 2, row, row+1);
  row++;
  
  TBLATT(robtk_sep_widget(ui->sep_inout[3]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 
}

static RobWidget * toplevel(QMidiArpArpUI* ui, void * const top)
{
  
  /* main widget: layout */
  ui->hbox = rob_hbox_new(FALSE, 2);
  robwidget_make_toplevel(ui->hbox, top);
  ROBWIDGET_SETNAME(ui->hbox, "QMidiArp Arp");

  /* setup UI */
  ui->darea = robwidget_new(ui);
  robwidget_set_alignment(ui->darea, 0, 0);
  robwidget_set_expose_event(ui->darea, expose_event);
  robwidget_set_size_request(ui->darea, size_request);
  
#ifdef LVGL_RESIZEABLE
  robwidget_set_size_allocate(ui->darea, size_allocate);
#endif


// Add in_out box
  in_out_box_new(ui);

// Control elements on the bottom

  //ui->lbl_pattern_text = robtk_lbl_new(ui->pattern);
  //ui->lbl_pattern_text->min_width_scaled = 300;
  //ui->lbl_pattern_text->w_width = 300;
  
  ui->lbl_repeat_mode = robtk_lbl_new("Repeat");

  ui->sel_repeat_mode = robtk_select_new();
  robtk_select_add_item(ui->sel_repeat_mode, 0, "Static");
  robtk_select_add_item(ui->sel_repeat_mode, 1, "Up");
  robtk_select_add_item(ui->sel_repeat_mode, 2, "Down");
  robtk_select_add_item(ui->sel_repeat_mode, 3, "Random");
  robtk_select_add_item(ui->sel_repeat_mode, 4, "As Played");

  robtk_select_set_item(ui->sel_repeat_mode, 1);
  robtk_select_set_default_item(ui->sel_repeat_mode, 1);
  robtk_select_set_callback(ui->sel_repeat_mode, update_repeat_mode, ui);

  ui->btn_mute = robtk_cbtn_new("Mute", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_mute, .8, .8, .2);
  robtk_cbtn_set_color_off(ui->btn_mute, .3, .3, .1);
  robtk_cbtn_set_callback(ui->btn_mute, update_mute, ui);
  
  ui->btn_defer = robtk_cbtn_new("D", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_defer, .2, .3, 1.);
  robtk_cbtn_set_color_off(ui->btn_defer, .1, .1, .3);
  robtk_cbtn_set_callback(ui->btn_defer, update_defer, ui);
  
  ui->btn_latch = robtk_cbtn_new("Latch", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_latch, .3, .8, .5);
  robtk_cbtn_set_color_off(ui->btn_latch, .1, .1, .1);
  robtk_cbtn_set_callback(ui->btn_latch, update_latch, ui);
  
  // Octave mode/high/low selectboxes (comboboxes)
  ui->lbl_oct_mode = robtk_lbl_new("Octave Mode and Range");
  
  ui->sel_oct_mode = robtk_select_new();
  robtk_select_add_item(ui->sel_oct_mode, 0, "Static");
  robtk_select_add_item(ui->sel_oct_mode, 1, "Up");
  robtk_select_add_item(ui->sel_oct_mode, 2, "Down");
  robtk_select_add_item(ui->sel_oct_mode, 3, "Bounce");

  robtk_select_set_item(ui->sel_oct_mode, 0);
  robtk_select_set_default_item(ui->sel_oct_mode, 0);
  robtk_select_set_callback(ui->sel_oct_mode, update_oct_mode, ui);

  ui->sel_oct_low = robtk_select_new();
  robtk_select_add_item(ui->sel_oct_low, 0, "-3");
  robtk_select_add_item(ui->sel_oct_low, 1, "-2");
  robtk_select_add_item(ui->sel_oct_low, 2, "-1");
  robtk_select_add_item(ui->sel_oct_low, 3,  "0");

  robtk_select_set_item(ui->sel_oct_low, 3);
  robtk_select_set_default_item(ui->sel_oct_low, 3);
  robtk_select_set_callback(ui->sel_oct_low, update_oct_low, ui);


  ui->sel_oct_high = robtk_select_new();
  robtk_select_add_item(ui->sel_oct_high, 0, "0");
  robtk_select_add_item(ui->sel_oct_high, 1, "1");
  robtk_select_add_item(ui->sel_oct_high, 2, "2");
  robtk_select_add_item(ui->sel_oct_high, 3, "3");

  robtk_select_set_item(ui->sel_oct_high, 0);
  robtk_select_set_default_item(ui->sel_oct_high, 0);
  robtk_select_set_callback(ui->sel_oct_high, update_oct_high, ui);


// Dial controls for Velocity, Notelength and Transpose

  ui->lbl_rnd_shift = robtk_lbl_new("Rnd Shift");
  ui->lbl_rnd_vel = robtk_lbl_new("Rnd Vel");
  ui->lbl_rnd_len = robtk_lbl_new("Rnd Len");
  ui->lbl_attack = robtk_lbl_new("Env Attack");
  ui->lbl_release = robtk_lbl_new("Env Release");

  int dial_defaults[5] = {0, 0, 0, 0, 0};
  
  for (int i = 0; i < 5; i++) {
    if (i < 3) {
      ui->dial_control[i] = robtk_dial_new_with_size(0, 100, 1,
                        75, 60, 40, 30, 25);
    }
    else {
      ui->dial_control[i] = robtk_dial_new_with_size(0, 20, 1,
                        75, 60, 40, 30, 25);
    }
    char int_str[16];
    sprintf(int_str, "%d", dial_defaults[i]);
    ui->dial_control_ann[i] = robtk_lbl_new(int_str);
    robtk_dial_set_value(ui->dial_control[i], dial_defaults[i]);
    ui->dial_control[i]->displaymode = 7;
    ui->dial_control[i]->dcol[2][0] = .3;
    ui->dial_control[i]->dcol[2][1] = .9;
    ui->dial_control[i]->dcol[2][2] = .6;

    ui->dial_control_box[i] = rob_vbox_new(FALSE, 4);
    rob_vbox_child_pack(ui->dial_control_box[i], robtk_lbl_widget(ui->dial_control_ann[i]), TRUE, TRUE);
    rob_vbox_child_pack(ui->dial_control_box[i], robtk_dial_widget(ui->dial_control[i]), TRUE, TRUE);
  }
  
  robtk_dial_set_callback(ui->dial_control[0], update_rnd_shift, ui);
  robtk_dial_set_callback(ui->dial_control[1], update_rnd_vel, ui);
  robtk_dial_set_callback(ui->dial_control[2], update_rnd_len, ui);
  robtk_dial_set_callback(ui->dial_control[3], update_attack, ui);
  robtk_dial_set_callback(ui->dial_control[4], update_release, ui);

  /* Various stretch separators */

  for (int i = 0; i < 5; i++) {
    ui->sep[i] = robtk_sep_new(TRUE);
    robtk_sep_set_linewidth(ui->sep[i], 0);
  }

  /* LAYOUT */

  /* Repeat and octave mode */

  ui->ctable_wave = rob_table_new(/*rows*/3, /*cols*/ 14, FALSE);

#define TBLWAVEADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable_wave, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_EXANDF)

#define TBLWAVEATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable_wave, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  int row = 0;
  TBLWAVEADD(robtk_lbl_widget(ui->lbl_repeat_mode), 6, 7, row, row+1);
  TBLWAVEADD(robtk_lbl_widget(ui->lbl_oct_mode), 8, 11, row, row+1);
  row++;
  TBLWAVEADD(robtk_cbtn_widget(ui->btn_mute), 0, 2, row, row+1);
  TBLWAVEADD(robtk_cbtn_widget(ui->btn_defer), 3, 4, row, row+1);
  TBLWAVEADD(robtk_cbtn_widget(ui->btn_latch), 4, 5, row, row+1);
  TBLWAVEADD(robtk_select_widget(ui->sel_repeat_mode), 6, 7, row, row+1);
  TBLWAVEADD(robtk_select_widget(ui->sel_oct_mode), 8, 9, row, row+1);
  TBLWAVEADD(robtk_select_widget(ui->sel_oct_low), 10, 11, row, row+1);
  TBLWAVEADD(robtk_select_widget(ui->sel_oct_high), 12, 13, row, row+1);
  
  row++;
  TBLWAVEATT(robtk_sep_widget(ui->sep[0]), 0, 9, row, row+1, RTK_EXANDF, RTK_EXANDF); 

  /* Spin buttons on the bottom */

  ui->ctable_spin = rob_table_new(/*rows*/3, /*cols*/ 10, FALSE);

#define TBLSPINADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable_spin, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLSPINATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable_spin, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  row = 0;
  TBLSPINADD(ui->dial_control_box[0], 0, 1, row, row+1);
  TBLSPINADD(ui->dial_control_box[1], 2, 3, row, row+1);
  TBLSPINADD(ui->dial_control_box[2], 4, 5, row, row+1);
  TBLSPINADD(ui->dial_control_box[3], 6, 7, row, row+1);
  TBLSPINADD(ui->dial_control_box[4], 8, 9, row, row+1);
  row++;
  TBLSPINADD(robtk_lbl_widget(ui->lbl_rnd_shift), 0, 1, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_rnd_vel), 2, 3, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_rnd_len), 4, 5, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_attack), 6, 7, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_release), 8, 9, row, row+1);
  row++;
  TBLSPINATT(robtk_sep_widget(ui->sep[1]), 0, 9, row, row+1, RTK_EXANDF, RTK_EXANDF); 

  /* main layout */
  
  // Workaround to ensure drawing of other widgets: extra hbox for the draw area 
  ui->hbox_da = rob_hbox_new(FALSE, 4);
  rob_vbox_child_pack(ui->hbox_da, ui->darea, TRUE, TRUE);
  
  ui->vbox = rob_vbox_new(FALSE, 4);
  rob_vbox_child_pack(ui->vbox, ui->hbox_da, TRUE, TRUE);
  //rob_vbox_child_pack(ui->vbox, ui->hbox_pattern, FALSE, FALSE);
  rob_vbox_child_pack(ui->vbox, ui->ctable_wave, FALSE, FALSE);
  rob_vbox_child_pack(ui->vbox, ui->ctable_spin, FALSE, FALSE);
  rob_hbox_child_pack(ui->vbox, robtk_sep_widget(ui->sep[2]), TRUE, TRUE);

  rob_hbox_child_pack(ui->hbox, ui->vbox, TRUE, TRUE);
  rob_hbox_child_pack(ui->hbox, ui->in_out_box, FALSE, FALSE);
  
  return ui->hbox;
}

/******************************************************************************
 * LV2
 */

static LV2UI_Handle
instantiate(
    void* const               ui_toplevel,
    const LV2UI_Descriptor*   descriptor,
    const char*               plugin_uri,
    const char*               bundle_path,
    LV2UI_Write_Function      write_function,
    LV2UI_Controller          controller,
    RobWidget**               widget,
    const LV2_Feature* const* features)
{
  QMidiArpArpUI* ui = (QMidiArpArpUI*)calloc(1, sizeof(QMidiArpArpUI));

  if (!ui) {
    fprintf(stderr, "UI: out of memory\n");
    return NULL;
  }

  *widget = NULL;
  
/* Drawing parameters */
  ui->maxOctave = 0;
  ui->minOctave = 0;
  ui->nSteps = 1.0;
  ui->minStepWidth = 1.0;
  ui->patternMaxIndex = 0;
  ui->patternLen = 0;
  
/* Default pattern */
  ui->pattern = (char *) malloc(PATTERNBUFSIZE);
  ui->pattern[0] = '\0';
  strncat(ui->pattern, ">>0", PATTERNBUFSIZE - 1);
  ui->patternLen = strlen(ui->pattern);
  
  
  ui->currentIndex = 0;
  ui->isMuted = false;
  
  
  ui->uiIsUp = false;
  ui->draw_only_cursor = false;
  ui->receivedPatternOnce = false;
  ui->receivePatternFlag = false;


  for (int i = 0; features[i]; ++i) {
    if (!strcmp(features[i]->URI, LV2_URID_URI "#map")) {
      ui->map = (LV2_URID_Map*)features[i]->data;
    }
  }

  if (!ui->map) {
    fprintf(stderr, "UI: Host does not support urid:map\n");
    free(ui);
    return NULL;
  }


#ifdef LVGL_RESIZEABLE
  ui->w_width = 520;
  ui->w_height = ARPSCR_MIN_H + CSR_MIN_H + CSR_VMARG;
#else
  ui->w_height = 500;
#endif

  /* initialize private data structure */
  ui->write      = write_function;
  ui->controller = controller;

  ui->error      = false;

  map_uris(ui->map, &ui->uris);
  
  lv2_atom_forge_init(&ui->forge, ui->map);

  *widget = toplevel(ui, ui_toplevel);

  /* On Screen Display -- annotations */
  ui->font[0] = pango_font_description_from_string("Mono 9");
  ui->font[1] = pango_font_description_from_string("Sans 10");
  ui->font[2] = pango_font_description_from_string("Sans 8px");
  ui->font[3] = pango_font_description_from_string("Mono 8");

  
  //create cairo draw area for sequence and cursor;
  cairo_t *cr;
  if (!ui->gridnlabels) {
    ui->gridnlabels = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
    DAWIDTH, DAHEIGHT);
  }
  cr = cairo_create(ui->gridnlabels);
  CairoSetSouerceRGBA(color_blk);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_rectangle (cr, 0, 0, DAWIDTH, DAHEIGHT);
  cairo_fill (cr);
  cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
  cairo_destroy(cr);

  /* send message to DSP backend:
   * enable message transmission & request state
   */
  ui_enable(ui);
  
  robwidget_hide(ui->ctable_notefilter, FALSE);

  return ui;
}

static enum LVGLResize
plugin_scale_mode(LV2UI_Handle handle)
{
  return LVGL_LAYOUT_TO_FIT;
}

static void
in_out_box_destroy(QMidiArpArpUI *ui) 
{
  robtk_cbtn_destroy(ui->btn_trigkbd);
  robtk_cbtn_destroy(ui->btn_restartkbd);
  robtk_cbtn_destroy(ui->btn_triglegato);

  robtk_select_destroy(ui->ch_in);
  robtk_select_destroy(ui->ch_out);

  robtk_spin_destroy(ui->spb_index_in0);
  robtk_spin_destroy(ui->spb_index_in1);
  robtk_spin_destroy(ui->spb_range_in0);
  robtk_spin_destroy(ui->spb_range_in1);
  robtk_cbtn_destroy(ui->btn_filter);
  robtk_lbl_destroy(ui->lbl_filter_index);
  robtk_lbl_destroy(ui->lbl_filter_range);
  rob_table_destroy(ui->ctable_notefilter);

  for (int i = 0; i < 4; i++) {
    robtk_sep_destroy(ui->sep_inout[i]);
  }
  robtk_lbl_destroy(ui->lbl_inbox);
  robtk_lbl_destroy(ui->lbl_outbox);
  robtk_lbl_destroy(ui->lbl_ch_in);
  robtk_lbl_destroy(ui->lbl_ch_out);

  robtk_cbtn_destroy(ui->btn_transport);
  robtk_spin_destroy(ui->spb_tempo);
  robtk_lbl_destroy(ui->lbl_tempo);

  rob_table_destroy(ui->in_out_box);
}

static void
cleanup(LV2UI_Handle handle)
{
  QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;
  /* send message to DSP backend:
   * disable message transmission
   */
  ui_disable(ui);
  cairo_surface_destroy(ui->gridnlabels);
  pango_font_description_free(ui->font[0]);
  pango_font_description_free(ui->font[1]);
  pango_font_description_free(ui->font[2]);
  pango_font_description_free(ui->font[3]);
  

  robtk_lbl_destroy(ui->lbl_attack);
  robtk_lbl_destroy(ui->lbl_release);
  robtk_lbl_destroy(ui->lbl_rnd_len);
  robtk_lbl_destroy(ui->lbl_rnd_vel);
  robtk_lbl_destroy(ui->lbl_rnd_shift);
  
  robtk_lbl_destroy(ui->lbl_oct_mode);
  robtk_lbl_destroy(ui->lbl_repeat_mode);
  
  robtk_select_destroy(ui->sel_oct_mode);
  robtk_select_destroy(ui->sel_oct_low);
  robtk_select_destroy(ui->sel_oct_high);

  robtk_cbtn_destroy(ui->btn_latch);
  

  robtk_cbtn_destroy(ui->btn_mute);
  robtk_cbtn_destroy(ui->btn_defer);
  
  
  for (int i = 0; i < 5; i++) {
    robtk_dial_destroy(ui->dial_control[i]);
    robtk_lbl_destroy(ui->dial_control_ann[i]);
    rob_box_destroy(ui->dial_control_box[i]);
  }

  for (int i = 0; i < 5; i++) {
    robtk_sep_destroy(ui->sep[i]);
  }
  
  rob_table_destroy(ui->ctable_wave);
  rob_table_destroy(ui->ctable_spin);
  

// In Out box
  in_out_box_destroy(ui);

  robwidget_destroy(ui->darea);
  
  rob_box_destroy(ui->hbox_da);
  //rob_box_destroy(ui->hbox_pattern);
  rob_box_destroy(ui->vbox);
  rob_box_destroy(ui->hbox);

  free(ui);
}

/** receive data from the DSP-backend.
 *
 * this callback runs in the "communication" thread of the LV2-host
 * jalv and ardour do this via a g_timeout() function at ~25fps
 * g_timeout is the same thread a the UI display (no locking is needed)
 * but the openGL version does not use lv2idle and requires locking.
 *
 * the atom-events from the DSP backend are written into a ringbuffer
 * in the host (in the DSP|jack realtime thread) the host then
 * empties this ringbuffer by sending port_event()s to the UI at some
 * random time later.  When CPU and DSP load are large the host-buffer
 * may overflow and some events may get lost.
 *
 * This thread does is not [usually] the 'drawing' thread (it does not
 * have X11 or gl context).
 */
static void
port_event(LV2UI_Handle handle,
           uint32_t     port_index,
           uint32_t     buffer_size,
           uint32_t     format,
           const void*  buffer)
{
  QMidiArpArpUI* ui = (QMidiArpArpUI*)handle;
  LV2_Atom* atom = (LV2_Atom*)buffer;
  const QMidiArpURIs* uris = &ui->uris;

  /* check type of data received
   *  format == 0: [float] control-port event
   *  format > 0: message
   *  Every event message is sent as separate port-event
   */

    if (format == uris->atom_eventTransfer
      && (atom->type == uris->atom_Object) ) {
        receivePattern(handle, atom);
        updatePattern(handle);
    }
    else if (format == 0 && buffer_size == sizeof(float)) {

        float fValue = *(float *) buffer;
        // printf("port event index %d  -  value %f\n", port_index, fValue);
        switch (port_index) {
          case PATTERN_PRESET:
                  //patternPresetBox->setCurrentIndex(fValue);
                  //updatePattern(patternPresets.at(fValue));
          break;
          case RPATTERNFLAG:
                  //~ if ((int)fValue != receivePatternFlag) {
                      //~ receivePatternFlag = (int)fValue;
                  //~ }
          break;
          case ATTACK:
                  robtk_dial_set_value(ui->dial_control[0], (int)fValue);
          break;
          case RELEASE:
                  robtk_dial_set_value(ui->dial_control[1], (int)fValue);
          break;
          case RANDOM_TICK:
                  robtk_dial_set_value(ui->dial_control[2], (int)fValue);
          break;
          case RANDOM_LEN:
                  robtk_dial_set_value(ui->dial_control[3], (int)fValue);
          break;
          case RANDOM_VEL:
                  robtk_dial_set_value(ui->dial_control[4], (int)fValue);
          break;
          case OCTAVE_MODE:
                  robtk_select_set_item(ui->sel_oct_mode, (int)fValue);
          break;
          case OCTAVE_LOW:
                  robtk_select_set_item(ui->sel_oct_low, (int)fValue + 3);
          break;
          case REPEAT_MODE:
                  robtk_select_set_item(ui->sel_repeat_mode, (int)fValue);
          break;
          case OCTAVE_HIGH:
                  robtk_select_set_item(ui->sel_oct_high, (int)fValue);
          break;
          case LATCH_MODE:
                  robtk_cbtn_set_active(ui->btn_latch, (fValue > 0));
          break;
          case CH_OUT:
                  robtk_select_set_item(ui->ch_out, (int)fValue);
          break;
          case CH_IN:
                  robtk_select_set_item(ui->ch_in, (int)fValue);
          break;
          case INDEX_IN1:
                  robtk_spin_set_value(ui->spb_index_in0, (int)fValue);
          break;
          case INDEX_IN2:
                  robtk_spin_set_value(ui->spb_index_in1, (int)fValue);
          break;
          case RANGE_IN1:
                  robtk_spin_set_value(ui->spb_range_in0, (int)fValue);
          break;
          case RANGE_IN2:
                  robtk_spin_set_value(ui->spb_range_in1, (int)fValue);
          break;
          case CURSOR_POS:
                  if (ui->currentIndex != (int)fValue) {
                    ui->currentIndex = (int)fValue;
                    queue_draw(ui->darea);
                  }
          break;
          case MUTE:
                  if (ui->isMuted != (int)fValue) {
                    ui->isMuted = (int)fValue;
                    queue_draw(ui->darea);
                    robtk_cbtn_set_active(ui->btn_mute, (fValue > 0));
                  }
          break;
          case ENABLE_RESTARTBYKBD:
                  robtk_cbtn_set_active(ui->btn_restartkbd, (fValue > 0));
          break;
          case ENABLE_TRIGBYKBD:
                  robtk_cbtn_set_active(ui->btn_trigkbd, (fValue > 0));
          break;
          case ENABLE_TRIGLEGATO:
                  robtk_cbtn_set_active(ui->btn_triglegato, (fValue > 0));
          break;
          case DEFER:
                  robtk_cbtn_set_active(ui->btn_defer, (fValue > 0));
          break;
          case TRANSPORT_MODE:
                  robtk_cbtn_set_active(ui->btn_transport, (fValue > 0));
          break;
          case TEMPO:
                  robtk_spin_set_value(ui->spb_tempo, (int)fValue);
          break;
          default:
          break;
        }
    }
}

static const void*
extension_data(const char* uri)
{
  return NULL;
}
