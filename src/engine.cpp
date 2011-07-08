/**
 * @file engine.cpp
 * @brief Implements the Engine module management class
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QString>

#include "seqdriver.h"
#include "engine.h"


Engine::Engine(int p_portCount, QWidget *parent) : QWidget(parent), modified(false)
{
    portCount = p_portCount;
    seqDriver = new SeqDriver(&midiArpList, &midiLfoList, &midiSeqList, portCount, this);
    connect(seqDriver, SIGNAL(controlEvent(int, int, int)),
            this, SLOT(handleController(int, int, int)));
    midiLearnFlag = false;
    grooveTick = 0;
    grooveVelocity = 0;
    grooveLength = 0;
}

Engine::~Engine(){
}
//Arp handling
void Engine::addMidiArp(MidiArp *midiArp)
{
    midiArpList.append(midiArp);
}

void Engine::addArpWidget(ArpWidget *arpWidget)
{
    arpWidgetList.append(arpWidget);
    modified = true;
}

void Engine::removeMidiArp(MidiArp *midiArp)
{
    if (seqDriver->runArp && (moduleWindowCount() < 1)) {
        seqDriver->setQueueStatus(false);
    }
    int i = midiArpList.indexOf(midiArp);
    if (i != -1)
        delete midiArpList.takeAt(i);
}

void Engine::removeArpWidget(ArpWidget *arpWidget)
{
    removeMidiArp(arpWidget->getMidiWorker());
    arpWidgetList.removeOne(arpWidget);
    modified = true;
}

void Engine::updatePatternPresets(const QString& n, const QString& p, int index)
{
    int l1;
    for (l1 = 0; l1 < midiArpCount(); l1++) {
        arpWidgetList.at(l1)->updatePatternPresets(n, p, index);
    }
}

int Engine::midiArpCount()
{
    return(midiArpList.count());
}

int Engine::arpWidgetCount()
{
    return(arpWidgetList.count());
}

MidiArp *Engine::midiArp(int index)
{
    return(midiArpList.at(index));
}

ArpWidget *Engine::arpWidget(int index)
{
    return(arpWidgetList.at(index));
}


void Engine::setGrooveTick(int val)
{
    grooveTick = val;
    sendGroove();
    modified = true;
}

void Engine::setGrooveVelocity(int val)
{
    grooveVelocity = val;
    sendGroove();
    modified = true;
}

void Engine::setGrooveLength(int val)
{
    grooveLength = val;
    sendGroove();
    modified = true;
}

void Engine::sendGroove()
{
    for (int l1 = 0; l1 < midiArpList.count(); l1++) {
        midiArpList.at(l1)->newGrooveValues(grooveTick, grooveVelocity,
                grooveLength);
    }
}

//LFO handling

void Engine::addMidiLfo(MidiLfo *midiLfo)
{
    midiLfoList.append(midiLfo);
}

void Engine::addLfoWidget(LfoWidget *lfoWidget)
{
    lfoWidgetList.append(lfoWidget);
    modified = true;
}

void Engine::removeMidiLfo(MidiLfo *midiLfo)
{
    if (seqDriver->runArp && (moduleWindowCount() < 1)) {
        seqDriver->setQueueStatus(false);
    }
    int i = midiLfoList.indexOf(midiLfo);
    if (i != -1)
        delete midiLfoList.takeAt(i);
}

void Engine::removeLfoWidget(LfoWidget *lfoWidget)
{
    removeMidiLfo(lfoWidget->getMidiWorker());
    lfoWidgetList.removeOne(lfoWidget);
    modified = true;
}

int Engine::midiLfoCount()
{
    return(midiLfoList.count());
}

int Engine::lfoWidgetCount()
{
    return(lfoWidgetList.count());
}

MidiLfo *Engine::midiLfo(int index)
{
    return(midiLfoList.at(index));
}

LfoWidget *Engine::lfoWidget(int index)
{
    return(lfoWidgetList.at(index));
}

//SEQ handling

void Engine::addMidiSeq(MidiSeq *midiSeq)
{
    midiSeqList.append(midiSeq);
}

void Engine::addSeqWidget(SeqWidget *seqWidget)
{
    seqWidgetList.append(seqWidget);
    modified = true;
}

void Engine::removeMidiSeq(MidiSeq *midiSeq)
{
    if (seqDriver->runArp && (moduleWindowCount() < 1)) {
        seqDriver->setQueueStatus(false);
    }
    int i = midiSeqList.indexOf(midiSeq);
    if (i != -1)
        delete midiSeqList.takeAt(i);
}

void Engine::removeSeqWidget(SeqWidget *seqWidget)
{
    removeMidiSeq(seqWidget->getMidiWorker());
    seqWidgetList.removeOne(seqWidget);
    modified = true;
}

int Engine::midiSeqCount()
{
    return(midiSeqList.count());
}

int Engine::seqWidgetCount()
{
    return(seqWidgetList.count());
}

MidiSeq *Engine::midiSeq(int index)
{
    return(midiSeqList.at(index));
}

SeqWidget *Engine::seqWidget(int index)
{
    return(seqWidgetList.at(index));
}

//module Window handling (dockWidgets)

void Engine::addModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.append(moduleWindow);
    modified = true;
}

void Engine::removeModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.removeOne(moduleWindow);
    delete moduleWindow;
    modified = true;
}

QDockWidget *Engine::moduleWindow(int index)
{
    return(moduleWindowList.at(index));
}

int Engine::moduleWindowCount()
{
    return(moduleWindowList.count());
}

void Engine::updateIDs(int curID)
{
    int l1, tempDockID;
    for (l1 = 0; l1 < arpWidgetCount(); l1++) {
        arpWidget(l1)->ID = l1;
        tempDockID = arpWidget(l1)->parentDockID;
        if (tempDockID > curID) {
            arpWidget(l1)->parentDockID = tempDockID - 1;
            }
    }
    for (l1 = 0; l1 < lfoWidgetCount(); l1++) {
        lfoWidget(l1)->ID = l1;
        tempDockID = lfoWidget(l1)->parentDockID;
        if (tempDockID > curID) {
            lfoWidget(l1)->parentDockID = tempDockID - 1;
        }
    }
    for (l1 = 0; l1 < seqWidgetCount(); l1++) {
        seqWidget(l1)->ID = l1;
        tempDockID = seqWidget(l1)->parentDockID;
        if (tempDockID > curID) {
            seqWidget(l1)->parentDockID = tempDockID - 1;
        }
    }
}

//general

bool Engine::isModified()
{
    bool arpmodified = false;
    bool lfomodified = false;
    bool seqmodified = false;

    for (int l1 = 0; l1 < arpWidgetCount(); l1++)
        if (arpWidget(l1)->isModified()) {
            arpmodified = true;
            break;
        }
    for (int l1 = 0; l1 < lfoWidgetCount(); l1++)
        if (lfoWidget(l1)->isModified()) {
            lfomodified = true;
            break;
        }

    for (int l1 = 0; l1 < seqWidgetCount(); l1++)
        if (seqWidget(l1)->isModified()) {
            seqmodified = true;
            break;
        }

    return modified || seqDriver->isModified()
                    || arpmodified || lfomodified || seqmodified;
}

void Engine::setModified(bool m)
{
    modified = m;
    seqDriver->setModified(m);

    for (int l1 = 0; l1 < arpWidgetCount(); l1++)
        arpWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < lfoWidgetCount(); l1++)
        lfoWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < seqWidgetCount(); l1++)
        seqWidget(l1)->setModified(m);
}

int Engine::getPortCount()
{
    return(portCount);
}

void Engine::runQueue(bool on)
{
    if (midiArpList.count() > 0)
        seqDriver->setQueueStatus(on);
}

int Engine::getAlsaClientId()
{
    return seqDriver->getAlsaClientId();
}

void Engine::handleController(int ccnumber, int channel, int value)
{
    bool m;
    int min, max, sval;
    QVector<MidiCC> cclist;
    if (!midiLearnFlag) {
        for (int l1 = 0; l1 < arpWidgetCount(); l1++) {
            cclist = arpWidget(l1)->midiControl->ccList;
            for (int l2 = 0; l2 < cclist.count(); l2++) {
                min = cclist.at(l2).min;
                max = cclist.at(l2).max;

                if ((ccnumber == cclist.at(l2).ccnumber) &&
                    (channel == cclist.at(l2).channel)) {
                    switch (cclist.at(l2).ID) {
                        case 0: if (min == max) {
                                    if (value == max) {
                                        m = arpWidget(l1)->muteOut->isChecked();
                                        arpWidget(l1)->muteOut->setChecked(!m);
                                        return;
                                    }
                        case 1:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                arpWidget(l1)->selectPatternPreset(sval);
                                return;
                        break;
                                }
                                else {
                                    if (value == max) {
                                        arpWidget(l1)->muteOut->setChecked(false);
                                    }
                                    if (value == min) {
                                        arpWidget(l1)->muteOut->setChecked(true);
                                    }
                                }
                        break;
                        default:
                        break;
                    }
                }
            }
        }

        for (int l1 = 0; l1 < lfoWidgetCount(); l1++) {
            cclist = lfoWidget(l1)->midiControl->ccList;
            for (int l2 = 0; l2 < cclist.count(); l2++) {
                min = cclist.at(l2).min;
                max = cclist.at(l2).max;
                if ((ccnumber == cclist.at(l2).ccnumber) &&
                    (channel == cclist.at(l2).channel)) {
                    switch (cclist.at(l2).ID) {
                        case 0: if (min == max) {
                                    if (value == max) {
                                        m = lfoWidget(l1)->muteOut->isChecked();
                                        lfoWidget(l1)->muteOut->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        lfoWidget(l1)->muteOut->setChecked(false);
                                    }
                                    if (value == min) {
                                        lfoWidget(l1)->muteOut->setChecked(true);
                                    }
                                }
                        break;

                        case 1:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->amplitude->setValue(sval);
                                return;
                        break;

                        case 2:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->offset->setValue(sval);
                                return;
                        break;
                        case 3:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->waveFormBox->setCurrentIndex(sval);
                                lfoWidget(l1)->updateWaveForm(sval);
                                return;
                        break;
                        case 4:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->freqBox->setCurrentIndex(sval);
                                lfoWidget(l1)->updateFreq(sval);
                                return;
                        break;
                        case 5: if (min == max) {
                                    if (value == max) {
                                        m = lfoWidget(l1)->recordAction->isChecked();
                                        lfoWidget(l1)->recordAction->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        lfoWidget(l1)->recordAction->setChecked(true);
                                    }
                                    if (value == min) {
                                        lfoWidget(l1)->recordAction->setChecked(false);
                                    }
                                }
                        break;
                        case 6:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->resBox->setCurrentIndex(sval);
                                lfoWidget(l1)->updateRes(sval);
                                return;
                        break;
                        case 7:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                lfoWidget(l1)->sizeBox->setCurrentIndex(sval);
                                lfoWidget(l1)->updateSize(sval);
                                return;
                        break;

                        default:
                        break;
                    }
                }
            }
        }

        for (int l1 = 0; l1 < seqWidgetCount(); l1++) {
            cclist = seqWidget(l1)->midiControl->ccList;
            for (int l2 = 0; l2 < cclist.count(); l2++) {
                min = cclist.at(l2).min;
                max = cclist.at(l2).max;
                if ((ccnumber == cclist.at(l2).ccnumber) &&
                    (channel == cclist.at(l2).channel)) {
                    switch (cclist.at(l2).ID) {
                        case 0: if (min == max) {
                                    if (value == max) {
                                        m = seqWidget(l1)->muteOut->isChecked();
                                        seqWidget(l1)->muteOut->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        seqWidget(l1)->muteOut->setChecked(false);
                                    }
                                    if (value == min) {
                                        seqWidget(l1)->muteOut->setChecked(true);
                                    }
                                }
                        break;

                        case 1:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                seqWidget(l1)->velocity->setValue(sval);
                                return;
                        break;

                        case 2:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                seqWidget(l1)->notelength->setValue(sval);
                                return;
                        break;

                        case 3: if (min == max) {
                                    if (value == max) {
                                        m = seqWidget(l1)->recordAction->isChecked();
                                        seqWidget(l1)->recordAction->setChecked(!m);
                                        return;
                                    }
                                }
                                else {
                                    if (value == max) {
                                        seqWidget(l1)->recordAction->setChecked(true);
                                    }
                                    if (value == min) {
                                        seqWidget(l1)->recordAction->setChecked(false);
                                    }
                                }
                        break;
                        case 4:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                seqWidget(l1)->resBox->setCurrentIndex(sval);
                                seqWidget(l1)->updateRes(sval);
                                return;
                        break;
                        case 5:
                                sval = min + ((double)value * (max - min)
                                        / 127);
                                seqWidget(l1)->sizeBox->setCurrentIndex(sval);
                                seqWidget(l1)->updateSize(sval);
                                return;
                        break;

                        default:
                        break;
                    }
                }
            }
        }
    }
    else {
        int min = (midiLearnID) ? 0 : 127; //if control is toggle min=max
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Arp")) {
            arpWidget(midiLearnModuleID)->midiControl->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("LFO")) {
            lfoWidget(midiLearnModuleID)->midiControl->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Seq")) {
            seqWidget(midiLearnModuleID)->midiControl->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }

        midiLearnFlag = false;
    }
}

void Engine::setMidiLearn(int moduleWindowID, int moduleID, int controlID)
{
    if (0 > controlID) {
        midiLearnFlag = false;
        return;
    }
    else {
        midiLearnFlag = true;
        midiLearnWindowID = moduleWindowID;
        midiLearnModuleID = moduleID;
        midiLearnID = controlID;
    }
}

void Engine::setCompactStyle(bool on)
{
    int l1;
    if (on) {
        for (l1 = 0; l1 < moduleWindowCount(); l1++)
            moduleWindowList.at(l1)->setStyleSheet(COMPACT_STYLE);
    }
    else {
        for (l1 = 0; l1 < moduleWindowCount(); l1++)
            moduleWindowList.at(l1)->setStyleSheet("");
    }
}
