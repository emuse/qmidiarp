/*!
 * @file seqscreen.h
 * @brief Header for the SeqScreen class
 *
 * @section LICENSE
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

#include <QLabel>
#include <QMouseEvent>
#include <QString>
#include <QWidget>
#include <QSizePolicy>
#include <QSize>

#include "midiseq.h"

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
class SeqScreen : public QWidget
{
  Q_OBJECT

  private:
    QVector<Sample> p_data, data;
    int grooveTick, grooveVelocity, grooveLength;
    int mouseX, mouseY;
    int w, h;
    bool recordMode;
    int currentRecStep;
    int currentIndex;
    bool needsRedraw;
    bool isMuted;
    QPointF trg[3];

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    SeqScreen(QWidget* parent=0);
    ~SeqScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    int baseOctave, nOctaves;
    int loopMarker;
    void emitMouseEvent(QMouseEvent *event, int pressed);

  signals:
    void mouseEvent(double, double, int, int pressed);

  public slots:
    void updateData(const QVector<Sample>& data);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void setRecordMode(bool on);
    void setCurrentRecStep(int currentRecStep);
    void setLoopMarker(int pos);
    void newGrooveValues(int tick, int vel, int length);
    void setMuted(bool on);
    void updateDraw();
    void updateDispVert(int mode);
};

#endif
