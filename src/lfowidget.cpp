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
#include <QGridLayout>
#include <QStringList>
#include <QGroupBox>
#include <QLineEdit>
#include <QInputDialog>

#include "midilfo.h"
#include "lfowidget.h"
#include "slider.h"
#include "lfoscreen.h"
#include "config.h"


LfoWidget::LfoWidget(MidiLfo *p_midiLfo, int portCount, QWidget *parent):
    QWidget(parent), midiLfo(p_midiLfo), modified(false)
{

    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);
    connect(muteOut, SIGNAL(toggled(bool)), midiLfo, SLOT(muteLfo(bool)));
    muteLabel->setBuddy(muteOut);

    QLabel *ccnumberLabel = new QLabel(tr("&MIDI CC#"), portBox);
    ccnumberBox = new QSpinBox(portBox);
    ccnumberLabel->setBuddy(ccnumberBox);
    ccnumberBox->setRange(0, 127);
    ccnumberBox->setValue(74);
    ccnumberBox->setToolTip(tr("MIDI Controller number sent to output"));
    connect(ccnumberBox, SIGNAL(valueChanged(int)), this,
            SLOT(updateCcnumber(int)));

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
    portBoxLayout->addWidget(ccnumberLabel, 1, 0);
    portBoxLayout->addWidget(ccnumberBox, 1, 1);
    portBoxLayout->addWidget(portLabel, 2, 0);
    portBoxLayout->addWidget(portOut, 2, 1);
    portBoxLayout->addWidget(channelLabel, 3, 0);
    portBoxLayout->addWidget(channelOut, 3, 1);

    QVBoxLayout* outputLayout = new QVBoxLayout;
    outputLayout->addLayout(portBoxLayout);
    outputLayout->addStretch();

    portBox->setLayout(outputLayout);

    // group box for wave setup
    QGroupBox *patternBox = new QGroupBox(tr("Wave"), this);

    lfoScreen = new LfoScreen(this); 
    lfoScreen->setMinimumHeight(80);
    connect(lfoScreen, SIGNAL(lfoMouseMoved(double, double, int)), this,
            SLOT(mouseMoved(double, double, int)));
    connect(lfoScreen, SIGNAL(lfoMousePressed(double, double, int)), this,
            SLOT(mousePressed(double, double, int)));
    QLabel *waveFormBoxLabel = new QLabel(tr("&Waveform"), patternBox);
    waveFormBox = new QComboBox(patternBox);
    waveFormBoxLabel->setBuddy(waveFormBox);
    loadWaveForms();
    waveFormBox->insertItems(0, waveForms);
    waveFormBox->setCurrentIndex(0);
    waveFormBox->setToolTip(tr("Waveform Basis"));
    waveFormBox->setMinimumContentsLength(8);
    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));

    QLabel *freqBoxLabel = new QLabel(tr("&Frequency (cycles/beat)"),
            patternBox);
    freqBox = new QComboBox(patternBox);
    freqBoxLabel->setBuddy(freqBox);
    QStringList names;
    names << "1/4" << "1/2" << "3/4" << "1" << "2" << "3" 
        << "4" << "5" << "6" << "7" << "8";
    freqBox->insertItems(0, names);
    freqBox->setCurrentIndex(3);
    freqBox->setToolTip(
            tr("Frequency: Number of wave cycles produced every beat"));
    freqBox->setMinimumContentsLength(3);
    connect(freqBox, SIGNAL(activated(int)), this,
            SLOT(updateFreq(int)));

    QLabel *resBoxLabel = new QLabel(tr("&Resolution (events/beat)"),
            patternBox);
    resBox = new QComboBox(patternBox);
    resBoxLabel->setBuddy(resBox);
    names.clear();
    names << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "96" << "192";
    resBox->insertItems(0, names);
    resBox->setCurrentIndex(4);
    resBox->setToolTip(
            tr("Resolution: Number of events produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));

    QLabel *sizeBoxLabel = new QLabel(tr("&Length (beats)"), patternBox);
    sizeBox = new QComboBox(patternBox);
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(0);
    sizeBox->setToolTip(tr("Length of LFO wave in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));

    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), patternBox);
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));

    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), patternBox);
    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));
    
    QVBoxLayout* sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(amplitude);
    sliderLayout->addWidget(offset);
    sliderLayout->addStretch();

    QGridLayout *patternBoxLayout = new QGridLayout;
    patternBoxLayout->addWidget(waveFormBoxLabel, 0, 0);
    patternBoxLayout->addWidget(waveFormBox, 0, 1);

    patternBoxLayout->addWidget(freqBoxLabel, 1, 0);
    patternBoxLayout->addWidget(freqBox, 1, 1);

    patternBoxLayout->addWidget(resBoxLabel, 2, 0);
    patternBoxLayout->addWidget(resBox, 2, 1);

    patternBoxLayout->addWidget(sizeBoxLabel, 3, 0);
    patternBoxLayout->addWidget(sizeBox, 3, 1);
    
    QGridLayout* waveBoxLayout = new QGridLayout;
    waveBoxLayout->addWidget(lfoScreen, 0, 0, 1, 2);
    waveBoxLayout->addLayout(patternBoxLayout, 1, 0);
    waveBoxLayout->addLayout(sliderLayout, 1, 1);

    patternBox->setLayout(waveBoxLayout); 

    QHBoxLayout *lfoWidgetLayout = new QHBoxLayout;
    lfoWidgetLayout->addWidget(patternBox);
    lfoWidgetLayout->addWidget(portBox);

    setLayout(lfoWidgetLayout);
    updateAmp(64);
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
    modified = true;
}

void LfoWidget::updateChannelOut(int value)
{
    midiLfo->channelOut = value - 1;
    modified = true;
}

void LfoWidget::writeLfo(QTextStream& arpText)
{
    arpText << midiLfo->channelOut << ' ' 
        << midiLfo->portOut << ' '
        << midiLfo->ccnumber << '\n';
    arpText << freqBox->currentIndex() << ' '
        << resBox->currentIndex() << ' '
        << sizeBox->currentIndex() << ' '
        << midiLfo->amp << ' '
        << midiLfo->offs << '\n';
    arpText << waveFormBox->currentIndex() << '\n';
    arpText << "EOP\n"; // End Of Pattern
    modified = false;
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
    ccnumberBox->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    freqBox->setCurrentIndex(qs2.toInt());
    updateFreq(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    resBox->setCurrentIndex(qs2.toInt());
    updateRes(qs2.toInt());
    qs2 = qs.section(' ', 2, 2); 
    sizeBox->setCurrentIndex(qs2.toInt());
    updateSize(qs2.toInt());
    qs2 = qs.section(' ', 3, 3); 
    amplitude->setValue(qs2.toInt());
    qs2 = qs.section(' ', 4, 4); 
    offset->setValue(qs2.toInt());
    qs = arpText.readLine();
    waveFormBox->setCurrentIndex(qs.toInt());
    updateWaveForm(qs.toInt());
    qs = arpText.readLine();
    modified = false;
}                                      


void LfoWidget::updateCcnumber(int val)
{
    midiLfo->ccnumber = val;
    modified = true;
}

void LfoWidget::setPortOut(int value)
{
    portOut->setValue(value);
    modified = true;
}

void LfoWidget::setChannelOut(int value)
{
    channelOut->setValue(value);
    modified = true;
}

void LfoWidget::updateWaveForm(int val)
{
    midiLfo->updateWaveForm(val);
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::loadWaveForms()
{
    waveForms << tr("Sine") << tr("Saw up") << tr("Triangle") 
        << tr("Saw down") << tr("Square") << tr("Custom");
}

void LfoWidget::updateFreq(int val)
{
    midiLfo->freq = lfoFreqValues[val];
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateRes(int val)
{
    midiLfo->res = lfoResValues[val];
    midiLfo->resizeAll();
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateSize(int val)
{
    midiLfo->size = val + 1;
    midiLfo->resizeAll();
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateAmp(int val)
{
    midiLfo->amp = val;
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateOffs(int val)
{
    midiLfo->offs = val;
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::mouseMoved(double mouseX, double mouseY, int buttons)
{
    if ((buttons == 1) && (waveFormBox->currentIndex() == 5)) {
        midiLfo->setCustomWavePoint(mouseX, mouseY);
        midiLfo->getData(&lfoData);
        lfoScreen->updateScreen(lfoData);
        }
}
void LfoWidget::mousePressed(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        midiLfo->toggleMutePoint(mouseX);
        midiLfo->getData(&lfoData);
        lfoScreen->updateScreen(lfoData);
    } else
        if (waveFormBox->currentIndex() == 5) {
            midiLfo->setCustomWavePoint(mouseX, mouseY);
            midiLfo->getData(&lfoData);
            lfoScreen->updateScreen(lfoData);
        }
}

bool LfoWidget::isModified()
{
    return modified;
}

void LfoWidget::setModified(bool m)
{
    modified = m;
}

