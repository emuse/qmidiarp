/*!
 * @file lfowidget_lv2_rtk.c
 * @brief Implementation of the RobTk-based LV2 GUI for the QMidiArp Seq plugin.
 *
 *
 *      Copyright 2009 - 2025 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      The structure as well as some lines of code and functions were 
 *      taken from sisco lv2 by Robin Garaeus: https://github.com/x42/sisco.lv2 
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
#include "lfowidget_lv2_rtk.h"
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
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;    
    ui->write(ui->controller, index, sizeof(float), 0, &fValue);
}

void sendFlipWaveVertical(LV2UI_Handle handle)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;
    const QMidiArpURIs* uris = &ui->uris;
    uint8_t obj_buf[64];
    int state;

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&ui->forge, 0);

    /* prepare forge buffer and initialize atom-sequence */
    lv2_atom_forge_set_buffer(&ui->forge, obj_buf, 16);

    state = uris->flip_wave;

    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&ui->forge, &frame, 1, state);

    /* close-off frame */
    lv2_atom_forge_pop(&ui->forge, &frame);
    ui->write(ui->controller, MidiIn, lv2_atom_total_size(msg), ui->uris.atom_eventTransfer, msg);
    if (robtk_select_get_item(ui->waveform) != 5) ui->copiedToCustomFlag = true;
}

void receiveWave(LV2UI_Handle handle, LV2_Atom* atom)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;
    const QMidiArpURIs* uris = &ui->uris;
    
    if ( (atom->type != uris->atom_Blank) 
            && (atom->type != uris->atom_Object)) {
              return;
            }

    /* cast the buffer to Atom Object */
    LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
    LV2_Atom *a0 = NULL;
    lv2_atom_object_get(obj, uris->hex_customwave, &a0, NULL);
    if (obj->body.otype != uris->hex_customwave) return;

    /* handle wave' data vector */
    LV2_Atom_Vector* voi = (LV2_Atom_Vector*)LV2_ATOM_BODY(a0);
    /* check if atom is indeed a vector of the expected type*/
    if (voi->atom.type != uris->atom_Int) return;

    /* get number of elements in vector
    * = (raw 8bit data-length - header-length) / sizeof(expected data type:int) */
    const size_t n_elem = (a0->size - sizeof(LV2_Atom_Vector_Body)) / voi->atom.size;
    /* typecast, dereference pointer to vector */
    const int *recdata = (int*) LV2_ATOM_BODY(&voi->atom);
    
    int ofs = 127;
    for (uint32_t l1 = 0; l1 < n_elem; l1++) {
        ui->datavalues[l1] = recdata[l1];
        ui->dataticks[l1] = l1 * TPQN / ui->res;
        if ((l1 < n_elem - 1) && (recdata[l1] > -1) && (recdata[l1] < ofs)) {
          ofs = recdata[l1];
        }

    }
    ui->data_count = n_elem;

    if (ui->copiedToCustomFlag) {
        robtk_select_set_item(ui->waveform, 5);
        updateParam(ui, WAVEFORM, 5);
        ui->copiedToCustomFlag = false;
    }
    if (ofs != robtk_dial_get_value(ui->dial_control[1])) {
        ui->offset_suppress_callback = true;
        robtk_dial_set_value(ui->dial_control[1], ofs);
        ui->offset_suppress_callback = false;
    }
}

static void 
ui_state(LV2UI_Handle handle)
{
  (void)handle;
}

/** notfiy backend that UI is closed */
static void ui_disable(LV2UI_Handle handle)
{
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;
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

static bool update_noteoff (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, ENABLE_NOTEOFF, robtk_cbtn_get_active(ui->btn_noteoff));
    return TRUE;
}

static bool update_trigkbd (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, ENABLE_TRIGBYKBD, robtk_cbtn_get_active(ui->btn_trigkbd));
    return TRUE;
}

static bool update_restartkbd (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, ENABLE_RESTARTBYKBD, robtk_cbtn_get_active(ui->btn_restartkbd));
    return TRUE;

}

static bool update_triglegato (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, ENABLE_TRIGLEGATO, robtk_cbtn_get_active(ui->btn_triglegato));
    return TRUE;
}

static bool update_notefilter (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
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
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, INDEX_IN1, robtk_spin_get_value(ui->spb_index_in0));
    return TRUE;

}

static bool update_index_in1 (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, INDEX_IN2, robtk_spin_get_value(ui->spb_index_in1));
    return TRUE;

}

static bool update_range_in0 (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, RANGE_IN1, robtk_spin_get_value(ui->spb_range_in0));
    return TRUE;

}

static bool update_range_in1 (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, RANGE_IN2, robtk_spin_get_value(ui->spb_range_in1));
    return TRUE;

}

static bool update_transport (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, TRANSPORT_MODE, robtk_cbtn_get_active(ui->btn_transport));
    return TRUE;
}

static bool update_tempo (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, TEMPO, robtk_spin_get_value(ui->spb_tempo));
    return TRUE;
}


static bool update_ch_in (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, CH_IN, robtk_select_get_item(ui->ch_in));
    return TRUE;
}

static bool update_ch_out (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, CH_OUT, robtk_select_get_item(ui->ch_out));
    return TRUE;
}

static bool update_cc_in (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, CC_IN, robtk_select_get_item(ui->cc_in));
    return TRUE;
}

static bool update_cc_out (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, CC_OUT, robtk_select_get_item(ui->cc_out));
    return TRUE;
}
/* end common update functions */


static bool update_amplitude (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, AMPLITUDE, robtk_dial_get_value(ui->dial_control[0]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[0]));
    robtk_lbl_set_text(ui->dial_control_ann[0], txt);
    return TRUE;

}

static bool update_offset (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[1]));
    robtk_lbl_set_text(ui->dial_control_ann[1], txt);
    if (ui->offset_suppress_callback) return FALSE;

    updateParam(ui, OFFSET, robtk_dial_get_value(ui->dial_control[1]));
    return TRUE;

}

static bool update_phase (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, PHASE, robtk_dial_get_value(ui->dial_control[2]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[2]));
    robtk_lbl_set_text(ui->dial_control_ann[2], txt);
    return TRUE;

}

static bool update_res (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    uint8_t val = robtk_select_get_item(ui->res_box);
    if (val >= sizeof(lfoResValues)/sizeof(lfoResValues[0]))
      return TRUE;

    ui->res = lfoResValues[val];
    updateParam(ui, RESOLUTION, val);
    return TRUE;

}

static bool update_size (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    uint8_t val = robtk_select_get_item(ui->size_box);
    if (val >= sizeof(lfoSizeValues)/sizeof(lfoSizeValues[0]))
      return TRUE;

    ui->size = lfoSizeValues[val];
    updateParam(ui, SIZE, val);
    return TRUE;

}

static bool update_mute (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, MUTE, robtk_cbtn_get_active(ui->btn_mute));
    ui->isMuted = robtk_cbtn_get_active(ui->btn_mute);
    queue_draw(ui->darea);
    return TRUE;
}

static bool update_record (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    
    robtk_select_set_item(ui->waveform, 5);
    ui->recordMode = robtk_cbtn_get_active(ui->btn_record);
    updateParam(ui, RECORD, robtk_cbtn_get_active(ui->btn_record));
    return TRUE;
}

static bool update_defer (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, DEFER, robtk_cbtn_get_active(ui->btn_defer));
    return TRUE;
}

static bool update_loop_mode (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, LOOPMODE, robtk_select_get_item(ui->loop_mode));
    return TRUE;
}

static bool update_flip_wave (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    sendFlipWaveVertical(ui);
    return TRUE;
}

static bool update_waveform (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    if (robtk_select_get_item(ui->waveform) == 5) {
      robtk_dial_set_sensitive(ui->dial_control[0], false);
      robtk_dial_set_sensitive(ui->dial_control[2], false);
      robtk_select_set_sensitive(ui->sel_freq, false);
    }
    else {
      robtk_dial_set_sensitive(ui->dial_control[0], true);
      robtk_dial_set_sensitive(ui->dial_control[2], true);
      robtk_select_set_sensitive(ui->sel_freq, true);
    }
      
    updateParam(ui, WAVEFORM, robtk_select_get_item(ui->waveform));
    return TRUE;
}

static bool update_freq (RobWidget *widget, void* data)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) data;
    updateParam(ui, FREQUENCY, robtk_select_get_item(ui->sel_freq));
    return TRUE;
}

static void update_mouse(RobWidget* handle) {  
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*) GET_HANDLE(handle);

  if (ui->mouse_x <  -0.05 || ui->mouse_x >  1.05 ||
      ui->mouse_y <  -0.2 || ui->mouse_y >  1.05 ) {
    ui->mouse_pressed = 2;
    ui->mouse_buttons = 0;
  }
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*) GET_HANDLE(handle);
  
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*) GET_HANDLE(handle);
  ui->mouse_pressed = 2;
  ui->mouse_buttons = 0;
  update_mouse(handle);
  return NULL;
}

static RobWidget* mouse_move(RobWidget* handle, RobTkBtnEvent *ev) {
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*) GET_HANDLE(handle);
  
  int w = DAWIDTH;
  int h = LFOSCR_MIN_H;

  //printf("UI Mouse move %d1 %d2 \n", ev->x, ev->y);
  
  ui->mouse_x = ((double)ev->x - LFOSCR_HMARG) / (w - 2 * LFOSCR_HMARG);
  ui->mouse_y = 1. - ((double)ev->y - LFOSCR_VMARG) /
                (h - 2 * LFOSCR_VMARG);

  //printf("Relative coords %f1 %f2 \n", ui->mouse_x, ui->mouse_y);

  if (ui->mouse_buttons > 0) {
    update_mouse(handle);
    return handle;
  }
  return handle;
}

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

static void expose_cursor(RobWidget* handle, cairo_t* cr)
{
    QMidiArpLfoUI* ui = (QMidiArpLfoUI*) GET_HANDLE(handle);
    const int w = DAWIDTH;
    const int h = DAHEIGHT;
    const int nPoints = ui->data_count -1;
    const int xscale = (w - 2 * CSR_HMARG);
    const int streak_length = (int)(xscale / nPoints) > 4 ? (int)(xscale / nPoints) : 4;
    const int notestreak_thick = 2;
    const int ypos =  h - LFOSCR_VMARG - CSR_MIN_H + CSR_VMARG;
    int xpos;
    int x;

    cairo_set_line_width(cr, notestreak_thick * 2);
    cairo_rectangle (cr, 0, LFOSCR_MIN_H - LFOSCR_VMARG + CSR_VMARG, 
                        DAWIDTH, CSR_MIN_H);   
    cairo_clip(cr);
    cairo_set_source_rgba(cr, 50./256, 10./256, 10./256, 1);
    cairo_move_to(cr, CSR_HMARG, ypos);
    cairo_line_to(cr, w - CSR_HMARG, ypos);
    cairo_stroke(cr);

    
    // Cursor
    cairo_set_source_rgba (cr, 180./256, 130./256, 50./256, 1);
    x = ui->currentIndex * xscale / nPoints;
    xpos = CSR_HMARG + x;
    cairo_move_to(cr, xpos, ypos);
    cairo_line_to(cr, xpos + streak_length, ypos);
    cairo_stroke(cr);
}

/* gdk drawing area draw callback
 * -- this runs in gtk's main thread */
static bool expose_event(RobWidget* handle, cairo_t* cr, cairo_rectangle_t *ev)
{
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*) GET_HANDLE(handle);

  /* limit cairo-drawing to widget */
  cairo_rectangle (cr, 0, 0, DAWIDTH, DAHEIGHT);
  cairo_clip(cr);

  /* limit cairo-drawing to exposed area */
  cairo_rectangle (cr, ev->x, ev->y, ev->width, ev->height);
  cairo_clip(cr);
  cairo_set_source_surface(cr, ui->gridnlabels, 0, 0);
  if (!ui->draw_only_cursor) {
    if (ui->isMuted) {
      cairo_set_source_rgba(cr, 70./256, 70./256, 70./256, 1);
    }
    else {
      cairo_set_source_rgba(cr, 50./256, 10./256, 10./256, 1);
    }
    cairo_paint (cr);

    cairo_save(cr);
    /* limit cairo-drawing to scope-area */
    cairo_rectangle (cr, 0, 0, DAWIDTH, DAHEIGHT);
    cairo_clip(cr);
  
    cairo_set_line_width(cr, 1.0);
  
    //QMidiarp Seq screen
    int beat = 4;
    int tmpval = 0;
    int ypos, xpos, yscale;
    double xscale;
    
    int w = DAWIDTH;
    int h = LFOSCR_MIN_H;
    int ofs;
    int x, x1;
    int notestreak_thick = 2;

    // Not in use with LV2 plugin
    double grooveTick = 0;
  
    // Text colors
    const float color_txt2[4] = {180./256, 130./256, 50./256, 1};
    const float color_txt3[4] = {180./256, 130./256, 50./256, 1};
    
    if (ui->data_count < 1) 
      return FALSE;
    
    //Grid setup
    int nsteps = (int)( (double)ui->dataticks[ui->data_count - 1] / TPQN + .5);
    if (!nsteps) nsteps = 1;
    int beatRes = (ui->data_count - 1) / nsteps;
    int beatDiv = ((ui->data_count - 1) > 64) ? 64 / nsteps : beatRes;
    int npoints = beatRes * nsteps;
    xscale = (w - 2 * LFOSCR_HMARG);
    yscale = h - 2 * LFOSCR_VMARG;
  
    //Determine font geometry
    int tx_w, tx_h;
    get_text_geometry("1", ui->font[2], &tx_w, &tx_h);
      
    //Beat separators
    cairo_set_source_rgba (cr, 160./256, 20./256, 20./256, .3);
    for (int l1 = 0; l1 < nsteps + 1; l1++) {
      if (l1 < 10) {
        ofs = w / nsteps * .5 - 4 + LFOSCR_HMARG;
      } else {
        ofs = w / nsteps * .5 - 6 + LFOSCR_HMARG;
      }
      if ((bool)(l1%beat)) {
        cairo_set_source_rgba (cr, 180./256, 100./256, 60./256, 1);
      } else {
        cairo_set_source_rgba (cr, 180./256, 100./256, 100./256, 1);
      }
      x = l1 * xscale / nsteps;
      cairo_move_to(cr, LFOSCR_HMARG + x, LFOSCR_VMARG);
      cairo_line_to(cr, LFOSCR_HMARG + x, h - LFOSCR_VMARG);
      cairo_stroke(cr);
  
      if (l1 < nsteps) {
        //Beat numbers          
        if ((nsteps < 32) || !((l1 + 5) % 4)) {
          char int_str[20];
          sprintf(int_str, "%d", l1+1);
          render_text(cr, int_str, ui->font[2], ofs + x + tx_w / 2, 
                      LFOSCR_VMARG - tx_h / 2, 0, 1, color_txt2);
          // Beat divisor separators
          cairo_set_source_rgba (cr, 120./256, 60./256, 20./256, 1);
          for (int l2 = 1; l2 < beatDiv; l2++) {
            x1 = x + l2 * xscale / nsteps / beatDiv;
            if (x1 < xscale) {
              cairo_move_to(cr, LFOSCR_HMARG + x1, LFOSCR_VMARG);
              cairo_line_to(cr, LFOSCR_HMARG + x1, h - LFOSCR_VMARG);
              cairo_stroke(cr);
            }
          }
        }
      }
      ui->xMax = LFOSCR_HMARG + x;
    }
    
    //Draw function
  
    cairo_set_line_width(cr, notestreak_thick);
  
    int l1 = 0;
    while (l1 < npoints) {
      x = (l1 + .01 * (double)grooveTick * (l1 % 2)) * xscale / npoints;
      tmpval = ui->datavalues[l1];
      ypos = yscale - yscale * abs(tmpval) / 128 + LFOSCR_VMARG;
      xpos = LFOSCR_HMARG + x;
      if (tmpval < 0) {
        cairo_set_source_rgba (cr, 100./256, 40./256, 5./256, 1);
      }
      else {
        cairo_set_source_rgba (cr, 180./256, 130./256, 50./256, 1);
      }
      cairo_move_to(cr, xpos, ypos);
      cairo_line_to(cr, xpos + (xscale / nsteps / beatRes), ypos);
      cairo_stroke(cr);
      l1++;
      l1+=npoints/(TPQN*4);
    }
    
    //Horizontal separators and numbers
    cairo_set_line_width(cr, 1);
    for (int l1 = 0; l1 <3; l1++) {    
      ypos = yscale * l1 / 2 + LFOSCR_VMARG;
  
      cairo_set_source_rgba (cr, 180./256, 120./256, 40./256, 1);
      cairo_move_to(cr, LFOSCR_HMARG, ypos);
      cairo_line_to(cr, ui->xMax, ypos);
      cairo_stroke(cr);
      char int_str[20];
      sprintf(int_str, "%d", (2 - l1) * 64);
      render_text(cr, int_str, ui->font[2], LFOSCR_HMARG - 6 + tx_w / 2,
              yscale * l1 / 2 + LFOSCR_VMARG + 8 - tx_h / 2, 0, 
              1, color_txt3);
    }
    cairo_restore(cr);
  }
  expose_cursor(handle, cr);
  ui->draw_only_cursor = false;
  
  return TRUE;
}

/******************************************************************************
 * RobWidget
 */

#ifdef LVGL_RESIZEABLE
static void
size_request(RobWidget* handle, int *w, int *h) {
  (void)handle;
  *w = 520;
  *h = LFOSCR_MIN_H + CSR_MIN_H + CSR_VMARG;
}

static void
size_allocate(RobWidget* handle, int w, int h) {
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)GET_HANDLE(handle);
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)GET_HANDLE(handle);
  *w = DAWIDTH;
  //*w = 720;
  *h = LFOSCR_MIN_H + CSR_MIN_H + CSR_VMARG; // 254;
}

#endif

#define robtk_dial_new_narrow(min, max, step) \
  robtk_dial_new_with_size(min, max, step, \
      GSP_WIDTH, GSP_HEIGHT, GSP_CX, GSP_CY, GSP_RADIUS)


static void in_out_box_new(QMidiArpLfoUI* ui)
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

  robtk_cbtn_set_color_on(ui->btn_restartkbd, .7, .5, .2);
  robtk_cbtn_set_color_off(ui->btn_restartkbd, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_trigkbd, .7, .5, .2);
  robtk_cbtn_set_color_off(ui->btn_trigkbd, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_triglegato, .7, .5, .2);
  robtk_cbtn_set_color_off(ui->btn_triglegato, .1, .1, .3);


// Additions to in_out_box elements on the right only LFO and SEQ
  ui->btn_noteoff = robtk_cbtn_new("Note Off", GBT_LED_LEFT, true);
  robtk_cbtn_set_color_on(ui->btn_noteoff, .7, .5, .2);
  robtk_cbtn_set_color_off(ui->btn_noteoff, .1, .1, .3);

  robtk_cbtn_set_callback(ui->btn_noteoff, update_noteoff, ui);
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

  ui->lbl_cc_in = robtk_lbl_new("MIDI CC");
  ui->cc_in = robtk_select_new();
  for (int i = 0; i < 127; i++) {
        char int_str[16];
        sprintf(int_str, "%d", i );
        robtk_select_add_item(ui->cc_in, i, int_str);
    }
    
  robtk_select_set_item(ui->cc_in, 74);
  robtk_select_set_default_item(ui->cc_in, 74);
  robtk_select_set_callback(ui->cc_in, update_cc_in, ui);

  // Note filter button and select boxes
  ui->btn_filter = robtk_cbtn_new("Note Filter", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_filter, .2, .3, 1.);
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
  
  // Channel and CC Out selectbox (combobox)


  ui->lbl_cc_out = robtk_lbl_new("MIDI CC");
  ui->cc_out = robtk_select_new();
  for (int i = 0; i < 127; i++) {
        char int_str[16];
        sprintf(int_str, "%d", i );
        robtk_select_add_item(ui->cc_out, i, int_str);
    }
    
  robtk_select_set_item(ui->cc_out, 74);
  robtk_select_set_default_item(ui->cc_out, 74);
  robtk_select_set_callback(ui->cc_out, update_cc_out, ui);

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
  robtk_cbtn_set_color_on(ui->btn_transport, .7, .5, .2);
  robtk_cbtn_set_color_off(ui->btn_transport, .1, .1, .3);
  robtk_cbtn_set_callback(ui->btn_transport, update_transport, ui);

  robtk_cbtn_set_active(ui->btn_transport, true);
  
  ui->lbl_tempo = robtk_lbl_new("Tempo");
  ui->spb_tempo = robtk_spin_new(5, 200, 1);
  robtk_spin_set_value(ui->spb_tempo, 120);
  robtk_spin_set_callback(ui->spb_tempo, update_tempo, ui);

  /* LAYOUT */

  ui->in_out_box = rob_table_new(/*rows*/16, /*cols*/ 5, FALSE);

#define TBLADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->in_out_box, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->in_out_box, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  row = 0;
  // Input

  TBLADD(robtk_lbl_widget(ui->lbl_inbox), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_noteoff), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_restartkbd), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_trigkbd), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_triglegato), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_select_widget(ui->cc_in), 0, 1, row, row+1);
  TBLADD(robtk_lbl_widget(ui->lbl_cc_in), 1, 2, row, row+1);
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
  TBLADD(robtk_select_widget(ui->cc_out), 0, 1, row, row+1);
  TBLADD(robtk_lbl_widget(ui->lbl_cc_out), 1, 2, row, row+1);
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

static RobWidget * toplevel(QMidiArpLfoUI* ui, void * const top)
{
  
  /* main widget: layout */
  ui->hbox = rob_hbox_new(FALSE, 2);
  robwidget_make_toplevel(ui->hbox, top);
  ROBWIDGET_SETNAME(ui->hbox, "QMidiArp LFO");

  /* setup UI */
  ui->darea = robwidget_new(ui);
  robwidget_set_alignment(ui->darea, 0, 0);
  robwidget_set_expose_event(ui->darea, expose_event);
  robwidget_set_size_request(ui->darea, size_request);
  
#ifdef LVGL_RESIZEABLE
  robwidget_set_size_allocate(ui->darea, size_allocate);
#endif

  robwidget_set_mousedown(ui->darea, mouse_down);
  robwidget_set_mousemove(ui->darea, mouse_move);
  robwidget_set_mouseup  (ui->darea, mouse_up);

// Add in_out box
  in_out_box_new(ui);

// Control elements on the bottom

  ui->loop_mode = robtk_select_new();
  robtk_select_add_item(ui->loop_mode, 0, "->_>");
  robtk_select_add_item(ui->loop_mode, 1, " <_<-");
  robtk_select_add_item(ui->loop_mode, 2, "->_<");
  robtk_select_add_item(ui->loop_mode, 3, " >_<-");
  robtk_select_add_item(ui->loop_mode, 4, "->_|");
  robtk_select_add_item(ui->loop_mode, 5, " |_<-");
  robtk_select_add_item(ui->loop_mode, 6, "RANDM");

  robtk_select_set_item(ui->loop_mode, 0);
  robtk_select_set_default_item(ui->loop_mode, 0);
  robtk_select_set_callback(ui->loop_mode, update_loop_mode, ui);

  ui->btn_mute = robtk_cbtn_new("Mute", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_mute, .8, .8, .2);
  robtk_cbtn_set_color_off(ui->btn_mute, .3, .3, .1);
  robtk_cbtn_set_callback(ui->btn_mute, update_mute, ui);
  
  ui->btn_defer = robtk_cbtn_new("D", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_defer, .2, .3, 1.);
  robtk_cbtn_set_color_off(ui->btn_defer, .1, .1, .3);
  robtk_cbtn_set_callback(ui->btn_defer, update_defer, ui);
  
  ui->btn_record = robtk_cbtn_new("Record", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_record, .8, .2, .2);
  robtk_cbtn_set_color_off(ui->btn_record, .3, .1, .1);
  robtk_cbtn_set_callback(ui->btn_record, update_record, ui);
  
  // Resolution and Size selectboxes (comboboxes)
  ui->lbl_res = robtk_lbl_new("Resolution");
  ui->res_box = robtk_select_new();
  for (uint8_t i = 0; i < sizeof(lfoResValues)/sizeof(lfoResValues[0]); i++) {
        char int_str[16];
        sprintf(int_str, "%d", lfoResValues[i]);
        robtk_select_add_item(ui->res_box, i, int_str);
    }
    
  robtk_select_set_item(ui->res_box, 4);
  robtk_select_set_default_item(ui->res_box, 4);
  robtk_select_set_callback(ui->res_box, update_res, ui);

  ui->lbl_size = robtk_lbl_new("Length");
  ui->size_box = robtk_select_new();
  for (uint8_t i = 0; i < sizeof(lfoSizeValues)/sizeof(lfoSizeValues[0]); i++) {
        char int_str[16];
        sprintf(int_str, "%d", lfoSizeValues[i]);
        robtk_select_add_item(ui->size_box, i, int_str);
    }

  robtk_select_set_item(ui->size_box, 4);
  robtk_select_set_default_item(ui->size_box, 4);
  robtk_select_set_callback(ui->size_box, update_size, ui);

// Waveform and frequency

  ui->waveform = robtk_select_new();
  robtk_select_add_item(ui->waveform, 0, "Sine");
  robtk_select_add_item(ui->waveform, 1, "Saw Up");
  robtk_select_add_item(ui->waveform, 2, "Triangle");
  robtk_select_add_item(ui->waveform, 3, "Saw Down");
  robtk_select_add_item(ui->waveform, 4, "Rectangle");
  robtk_select_add_item(ui->waveform, 5, "CUSTOM");

  robtk_select_set_item(ui->waveform, 0);
  robtk_select_set_default_item(ui->waveform, 0);
  robtk_select_set_callback(ui->waveform, update_waveform, ui);

  ui->btn_flip = robtk_pbtn_new("Flip");
  robtk_pbtn_set_callback(ui->btn_flip, update_flip_wave, ui);  

  ui->lbl_freq = robtk_lbl_new("Frequency");
  ui->sel_freq = robtk_select_new();
  for (uint8_t i = 0; i < sizeof(lfoFreqValues)/sizeof(lfoFreqValues[0]); i++) {
        char int_str[16];
        sprintf(int_str, "%d", lfoFreqValues[i]);
        robtk_select_add_item(ui->sel_freq, i, int_str);
    }
    
  robtk_select_set_item(ui->sel_freq, 4);
  robtk_select_set_default_item(ui->sel_freq, 4);
  robtk_select_set_callback(ui->sel_freq, update_freq, ui);

  

// Dial controls for Velocity, Notelength and Transpose

  ui->lbl_amplitude = robtk_lbl_new("Amplitude");
  ui->lbl_offset = robtk_lbl_new("Offset");
  ui->lbl_phase = robtk_lbl_new("Phase");

  int dial_defaults[3] = {127, 64, 0};
  
  for (int i = 0; i < 3; i++) {
    ui->dial_control[i] = robtk_dial_new_with_size(0, 127, 1,
                          75, 60, 40, 30, 25);
    char int_str[16];
    sprintf(int_str, "%d", dial_defaults[i]);
    ui->dial_control_ann[i] = robtk_lbl_new(int_str);
    robtk_dial_set_alignment(ui->dial_control[i], 0, 0.5);
    robtk_dial_set_value(ui->dial_control[i], dial_defaults[i]);
    ui->dial_control[i]->displaymode = 7;
    ui->dial_control[i]->dcol[2][0] = .9;
    ui->dial_control[i]->dcol[2][1] = .6;
    ui->dial_control[i]->dcol[2][2] = .3;

    ui->dial_control_box[i] = rob_vbox_new(FALSE, 4);
    rob_vbox_child_pack(ui->dial_control_box[i], robtk_lbl_widget(ui->dial_control_ann[i]), TRUE, TRUE);
    rob_vbox_child_pack(ui->dial_control_box[i], robtk_dial_widget(ui->dial_control[i]), TRUE, TRUE);
  }
  
  robtk_dial_set_callback(ui->dial_control[0], update_amplitude, ui);
  robtk_dial_set_callback(ui->dial_control[1], update_offset, ui);
  robtk_dial_set_callback(ui->dial_control[2], update_phase, ui);

  /* Various stretch separators */

  for (int i = 0; i < 5; i++) {
    ui->sep[i] = robtk_sep_new(TRUE);
    robtk_sep_set_linewidth(ui->sep[i], 0);
  }

  /* LAYOUT */

  /* Wave and frequency */

  ui->ctable_wave = rob_table_new(/*rows*/2, /*cols*/ 4, FALSE);

#define TBLWAVEADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable_wave, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLWAVEATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable_wave, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  int row = 0;
  TBLWAVEADD(robtk_select_widget(ui->waveform), 0, 1, row, row+1);
  TBLWAVEADD(robtk_pbtn_widget(ui->btn_flip), 2, 3, row, row+1);
  row++;
  TBLWAVEADD(robtk_lbl_widget(ui->lbl_freq), 0, 1, row, row+1);
  TBLWAVEADD(robtk_select_widget(ui->sel_freq), 2, 3, row, row+1);
  row++;
  TBLWAVEATT(robtk_sep_widget(ui->sep[3]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 

  /* Spin buttons on the bottom */

  ui->ctable_spin = rob_table_new(/*rows*/3, /*cols*/ 6, FALSE);

#define TBLSPINADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable_spin, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLSPINATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable_spin, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  row = 0;
  TBLSPINADD(ui->dial_control_box[0], 0, 1, row, row+1);
  TBLSPINADD(ui->dial_control_box[1], 2, 3, row, row+1);
  TBLSPINADD(ui->dial_control_box[2], 4, 5, row, row+1);
  row++;
  TBLSPINADD(robtk_lbl_widget(ui->lbl_amplitude), 0, 1, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_offset), 2, 3, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_phase), 4, 5, row, row+1);
  row++;
  TBLSPINATT(robtk_sep_widget(ui->sep[0]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 
  

  /* Control box on the bottom left */

  ui->ctable2 = rob_table_new(/*rows*/5, /*cols*/ 10, FALSE);

#define TBL2ADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable2, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBL2ATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable2, WIDGET, X0, X1, Y0, Y1, 2, 2, XX, XY)
  
  row = 0;
  
  TBL2ADD(robtk_select_widget(ui->loop_mode), 0, 9, row, row+1);
  row++;
  TBL2ADD(robtk_cbtn_widget(ui->btn_mute), 0, 7, row, row+1);
  TBL2ADD(robtk_cbtn_widget(ui->btn_defer), 7, 9, row, row+1);
  row++;
  TBL2ADD(robtk_cbtn_widget(ui->btn_record), 0, 9, row, row+1);
  row++;
  TBL2ADD(robtk_lbl_widget(ui->lbl_res), 0, 5, row, row+1);
  TBL2ADD(robtk_select_widget(ui->res_box), 5, 9, row, row+1);
  row++;
  TBL2ADD(robtk_lbl_widget(ui->lbl_size), 0, 5, row, row+1);
  TBL2ADD(robtk_select_widget(ui->size_box), 5, 9, row, row+1);
  row++;
  TBL2ATT(robtk_sep_widget(ui->sep[4]), 0, 4, row, row+1, RTK_EXANDF, RTK_EXANDF); 

  /* main layout */

  ui->hbox_bottom = rob_hbox_new(FALSE, 4);
  ui->hbox_stretch = rob_hbox_new(FALSE, 4);
  rob_hbox_child_pack(ui->hbox_bottom, ui->ctable2, FALSE, FALSE);
  rob_hbox_child_pack(ui->hbox_bottom, ui->ctable_wave, FALSE, FALSE);
  rob_hbox_child_pack(ui->hbox_bottom, ui->ctable_spin, FALSE, FALSE);
  rob_hbox_child_pack(ui->hbox_bottom, ui->hbox_stretch, TRUE, TRUE);
  
  // Workaround to ensure drawing of other widgets: extra hbox for the draw area 
  ui->hbox_da = rob_hbox_new(FALSE, 4);
  rob_vbox_child_pack(ui->hbox_da, ui->darea, TRUE, TRUE);
  
  ui->vbox = rob_vbox_new(FALSE, 4);
  rob_vbox_child_pack(ui->vbox, ui->hbox_da, TRUE, TRUE);
  rob_vbox_child_pack(ui->vbox, ui->hbox_bottom, FALSE, FALSE);
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)calloc(1, sizeof(QMidiArpLfoUI));

  if (!ui) {
    fprintf(stderr, "UI: out of memory\n");
    return NULL;
  }

  *widget = NULL;
  
  ui->currentRecStep = 0;
  ui->currentIndex = 0;
  ui->mouseY = 0;
  ui->recordMode = false;
  ui->isMuted = false;
  
  ui->mouse_pressed = 0; //released
  ui->mouse_buttons = 0;
  ui->mouse_x = 0;
  ui->mouse_y = 0;
  ui->res = 4;
  ui->size = 4;
  
  ui->xMax = 0;
  
  ui->data_count = 0;
  for (int l1 = 0; l1 < WAVEBUFSIZE; l1++) ui->datavalues[l1] = 60;
  for (int l1 = 0; l1 < WAVEBUFSIZE; l1++) ui->dataticks[l1] = l1 * TPQN / 4;

  ui->uiIsUp = false;
  ui->offset_suppress_callback = false;
  ui->draw_only_cursor = false;


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
  ui->w_height = LFOSCR_MIN_H + CSR_MIN_H + CSR_VMARG;
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
in_out_box_destroy(QMidiArpLfoUI *ui) 
{
  robtk_cbtn_destroy(ui->btn_noteoff);
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;
  /* send message to DSP backend:
   * disable message transmission
   */
  ui_disable(ui);
  cairo_surface_destroy(ui->gridnlabels);
  pango_font_description_free(ui->font[0]);
  pango_font_description_free(ui->font[1]);
  pango_font_description_free(ui->font[2]);
  pango_font_description_free(ui->font[3]);
  

  robtk_select_destroy(ui->cc_in);
  robtk_select_destroy(ui->cc_out);
  robtk_lbl_destroy(ui->lbl_cc_in);
  robtk_lbl_destroy(ui->lbl_cc_out);

  robtk_lbl_destroy(ui->lbl_size);
  robtk_lbl_destroy(ui->lbl_res);
  robtk_select_destroy(ui->loop_mode);
  
  robtk_lbl_destroy(ui->lbl_amplitude);
  robtk_lbl_destroy(ui->lbl_offset);
  robtk_lbl_destroy(ui->lbl_phase);

  robtk_cbtn_destroy(ui->btn_mute);
  robtk_cbtn_destroy(ui->btn_record);
  robtk_cbtn_destroy(ui->btn_defer);
  
  robtk_select_destroy(ui->res_box);
  robtk_select_destroy(ui->size_box);
  robtk_select_destroy(ui->waveform);
  robtk_pbtn_destroy(ui->btn_flip);
  robtk_lbl_destroy(ui->lbl_freq);

  
  for (int i = 0; i < 3; i++) {
    robtk_dial_destroy(ui->dial_control[i]);
    robtk_lbl_destroy(ui->dial_control_ann[i]);
    rob_box_destroy(ui->dial_control_box[i]);
  }

  for (int i = 0; i < 5; i++) {
    robtk_sep_destroy(ui->sep[i]);
  }
  
  rob_table_destroy(ui->ctable_wave);
  rob_table_destroy(ui->ctable_spin);
  rob_table_destroy(ui->ctable2);
  

// In Out box
  in_out_box_destroy(ui);

  robwidget_destroy(ui->darea);
  
  rob_box_destroy(ui->hbox_da);
  rob_box_destroy(ui->hbox_stretch);
  rob_box_destroy(ui->hbox_bottom);
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
  QMidiArpLfoUI* ui = (QMidiArpLfoUI*)handle;
  LV2_Atom* atom = (LV2_Atom*)buffer;
  const QMidiArpURIs* uris = &ui->uris;

  /* check type of data received
   *  format == 0: [float] control-port event
   *  format > 0: message
   *  Every event message is sent as separate port-event
   */

    if (format == uris->atom_eventTransfer
      && (atom->type == uris->atom_Object) ) {
        receiveWave(handle, atom);
        queue_draw(ui->darea);
    }
    else if (format == 0 && buffer_size == sizeof(float)) {

        float fValue = *(float *) buffer;
        // printf("port event index %d  -  value %f\n", port_index, fValue);
        switch (port_index) {
          case AMPLITUDE:
                  robtk_dial_set_value(ui->dial_control[0], (int)fValue);
          break;
          case OFFSET:
                  robtk_dial_set_value(ui->dial_control[1], (int)fValue);
          break;
          case RESOLUTION:
                  robtk_select_set_item(ui->res_box, (int)fValue);
                  ui->res = seqResValues[(int)fValue];
          break;
          case SIZE:
                  robtk_select_set_item(ui->size_box, (int)fValue);
                  ui->size = seqSizeValues[(int)fValue];
          break;
          case PHASE:
                  robtk_dial_set_value(ui->dial_control[2], (int)fValue);
          break;
          case CH_OUT:
                  robtk_select_set_item(ui->ch_out, (int)fValue);
          break;
          case CH_IN:
                  robtk_select_set_item(ui->ch_in, (int)fValue);
          break;
          case CC_OUT:
                  robtk_select_set_item(ui->cc_out, (int)fValue);
          break;
          case CC_IN:
                  robtk_select_set_item(ui->cc_in, (int)fValue);
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
                    if (ui->mouse_pressed == 0) ui->draw_only_cursor = true;
                    queue_draw_area(ui->darea, 0, LFOSCR_MIN_H - LFOSCR_VMARG + CSR_VMARG, DAWIDTH, 
                                        CSR_MIN_H);
                  }
          break;
          case WAVEFORM:
                  robtk_select_set_item(ui->waveform, (int)fValue);
          break;
          case LOOPMODE:
                  robtk_select_set_item(ui->loop_mode, (int)fValue);
          break;
          case MUTE:
                  if (ui->isMuted != (int)fValue) {
                    ui->isMuted = (int)fValue;
                    queue_draw(ui->darea);
                    robtk_cbtn_set_active(ui->btn_mute, (fValue > 0));
                  }
          break;
          break;
          case MOUSEX:
          case MOUSEY:
          case MOUSEBUTTON:
          case MOUSEPRESSED:
          break;
          case ENABLE_NOTEOFF:
                  robtk_cbtn_set_active(ui->btn_noteoff, (fValue > 0));
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
          case RECORD:
                  if (ui->recordMode != (int)fValue) {
                    ui->recordMode = (int)fValue;
                    queue_draw(ui->darea);
                    robtk_cbtn_set_active(ui->btn_record, (fValue > 0));
                  }
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
