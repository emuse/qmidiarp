/*!
 * @file storagebutton.h
 * @brief Headers for the StorageButton QToolbutton class.
 *
 *
 *      Copyright 2009 - 2024 <qmidiarp-devel@lists.sourceforge.net>
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
#include <QMouseEvent>

/*!
 * Special Label that changes background color when mouse enters
 */
class HitLabel : public QLabel
{
    Q_OBJECT
    
    public:
    HitLabel(const QString& text, QWidget * parent);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
};
/*!
 * The StorageButton class derives from QToolButton and implements
 * supplemental display elements such as two more text labels.
 * It also provides a special area within the button that, when clicked,
 * sets a property "forceStay" indicating to the receiver that the
 * automatic jump settings shall be ignored.

 * @brief Button widget adding a more text labels to QToolButton
 */
class StorageButton : public QToolButton

{
  Q_OBJECT

  public:
    QLabel *secondText;
    QLabel *thirdText;
    HitLabel *forceLabel;

    StorageButton(QWidget * parent);
    ~StorageButton();

  public slots:
    void setSecondText(const QString & newtext, int type = 0);
    void setNRep(int nrep = 1);
    void setBGColor(int color);
};
#endif
