/*!
 * @file slider.h
 * @brief Widget class combining slider and spinbox and label.
 *
 * The spinbox and slider are coupled so that the slider value is
 * visualized. The stepwidth when controller by keyboard cursor keys
 * can be set as well as the display orientation.
 *
 *
 *      Copyright 2009 - 2019 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef SLIDER_H
#define SLIDER_H

#include <QSlider>
#include <QSpinBox>


class Slider : public QWidget

/*!
 * @brief Widget class combining slider and spinbox and label.
 *
 * The spinbox and slider are coupled so that the slider value is
 * visualized. The stepwidth when controller by keyboard cursor keys
 * can be set as well as the display orientation.
 *
 */
{
  Q_OBJECT

  private:
    QSlider *slider;
    QSpinBox *sliderSpin;

  public:
    Slider(int minValue, int maxValue, int pageStep, int tickStep, int value,
           Qt::Orientation orientation, const QString& label, QWidget * parent);
    int value();
    bool valueChangedSignalSuppressed;

  signals:
    void valueChanged(int);
    void sliderMoved(int);

  public slots:
    void setValue(int);
    void setMin(int);
    void setMax(int);

  private slots:
    void updateSpinBox(int);
    void fillSpinBox(int);
    void emitAsMoved();
};

#endif
