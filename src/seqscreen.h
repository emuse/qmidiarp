/*!
 * @file seqscreen.h
 * @brief Header for the SeqScreen class
 *
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef SEQSCREEN_H
#define SEQSCREEN_H

#include "midiseq.h"
#include "screen.h"

#define SEQSCR_MIN_W   180
#define SEQSCR_MIN_H   216
#define SEQSCR_VMARG    14
#define SEQSCR_HMARG    20

/*! @brief Drawing widget for visualization of sequences using QPainter
 *
 * SeqScreen is created and embedded by SeqWidget. The painter callback
 * produces a streak map of a waveform as a piano roll display. The
 * display is updated by calling SeqScreen::updateData() with the
 * Sample vector as argument followed by updateDraw().
 * SeqScreen emits mouse events combining the Qt mousePressed()
 * and mouseMoved() events. The mouse position is transferred as a
 * double from 0 ... 1.0 representing the relative mouse position on the
 * entire SeqScreen display area.
 */
class SeqScreen : public Screen
{
  Q_OBJECT

  private:
    QVector<Sample> p_data, data;
    int currentRecStep;
    int baseOctave, nOctaves;
    QPointF trg[3];
    void emitMouseEvent(QMouseEvent *event, int pressed);

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    SeqScreen();
    int loopMarker;
    
  public slots:
    void updateData(const QVector<Sample>& data);
    void setCurrentRecStep(int currentRecStep);
    void setLoopMarker(int pos);
    void updateDispVert(int mode);
};

#endif
