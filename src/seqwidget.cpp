/*
 *      seqwidget.cpp
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

#include <QLabel>
#include <QBoxLayout>
#include <QGridLayout>
#include <QStringList>
#include <QGroupBox>
#include <QInputDialog>

#include "midiseq.h"
#include "seqwidget.h"
#include "slider.h"
#include "seqscreen.h"
#include "pixmaps/seqwavcp.xpm"
#include "config.h"


SeqWidget::SeqWidget(MidiSeq *p_midiSeq, int portCount, QWidget *parent):
    QWidget(parent), midiSeq(p_midiSeq), modified(false)
{
    // Input group box on right top
    QGroupBox *inBox = new QGroupBox(tr("Input"), this);

    QLabel *enableNoteInLabel = new QLabel(tr("&Note"),inBox);
    enableNoteIn = new QCheckBox(this);
    connect(enableNoteIn, SIGNAL(toggled(bool)), this, SLOT(updateEnableNoteIn(bool)));
    enableNoteInLabel->setBuddy(enableNoteIn);
    enableNoteIn->setToolTip(tr("Transpose the sequence following incoming notes"));
    
    QLabel *enableVelInLabel = new QLabel(tr("&Velocity"),inBox);
    enableVelIn = new QCheckBox(this);
    connect(enableVelIn, SIGNAL(toggled(bool)), this, SLOT(updateEnableVelIn(bool)));
    enableVelInLabel->setBuddy(enableVelIn);
    enableVelIn->setToolTip(tr("Set sequence velocity to that of incoming notes"));
    
    enableNoteIn->setChecked(true);
    enableVelIn->setChecked(true);
    
    QLabel *chInLabel = new QLabel(tr("&Channel"), inBox);
    chIn = new QSpinBox(inBox);
    chIn->setRange(1, 16);
    chIn->setKeyboardTracking(false);
    chInLabel->setBuddy(chIn);
    connect(chIn, SIGNAL(valueChanged(int)), this, SLOT(updateChIn(int)));

    QGridLayout *inBoxLayout = new QGridLayout;

    inBoxLayout->addWidget(enableNoteInLabel, 0, 0);
    inBoxLayout->addWidget(enableNoteIn, 0, 1);
    inBoxLayout->addWidget(enableVelInLabel, 1, 0);
    inBoxLayout->addWidget(enableVelIn, 1, 1);
    inBoxLayout->addWidget(chInLabel, 2, 0);
    inBoxLayout->addWidget(chIn, 2, 1);

    inBox->setLayout(inBoxLayout); 


    // Output group box on right bottom
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);
    connect(muteOut, SIGNAL(toggled(bool)), midiSeq, SLOT(muteSeq(bool)));
    muteLabel->setBuddy(muteOut);

    QLabel *portLabel = new QLabel(tr("&Port"), portBox);
    portOut = new QSpinBox(portBox);
    portLabel->setBuddy(portOut);
    portOut->setRange(1, portCount);
    portOut->setKeyboardTracking(false);
    connect(portOut, SIGNAL(valueChanged(int)), this, SLOT(updatePortOut(int)));

    QLabel *channelLabel = new QLabel(tr("C&hannel"), portBox);
    channelOut = new QSpinBox(portBox);
    channelLabel->setBuddy(channelOut);
    channelOut->setRange(1, 16);
    channelOut->setKeyboardTracking(false);
    connect(channelOut, SIGNAL(valueChanged(int)), this,
            SLOT(updateChannelOut(int)));

    QGridLayout *portBoxLayout = new QGridLayout;
    portBoxLayout->addWidget(muteLabel, 0, 0);
    portBoxLayout->addWidget(muteOut, 0, 1);
    portBoxLayout->addWidget(portLabel, 1, 0);
    portBoxLayout->addWidget(portOut, 1, 1);
    portBoxLayout->addWidget(channelLabel, 2, 0);
    portBoxLayout->addWidget(channelOut, 2, 1);

    QVBoxLayout* outputLayout = new QVBoxLayout;
    outputLayout->addLayout(portBoxLayout);

    portBox->setLayout(outputLayout);

    // group box for sequence setup
    QGroupBox *patternBox = new QGroupBox(tr("Sequence"), this);

    seqScreen = new SeqScreen(this); 
    seqScreen->setToolTip(
        tr("Right button to mute points, left button to draw custom wave"));
    seqScreen->setMinimumHeight(SEQSCREEN_MINIMUM_HEIGHT);
    connect(seqScreen, SIGNAL(seqMouseMoved(double, double, int)), this,
            SLOT(mouseMoved(double, double, int)));
    connect(seqScreen, SIGNAL(seqMousePressed(double, double, int)), this,
            SLOT(mousePressed(double, double, int)));
            
    QLabel *waveFormBoxLabel = new QLabel(tr("&Sequence"), patternBox);
    waveFormBox = new QComboBox(patternBox);
    waveFormBoxLabel->setBuddy(waveFormBox);
    loadWaveForms();
    waveFormBox->insertItems(0, waveForms);
    waveFormBox->setCurrentIndex(0);
    waveFormBox->setToolTip(tr("Preset Number"));
    waveFormBox->setMinimumContentsLength(8);
    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
    
    QLabel *resBoxLabel = new QLabel(tr("&Resolution"),
            patternBox);
    resBox = new QComboBox(patternBox);
    resBoxLabel->setBuddy(resBox);
    QStringList names;
    names.clear();
    names << "1" << "2" << "4" << "8" << "16";
    resBox->insertItems(0, names);
    resBox->setCurrentIndex(2);
    resBox->setToolTip(
            tr("Resolution (notes/beat): Number of notes produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));

    QLabel *sizeBoxLabel = new QLabel(tr("&Length"), patternBox);
    sizeBox = new QComboBox(patternBox);
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(3);
    sizeBox->setToolTip(tr("Length of Sequence in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));
    
    copyToCustomButton = new QToolButton(this); 
    copyToCustomAction = new QAction( QIcon(seqwavcp_xpm),
            tr("C&opy to new wave"), this);
    connect(copyToCustomAction, SIGNAL(triggered()), this,
            SLOT(copyToCustom()));
    copyToCustomButton->setDefaultAction(copyToCustomAction);
    
//temporarily hide these elements until multiple patterns are implemented  
    copyToCustomAction->setEnabled(false);
    waveFormBox->setEnabled(false);
    copyToCustomButton->setVisible(false);
    waveFormBox->setVisible(false);
    waveFormBoxLabel->setVisible(false);


    velocity = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("Veloc&ity"), patternBox);
    connect(velocity, SIGNAL(valueChanged(int)), this,
            SLOT(updateVelocity(int)));
            
    notelength = new Slider(0, 255, 1, 16, 64, Qt::Horizontal,
            tr("N&ote Length"), patternBox);
    connect(notelength, SIGNAL(valueChanged(int)), this,
            SLOT(updateNoteLength(int)));
            
    transpose = new Slider(-24, 24, 1, 2, 0, Qt::Horizontal,
            tr("&Transpose"), patternBox);
    connect(transpose, SIGNAL(valueChanged(int)), this,
            SLOT(updateTranspose(int)));

    
    QGridLayout* sliderLayout = new QGridLayout;
    sliderLayout->addWidget(copyToCustomButton, 0 , 0);
    sliderLayout->addWidget(velocity, 1, 0);
    sliderLayout->addWidget(notelength, 2, 0);
    sliderLayout->addWidget(transpose, 3, 0);
    sliderLayout->setRowStretch(4, 1);

    QGridLayout *patternBoxLayout = new QGridLayout;
    patternBoxLayout->addWidget(waveFormBoxLabel, 0, 0);
    patternBoxLayout->addWidget(waveFormBox, 0, 1);
    patternBoxLayout->addWidget(resBoxLabel, 1, 0);
    patternBoxLayout->addWidget(resBox, 1, 1);
    patternBoxLayout->addWidget(sizeBoxLabel, 2, 0);
    patternBoxLayout->addWidget(sizeBox, 2, 1);
    patternBoxLayout->setRowStretch(3, 1);
    
    QGridLayout* waveBoxLayout = new QGridLayout;
    waveBoxLayout->addWidget(seqScreen, 0, 0, 1, 2);
    waveBoxLayout->addLayout(patternBoxLayout, 1, 0);
    waveBoxLayout->addLayout(sliderLayout, 1, 1);

    patternBox->setLayout(waveBoxLayout); 
    
    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();
    
    QHBoxLayout *seqWidgetLayout = new QHBoxLayout;
    seqWidgetLayout->addWidget(patternBox);
    seqWidgetLayout->addLayout(inOutBoxLayout);

    setLayout(seqWidgetLayout);
    updateVelocity(64);
    updateWaveForm(0);
}

SeqWidget::~SeqWidget()
{
}

MidiSeq *SeqWidget::getMidiSeq()
{
    return (midiSeq);
}

void SeqWidget::writeSeq(QTextStream& arpText)
{
    int l1 = 0;
    arpText << midiSeq->enableNoteIn << ' ' 
        << midiSeq->enableVelIn << ' '
        << midiSeq->chIn << '\n';
    arpText << midiSeq->channelOut << ' ' 
        << midiSeq->portOut << ' '
        << midiSeq->notelength << '\n';
    arpText << resBox->currentIndex() << ' '
        << sizeBox->currentIndex() << ' '
        << midiSeq->vel << ' '
        << midiSeq->transp << '\n';
    arpText << waveFormBox->currentIndex() << '\n';
    // Write Mute Mask
    while (l1 < midiSeq->muteMask.count()) {
        arpText << midiSeq->muteMask.at(l1) << ' ';
        l1++;
        if (!(l1 % 32)) arpText << "\n";
    }
    arpText << "EOM\n"; // End Of Mute
    // Write Custom Sequence
    l1 = 0;
    while (l1 < midiSeq->customWave.count()) {
        arpText << midiSeq->customWave.at(l1).value << ' ';
        l1++;
        if (!(l1 % 16)) arpText << "\n";
    }
    arpText << "EOS\n"; // End Of Wave
    modified = false;
}                                      

void SeqWidget::readSeq(QTextStream& arpText)
{
    QString qs, qs2;
    int l1, lt, wvtmp;
    SeqSample seqSample;
    
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    enableNoteIn->setChecked(qs2.toInt());
    qs2 = qs.section(' ', 1, 1); 
    enableVelIn->setChecked(qs2.toInt());
    qs2 = qs.section(' ', 2, 2); 
    chIn->setValue(qs2.toInt() + 1);
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    channelOut->setValue(qs2.toInt() + 1);
    qs2 = qs.section(' ', 1, 1); 
    portOut->setValue(qs2.toInt() + 1);
    qs2 = qs.section(' ', 2, 2); 
    notelength->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    resBox->setCurrentIndex(qs2.toInt());
    updateRes(qs2.toInt());
    qs2 = qs.section(' ', 1, 1); 
    sizeBox->setCurrentIndex(qs2.toInt());
    updateSize(qs2.toInt());
    qs2 = qs.section(' ', 2, 2); 
    velocity->setValue(qs2.toInt());
    qs2 = qs.section(' ', 3, 3); 
    transpose->setValue(qs2.toInt());
    qs = arpText.readLine();
    wvtmp = qs.toInt();
    
    // Read Mute Mask
    int step = TICKS_PER_QUARTER / midiSeq->res;
    qs = arpText.readLine();
    if (qs.isEmpty() || (qs == "EOP")) return;
    qs2 = qs.section(' ', 0, 0);
    midiSeq->muteMask.clear();
    l1 = 0;
    while (qs2 !="EOM") {
        // TODO: the following line produces a pointer deref warning, why?
        midiSeq->muteMask.append(qs2.toInt());
        l1++;
        if (!(l1%32)) qs = arpText.readLine();
        qs2 = qs.section(' ', l1%32, l1%32);
    }
    
    // Read Custom Waveform
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    midiSeq->customWave.clear();
    l1 = 0;
    lt = 0;
    while (qs2 !="EOS") {
        seqSample.value=qs2.toInt();
        seqSample.tick = lt;
        seqSample.muted = midiSeq->muteMask.at(l1);
        midiSeq->customWave.append(seqSample);
        lt+=step;
        l1++;
        if (!(l1%16)) qs = arpText.readLine();
        qs2 = qs.section(' ', l1%16, l1%16);
    }
    waveFormBox->setCurrentIndex(wvtmp);
    updateWaveForm(wvtmp);
    modified = false;
}                                      

void SeqWidget::loadWaveForms()
{
    waveForms << tr("Custom");
}

void SeqWidget::setEnableNoteIn(bool on)
{
    enableNoteIn->setChecked(on);
    enableVelIn->setEnabled(on);
    modified = true;
}

void SeqWidget::setEnableVelIn(bool on)
{
    enableVelIn->setChecked(on);
    modified = true;
}

void SeqWidget::setChIn(int value)
{
    chIn->setValue(value);
    modified = true;
}

void SeqWidget::setPortOut(int value)
{
    portOut->setValue(value);
    modified = true;
}

void SeqWidget::setChannelOut(int value)
{
    channelOut->setValue(value);
    modified = true;
}

void SeqWidget::updateChIn(int value)
{
    midiSeq->chIn = value - 1;
    modified = true;
}

void SeqWidget::updateEnableNoteIn(bool on)
{
    midiSeq->enableNoteIn = on;
    enableVelIn->setEnabled(on);
    modified = true;
}

void SeqWidget::updateEnableVelIn(bool on)
{
    midiSeq->enableVelIn = on;
    modified = true;
}

void SeqWidget::updatePortOut(int value)
{
    midiSeq->portOut = value - 1;
    modified = true;
}

void SeqWidget::updateChannelOut(int value)
{
    midiSeq->channelOut = value - 1;
    modified = true;
}

void SeqWidget::updateNoteLength(int val)
{
    midiSeq->notelength = val;
    modified = true;
}

void SeqWidget::updateWaveForm(int val)
{
    midiSeq->updateWaveForm(val);
    midiSeq->getData(&seqData);
    seqScreen->updateScreen(seqData);
    modified = true;
}

void SeqWidget::updateRes(int val)
{
    midiSeq->res = seqResValues[val];
    midiSeq->resizeAll();
    midiSeq->getData(&seqData);
    seqScreen->updateScreen(seqData);
    modified = true;
}

void SeqWidget::updateSize(int val)
{
    midiSeq->size = val + 1;
    midiSeq->resizeAll();
    midiSeq->getData(&seqData);
    seqScreen->updateScreen(seqData);
    modified = true;
}

void SeqWidget::updateVelocity(int val)
{
    midiSeq->vel = val;
    modified = true;
}

void SeqWidget::updateTranspose(int val)
{
    midiSeq->transp = val;
    modified = true;
}

void SeqWidget::copyToCustom()
{
    midiSeq->copyToCustom();
    waveFormBox->setCurrentIndex(0);
    updateWaveForm(0);
    modified = true;
}

void SeqWidget::mouseMoved(double mouseX, double mouseY, int buttons)
{
    if (buttons == 1) {
        midiSeq->setCustomWavePoint(mouseX, mouseY);
        midiSeq->getData(&seqData);
        seqScreen->updateScreen(seqData);
        modified = true;
        }
}

void SeqWidget::mousePressed(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        midiSeq->toggleMutePoint(mouseX);
        midiSeq->getData(&seqData);
        seqScreen->updateScreen(seqData);
        modified = true;
    } else {
        midiSeq->setCustomWavePoint(mouseX, mouseY);
        midiSeq->getData(&seqData);
        seqScreen->updateScreen(seqData);
        modified = true;
    }
}

bool SeqWidget::isModified()
{
    return modified;
}

void SeqWidget::setModified(bool m)
{
    modified = m;
}

