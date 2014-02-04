/*!
 * @file cursor.h
 * @brief Header for the Cursor class
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2014 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef CURSOR_H
#define CURSOR_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QSizePolicy>
#include <QSize>

#include "midilfo.h"

#define CSR_MIN_W   250
#define CSR_MIN_H     6
#define CSR_VMARG    10
#define CSR_HMARG    20


/*! @brief Drawing widget for visualization the cursor using QPainter
 *
 * Cursor is created and embedded by SeqWidget. The painter
 * produces a streak whose location is a function of module resolution,
 * size transferred through the Cursor::updateNumbers()
 * and and frame position transferred by Cursor::updatePosition(). The
 * drawing update is done by Cursor::updateDraw().
 */
class Cursor : public QWidget
{
  Q_OBJECT

  private:
    int w, h;
    QChar modType;
    int nPoints, nSteps;
    int currentIndex;
    bool needsRedraw;

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    Cursor(QChar modtype = 'L', QWidget* parent=0);
    ~Cursor();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  public slots:
    void updateDraw();
    void updatePosition(int index);
    void updateNumbers(int res, int size);
};

#endif
