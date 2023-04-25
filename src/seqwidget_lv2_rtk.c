/*!
 * @file seqwidget_lv2_rtk.c
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
#include "seqwidget_lv2_rtk.h"
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

void receiveWave(LV2UI_Handle handle, LV2_Atom* atom)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*)handle;
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

    for (uint32_t l1 = 0; l1 < n_elem; l1++) {
        ui->datavalues[l1] = recdata[l1];
        ui->dataticks[l1] = l1 * TPQN / ui->res;
    }
    ui->data_count = n_elem;
}

static void 
ui_state(LV2UI_Handle handle)
{
  (void)handle;
}

/** notfiy backend that UI is closed */
static void ui_disable(LV2UI_Handle handle)
{
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)handle;
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
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)handle;
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

void updateParam(LV2UI_Handle handle, int index, float fValue)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*)handle;    
    ui->write(ui->controller, index, sizeof(float), 0, &fValue);
}

static bool update_notein (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, ENABLE_NOTEIN, robtk_cbtn_get_active(ui->btn_notein));
    return TRUE;
}

static bool update_velin (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, ENABLE_VELIN, robtk_cbtn_get_active(ui->btn_velin));
    return TRUE;
}

static bool update_noteoff (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, ENABLE_NOTEOFF, robtk_cbtn_get_active(ui->btn_noteoff));
    return TRUE;
}

static bool update_trigkbd (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, ENABLE_TRIGBYKBD, robtk_cbtn_get_active(ui->btn_trigkbd));
    return TRUE;
}

static bool update_restartkbd (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, ENABLE_RESTARTBYKBD, robtk_cbtn_get_active(ui->btn_restartkbd));
    return TRUE;

}

static bool update_triglegato (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, ENABLE_TRIGLEGATO, robtk_cbtn_get_active(ui->btn_triglegato));
    return TRUE;
}

static bool update_notefilter (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
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
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, INDEX_IN1, robtk_spin_get_value(ui->spb_index_in0));
    return TRUE;

}

static bool update_index_in1 (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, INDEX_IN2, robtk_spin_get_value(ui->spb_index_in1));
    return TRUE;

}

static bool update_range_in0 (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, RANGE_IN1, robtk_spin_get_value(ui->spb_range_in0));
    return TRUE;

}

static bool update_range_in1 (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, RANGE_IN2, robtk_spin_get_value(ui->spb_range_in1));
    return TRUE;

}

static bool update_transport (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, TRANSPORT_MODE, robtk_cbtn_get_active(ui->btn_transport));
    return TRUE;
}

static bool update_tempo (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, TEMPO, robtk_spin_get_value(ui->spb_tempo));
    return TRUE;
}

static bool update_ch_in (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, CH_IN, robtk_select_get_item(ui->ch_in));
    return TRUE;
}

static bool update_ch_out (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, CH_OUT, robtk_select_get_item(ui->ch_out));
    return TRUE;
}
/* end common update functions */

static bool update_velocity (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, VELOCITY, robtk_dial_get_value(ui->dial_control[0]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[0]));
    robtk_lbl_set_text(ui->dial_control_ann[0], txt);
    return TRUE;

}

static bool update_notelength (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, NOTELENGTH, robtk_dial_get_value(ui->dial_control[1]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[1]));
    robtk_lbl_set_text(ui->dial_control_ann[1], txt);
    return TRUE;

}

static bool update_transpose (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, TRANSPOSE, robtk_dial_get_value(ui->dial_control[2]));
    char txt[16];
    snprintf(txt, 16, "%d", (int)robtk_dial_get_value(ui->dial_control[2]));
    robtk_lbl_set_text(ui->dial_control_ann[2], txt);
    return TRUE;

}

static bool update_res (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    uint64_t val = robtk_select_get_item(ui->res_box);
    if (val >= sizeof(seqResValues)/sizeof(seqResValues[0]))
      return TRUE;

    ui->res = seqResValues[val];
    updateParam(ui, RESOLUTION, val);
    return TRUE;

}

static bool update_size (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    uint64_t val = robtk_select_get_item(ui->size_box);
    if (val >= sizeof(seqSizeValues)/sizeof(seqSizeValues[0]))
      return TRUE;

    ui->size = seqSizeValues[val];
    updateParam(ui, SIZE, val);
    return TRUE;

}

static bool update_mute (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, MUTE, robtk_cbtn_get_active(ui->btn_mute));
    ui->isMuted = robtk_cbtn_get_active(ui->btn_mute);
    queue_draw(ui->darea);
    return TRUE;

}

static bool update_record (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, RECORD, robtk_cbtn_get_active(ui->btn_record));
    ui->recordMode = robtk_cbtn_get_active(ui->btn_record);
    queue_draw(ui->darea);
    return TRUE;
}

static bool update_defer (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, DEFER, robtk_cbtn_get_active(ui->btn_defer));
    return TRUE;
}

static bool update_loop_mode (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    updateParam(ui, LOOPMODE, robtk_select_get_item(ui->loop_mode));
    return TRUE;
}

static bool update_disp_vert (RobWidget *widget, void* data)
{
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) data;
    int val = 0;
    for (int i = 0; i < 4; i++) {
      if (robtk_rbtn_get_active(ui->rbtn_disp_vert[i])) {
        val = i;
        break;
      }
    }
    
    switch (val) {
        case 0:
            ui->nOctaves = 4;
            ui->baseOctave = 3;
        break;
        case 1:
            ui->nOctaves = 2;
            ui->baseOctave = 5;
        break;
        case 2:
            ui->nOctaves = 2;
            ui->baseOctave = 4;
        break;
        case 3:
            ui->nOctaves = 2;
            ui->baseOctave = 3;
        break;
        default:
            ui->nOctaves = 4;
            ui->baseOctave = 3;
    }
    updateParam(ui, DISPLAY_ZOOM, val);
    queue_draw(ui->darea);
    return TRUE;
}

static void update_mouse(RobWidget* handle) {  
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*) GET_HANDLE(handle);

  if (ui->mouse_x <  -0.02 || ui->mouse_x >  1.02 ||
      ui->mouse_y <  -0.15 || ui->mouse_y >  1.05 ) {
    ui->mouse_pressed = 2;
    ui->mouse_buttons = 0;
  }
  if (ui->mouse_x < 0.) ui->mouse_x = 0;
  if (ui->mouse_x > 1.) ui->mouse_x = 1;
  if (ui->mouse_y > .99) ui->mouse_y = .99;
  if (ui->mouse_y < -.1) ui->mouse_y = -.1;

  double mouseX = ui->mouse_x;
  double mouseY = ui->mouse_y;

  if ((mouseY < 0) && (ui->mouse_pressed != 2)) { 
    // we have to recalculate loopMarker for screen update
      if (mouseX < 0) mouseX = 0;
      if (ui->mouse_buttons == 2) mouseX = - mouseX;
      const int npoints = ui->res * ui->size;
      int lm;
      if (mouseX > 0) lm = mouseX * (double)npoints + .5;
      else lm = mouseX * (double)npoints - .5;
      if (abs(lm) >= npoints) lm = 0;
      ui->loopMarker = lm;
      updateParam(ui, LOOPMARKER, lm);
  }
  if (ui->mouse_pressed != 0) {
    ui->draw_only_cursor = false;

    updateParam(ui, MOUSEX, mouseX);
    updateParam(ui, MOUSEY, mouseY);
    updateParam(ui, MOUSEBUTTON, ui->mouse_buttons);
    updateParam(ui, MOUSEPRESSED, ui->mouse_pressed);
  }
}

static RobWidget* mouse_down(RobWidget* handle, RobTkBtnEvent *ev) {
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*) GET_HANDLE(handle);
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
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*) GET_HANDLE(handle);
  ui->mouse_pressed = 2;
  ui->mouse_buttons = 0;
  update_mouse(handle);
  return NULL;
}

static RobWidget* mouse_move(RobWidget* handle, RobTkBtnEvent *ev) {
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*) GET_HANDLE(handle);
  
  int w = DAWIDTH;
  int h = SEQSCR_MIN_H;

  // printf("Mouse move %d1 %d2 \n", ev->x, ev->y);
  
  ui->mouse_x = ((double)ev->x - SEQSCR_HMARG) / (w - 2 * SEQSCR_HMARG);
  ui->mouse_y = 1. - ((double)ev->y - SEQSCR_VMARG_TOP) /
                (h - SEQSCR_VMARG_TOP- SEQSCR_VMARG_BOT);

  //printf("Relative coords %f1 %f2 \n", ui->mouse_x, ui->mouse_y);

  if (ui->mouse_buttons > 0) {
    update_mouse(handle);
    return handle;
  }
  return NULL;
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
    QMidiArpSeqUI* ui = (QMidiArpSeqUI*) GET_HANDLE(handle);
    int nPoints = ui->data_count -1;
    int xpos, xscale;
    int x;
    int notestreak_thick = 2;
    int w = DAWIDTH;
    int h = DAHEIGHT;

    cairo_set_line_width(cr, notestreak_thick * 2);
    cairo_rectangle (cr, 0, h - CSR_VMARG, DAWIDTH, CSR_MIN_H + CSR_VMARG);
    cairo_clip(cr);
    cairo_set_source_rgba(cr, 10./256, 10./256, 50./256, 1);
    cairo_move_to(cr, 0, h - CSR_VMARG + CSR_MIN_H);
    cairo_line_to(cr, DAWIDTH, h - CSR_VMARG + CSR_MIN_H);
    cairo_stroke(cr);

    xscale = (w - 2 * CSR_HMARG);

    // Cursor
    cairo_set_source_rgba(cr, 50./256, 180./256, 220./256, 1);
    x = ui->currentIndex * xscale / nPoints;
    xpos = CSR_HMARG + x + notestreak_thick;
    cairo_move_to(cr, xpos, h - CSR_VMARG + CSR_MIN_H);
    cairo_line_to(cr, xpos + (xscale / nPoints) - notestreak_thick * 2, h - CSR_VMARG + CSR_MIN_H);
    cairo_stroke(cr);
}

/* gdk drawing area draw callback
 * -- this runs in gtk's main thread */
static bool expose_event(RobWidget* handle, cairo_t* cr, cairo_rectangle_t *ev)
{
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*) GET_HANDLE(handle);

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
      cairo_set_source_rgba(cr, 10./256, 10./256, 50./256, 1);
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
    int h = SEQSCR_MIN_H;
    int ofs;
    int x, x1;
    int minOctave = ui->baseOctave;
    int maxOctave = ui->nOctaves + minOctave;
    int notestreak_thick = 16 / ui->nOctaves;
    
    // Not in use with LV2 plugin
    double grooveTick = 0;
  
    // Text colors
    const float color_txt1[4] = {90./256, 250./256, 120./256, 1};
    const float color_txt2[4] = {100./256, 150./256, 180./256, 1};
    const float color_txt3[4] = {30./256, 60./256, 180./256, 1};
    
    if (ui->data_count < 1) 
      return FALSE;
    
    //Grid setup
    int nsteps = (int)( (double)ui->dataticks[ui->data_count - 1] / TPQN + .5);
    int beatRes = (ui->data_count - 1) / nsteps;
    int beatDiv = (beatRes * nsteps > 64) ? 64 / nsteps : beatRes;
    int npoints = beatRes * nsteps;
    xscale = (double)TPQN * (w - 2 * SEQSCR_HMARG) / ui->dataticks[ui->data_count - 1] ;
    yscale = h - SEQSCR_VMARG_BOT - SEQSCR_VMARG_TOP;
  
    //Loop Marker Area
    cairo_set_source_rgba (cr, 20./256, 20./256, 90./256, 1);
    cairo_rectangle (cr, SEQSCR_HMARG, h - SEQSCR_VMARG_BOT, w - 2 * SEQSCR_HMARG, SEQSCR_VMARG_BOT);
    cairo_fill(cr);
    //Determine font geometry
    int tx_w, tx_h;
    get_text_geometry("1", ui->font[1], &tx_w, &tx_h);
    render_text(cr, "L", ui->font[1], SEQSCR_HMARG / 2, h - SEQSCR_VMARG_BOT / 2, 0, 1, color_txt1);
    
    //Draw current record step
    if (ui->recordMode) {
      cairo_set_source_rgba (cr, 5./256, 40./256, 100./256, 1);
      cairo_rectangle (cr, ui->currentRecStep * xscale * nsteps / npoints + SEQSCR_HMARG
                , SEQSCR_VMARG_TOP
                , xscale * nsteps / npoints
                , yscale);
    }
    cairo_fill(cr);
  
    //Beat separators
    cairo_set_source_rgba (cr, 20./256, 20./256, 160./256, .3);
    for (int l1 = 0; l1 < nsteps + 1; l1++) {
  
      if (l1 < 10) {
        ofs = w / nsteps * .5 - 4 + SEQSCR_HMARG;
      } else {
        ofs = w / nsteps * .5 - 6 + SEQSCR_HMARG;
      }
      if ((bool)(l1%beat)) {
        cairo_set_source_rgba (cr, 60./256, 100./256, 180./256, 1);
      } else {
        cairo_set_source_rgba (cr, 100./256, 100./256, 180./256, 1);
      }
      x = l1 * xscale;
      cairo_move_to(cr, SEQSCR_HMARG + x, SEQSCR_VMARG_TOP);
      cairo_line_to(cr, SEQSCR_HMARG + x, h);
      cairo_stroke(cr);
  
      if (l1 < nsteps) {
        //Beat numbers          
        if ((nsteps < 32) || !((l1 + 5) % 4)) {
          char int_str[20];
          sprintf(int_str, "%d", l1+1);
          render_text(cr, int_str, ui->font[1], ofs + x + tx_w / 2, SEQSCR_VMARG_TOP - tx_h / 2, 0, 1, color_txt2);
        }  
        // Beat divisor separators
        cairo_set_source_rgba (cr, 20./256, 60./256, 120./256, 1);
        for (int l2 = 1; l2 < beatDiv; l2++) {
          x1 = x + l2 * xscale / beatDiv;
          if (x1 < xscale * nsteps) {
            cairo_move_to(cr, SEQSCR_HMARG + x1, SEQSCR_VMARG_BOT);
            cairo_line_to(cr, SEQSCR_HMARG + x1, h);
            cairo_stroke(cr);
          }
        }
      }
    }
    //Horizontal separators and numbers
    if (!ui->isMuted) {
      for (int l1 = 0; l1 <= ui->nOctaves * 12; l1++) {
        int l3 = l1%12;
    
        ypos = yscale * l1 / ui->nOctaves / 12 + SEQSCR_VMARG_TOP;
    
        if (!l3) {
          cairo_set_source_rgba (cr, 30./256, 60./256, 180./256, 1);
          char int_str[20];
          sprintf(int_str, "%d", maxOctave - l1 / 12);
          render_text(cr, int_str, ui->font[1], w - SEQSCR_HMARG / 2 - 4 + tx_w / 2,
                  ypos + SEQSCR_VMARG_TOP - 5 - yscale / ui->nOctaves / 2 - tx_h / 2, 0, 
                  1, color_txt3);
        }
        else
          cairo_set_source_rgba (cr, 10./256, 20./256, 100./256, 1);
    
        cairo_move_to(cr, 0, ypos);
        cairo_line_to(cr, w - SEQSCR_HMARG, ypos);
        cairo_stroke(cr);
        if ((l3 == 2) || (l3 == 4) || (l3 == 6) || (l3 == 9) || (l3 == 11)) {
          cairo_set_source_rgba (cr, 20./256, 60./256, 180./256, 1);
          cairo_set_line_width(cr, notestreak_thick);
          cairo_move_to(cr, 0, ypos - notestreak_thick / 2);
          cairo_line_to(cr, SEQSCR_HMARG / 2, ypos- notestreak_thick / 2);
          cairo_stroke(cr);
          cairo_set_line_width(cr, 1.);
        }
      }
    }
    cairo_set_source_rgba (cr, 30./256, 60./256, 180./256, 1);
    cairo_move_to(cr, 0, h - 2);
    cairo_line_to(cr, w - SEQSCR_HMARG, h - 2);
    cairo_stroke(cr);
  
    //Draw function
  
    cairo_set_line_width(cr, notestreak_thick);
  
    for (int l1 = 0; l1 < npoints; l1++) {
      x = (l1 + .01 * (double)grooveTick * (l1 % 2)) * nsteps * xscale / npoints;
      tmpval = ui->datavalues[l1];
      if ((abs(tmpval) >= 12 * ui->baseOctave) && (abs(tmpval) < 12 * maxOctave)) {
        ypos = yscale - yscale
            * (abs(tmpval) - 12 * ui->baseOctave) / ui->nOctaves / 12
            + SEQSCR_VMARG_TOP - notestreak_thick / 2;
        xpos = SEQSCR_HMARG + x;
        if (tmpval < 0) {
          cairo_set_source_rgba (cr, 15./256, 50./256, 120./256, 1);
        }
        else {
          cairo_set_source_rgba (cr, 50./256, 130./256, 180./256, 1);
        }
        cairo_move_to(cr, xpos, ypos);
        cairo_line_to(cr, xpos + (xscale / beatRes), ypos);
        cairo_stroke(cr);
      }
    }
    
    // Helper tickline on keyboard
    ypos = yscale - yscale * (int)((1. - (ui->mouse_y - SEQSCR_VMARG_TOP)
            / yscale) * ui->nOctaves * 12) / ui->nOctaves / 12
            + SEQSCR_VMARG_TOP - 1 - notestreak_thick / 2;
  
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgba (cr, 50./256, 160./256, 220./256, 1);
    cairo_move_to(cr, SEQSCR_HMARG / 2, ypos);
    cairo_line_to(cr, SEQSCR_HMARG *2 / 3, ypos);
    cairo_stroke(cr);
    
    // Loop Marker
    if (ui->loopMarker) {
      cairo_set_line_width(cr, 2);
      cairo_set_source_rgba (cr, 80./256, 250./256, 120./256, 1);
      x = abs(ui->loopMarker) * xscale * nsteps / npoints;
      xpos = SEQSCR_HMARG + x + 2 / 2;
      ypos = h - SEQSCR_VMARG_BOT;
      tmpval = SEQSCR_VMARG_BOT / 2;
  
      cairo_move_to(cr, xpos, ypos + 2);      
      if (ui->loopMarker > 0) {
        cairo_line_to(cr, xpos - tmpval + 2, ypos + tmpval);
        cairo_stroke(cr);
        cairo_move_to(cr, xpos - tmpval + 2, ypos + tmpval);
      }
      else {
        cairo_line_to(cr, xpos + tmpval - 2, ypos + tmpval);
        cairo_stroke(cr);
        cairo_move_to(cr, xpos + tmpval - 2, ypos + tmpval);
      }
      cairo_line_to(cr, xpos, h - 2);
      cairo_stroke(cr);
      cairo_move_to(cr, xpos, h - 2);
      cairo_line_to(cr, xpos, ypos + 2);
      cairo_stroke(cr);
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
  *w = 720;
  *h = SEQSCR_MIN_H + CSR_MIN_H + CSR_VMARG;
}

static void
size_allocate(RobWidget* handle, int w, int h) {
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)GET_HANDLE(handle);
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
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)GET_HANDLE(handle);
  *w = DAWIDTH;
  //*w = 720;
  *h = SEQSCR_MIN_H + CSR_MIN_H + CSR_VMARG; // 254;
}

#endif

static void in_out_box_new(QMidiArpSeqUI* ui)
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

  robtk_cbtn_set_color_on(ui->btn_restartkbd, .2, .3, .9);
  robtk_cbtn_set_color_off(ui->btn_restartkbd, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_trigkbd, .2, .3, 9.);
  robtk_cbtn_set_color_off(ui->btn_trigkbd, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_triglegato, .2, .3, .9);
  robtk_cbtn_set_color_off(ui->btn_triglegato, .1, .1, .3);


// Additions to in_out_box elements on the right only LFO and SEQ
  ui->btn_notein = robtk_cbtn_new("Note In", GBT_LED_LEFT, true);
  ui->btn_velin = robtk_cbtn_new("Vel In", GBT_LED_LEFT, true);
  ui->btn_noteoff = robtk_cbtn_new("Note Off", GBT_LED_LEFT, true);
  robtk_cbtn_set_color_on(ui->btn_notein, .2, .3, .9);
  robtk_cbtn_set_color_off(ui->btn_notein, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_velin, .2, .3, .9);
  robtk_cbtn_set_color_off(ui->btn_velin, .1, .1, .3);
  robtk_cbtn_set_color_on(ui->btn_noteoff, .2, .3, .9);
  robtk_cbtn_set_color_off(ui->btn_noteoff, .1, .1, .3);

  robtk_cbtn_set_callback(ui->btn_notein, update_notein, ui);
  robtk_cbtn_set_callback(ui->btn_velin, update_velin, ui);
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
  
  // ChannelOut selectbox (combobox)

  ui->lbl_ch_out = robtk_lbl_new("Ch");
  ui->ch_out = robtk_select_new();
  for (int i = 0; i < 16; i++) {
        char int_str[16];
        sprintf(int_str, "%d", i + 1 );
        robtk_select_add_item(ui->ch_out, i, int_str);
    }
    
  robtk_select_set_item(ui->ch_out, 0);
  robtk_select_set_default_item(ui->ch_out, 0);
  robtk_select_set_callback(ui->ch_in, update_ch_out, ui);

// Transport controls

  // Host / Internal transport
  ui->btn_transport = robtk_cbtn_new("Host transport", GBT_LED_LEFT, false);
  robtk_cbtn_set_color_on(ui->btn_transport, .2, .3, .9);
  robtk_cbtn_set_color_off(ui->btn_transport, .1, .1, .3);
  robtk_cbtn_set_callback(ui->btn_transport, update_transport, ui);

  robtk_cbtn_set_active(ui->btn_transport, true);
  
  ui->lbl_tempo = robtk_lbl_new("Tempo");
  ui->spb_tempo = robtk_spin_new(5, 200, 1);
  robtk_spin_set_value(ui->spb_tempo, 120);
  robtk_spin_set_callback(ui->spb_tempo, update_tempo, ui);

  /* LAYOUT */

  ui->in_out_box = rob_table_new(/*rows*/14, /*cols*/ 5, FALSE);

#define TBLADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->in_out_box, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->in_out_box, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  row = 0;
  // Input

  TBLADD(robtk_lbl_widget(ui->lbl_inbox), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_notein), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_velin), 0, 2, row, row+1);
  row++;
  TBLADD(robtk_cbtn_widget(ui->btn_noteoff), 0, 2, row, row+1);
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

static RobWidget * toplevel(QMidiArpSeqUI* ui, void * const top)
{
  
  /* main widget: layout */
  ui->hbox = rob_hbox_new(FALSE, 2);
  robwidget_make_toplevel(ui->hbox, top);
  ROBWIDGET_SETNAME(ui->hbox, "QMidiArp Seq");

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
  for (uint i = 0; i < sizeof(seqResValues)/sizeof(seqResValues[0]); i++) {
        char int_str[16];
        sprintf(int_str, "%d", seqResValues[i]);
        robtk_select_add_item(ui->res_box, i, int_str);
    }
    
  robtk_select_set_item(ui->res_box, 4);
  robtk_select_set_default_item(ui->res_box, 4);
  robtk_select_set_callback(ui->res_box, update_res, ui);

  ui->lbl_size = robtk_lbl_new("Length");
  ui->size_box = robtk_select_new();
  for (uint i = 0; i < sizeof(seqSizeValues)/sizeof(seqSizeValues[0]); i++) {
        char int_str[16];
        sprintf(int_str, "%d", seqSizeValues[i]);
        robtk_select_add_item(ui->size_box, i, int_str);
    }

  robtk_select_set_item(ui->size_box, 4);
  robtk_select_set_default_item(ui->size_box, 4);
  robtk_select_set_callback(ui->size_box, update_size, ui);

// Dial controls for Velocity, Notelength and Transpose

  ui->lbl_velocity = robtk_lbl_new("Velocity");
  ui->lbl_notelength = robtk_lbl_new("Note Length");
  ui->lbl_transpose = robtk_lbl_new("Transpose");

  int dial_defaults[3] = {127, 64, 0};
  
  for (int i = 0; i < 3; i++) {
    if (i < 2) {
      ui->dial_control[i] = robtk_dial_new_with_size(0, 127, 1,
                          75, 60, 40, 30, 25);
    }
    else {
      ui->dial_control[i] = robtk_dial_new_with_size(-24, 24, 1,
                          75, 60, 40, 30, 25);
    }
    char int_str[16];
    sprintf(int_str, "%d", dial_defaults[i]);
    ui->dial_control_ann[i] = robtk_lbl_new(int_str);
    robtk_dial_set_alignment(ui->dial_control[i], 0, 0.5);
    robtk_dial_set_value(ui->dial_control[i], dial_defaults[i]);
    ui->dial_control[i]->displaymode = 7;
    ui->dial_control[i]->dcol[2][0] = .3;
    ui->dial_control[i]->dcol[2][1] = .6;
    ui->dial_control[i]->dcol[2][2] = .9;

    ui->dial_control_box[i] = rob_vbox_new(FALSE, 4);
    rob_vbox_child_pack(ui->dial_control_box[i], robtk_lbl_widget(ui->dial_control_ann[i]), TRUE, TRUE);
    rob_vbox_child_pack(ui->dial_control_box[i], robtk_dial_widget(ui->dial_control[i]), TRUE, TRUE);
  }
  
  robtk_dial_set_callback(ui->dial_control[0], update_velocity, ui);
  robtk_dial_set_callback(ui->dial_control[1], update_notelength, ui);
  robtk_dial_set_callback(ui->dial_control[2], update_transpose, ui);

  // Vertical disp zoom radiobuttons
  ui->rbtn_disp_vert[0] = robtk_rbtn_new("F", NULL);
  ui->disp_rbtn_group = robtk_rbtn_group(ui->rbtn_disp_vert[0]);
  ui->rbtn_disp_vert[1] = robtk_rbtn_new("U", ui->disp_rbtn_group);
  ui->rbtn_disp_vert[2] = robtk_rbtn_new("M", ui->disp_rbtn_group);
  ui->rbtn_disp_vert[3] = robtk_rbtn_new("L", ui->disp_rbtn_group);
  robtk_rbtn_set_active(ui->rbtn_disp_vert[0], true);
  
  for (int i = 0; i < 3; i++) {
    robtk_rbtn_set_callback(ui->rbtn_disp_vert[i], update_disp_vert, ui);
  }
  
  /* radiobuttons */
  
  ui->hbox_disp_vert = rob_hbox_new(FALSE, 4);

  /* Various stretch separators */

  for (int i = 0; i < 3; i++) {
    ui->sep[i] = robtk_sep_new(TRUE);
    robtk_sep_set_linewidth(ui->sep[i], 0);
  }

  /* LAYOUT */

  /* Spin buttons on the bottom */

  ui->ctable_spin = rob_table_new(/*rows*/3, /*cols*/ 6, FALSE);

#define TBLSPINADD(WIDGET, X0, X1, Y0, Y1) \
  rob_table_attach(ui->ctable_spin, WIDGET, X0, X1, Y0, Y1, 0, 0, RTK_EXANDF, RTK_SHRINK)

#define TBLSPINATT(WIDGET, X0, X1, Y0, Y1, XX, XY) \
  rob_table_attach(ui->ctable_spin, WIDGET, X0, X1, Y0, Y1, 0, 0, XX, XY)

  int row = 0;
  TBLSPINADD(ui->dial_control_box[0], 0, 1, row, row+1);
  TBLSPINADD(ui->dial_control_box[1], 2, 3, row, row+1);
  TBLSPINADD(ui->dial_control_box[2], 4, 5, row, row+1);
  row++;
  TBLSPINADD(robtk_lbl_widget(ui->lbl_velocity), 0, 1, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_notelength), 2, 3, row, row+1);
  TBLSPINADD(robtk_lbl_widget(ui->lbl_transpose), 4, 5, row, row+1);
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

  rob_hbox_child_pack(ui->hbox_disp_vert, robtk_sep_widget(ui->sep[1]), TRUE, TRUE);
  for (int i = 0; i < 4; i++) {
    rob_hbox_child_pack(ui->hbox_disp_vert, robtk_rbtn_widget(ui->rbtn_disp_vert[i]), FALSE, FALSE);
  }
  
  /* main layout */

  ui->hbox_bottom = rob_hbox_new(FALSE, 4);
  ui->hbox_stretch = rob_hbox_new(FALSE, 4);
  rob_hbox_child_pack(ui->hbox_bottom, ui->ctable2, FALSE, FALSE);
  rob_hbox_child_pack(ui->hbox_bottom, ui->ctable_spin, FALSE, FALSE);
  rob_hbox_child_pack(ui->hbox_bottom, ui->hbox_stretch, TRUE, TRUE);
  rob_vbox_child_pack(ui->hbox_bottom, ui->hbox_disp_vert, FALSE, FALSE);
  
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
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)calloc(1, sizeof(QMidiArpSeqUI));

  if (!ui) {
    fprintf(stderr, "UI: out of memory\n");
    return NULL;
  }

  *widget = NULL;
  
  ui->baseOctave = 3;
  ui->nOctaves= 4;
  ui->currentRecStep = 0;
  ui->dispVert = 0;
  ui->loopMarker = 0;
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
  
  ui->data_count = 0;
  for (int l1 = 0; l1 < WAVEBUFSIZE; l1++) ui->datavalues[l1] = 60;
  for (int l1 = 0; l1 < WAVEBUFSIZE; l1++) ui->dataticks[l1] = l1 * TPQN / 4;

  ui->uiIsUp = false;
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
  ui->w_width = 720;
  ui->w_height = SEQSCR_MIN_H + CSR_MIN_H + CSR_VMARG;
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
in_out_box_destroy(QMidiArpSeqUI *ui) 
{
  robtk_cbtn_destroy(ui->btn_notein);
  robtk_cbtn_destroy(ui->btn_velin);
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
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)handle;
  /* send message to DSP backend:
   * disable message transmission
   */
  ui_disable(ui);
  cairo_surface_destroy(ui->gridnlabels);
  pango_font_description_free(ui->font[0]);
  pango_font_description_free(ui->font[1]);
  pango_font_description_free(ui->font[2]);
  pango_font_description_free(ui->font[3]);
  
  robtk_lbl_destroy(ui->lbl_size);
  robtk_lbl_destroy(ui->lbl_res);
  
  robtk_lbl_destroy(ui->lbl_velocity);
  robtk_lbl_destroy(ui->lbl_notelength);
  robtk_lbl_destroy(ui->lbl_transpose);

  robtk_cbtn_destroy(ui->btn_mute);
  robtk_cbtn_destroy(ui->btn_record);
  robtk_cbtn_destroy(ui->btn_defer);
  
  robtk_select_destroy(ui->res_box);
  robtk_select_destroy(ui->size_box);
  robtk_select_destroy(ui->loop_mode);
  
  for (int i = 0; i < 3; i++) {
    robtk_dial_destroy(ui->dial_control[i]);
    robtk_lbl_destroy(ui->dial_control_ann[i]);
    rob_box_destroy(ui->dial_control_box[i]);
  }

  for (int i = 0; i < 3; i++) {
    robtk_sep_destroy(ui->sep[i]);
  }
  
  rob_table_destroy(ui->ctable_spin);
  rob_table_destroy(ui->ctable2);
  
  
  for (int i = 0; i < 4; i++) {
    robtk_rbtn_destroy(ui->rbtn_disp_vert[i]);
  }
  
// In Out box
  in_out_box_destroy(ui);

  robwidget_destroy(ui->darea);
  
  rob_box_destroy(ui->hbox_da);
  rob_box_destroy(ui->hbox_disp_vert);
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
  QMidiArpSeqUI* ui = (QMidiArpSeqUI*)handle;
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
          case VELOCITY:
                  robtk_dial_set_value(ui->dial_control[0], (int)fValue);
          break;
          case NOTELENGTH:
                  robtk_dial_set_value(ui->dial_control[1], (int)fValue);
          break;
          case RESOLUTION:
                  robtk_select_set_item(ui->res_box, (int)fValue);
          break;
          case SIZE:
                  robtk_select_set_item(ui->size_box, (int)fValue);
          break;
          case TRANSPOSE:
                  robtk_dial_set_value(ui->dial_control[2], (int)fValue);
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
                    if (ui->mouse_pressed == 0) ui->draw_only_cursor = true;
                    queue_draw_area(ui->darea, 0, SEQSCR_MIN_H, DAWIDTH, 
                                        CSR_MIN_H + CSR_VMARG);
                  }
          break;
          case LOOPMARKER:
                  if (ui->loopMarker != (int)fValue) {
                    ui->loopMarker = (int)fValue;
                    queue_draw(ui->darea);
                  }
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
          case DISPLAY_ZOOM:
                  if ((ui->dispVert != (int)fValue) && (4 > (int)fValue)) {
                    ui->dispVert = (int)fValue;
                    queue_draw(ui->darea);
                    robtk_rbtn_set_active(ui->rbtn_disp_vert[ui->dispVert], true);
                  }
          break;
          case MOUSEX:
          case MOUSEY:
          case MOUSEBUTTON:
          case MOUSEPRESSED:
          break;
          case ENABLE_NOTEIN:
                  robtk_cbtn_set_active(ui->btn_notein, (fValue > 0));
          break;
          case ENABLE_VELIN:
                  robtk_cbtn_set_active(ui->btn_velin, (fValue > 0));
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
          case CURR_RECSTEP:
                  //record step has changed
                  if (ui->currentRecStep != (int)fValue) {
                      ui->currentRecStep = (int)fValue;
                      queue_draw(ui->darea);
                  }
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
