/*!
 * @file indicator.h
 * @brief Header for the Indicator class
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
#ifndef INDICATOR_H
#define INDICATOR_H

#include <QSizePolicy>
#include <QSize>
#include <QWidget>

/*! @brief Drawing widget for visualization of current pattern positions
 *
 */
class Indicator : public QWidget
{
  Q_OBJECT

  private:
    int p_angle;
    int p_size;
    bool needsRedraw;
    bool isMuted;
    QColor fillColor;

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    Indicator(int size, QChar modType = ' ', QWidget* parent=0);
    ~Indicator();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  public slots:
    void updatePercent(int percent);
    void setMuted(bool on);
    void updateDraw();
};

#endif
