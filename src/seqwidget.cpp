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
#include <QMessageBox>

#include "midiseq.h"
#include "seqwidget.h"
#include "slider.h"
#include "seqscreen.h"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
#include "pixmaps/seqwavcp.xpm"
#include "config.h"


SeqWidget::SeqWidget(MidiSeq *p_midiSeq, int portCount, bool compactStyle, QWidget *parent):
    QWidget(parent), midiSeq(p_midiSeq), modified(false)
{
    // Management Buttons on the right top
    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

    renameAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."), this);
    renameAction->setToolTip(tr("Rename this Sequencer"));
    QToolButton *renameButton = new QToolButton(this);
    renameButton->setDefaultAction(renameAction);
    connect(renameAction, SIGNAL(triggered()), this, SLOT(moduleRename()));
    
    deleteAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."), this);
    deleteAction->setToolTip(tr("Delete this Sequencer"));
    QToolButton *deleteButton = new QToolButton(this);
    deleteButton->setDefaultAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));
    
    manageBoxLayout->addStretch();
    manageBoxLayout->addWidget(renameButton);
    manageBoxLayout->addWidget(deleteButton);

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

    cancelMidiLearnAction = new QAction(tr("Cancel MIDI &Learning"), this);
    connect(cancelMidiLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnCancel()));
    cancelMidiLearnAction->setEnabled(false);

    muteOut->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    
    QAction *muteLearnAction = new QAction(tr("MIDI &Learn"), this);
    muteOut->addAction(muteLearnAction);
    connect(muteLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnMute()));
    QAction *muteForgetAction = new QAction(tr("MIDI &Forget"), this);
    muteOut->addAction(muteForgetAction);
    connect(muteForgetAction, SIGNAL(triggered()), this, SLOT(midiForgetMute()));

    muteOut->addAction(cancelMidiLearnAction);

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

    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
    inOutBoxLayout->addLayout(manageBoxLayout);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();
    
    // group box for sequence setup
    QGroupBox *seqBox = new QGroupBox(tr("Sequence"), this);

    seqScreen = new SeqScreen(this); 
    seqScreen->setToolTip(
        tr("Right button to mute points, left button to draw custom wave"));
    seqScreen->setMinimumHeight(SEQSCREEN_MINIMUM_HEIGHT);
    connect(seqScreen, SIGNAL(seqMouseMoved(double, double, int)), this,
            SLOT(mouseMoved(double, double, int)));
    connect(seqScreen, SIGNAL(seqMousePressed(double, double, int)), this,
            SLOT(mousePressed(double, double, int)));
            
    QLabel *waveFormBoxLabel = new QLabel(tr("&Sequence"), seqBox);
    waveFormBox = new QComboBox(seqBox);
    waveFormBoxLabel->setBuddy(waveFormBox);
    loadWaveForms();
    waveFormBox->insertItems(0, waveForms);
    waveFormBox->setCurrentIndex(0);
    waveFormBox->setToolTip(tr("Preset Number"));
    waveFormBox->setMinimumContentsLength(8);
    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
    
    QLabel *resBoxLabel = new QLabel(tr("&Resolution"),
            seqBox);
    resBox = new QComboBox(seqBox);
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

    QLabel *sizeBoxLabel = new QLabel(tr("&Length"), seqBox);
    sizeBox = new QComboBox(seqBox);
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
            tr("Veloc&ity"), seqBox);
            
    velocity->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    
    QAction *velocityLearnAction = new QAction(tr("MIDI &Learn"), this);
    velocity->addAction(velocityLearnAction);
    connect(velocityLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnVel()));
    QAction *velocityForgetAction = new QAction(tr("MIDI &Forget"), this);
    velocity->addAction(velocityForgetAction);
    connect(velocityForgetAction, SIGNAL(triggered()), this, SLOT(midiForgetVel()));
    
    velocity->addAction(cancelMidiLearnAction);

    connect(velocity, SIGNAL(valueChanged(int)), this,
            SLOT(updateVelocity(int)));
            
    notelength = new Slider(0, 127, 1, 16, 64, Qt::Horizontal,
            tr("N&ote Length"), seqBox);

    notelength->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    
    QAction *notelengthLearnAction = new QAction(tr("MIDI &Learn"), this);
    notelength->addAction(notelengthLearnAction);
    connect(notelengthLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnNoteLen()));
    QAction *notelengthForgetAction = new QAction(tr("MIDI &Forget"), this);
    notelength->addAction(notelengthForgetAction);
    connect(notelengthForgetAction, SIGNAL(triggered()), this, SLOT(midiForgetNoteLen()));
    
    notelength->addAction(cancelMidiLearnAction);
    
    connect(notelength, SIGNAL(valueChanged(int)), this,
            SLOT(updateNoteLength(int)));
            
    transpose = new Slider(-24, 24, 1, 2, 0, Qt::Horizontal,
            tr("&Transpose"), seqBox);
    connect(transpose, SIGNAL(valueChanged(int)), this,
            SLOT(updateTranspose(int)));

    
    QGridLayout* sliderLayout = new QGridLayout;
    sliderLayout->addWidget(copyToCustomButton, 0 , 0);
    sliderLayout->addWidget(velocity, 1, 0);
    sliderLayout->addWidget(notelength, 2, 0);
    sliderLayout->addWidget(transpose, 3, 0);
    sliderLayout->setRowStretch(4, 1);
    if (compactStyle) {
        sliderLayout->setSpacing(0);
        sliderLayout->setMargin(1);
    }

    QGridLayout *paramBoxLayout = new QGridLayout;
    paramBoxLayout->addWidget(waveFormBoxLabel, 0, 0);
    paramBoxLayout->addWidget(waveFormBox, 0, 1);
    paramBoxLayout->addWidget(resBoxLabel, 1, 0);
    paramBoxLayout->addWidget(resBox, 1, 1);
    paramBoxLayout->addWidget(sizeBoxLabel, 2, 0);
    paramBoxLayout->addWidget(sizeBox, 2, 1);
    paramBoxLayout->setRowStretch(3, 1);
    
    QGridLayout* seqBoxLayout = new QGridLayout;
    seqBoxLayout->addWidget(seqScreen, 0, 0, 1, 2);
    seqBoxLayout->addLayout(paramBoxLayout, 1, 0);
    seqBoxLayout->addLayout(sliderLayout, 1, 1);
    seqBox->setLayout(seqBoxLayout); 
    
    QHBoxLayout *seqWidgetLayout = new QHBoxLayout;
    seqWidgetLayout->addWidget(seqBox, 1);
    seqWidgetLayout->addLayout(inOutBoxLayout, 0);

    setLayout(seqWidgetLayout);
    updateVelocity(64);
    updateWaveForm(0);
    ccList.clear();
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
    arpText << "MIDICC" << endl;
    for (int l1 = 0; l1 < ccList.count(); l1++) {
		arpText	<< ccList.at(l1).ID << ' '
				<< ccList.at(l1).ccnumber << ' '
				<< ccList.at(l1).min << ' '
				<< ccList.at(l1).max << endl;
	}
	arpText << "EOCC" << endl;

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
    if (qs == "MIDICC")
    {
        qs = arpText.readLine();
        while (qs != "EOCC") {
	        qs2 = qs.section(' ', 0, 0);
	        int ctrlID = qs2.toInt();
	        qs2 = qs.section(' ', 1, 1);
	        int ccnumber = qs2.toInt();
	        qs2 = qs.section(' ', 2, 2);
	        int min = qs2.toInt();
	        qs2 = qs.section(' ', 3, 3);
	        int max = qs2.toInt();
	        appendMidiCC(ctrlID, ccnumber, min, max);
	        qs = arpText.readLine();
		}
	qs = arpText.readLine();
    }

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
    midiSeq->notelength = val + val;
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

void SeqWidget::moduleDelete()
{
    QString qs;
    qs = tr("Delete \"%1\"?")
        .arg(name);
    if (QMessageBox::question(0, APP_NAME, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
    emit seqRemove(ID);
}

void SeqWidget::moduleRename()
{
    QString newname, oldname;
    bool ok;
    
    oldname = name;

    newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);
                
    if (ok && !newname.isEmpty()) {
        name = "Seq:" + newname;
        emit dockRename(name, parentDockID);
    }
}

void SeqWidget::appendMidiCC(int ctrlID, int ccnumber, int min, int max)
{
    MidiCC midiCC;
    int l1 = 0;
    switch (ctrlID) {
		case 0: midiCC.name = "MuteToggle";
		break;
		case 1: midiCC.name = "Velocity";
		break;
		case 2: midiCC.name = "NoteLength";
		break;
		default: midiCC.name = "Unknown";
	}
    midiCC.ID = ctrlID;
    midiCC.ccnumber = ccnumber;
    midiCC.min = min;
    midiCC.max = max;
    
	while ( (l1 < ccList.count()) && 
   		((ctrlID != ccList.at(l1).ID) || 
   		(ccnumber != ccList.at(l1).ccnumber)) ) l1++;
    
    if (ccList.count() == l1) {
	    ccList.append(midiCC);
	    qWarning("MIDI Controller %d appended for %s"
	    , ccnumber, qPrintable(midiCC.name));
	}
	else {
		qWarning("MIDI Controller %d already attributed to %s"
				, ccnumber, qPrintable(midiCC.name));
	}
		
    cancelMidiLearnAction->setEnabled(false);
    modified = true;
}

void SeqWidget::removeMidiCC(int ctrlID, int ccnumber)
{
	for (int l1 = 0; l1 < ccList.count(); l1++) {
		if (ccList.at(l1).ID == ctrlID) {
			if ((ccList.at(l1).ccnumber == ccnumber) || (0 > ccnumber)) {
				ccList.remove(l1);
				l1--;
				qWarning("controller removed");
			}
		}
	}
    modified = true;
}

void SeqWidget::midiLearnMute()
{
	emit setMidiLearn(parentDockID, ID, 0);
	qWarning("Requesting Midi Learn for MuteToggle");
    cancelMidiLearnAction->setEnabled(true);
}

void SeqWidget::midiForgetMute()
{
	removeMidiCC(0, -1);
}

void SeqWidget::midiLearnNoteLen()
{
	emit setMidiLearn(parentDockID, ID, 2);
	qWarning("Requesting Midi Learn for NoteLength");
    cancelMidiLearnAction->setEnabled(true);
}

void SeqWidget::midiForgetNoteLen()
{
	removeMidiCC(2, -1);
}

void SeqWidget::midiLearnVel()
{
	emit setMidiLearn(parentDockID, ID, 1);
	qWarning("Requesting Midi Learn for Velocity");
    cancelMidiLearnAction->setEnabled(true);
}

void SeqWidget::midiForgetVel()
{
	removeMidiCC(1, -1);
}

void SeqWidget::midiLearnCancel()
{
	emit setMidiLearn(parentDockID, ID, -1);
	qWarning("Cancelling Midi Learn request");
    cancelMidiLearnAction->setEnabled(false);
}
