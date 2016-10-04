/*!
 * @file inoutbox.cpp
 * @brief Implements the InOutBox GUI class
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
 */

#include <QBoxLayout>

#include "inoutbox.h"

#ifdef APPBUILD
InOutBox::InOutBox(int portCount, bool compactStyle,
    bool inOutVisible, const QString& nameprefix):
    modified(false)
{
    manageBox = new ManageBox(nameprefix);
#else
InOutBox::InOutBox(bool compactStyle,
    bool inOutVisible, const QString& nameprefix):
    modified(false)
{
#endif

    // Input group box on left side
    QGroupBox *inBox = new QGroupBox(tr("Input"));
	
    QLabel *enableNoteInLabel = new QLabel(tr("&Note"));
    enableNoteIn = new QCheckBox;
    enableNoteInLabel->setBuddy(enableNoteIn);
    enableNoteIn->setToolTip(tr("Transpose the sequence following incoming notes"));
    if (nameprefix != "Seq:") {
		enableNoteIn->hide();
		enableNoteInLabel->hide();
	}

    QLabel *enableVelInLabel = new QLabel(tr("&Velocity"));
    enableVelIn = new QCheckBox;
    enableVelInLabel->setBuddy(enableVelIn);
    enableVelIn->setToolTip(tr("Set sequence velocity to that of incoming notes"));
    if (nameprefix != "Seq:") {
		enableVelIn->hide();
		enableVelInLabel->hide();
	}

    QLabel *enableNoteOffLabel = new QLabel(tr("&Note Off"));
    enableNoteOff = new QCheckBox;
    enableNoteOffLabel->setBuddy(enableNoteOff);
    enableNoteOff->setToolTip(tr("Stop output when Note is released"));
    if (nameprefix == "Arp:") {
		enableNoteOff->hide();
		enableNoteOffLabel->hide();
	}

    QLabel *ccnumberInLabel = new QLabel(tr("MIDI &CC#"));
    ccnumberInBox = new QSpinBox;
    ccnumberInLabel->setBuddy(ccnumberInBox);
    ccnumberInBox->setRange(0, 127);
    ccnumberInBox->setKeyboardTracking(false);
    ccnumberInBox->setValue(74);
    ccnumberInBox->setToolTip(tr("MIDI Controller number to record"));
    if (nameprefix != "LFO:") {
		ccnumberInBox->hide();
		ccnumberInLabel->hide();
	}

    QLabel *enableRestartByKbdLabel = new QLabel(tr("&Restart"));
    enableRestartByKbd = new QCheckBox;
    enableRestartByKbdLabel->setBuddy(enableRestartByKbd);
    enableRestartByKbd->setToolTip(tr("Restart when a new note is received"));

    QLabel *enableTrigByKbdLabel = new QLabel(tr("&Trigger"));
    enableTrigByKbd = new QCheckBox;
    enableTrigByKbdLabel->setBuddy(enableTrigByKbd);
    enableTrigByKbd->setToolTip(tr("Retrigger when a new note is received"));

    QLabel *enableTrigLegatoLabel = new QLabel(tr("&Legato"));
    enableTrigLegato = new QCheckBox;
    enableTrigLegatoLabel->setBuddy(enableTrigLegato);
    enableTrigLegato->setToolTip(tr("Retrigger / restart upon new legato note as well"));

    QLabel *chInLabel = new QLabel(tr("&Channel"));
    chIn = new QComboBox;
    int l1;
    for (l1 = 0; l1 < 16; l1++) chIn->addItem(QString::number(l1 + 1));
    chInLabel->setBuddy(chIn);

    inputFilterBox = new QGroupBox(tr("Note Filter"));
    indexInLabel = new QLabel(tr("&Note"));
    indexIn[0] = new QSpinBox;
    indexIn[1] = new QSpinBox;
    indexInLabel->setBuddy(indexIn[0]);
    indexIn[0]->setRange(0, 127);
    indexIn[1]->setRange(0, 127);
    indexIn[1]->setValue(127);
    indexIn[0]->setKeyboardTracking(false);
    indexIn[1]->setKeyboardTracking(false);

    rangeInLabel = new QLabel(tr("&Velocity"));
    rangeIn[0] = new QSpinBox;
    rangeIn[1] = new QSpinBox;
    rangeInLabel->setBuddy(rangeIn[0]);
    rangeIn[0]->setRange(0, 127);
    rangeIn[1]->setRange(0, 127);
    rangeIn[1]->setValue(127);
    rangeIn[0]->setKeyboardTracking(false);
    rangeIn[1]->setKeyboardTracking(false);


    QGridLayout *inputFilterBoxLayout = new QGridLayout;
    inputFilterBoxLayout->addWidget(indexInLabel, 0, 0);
    inputFilterBoxLayout->addWidget(indexIn[0], 0, 1);
    inputFilterBoxLayout->addWidget(indexIn[1], 0, 2);
    inputFilterBoxLayout->addWidget(rangeInLabel, 1, 0);
    inputFilterBoxLayout->addWidget(rangeIn[0], 1, 1);
    inputFilterBoxLayout->addWidget(rangeIn[1], 1, 2);
    inputFilterBoxLayout->setMargin(2);
    inputFilterBoxLayout->setSpacing(2);
    connect(inputFilterBox, SIGNAL(toggled(bool)), this,
            SLOT(setInputFilterVisible(bool)));
    inputFilterBox->setCheckable(true);
    inputFilterBox->setChecked(false);
    inputFilterBox->setFlat(true);
    inputFilterBox->setLayout(inputFilterBoxLayout);

    QGridLayout *inBoxLayout = new QGridLayout;
    inBoxLayout->addWidget(ccnumberInLabel, 0, 0);
    inBoxLayout->addWidget(ccnumberInBox, 0, 1);
    inBoxLayout->addWidget(enableNoteInLabel, 1, 0);
    inBoxLayout->addWidget(enableNoteIn, 1, 1);
    inBoxLayout->addWidget(enableVelInLabel, 2, 0);
    inBoxLayout->addWidget(enableVelIn, 2, 1);
    inBoxLayout->addWidget(enableNoteOffLabel, 3, 0);
    inBoxLayout->addWidget(enableNoteOff, 3, 1);
    inBoxLayout->addWidget(enableRestartByKbdLabel, 4, 0);
    inBoxLayout->addWidget(enableRestartByKbd, 4, 1);
    inBoxLayout->addWidget(enableTrigByKbdLabel, 5, 0);
    inBoxLayout->addWidget(enableTrigByKbd, 5, 1);
    inBoxLayout->addWidget(enableTrigLegatoLabel, 6, 0);
    inBoxLayout->addWidget(enableTrigLegato, 6, 1);
    inBoxLayout->addWidget(chInLabel, 7, 0);
    inBoxLayout->addWidget(chIn, 7, 1);
    inBoxLayout->addWidget(inputFilterBox, 8, 0, 1, 2);
    if (compactStyle) {
        inBoxLayout->setMargin(2);
        inBoxLayout->setSpacing(1);
    }
    inBox->setLayout(inBoxLayout);


    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"));

    QLabel *ccnumberLabel = new QLabel(tr("MIDI &CC#"));
    ccnumberBox = new QSpinBox;
    ccnumberLabel->setBuddy(ccnumberBox);
    ccnumberBox->setRange(0, 127);
    ccnumberBox->setKeyboardTracking(false);
    ccnumberBox->setValue(74);
    ccnumberBox->setToolTip(tr("MIDI Controller number sent to output"));
    if (nameprefix != "LFO:") {
		ccnumberBox->hide();
		ccnumberLabel->hide();
	}
	

    QLabel *channelLabel = new QLabel(tr("C&hannel"));
    channelOut = new QComboBox;
    channelLabel->setBuddy(channelOut);
    for (l1 = 0; l1 < 16; l1++) channelOut->addItem(QString::number(l1 + 1));

    QGridLayout *portBoxLayout = new QGridLayout;
#ifdef APPBUILD
	QLabel *portLabel = new QLabel(tr("&Port"));
	portOut = new QComboBox;
	portLabel->setBuddy(portOut);
	for (l1 = 0; l1 < portCount; l1++) portOut->addItem(QString::number(l1 + 1));
    portBoxLayout->addWidget(ccnumberLabel, 0, 0);
    portBoxLayout->addWidget(ccnumberBox, 0, 1);
	portBoxLayout->addWidget(portLabel, 1, 0);
	portBoxLayout->addWidget(portOut, 1, 1);
#endif
    portBoxLayout->addWidget(channelLabel, 2, 0);
    portBoxLayout->addWidget(channelOut, 2, 1);
    if (compactStyle) {
        portBoxLayout->setMargin(2);
        portBoxLayout->setSpacing(1);
    }
    portBox->setLayout(portBoxLayout);


    // Layout for left/right placements of in/out group boxes
    inOutBoxWidget = new QWidget;
    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
#ifdef APPBUILD
    inOutBoxLayout->addWidget(manageBox);
#endif
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();
    inOutBoxWidget->setLayout(inOutBoxLayout);
    inOutBoxWidget->setVisible(inOutVisible);
}

#ifdef APPBUILD
void InOutBox::setPortOut(int value)
{
    portOut->setCurrentIndex(value);
    modified = true;
}
#endif

void InOutBox::setChannelOut(int value)
{
    channelOut->setCurrentIndex(value);
    modified = true;
}

bool InOutBox::isModified()
{
	return modified;
}

void InOutBox::setModified(bool m)
{
    modified = m;
}

void InOutBox::setInputFilterVisible(bool on)
{
    rangeIn[0]->setVisible(on);
    rangeIn[1]->setVisible(on);
    rangeInLabel->setVisible(on);
    indexIn[0]->setVisible(on);
    indexIn[1]->setVisible(on);
    indexInLabel->setVisible(on);
}

void InOutBox::checkIfInputFilterSet()
{
    if (((indexIn[1]->value() - indexIn[0]->value()) < 127)
            || ((rangeIn[1]->value() - rangeIn[0]->value()) < 127)) {
        inputFilterBox->setFlat(false);
        inputFilterBox->setTitle(tr("Note Filter - ACTIVE"));
    }
    else {
        inputFilterBox->setFlat(true);
        inputFilterBox->setTitle(tr("Note Filter"));
    }
}

void InOutBox::setChIn(int value)
{
    chIn->setCurrentIndex(value);
    modified = true;
}

void InOutBox::setIndexIn(int index, int value)
{
    indexIn[index]->setValue(value);
    modified = true;
}

void InOutBox::setRangeIn(int index, int value)
{
    rangeIn[index]->setValue(value);
    modified = true;
}

