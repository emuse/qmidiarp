/*!
 * @file storagebutton.h
 * @brief Headers for the StorageButton QToolbutton class.
 *
 *
 *      Copyright 2009 - 2017 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef STORAGEBUTTON_H
#define STORAGEBUTTON_H

#include <QLabel>
#include <QToolButton>

/*!
 * The StorageButton class derives from QToolButton and implements
 * supplemental display elements such as a second text label.

 * @brief Button widget adding a second text label to QToolButton
 */
class StorageButton : public QToolButton

{
  Q_OBJECT

  public:
    QLabel *secondText;

    StorageButton(QWidget * parent);
    ~StorageButton();

  public slots:

    void setSecondText(const QString & newtext, int type = 0);
};

#endif
