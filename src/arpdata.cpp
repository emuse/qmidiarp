#include <QString>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <alsa/asoundlib.h>

#include "seqdriver.h"
#include "arpdata.h"


ArpData::ArpData(QWidget *parent) : QWidget(parent)
{
    seqDriver = new SeqDriver(&midiArpList, this);
    //midiArpList.setAutoDelete(true);
}

ArpData::~ArpData(){
}

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
}

void ArpData::removeMidiArp(MidiArp *midiArp)
{
    if (seqDriver->runArp && (midiArpList.count() < 2)) {
        seqDriver->setQueueStatus(false);
    }
    midiArpList.removeAll(midiArp);
}

void ArpData::removeArpWidget(ArpWidget *arpWidget)
{
    arpWidgetList.removeAll(arpWidget);
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
    if (midiArpList.count()) {
        seqDriver->setQueueStatus(on);
    }
}
