/*!
 * @file storagebutton.cpp
 * @brief Implements the StorageButton QToolButton class.
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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
    thirdText = new QLabel(this);
    // thirdText->setStyleSheet(
    // "QFrame { color:rgba(255, 255, 250); background-color: rgba(0, 0, 0, 60%); }");
    boxlayout->setMargin(4);
    hitLabel = new QLabel(">|<", this);
    hitLabel->setStyleSheet("QFrame { color:rgba(255, 255, 250); \
                            background-color: rgba(100, 100, 100, 60%); }");
    boxlayout->addWidget(hitLabel);
    boxlayout->addStretch();
    boxlayout->addWidget(thirdText);
    boxlayout->addWidget(secondText);
    setFixedSize(QSize(104, 25));
    setLayout(boxlayout);
    setStyleSheet("font: 12pt; font-weight: bold");
    
    hitButtonPressed = false;
}

StorageButton::~StorageButton()
{
}

void StorageButton::setNRep(int nrep)
{
    thirdText->setText("[ "+QString::number(nrep)+" ]");
    if (nrep <= 1) {
        thirdText->hide();
    }
    else {
        thirdText->show();
    }
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

void StorageButton::setBGColor(int color)
{
    QString styleSheet;

    if (color == 1)         //green
        styleSheet = "QToolButton { background-color: rgba(50, 255, 50, 40%); }";
    else if (color == 2)    //yellow
        styleSheet = "QToolButton { background-color: rgba(150, 255, 150, 10%); }";
    else if (color == 3)    //blueish
        styleSheet = "QToolButton { }";
    else                    //no color
        styleSheet = "QToolButton { }";

    setStyleSheet(styleSheet);
}

void StorageButton::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton && hitForceButton(e->pos())) {
        setProperty("forceStay", true);
    }
    else {
        setProperty("forceStay", false);
    }
        
    QToolButton::mousePressEvent(e);
}

bool StorageButton::hitForceButton(const QPoint &pos) const
{
    return hitLabel->frameRect().contains(pos);
}

