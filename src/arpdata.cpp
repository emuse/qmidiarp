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
    if (midiArpList.count() > 0)
        seqDriver->setQueueStatus(on);
}
