/*
 *      lfowidget.cpp
 *      
 *      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
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

#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>

#include "midilfo.h"
#include "lfowidget.h"
#include "slider.h"
#include "lfoscreen.h"
#include "config.h"


LfoWidget::LfoWidget(MidiLfo *p_midiLfo, int portCount, QWidget *parent)
: QWidget(parent), midiLfo(p_midiLfo)
{

    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

	QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);
    connect(muteOut, SIGNAL(toggled(bool)), midiLfo, SLOT(muteLfo(bool)));
	muteLabel->setBuddy(muteOut);
	
    QLabel *lfoCCnumberLabel = new QLabel(tr("MIDI CC#"), portBox);
    lfoCCnumberBox = new QSpinBox(portBox);
	lfoCCnumberLabel->setBuddy(lfoCCnumberBox);
    lfoCCnumberBox->setRange(0, 127);
    lfoCCnumberBox->setValue(74);
    lfoCCnumberBox->setToolTip(tr("MIDI Controller number sent to output"));
    connect(lfoCCnumberBox, SIGNAL(valueChanged(int)), this,
            SLOT(updateLfoCCnumber(int)));

    QLabel *channelLabel = new QLabel(tr("C&hannel"), portBox);
    channelOut = new QSpinBox(portBox);
    channelLabel->setBuddy(channelOut);
    channelOut->setRange(1, 16);
 	channelOut->setKeyboardTracking(false);
    connect(channelOut, SIGNAL(valueChanged(int)), this,
            SLOT(updateChannelOut(int)));

    QLabel *portLabel = new QLabel(tr("&Port"), portBox);
    portOut = new QSpinBox(portBox);
    portLabel->setBuddy(portOut);
    portOut->setRange(1, portCount);
 	portOut->setKeyboardTracking(false);
    connect(portOut, SIGNAL(valueChanged(int)), this, SLOT(updatePortOut(int)));

    QGridLayout *portBoxLayout = new QGridLayout;
    portBoxLayout->addWidget(muteLabel, 0, 0);
    portBoxLayout->addWidget(muteOut, 0, 1);
    portBoxLayout->addWidget(lfoCCnumberLabel, 1, 0);
    portBoxLayout->addWidget(lfoCCnumberBox, 1, 1);
    portBoxLayout->addWidget(portLabel, 2, 0);
    portBoxLayout->addWidget(portOut, 2, 1);
    portBoxLayout->addWidget(channelLabel, 3, 0);
    portBoxLayout->addWidget(channelOut, 3, 1);
    portBox->setLayout(portBoxLayout);

    // group box for pattern setup
    QGroupBox *patternBox = new QGroupBox(tr("Wave"), this);
	
    QWidget *lfoScreenBox = new QWidget(patternBox);
    QHBoxLayout *lfoScreenBoxLayout = new QHBoxLayout;
    lfoScreen = new LfoScreen(patternBox); 
    lfoScreenBox->setMinimumHeight(80);
    lfoScreenBoxLayout->addWidget(lfoScreen);
    lfoScreenBoxLayout->setMargin(1);
    lfoScreenBoxLayout->setSpacing(1);
    lfoScreenBox->setLayout(lfoScreenBoxLayout);

    QLabel *waveFormBoxLabel = new QLabel(tr("Waveform"), patternBox);
    waveFormBox = new QComboBox(patternBox);
	waveFormBoxLabel->setBuddy(waveFormBox);
	loadWaveForms();
    waveFormBox->insertItems(0, waveForms);
    waveFormBox->setCurrentIndex(0);
    waveFormBox->setToolTip(tr("Waveform Basis"));
    waveFormBox->setMinimumContentsLength(5);
    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
			
    QLabel *lfoFreqBoxLabel = new QLabel(tr("Frequency (cycles/beat)"), patternBox);
    lfoFreqBox = new QComboBox(patternBox);
	lfoFreqBoxLabel->setBuddy(lfoResBox);
	QStringList names;
	names 	<< "1/4" << "1/2" << "3/4" << "1" << "2" << "3" 
			<< "4" << "5" << "6" << "7" << "8";
    lfoFreqBox->insertItems(0, names);
    lfoFreqBox->setCurrentIndex(3);
    lfoFreqBox->setToolTip(tr("Frequency: Number wave cycles produced every beat"));
    lfoFreqBox->setMinimumContentsLength(3);
    connect(lfoFreqBox, SIGNAL(activated(int)), this,
            SLOT(updateLfoFreq(int)));

    QLabel *lfoResBoxLabel = new QLabel(tr("Resolution (events/beat)"), patternBox);
    lfoResBox = new QComboBox(patternBox);
	lfoResBoxLabel->setBuddy(lfoResBox);
	names.clear();
	names << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "96" << "192";
    lfoResBox->insertItems(0, names);
    lfoResBox->setCurrentIndex(4);
    lfoResBox->setToolTip(tr("Resolution: Number of events produced every beat"));
    lfoResBox->setMinimumContentsLength(3);
    connect(lfoResBox, SIGNAL(activated(int)), this,
            SLOT(updateLfoRes(int)));
			
    QLabel *lfoSizeBoxLabel = new QLabel(tr("Length (beats)"), patternBox);
    lfoSizeBox = new QComboBox(patternBox);
	lfoSizeBoxLabel->setBuddy(lfoSizeBox);
	names.clear();
	names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    lfoSizeBox->insertItems(0, names);
    lfoSizeBox->setCurrentIndex(0);
    lfoSizeBox->setToolTip(tr("Length of LFO wave in beats"));
    lfoSizeBox->setMinimumContentsLength(3);
    connect(lfoSizeBox, SIGNAL(activated(int)), this,
            SLOT(updateLfoSize(int)));

    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal, tr("&Amplitude"), patternBox);
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateLfoAmp(int)));
	amplitude->setMinimumWidth(250);
    offset = new Slider(0, 127, 1, 8, 64, Qt::Horizontal, tr("&Offset"), patternBox);
    connect(offset, SIGNAL(valueChanged(int)), midiLfo,
            SLOT(updateOffset(int))); 
	offset->setDisabled(true);


    QGridLayout *patternBoxLayout = new QGridLayout;
    patternBoxLayout->addWidget(lfoScreenBox,0,0,1,3);
    patternBoxLayout->addWidget(waveFormBoxLabel,1,0);
    patternBoxLayout->addWidget(waveFormBox,1,1);
    patternBoxLayout->addWidget(amplitude,1,2,2,1);
	
    patternBoxLayout->addWidget(lfoFreqBoxLabel,2,0);
    patternBoxLayout->addWidget(lfoFreqBox,2,1);
    patternBoxLayout->addWidget(offset,3,2,2,1);
	
    patternBoxLayout->addWidget(lfoResBoxLabel,3,0);
    patternBoxLayout->addWidget(lfoResBox,3,1);
    patternBoxLayout->addWidget(lfoSizeBoxLabel,4,0);
    patternBoxLayout->addWidget(lfoSizeBox,4,1);
	patternBoxLayout->setColumnStretch(2,5);
    //patternBoxLayout->setMargin(1);
    //patternBoxLayout->setSpacing(1);
    patternBox->setLayout(patternBoxLayout); 



    QGridLayout *lfoWidgetLayout = new QGridLayout;
    lfoWidgetLayout->addWidget(patternBox,0,0);
    lfoWidgetLayout->addWidget(portBox,0,1);
    lfoWidgetLayout->setRowStretch(2,1);

    lfoWidgetLayout->setMargin(2);
    lfoWidgetLayout->setSpacing(5);
	
    setLayout(lfoWidgetLayout);
	updateLfoAmp(64);
}

LfoWidget::~LfoWidget()
{
}

MidiLfo *LfoWidget::getMidiLfo()
{
    return (midiLfo);
}

void LfoWidget::updatePortOut(int value)
{
    midiLfo->portOut = value - 1;
}

void LfoWidget::updateChannelOut(int value)
{
    midiLfo->channelOut = value - 1;
}

void LfoWidget::writeLfo(QTextStream& arpText)
{
    arpText << midiLfo->channelOut << ' ' 
		<< midiLfo->portOut << ' '
		<< midiLfo->lfoCCnumber << '\n';
    arpText << lfoFreqBox->currentIndex() << ' '
        << lfoResBox->currentIndex() << ' '
        << lfoSizeBox->currentIndex() << ' '
        << midiLfo->lfoAmp << ' '
		<< midiLfo->lfoOffs << '\n';
    arpText << waveFormBox->currentIndex() << '\n';
    arpText << "EOP\n"; // End Of Pattern
}                                      

void LfoWidget::readLfo(QTextStream& arpText)
{
    QString qs, qs2;

    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    channelOut->setValue(qs2.toInt() + 1);
    qs2 = qs.section(' ', 1, 1); 
    portOut->setValue(qs2.toInt() + 1);
    qs2 = qs.section(' ', 2, 2); 
    lfoCCnumberBox->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    lfoFreqBox->setCurrentIndex(qs2.toInt());
	updateLfoFreq(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    lfoResBox->setCurrentIndex(qs2.toInt());
	updateLfoRes(qs2.toInt());
    qs2 = qs.section(' ', 2, 2); 
    lfoSizeBox->setCurrentIndex(qs2.toInt());
	updateLfoSize(qs2.toInt());
    qs2 = qs.section(' ', 3, 3); 
    amplitude->setValue(qs2.toInt());
    qs2 = qs.section(' ', 4, 4); 
    offset->setValue(qs2.toInt());
    qs = arpText.readLine();
	waveFormBox->setCurrentIndex(qs.toInt());
	updateWaveForm(qs.toInt());
    qs = arpText.readLine();
	
}                                      


void LfoWidget::updateLfoCCnumber(int val)
{
	midiLfo->lfoCCnumber = val;
}

void LfoWidget::setPortOut(int value)
{
    portOut->setValue(value);
}

void LfoWidget::setChannelOut(int value)
{

    channelOut->setValue(value);
}

void LfoWidget::updateWaveForm(int val)
{
	midiLfo->waveFormIndex = val;
	midiLfo->getData(&lfoData);
	lfoScreen->updateLfoScreen(lfoData);
}

void LfoWidget::loadWaveForms()
{
    waveForms << tr("Sine") << tr("Saw up") << tr("Triangle") 
				<< tr("Saw down") << tr("Square");
}

void LfoWidget::updateLfoFreq(int val)
{
	midiLfo->lfoFreq = lfoFreqValues[val];
	midiLfo->getData(&lfoData);
	lfoScreen->updateLfoScreen(lfoData);
}

void LfoWidget::updateLfoRes(int val)
{
	midiLfo->lfoRes = lfoResValues[val];
	midiLfo->getData(&lfoData);
	lfoScreen->updateLfoScreen(lfoData);
}

void LfoWidget::updateLfoSize(int val)
{
	midiLfo->lfoSize = val + 1;
	midiLfo->getData(&lfoData);
	lfoScreen->updateLfoScreen(lfoData);
}

void LfoWidget::updateLfoAmp(int val)
{
	midiLfo->lfoAmp = val;
	midiLfo->getData(&lfoData);
	lfoScreen->updateLfoScreen(lfoData);
}

