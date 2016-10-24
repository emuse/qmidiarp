/*!
 * @file storagebutton.cpp
 * @brief Implements the StorageButton QToolButton class.
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
#include <QBoxLayout>

#include "storagebutton.h"

StorageButton::StorageButton(QWidget * parent): QToolButton(parent)
{
    QHBoxLayout *boxlayout = new QHBoxLayout(this);
    secondText = new QLabel(this);
    secondText->setFont(QFont("Helvetica", 8, QFont::Bold));
    boxlayout->setMargin(4);
    boxlayout->addStretch();
    boxlayout->addWidget(secondText);
    setFixedSize(QSize(104, 25));
    setLayout(boxlayout);
}

StorageButton::~StorageButton()
{
}

void StorageButton::setSecondText(const QString & newtext, int type)
{
    secondText->setText(newtext);
    switch (type) {
        case 0:
            secondText->setStyleSheet("");
        break;
        case 1:
            secondText->setStyleSheet(
            "QFrame { color:rgba(255, 255, 250); background-color: rgba(180, 0, 0, 60%); }");
        break;
        case 2:
            secondText->setStyleSheet(
            "QFrame { color:rgba(255, 255, 250); background-color: rgba(0, 0, 0, 40%); }");
        break;
    };
}
