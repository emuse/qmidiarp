#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <alsa/asoundlib.h>

#include "seqdriver.h"
#include "arpdata.h"


ArpData::ArpData(QWidget *parent) : QWidget(parent), modified(false)
{
    seqDriver = new SeqDriver(&midiArpList, &midiLfoList, this);
    //midiArpList.setAutoDelete(true);
}

ArpData::~ArpData(){
}
//Arp handling
void ArpData::addMidiArp(MidiArp *midiArp)
{
    midiArpList.append(midiArp);
    if (seqDriver->runQueueIfArp && !seqDriver->use_midiclock) {
        seqDriver->setQueueStatus(true);
    }
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

void ArpData::updatePatternPresets(QString n, QString p, int index)
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
    if (seqDriver->runQueueIfArp && !seqDriver->use_midiclock) {
        seqDriver->setQueueStatus(true);
    }
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

//general

bool ArpData::isModified()
{
    bool arpmodified = false;
    bool lfomodified = false;

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

    return modified || seqDriver->isModified() 
                    || arpmodified || lfomodified;
}

void ArpData::setModified(bool m)
{
    modified = m;
    seqDriver->setModified(m);

    for (int l1 = 0; l1 < arpWidgetCount(); l1++)
        arpWidget(l1)->setModified(m);

    for (int l1 = 0; l1 < lfoWidgetCount(); l1++)
        lfoWidget(l1)->setModified(m);
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
