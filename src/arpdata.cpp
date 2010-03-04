#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <alsa/asoundlib.h>

#include "seqdriver.h"
#include "arpdata.h"
#include "arpdata.h"


ArpData::ArpData(QWidget *parent) : QWidget(parent), modified(false)
{
    seqDriver = new SeqDriver(&midiArpList, &midiLfoList, &midiSeqList, this);
    connect(seqDriver, SIGNAL(controlEvent(int, int, int)), 
            this, SLOT(handleController(int, int, int)));
    midiLearnFlag = false;
}

ArpData::~ArpData(){
}
//Arp handling
void ArpData::addMidiArp(MidiArp *midiArp)
{
    midiArpList.append(midiArp);
}

void ArpData::addArpWidget(ArpWidget *arpWidget)
{
    arpWidgetList.append(arpWidget);
    modified = true;
}

void ArpData::removeMidiArp(MidiArp *midiArp)
{
    if (seqDriver->runArp && (midiArpList.count() < 1)) {
        seqDriver->setQueueStatus(false);
    }
    int i = midiArpList.indexOf(midiArp);
    if (i != -1)
        delete midiArpList.takeAt(i);
}

void ArpData::removeArpWidget(ArpWidget *arpWidget)
{
    removeMidiArp(arpWidget->getMidiArp());
    arpWidgetList.removeOne(arpWidget);
    modified = true;
}

void ArpData::updatePatternPresets(const QString& n, const QString& p, int index)
{
    int l1;
    for (l1 = 0; l1 < midiArpCount(); l1++) {
        arpWidgetList.at(l1)->updatePatternPresets(n, p, index);
    }
}

int ArpData::midiArpCount()
{
    return(midiArpList.count());
}

int ArpData::arpWidgetCount()
{
    return(arpWidgetList.count());
}

MidiArp *ArpData::midiArp(int index)
{
    return(midiArpList.at(index));
}

ArpWidget *ArpData::arpWidget(int index)
{
    return(arpWidgetList.at(index));
}

//LFO handling

void ArpData::addMidiLfo(MidiLfo *midiLfo)
{
    midiLfoList.append(midiLfo);
}

void ArpData::addLfoWidget(LfoWidget *lfoWidget)
{
    lfoWidgetList.append(lfoWidget);
    modified = true;
}

void ArpData::removeMidiLfo(MidiLfo *midiLfo)
{
    if (seqDriver->runArp && (midiArpList.count() < 1)) {
        seqDriver->setQueueStatus(false);
    }
    int i = midiLfoList.indexOf(midiLfo);
    if (i != -1)
        delete midiLfoList.takeAt(i);
}

void ArpData::removeLfoWidget(LfoWidget *lfoWidget)
{
    removeMidiLfo(lfoWidget->getMidiLfo());
    lfoWidgetList.removeOne(lfoWidget);
    modified = true;
}

int ArpData::midiLfoCount()
{
    return(midiLfoList.count());
}

int ArpData::lfoWidgetCount()
{
    return(lfoWidgetList.count());
}

MidiLfo *ArpData::midiLfo(int index)
{
    return(midiLfoList.at(index));
}

LfoWidget *ArpData::lfoWidget(int index)
{
    return(lfoWidgetList.at(index));
}

//SEQ handling

void ArpData::addMidiSeq(MidiSeq *midiSeq)
{
    midiSeqList.append(midiSeq);
}

void ArpData::addSeqWidget(SeqWidget *seqWidget)
{
    seqWidgetList.append(seqWidget);
    modified = true;
}

void ArpData::removeMidiSeq(MidiSeq *midiSeq)
{
    if (seqDriver->runArp && (midiSeqList.count() < 1)) {
        seqDriver->setQueueStatus(false);
    }
    int i = midiSeqList.indexOf(midiSeq);
    if (i != -1)
        delete midiSeqList.takeAt(i);
}

void ArpData::removeSeqWidget(SeqWidget *seqWidget)
{
    removeMidiSeq(seqWidget->getMidiSeq());
    seqWidgetList.removeOne(seqWidget);
    modified = true;
}

int ArpData::midiSeqCount()
{
    return(midiSeqList.count());
}

int ArpData::seqWidgetCount()
{
    return(seqWidgetList.count());
}

MidiSeq *ArpData::midiSeq(int index)
{
    return(midiSeqList.at(index));
}

SeqWidget *ArpData::seqWidget(int index)
{
    return(seqWidgetList.at(index));
}

//module Window handling (dockWidgets)

void ArpData::addModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.append(moduleWindow);
    modified = true;
}

void ArpData::removeModuleWindow(QDockWidget *moduleWindow)
{
    moduleWindowList.removeOne(moduleWindow);
    delete moduleWindow;
    modified = true;
}

QDockWidget *ArpData::moduleWindow(int index)
{
    return(moduleWindowList.at(index));
}

int ArpData::moduleWindowCount()
{
    return(moduleWindowList.count());
}

void ArpData::updateIDs(int curID)
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

bool ArpData::isModified()
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

void ArpData::setModified(bool m)
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

void ArpData::registerPorts(int num)
{
    portCount = num;
    seqDriver->registerPorts(num);
}

int ArpData::getPortCount()
{
    return(portCount);
}

void ArpData::runQueue(bool on)
{
    seqDriver->runQueue(on);
    if (midiArpList.count() > 0)
        seqDriver->setQueueStatus(on);
}

int ArpData::getAlsaClientId()
{
    return seqDriver->getAlsaClientId();
}

void ArpData::handleController(int ccnumber, int channel, int value)
{
    bool m;
    int min, max, sval;
    QVector<MidiCC> cclist;
    if (!midiLearnFlag) {
        for (int l1 = 0; l1 < arpWidgetCount(); l1++) {
            cclist = arpWidget(l1)->ccList;
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
            cclist = lfoWidget(l1)->ccList;
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
                        default:
                        break;
                    }
                }
            }
        }
        
        for (int l1 = 0; l1 < seqWidgetCount(); l1++) {
            cclist = seqWidget(l1)->ccList;
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
            arpWidget(midiLearnModuleID)->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("LFO")) {
            lfoWidget(midiLearnModuleID)->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        if (moduleWindow(midiLearnWindowID)->objectName().startsWith("Seq")) {
            seqWidget(midiLearnModuleID)->appendMidiCC(midiLearnID,
                    ccnumber, channel, min, 127);
        }
        
        midiLearnFlag = false;
    }
}

void ArpData::setMidiLearn(int moduleWindowID, int moduleID, int controlID)
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

void ArpData::setCompactStyle(bool on)
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
