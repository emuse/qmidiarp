/*!
 * @file lfowidget.cpp
 * @brief Implements the LfoWidget GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2013 <qmidiarp-devel@lists.sourceforge.net>
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
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>

#include "midilfo.h"
#include "lfowidget.h"
#include "slider.h"
#include "lfoscreen.h"

#include "pixmaps/lfowsine.xpm"
#include "pixmaps/lfowsawup.xpm"
#include "pixmaps/lfowsawdn.xpm"
#include "pixmaps/lfowtri.xpm"
#include "pixmaps/lfowsquare.xpm"
#include "pixmaps/lfowcustm.xpm"
#include "pixmaps/seqrecord.xpm"
#include "config.h"


LfoWidget::LfoWidget(MidiLfo *p_midiWorker, GlobStore *p_globStore,
    int portCount, bool compactStyle,
    bool mutedAdd, bool inOutVisible, const QString& name, QWidget *parent):
    QWidget(parent), midiWorker(p_midiWorker),
    globStore(p_globStore), modified(false)
{
    int l1;

    midiControl = new MidiControl(this);

    manageBox = new ManageBox("LFO:", true, this);

    // Input group box on right top
    QGroupBox *inBox = new QGroupBox(tr("Input"), this);

    QLabel *enableNoteOffLabel = new QLabel(tr("&Note Off"),inBox);
    enableNoteOff = new QCheckBox(this);
    connect(enableNoteOff, SIGNAL(toggled(bool)), this, SLOT(updateEnableNoteOff(bool)));
    enableNoteOffLabel->setBuddy(enableNoteOff);
    enableNoteOff->setToolTip(tr("Stop output when Note is released"));

    QLabel *enableRestartByKbdLabel = new QLabel(tr("&Restart"),inBox);
    enableRestartByKbd = new QCheckBox(this);
    connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, SLOT(updateEnableRestartByKbd(bool)));
    enableRestartByKbdLabel->setBuddy(enableRestartByKbd);
    enableRestartByKbd->setToolTip(tr("Restart sequence when a new note is received"));

    QLabel *enableTrigByKbdLabel = new QLabel(tr("&Trigger"),inBox);
    enableTrigByKbd = new QCheckBox(this);
    connect(enableTrigByKbd, SIGNAL(toggled(bool)), this, SLOT(updateEnableTrigByKbd(bool)));
    enableTrigByKbdLabel->setBuddy(enableTrigByKbd);
    enableTrigByKbd->setToolTip(tr("Retrigger sequence when a new note is received"));

    QLabel *enableTrigLegatoLabel = new QLabel(tr("&Legato"),inBox);
    enableTrigLegato = new QCheckBox(this);
    connect(enableTrigLegato, SIGNAL(toggled(bool)), this, SLOT(updateTrigLegato(bool)));
    enableTrigLegatoLabel->setBuddy(enableTrigLegato);
    enableTrigLegato->setToolTip(tr("Retrigger / restart upon new legato note as well"));

    enableNoteOff->setChecked(false);
    enableRestartByKbd->setChecked(false);
    enableTrigByKbd->setChecked(false);
    enableTrigLegato->setChecked(false);

    QLabel *ccnumberInLabel = new QLabel(tr("MIDI &CC#"), inBox);
    ccnumberInBox = new QSpinBox(inBox);
    ccnumberInLabel->setBuddy(ccnumberInBox);
    ccnumberInBox->setRange(0, 127);
    ccnumberInBox->setKeyboardTracking(false);
    ccnumberInBox->setValue(74);
    ccnumberInBox->setToolTip(tr("MIDI Controller number to record"));
    connect(ccnumberInBox, SIGNAL(valueChanged(int)), this,
            SLOT(updateCcnumberIn(int)));


    QLabel *chInLabel = new QLabel(tr("&Channel"), inBox);
    chIn = new QComboBox(inBox);
    for (l1 = 0; l1 < 16; l1++) chIn->addItem(QString::number(l1 + 1));
    chInLabel->setBuddy(chIn);
    connect(chIn, SIGNAL(activated(int)), this, SLOT(updateChIn(int)));

    QGridLayout *inBoxLayout = new QGridLayout;

    inBoxLayout->addWidget(ccnumberInLabel, 0, 0);
    inBoxLayout->addWidget(ccnumberInBox, 0, 1);
    inBoxLayout->addWidget(enableNoteOffLabel, 1, 0);
    inBoxLayout->addWidget(enableNoteOff, 1, 1);
    inBoxLayout->addWidget(enableRestartByKbdLabel, 2, 0);
    inBoxLayout->addWidget(enableRestartByKbd, 2, 1);
    inBoxLayout->addWidget(enableTrigByKbdLabel, 3, 0);
    inBoxLayout->addWidget(enableTrigByKbd, 3, 1);
    inBoxLayout->addWidget(enableTrigLegatoLabel, 4, 0);
    inBoxLayout->addWidget(enableTrigLegato, 4, 1);
    inBoxLayout->addWidget(chInLabel, 5, 0);
    inBoxLayout->addWidget(chIn, 5, 1);
    if (compactStyle) {
        inBoxLayout->setSpacing(1);
        inBoxLayout->setMargin(2);
    }

    inBox->setLayout(inBoxLayout);

    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *ccnumberLabel = new QLabel(tr("MIDI &CC#"), portBox);
    ccnumberBox = new QSpinBox(portBox);
    ccnumberLabel->setBuddy(ccnumberBox);
    ccnumberBox->setRange(0, 127);
    ccnumberBox->setKeyboardTracking(false);
    ccnumberBox->setValue(74);
    ccnumberBox->setToolTip(tr("MIDI Controller number sent to output"));
    connect(ccnumberBox, SIGNAL(valueChanged(int)), this,
            SLOT(updateCcnumber(int)));

    QLabel *portLabel = new QLabel(tr("&Port"), portBox);
    portOut = new QComboBox(portBox);
    portLabel->setBuddy(portOut);
    for (l1 = 0; l1 < portCount; l1++) portOut->addItem(QString::number(l1 + 1));
    connect(portOut, SIGNAL(activated(int)), this, SLOT(updatePortOut(int)));

    QLabel *channelLabel = new QLabel(tr("C&hannel"), portBox);
    channelOut = new QComboBox(portBox);
    channelLabel->setBuddy(channelOut);
    for (l1 = 0; l1 < 16; l1++) channelOut->addItem(QString::number(l1 + 1));
    connect(channelOut, SIGNAL(activated(int)), this,
            SLOT(updateChannelOut(int)));


    QGridLayout *portBoxLayout = new QGridLayout;
    portBoxLayout->addWidget(ccnumberLabel, 0, 0);
    portBoxLayout->addWidget(ccnumberBox, 0, 1);
    portBoxLayout->addWidget(portLabel, 1, 0);
    portBoxLayout->addWidget(portOut, 1, 1);
    portBoxLayout->addWidget(channelLabel, 2, 0);
    portBoxLayout->addWidget(channelOut, 2, 1);

    QVBoxLayout* outputLayout = new QVBoxLayout;
    outputLayout->addLayout(portBoxLayout);
    if (compactStyle) {
        outputLayout->setSpacing(1);
        outputLayout->setMargin(2);
    }

    portBox->setLayout(outputLayout);

    hideInOutBoxAction = new QAction(tr("&Show/hide in-out settings"), this);
    QToolButton *hideInOutBoxButton = new QToolButton(this);
    hideInOutBoxAction->setCheckable(true);
    hideInOutBoxAction->setChecked(inOutVisible);
    hideInOutBoxButton->setDefaultAction(hideInOutBoxAction);
    hideInOutBoxButton->setFixedSize(10, 80);
    hideInOutBoxButton->setArrowType (Qt::ArrowType(0));
    connect(hideInOutBoxAction, SIGNAL(toggled(bool)), this, SLOT(setInOutBoxVisible(bool)));

    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
    inOutBoxLayout->addWidget(manageBox);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();
    inOutBox = new QWidget(this);
    inOutBox->setLayout(inOutBoxLayout);
    inOutBox->setVisible(inOutVisible);

    // group box for wave setup
    QGroupBox *waveBox = new QGroupBox(tr("Wave"), this);

    screen = new LfoScreen(this);
    screen->setToolTip(
        tr("Right button to mute points\nLeft button to draw custom wave\nWheel to change offset"));
    screen->setMinimumHeight(80);

    connect(screen, SIGNAL(mouseMoved(double, double, int)), this,
            SLOT(mouseMoved(double, double, int)));
    connect(screen, SIGNAL(mousePressed(double, double, int)), this,
            SLOT(mousePressed(double, double, int)));
    connect(screen, SIGNAL(mouseWheel(int)), this,
            SLOT(mouseWheel(int)));

    cursor = new Cursor('L', this);

    QLabel *waveFormBoxLabel = new QLabel(tr("&Waveform"), waveBox);
    waveFormBox = new QComboBox(waveBox);
    waveFormBoxLabel->setBuddy(waveFormBox);
    //loadWaveForms();
    waveFormBox->addItem(QIcon(lfowsine_xpm),"");
    waveFormBox->addItem(QIcon(lfowsawup_xpm),"");
    waveFormBox->addItem(QIcon(lfowtri_xpm),"");
    waveFormBox->addItem(QIcon(lfowsawdn_xpm),"");
    waveFormBox->addItem(QIcon(lfowsquare_xpm),"");
    waveFormBox->addItem(QIcon(lfowcustm_xpm),"");
    waveFormBox->setCurrentIndex(0);
    waveFormBoxIndex = 0;
    waveFormBox->setToolTip(tr("Waveform Basis"));

    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
    midiControl->addMidiLearnMenu("WaveForm", waveFormBox, 3);


    QLabel *freqBoxLabel = new QLabel(tr("&Frequency"),
            waveBox);
    freqBox = new QComboBox(waveBox);
    freqBoxLabel->setBuddy(freqBox);
    QStringList names;
    names << "1/32" << "1/16" << "1/8" << "1/4"
        << "1/2" << "3/4" << "1" << "2" << "3"
        << "4" << "5" << "6" << "7" << "8";
    freqBox->insertItems(0, names);
    freqBox->setCurrentIndex(6);
    freqBoxIndex = 6;
    freqBox->setToolTip(
            tr("Frequency (cycles/beat): Number of wave cycles produced every beat"));
    freqBox->setMinimumContentsLength(3);
    connect(freqBox, SIGNAL(activated(int)), this,
            SLOT(updateFreq(int)));
    midiControl->addMidiLearnMenu("Frequency", freqBox, 4);

    QLabel *resBoxLabel = new QLabel(tr("&Resolution"),
            waveBox);
    resBox = new QComboBox(waveBox);
    resBoxLabel->setBuddy(resBox);
    names.clear();
    names << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "96" << "192";
    resBox->insertItems(0, names);
    resBox->setCurrentIndex(4);
    resBoxIndex = 4;
    resBox->setToolTip(
            tr("Resolution (events/beat): Number of events produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));
    midiControl->addMidiLearnMenu("Resolution", resBox, 6);

    QLabel *sizeBoxLabel = new QLabel(tr("&Length"), waveBox);
    sizeBox = new QComboBox(waveBox);
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8"
            << "12" << "16" << "24" << "32" ;
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(0);
    sizeBoxIndex = 0;
    sizeBox->setToolTip(tr("Length of LFO wave in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));
    midiControl->addMidiLearnMenu("Size", sizeBox, 7);

    loopBox = new QComboBox(waveBox);
    names.clear();
    names << "->_>" << " <_<-" << "->_<" << " >_<-" << "->_|" << " |_<-";
    loopBox->insertItems(0, names);
    loopBox->setCurrentIndex(0);
    loopBox->setToolTip(tr("Loop, bounce or play once going forward or backward"));
    loopBox->setMinimumContentsLength(5);
    connect(loopBox, SIGNAL(activated(int)), this,
            SLOT(updateLoop(int)));
    midiControl->addMidiLearnMenu("LoopMode", loopBox, 8);

    muteOutAction = new QAction(tr("&Mute"),this);
    muteOutAction->setCheckable(true);
    connect(muteOutAction, SIGNAL(toggled(bool)), this, SLOT(setMuted(bool)));
    muteOut = new QToolButton(this);
    muteOut->setDefaultAction(muteOutAction);
    muteOut->setFont(QFont("Helvetica", 8));
    muteOut->setMinimumSize(QSize(35,20));
    midiControl->addMidiLearnMenu("MuteToggle", muteOut, 0);

    deferChangesAction = new QAction("D", this);
    deferChangesAction->setToolTip(tr("Defer mute to pattern end"));
    deferChangesAction->setCheckable(true);
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, SLOT(updateDeferChanges(bool)));

    QToolButton *deferChangesButton = new QToolButton(this);
    deferChangesButton->setDefaultAction(deferChangesAction);
    deferChangesButton->setFixedSize(20, 20);

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"), waveBox);
    recordAction = new QAction(QIcon(seqrecord_xpm), tr("Re&cord"), waveBox);
    recordAction->setToolTip(tr("Record incoming controller"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton(waveBox);
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
    midiControl->addMidiLearnMenu("RecordToggle", recordButton, 5);

    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), waveBox);
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));
    midiControl->addMidiLearnMenu("Amplitude", amplitude, 1);


    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), waveBox);
    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));
    midiControl->addMidiLearnMenu("Offset", offset, 2);

    QVBoxLayout* sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(amplitude);
    sliderLayout->addWidget(offset);
    sliderLayout->addStretch();
    if (compactStyle) {
        sliderLayout->setSpacing(1);
        sliderLayout->setMargin(2);
    }

    QGridLayout *paramBoxLayout = new QGridLayout;
    paramBoxLayout->addWidget(loopBox, 0, 0, 1, 2);
    paramBoxLayout->addWidget(muteOut, 1, 0, 1, 1);
    paramBoxLayout->addWidget(deferChangesButton, 1, 1, 1, 2);
    paramBoxLayout->addWidget(recordButtonLabel, 2, 0);
    paramBoxLayout->addWidget(recordButton, 2, 1);
    paramBoxLayout->addWidget(waveFormBoxLabel, 0, 2);
    paramBoxLayout->addWidget(waveFormBox, 0, 3);
    paramBoxLayout->addWidget(freqBoxLabel, 1, 2);
    paramBoxLayout->addWidget(freqBox, 1, 3);
    paramBoxLayout->addWidget(resBoxLabel, 0, 4);
    paramBoxLayout->addWidget(resBox, 0, 5);
    paramBoxLayout->addWidget(sizeBoxLabel, 1, 4);
    paramBoxLayout->addWidget(sizeBox, 1, 5);
    paramBoxLayout->setColumnStretch(6, 6);

    if (compactStyle) {
        paramBoxLayout->setSpacing(1);
        paramBoxLayout->setMargin(2);
    }

    QGridLayout* waveBoxLayout = new QGridLayout;
    waveBoxLayout->addWidget(screen, 0, 0);
    waveBoxLayout->addWidget(cursor, 1, 0);
    waveBoxLayout->addLayout(paramBoxLayout, 2, 0);
    waveBoxLayout->addLayout(sliderLayout, 3, 0);
    if (compactStyle) {
        waveBoxLayout->setSpacing(0);
        waveBoxLayout->setMargin(2);
    }
    waveBox->setLayout(waveBoxLayout);

    muteOut->setChecked(mutedAdd);

    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(waveBox, 1);
    widgetLayout->addWidget(hideInOutBoxButton, 0);
    widgetLayout->addWidget(inOutBox, 0);

    parStore = new ParStore(globStore, name, muteOutAction, deferChangesAction, this);
    midiControl->addMidiLearnMenu("Restore_"+name, parStore->topButton, 9);
    connect(parStore, SIGNAL(store(int, bool)),
             this, SLOT(storeParams(int, bool)));
    connect(parStore, SIGNAL(restore(int)),
             this, SLOT(restoreParams(int)));


    setLayout(widgetLayout);
    updateAmp(64);

    lastMute = false;
    dataChanged = false;
    needsGUIUpdate = false;
}

LfoWidget::~LfoWidget()
{
    delete parStore;
}

MidiLfo *LfoWidget::getMidiWorker()
{
    return (midiWorker);
}

void LfoWidget::writeData(QXmlStreamWriter& xml)
{
    QByteArray tempArray;
    int l1;

    xml.writeStartElement(manageBox->name.left(3));
    xml.writeAttribute("name", manageBox->name.mid(manageBox->name.indexOf(':') + 1));
    xml.writeAttribute("inOutVisible", QString::number(inOutBox->isVisible()));
        xml.writeStartElement("input");
            xml.writeTextElement("enableNoteOff", QString::number(
                midiWorker->enableNoteOff));
            xml.writeTextElement("restartByKbd", QString::number(
                midiWorker->restartByKbd));
            xml.writeTextElement("trigByKbd", QString::number(
                midiWorker->trigByKbd));
            xml.writeTextElement("trigLegato", QString::number(
                midiWorker->trigLegato));
            xml.writeTextElement("channel", QString::number(
                midiWorker->chIn));
            xml.writeTextElement("ccnumber", QString::number(
                midiWorker->ccnumberIn));
        xml.writeEndElement();

        xml.writeStartElement("output");
            xml.writeTextElement("muted", QString::number(
                midiWorker->isMuted));
            xml.writeTextElement("defer", QString::number(
                midiWorker->deferChanges));
            xml.writeTextElement("port", QString::number(
                midiWorker->portOut));
            xml.writeTextElement("channel", QString::number(
                midiWorker->channelOut));
            xml.writeTextElement("ccnumber", QString::number(
                midiWorker->ccnumber));
        xml.writeEndElement();

        xml.writeStartElement("waveParams");
            xml.writeTextElement("loopmode", QString::number(
                loopBox->currentIndex()));
            xml.writeTextElement("waveform", QString::number(
                waveFormBox->currentIndex()));
            xml.writeTextElement("frequency", QString::number(
                freqBox->currentIndex()));
            xml.writeTextElement("resolution", QString::number(
                resBox->currentIndex()));
            xml.writeTextElement("size", QString::number(
                sizeBox->currentIndex()));
            xml.writeTextElement("amplitude", QString::number(
                midiWorker->amp));
            xml.writeTextElement("offset", QString::number(
                midiWorker->offs));
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiWorker->maxNPoints) {
            tempArray.append(midiWorker->muteMask.at(l1));
            l1++;
        }
        xml.writeStartElement("muteMask");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiWorker->maxNPoints) {
            tempArray.append(midiWorker->customWave.at(l1).value);
            l1++;
        }
        xml.writeStartElement("customWave");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        midiControl->writeData(xml);

        parStore->writeData(xml);

    xml.writeEndElement();
}

void LfoWidget::readData(QXmlStreamReader& xml)
{
    int tmp;
    int wvtmp = 0;
    Sample sample;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        if (xml.isStartElement() && (xml.name() == "input")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "enableNoteOff")
                    enableNoteOff->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "restartByKbd")
                    enableRestartByKbd->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "trigByKbd")
                    enableTrigByKbd->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "trigLegato")
                    enableTrigLegato->setChecked(xml.readElementText().toInt());
                if (xml.name() == "channel") {
                    tmp = xml.readElementText().toInt();
                    chIn->setCurrentIndex(tmp);
                    updateChIn(tmp);
                }
                else if (xml.name() == "ccnumber")
                    ccnumberInBox->setValue(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }

        if (xml.isStartElement() && (xml.name() == "output")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "muted")
                    muteOutAction->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "defer")
                    deferChangesAction->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "channel") {
                    tmp = xml.readElementText().toInt();
                    channelOut->setCurrentIndex(tmp);
                    updateChannelOut(tmp);
                }
                else if (xml.name() == "port") {
                    tmp = xml.readElementText().toInt();
                    portOut->setCurrentIndex(tmp);
                    updatePortOut(tmp);
                }
                else if (xml.name() == "ccnumber")
                    ccnumberBox->setValue(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }

        else if (xml.isStartElement() && (xml.name() == "waveParams")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "loopmode") {
                    tmp = xml.readElementText().toInt();
                    loopBox->setCurrentIndex(tmp);
                    updateLoop(tmp);
                }
                else if (xml.name() == "waveform")
                    wvtmp = xml.readElementText().toInt();
                else if (xml.name() == "frequency") {
                    tmp = xml.readElementText().toInt();
                    freqBox->setCurrentIndex(tmp);
                    updateFreq(tmp);
                }
                else if (xml.name() == "resolution") {
                    tmp = xml.readElementText().toInt();
                    resBox->setCurrentIndex(tmp);
                    updateRes(tmp);
                }
                else if (xml.name() == "size") {
                    tmp = xml.readElementText().toInt();
                    sizeBox->setCurrentIndex(tmp);
                    updateSize(tmp);
                }
                else if (xml.name() == "amplitude")
                    amplitude->setValue(xml.readElementText().toInt());
                else if (xml.name() == "offset")
                    offset->setValue(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "muteMask")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.isStartElement() && (xml.name() == "data")) {
                    QByteArray tmpArray =
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        midiWorker->muteMask.replace(l1, tmpArray.at(l1));
                    }
                    midiWorker->maxNPoints = tmpArray.count();
                }
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "customWave")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.isStartElement() && (xml.name() == "data")) {
                    QByteArray tmpArray =
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    int step = TPQN / midiWorker->res;
                    int lt = 0;
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        sample.value = tmpArray.at(l1);
                        sample.tick = lt;
                        sample.muted = midiWorker->muteMask.at(l1);
                        midiWorker->customWave.replace(l1, sample);
                        lt+=step;
                    }
                }
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "midiControllers")) {
            midiControl->readData(xml);
        }
        else if (xml.isStartElement() && (xml.name() == "globalStores")) {
            parStore->readData(xml);
        }
        else skipXmlElement(xml);
    }
    waveFormBox->setCurrentIndex(wvtmp);
    updateWaveForm(wvtmp);
    modified = false;
}

void LfoWidget::loadWaveForms()
{
    waveForms << tr("Sine") << tr("Saw up") << tr("Triangle")
        << tr("Saw down") << tr("Square") << tr("Custom");
}

void LfoWidget::updateCcnumber(int val)
{
    midiWorker->ccnumber = val;
    modified = true;
}

void LfoWidget::setChIn(int value)
{
    chIn->setCurrentIndex(value);
    modified = true;
}

void LfoWidget::updateChIn(int val)
{
    midiWorker->chIn = val;
    modified = true;
}

void LfoWidget::updateCcnumberIn(int val)
{
    midiWorker->ccnumberIn = val;
    modified = true;
}
void LfoWidget::updateEnableNoteOff(bool on)
{
    midiWorker->enableNoteOff = on;
    modified = true;
}

void LfoWidget::updateEnableRestartByKbd(bool on)
{
    midiWorker->restartByKbd = on;
    modified = true;
}

void LfoWidget::updateEnableTrigByKbd(bool on)
{
    midiWorker->trigByKbd = on;
    modified = true;
}

void LfoWidget::updateTrigLegato(bool on)
{
    midiWorker->trigLegato = on;
    modified = true;
}

void LfoWidget::updateWaveForm(int val)
{
    if (val > 5) return;
    waveFormBoxIndex = val;
    midiWorker->updateWaveForm(val);
    midiWorker->getData(&data);
    screen->updateData(data);
    bool isCustom = (val == 5);
    if (isCustom) newCustomOffset();
    amplitude->setDisabled(isCustom);
    freqBox->setDisabled(isCustom);
    modified = true;

}

void LfoWidget::updateScreen(int val)
{
    if (!midiWorker->isRecording)
        cursor->updatePosition(val);
}

void LfoWidget::updateFreq(int val)
{
    if (val > 13) return;
    freqBoxIndex = val;
    midiWorker->updateFrequency(lfoFreqValues[val]);
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::updateRes(int val)
{
    if (val > 8) return;
    resBoxIndex = val;
    midiWorker->updateResolution(lfoResValues[val]);
    midiWorker->getData(&data);
    screen->updateData(data);
    newCustomOffset();
    modified = true;
}

void LfoWidget::updateSize(int val)
{
    if (val > 11) return;
    sizeBoxIndex = val;
    midiWorker->updateSize(sizeBox->currentText().toInt());
    midiWorker->getData(&data);
    screen->updateData(data);
    newCustomOffset();
    modified = true;
}

void LfoWidget::updateLoop(int val)
{
    if (val > 5) return;
    midiWorker->updateLoop(val);
    modified = true;
}

void LfoWidget::updateAmp(int val)
{
    midiWorker->updateAmplitude(val);
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::updateOffs(int val)
{
    midiWorker->updateOffset(val);
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::copyToCustom()
{
    midiWorker->copyToCustom();
    waveFormBox->setCurrentIndex(5);
    updateWaveForm(5);
    modified = true;
}

void LfoWidget::newCustomOffset()
{
    int min = 127;
    int value;
    const int npoints = sizeBox->currentText().toInt()
                        * resBox->currentText().toInt();
    for (int l1 = 0; l1 < npoints; l1++) {
        value = data.at(l1).value;
        if (value < min) min = value;
    }
    midiWorker->cwmin = min;
    offset->setValue(min);
}

void LfoWidget::mouseMoved(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        midiWorker->setMutePoint(mouseX, lastMute);
    }
    else {
        if (waveFormBox->currentIndex() < 5) {
            copyToCustom();
        }
        midiWorker->setCustomWavePoint(mouseX, mouseY, false);
        newCustomOffset();
    }
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::mousePressed(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        lastMute = midiWorker->toggleMutePoint(mouseX);
    }
    else {
        if (waveFormBox->currentIndex() < 5) {
            copyToCustom();
        }
        midiWorker->setCustomWavePoint(mouseX, mouseY, true);
        newCustomOffset();
    }
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void LfoWidget::mouseWheel(int step)
{
    int cv;
    cv = offset->value() + step;
    if ((cv < 127) && (cv > 0))
    offset->setValue(cv + step);
}

void LfoWidget::setInOutBoxVisible(bool on)
{
    inOutBox->setVisible(on);
    modified=true;
}

void LfoWidget::setMuted(bool on)
{
    midiWorker->setMuted(on);
    screen->setMuted(midiWorker->isMuted);
    parStore->ndc->setMuted(midiWorker->isMuted);
}

void LfoWidget::updateDeferChanges(bool on)
{
    midiWorker->updateDeferChanges(on);
}

void LfoWidget::setRecord(bool on)
{
    if (!on) {
        midiWorker->isRecording = false;
        newCustomOffset();
    }
    midiWorker->recordMode = on;
    screen->setRecord(on);
}

void LfoWidget::setPortOut(int value)
{
    portOut->setCurrentIndex(value);
    modified = true;
}

void LfoWidget::setChannelOut(int value)
{
    channelOut->setCurrentIndex(value);
    modified = true;
}

void LfoWidget::updatePortOut(int value)
{
    midiWorker->portOut = value;
    modified = true;
}

void LfoWidget::updateChannelOut(int value)
{
    midiWorker->channelOut = value;
    modified = true;
}

bool LfoWidget::isModified()
{
    return (modified || midiControl->isModified());
}

void LfoWidget::setModified(bool m)
{
    modified = m;
    midiControl->setModified(m);
}

void LfoWidget::storeParams(int ix, bool empty)
{
    parStore->temp.empty = empty;
    parStore->temp.muteOut = muteOut->isChecked();
    parStore->temp.chIn = chIn->currentIndex();
    parStore->temp.ccnumberIn = ccnumberInBox->value();
    parStore->temp.ccnumber = ccnumberBox->value();
    parStore->temp.channelOut = channelOut->currentIndex();
    parStore->temp.portOut = portOut->currentIndex();
    parStore->temp.res = resBox->currentIndex();
    parStore->temp.size = sizeBox->currentIndex();
    parStore->temp.loopMode = loopBox->currentIndex();
    parStore->temp.freq = freqBox->currentIndex();
    parStore->temp.ampl = amplitude->value();
    parStore->temp.offs = offset->value();
    parStore->temp.waveForm = waveFormBox->currentIndex();

    parStore->temp.wave = getCustomWave().mid(0, midiWorker->maxNPoints);
    parStore->temp.muteMask = midiWorker->muteMask.mid(0, midiWorker->maxNPoints);

    parStore->tempToList(ix);
}

void LfoWidget::restoreParams(int ix)
{
    if (parStore->list.at(ix).empty) return;
    for (int l1 = 0; l1 < parStore->list.at(ix).wave.count(); l1++) {
        midiWorker->customWave.replace(l1, parStore->list.at(ix).wave.at(l1));
        midiWorker->muteMask.replace(l1, parStore->list.at(ix).muteMask.at(l1));
    }
    sizeBox->setCurrentIndex(parStore->list.at(ix).size);
    midiWorker->updateSize(sizeBox->currentText().toInt());

    midiWorker->updateResolution(lfoResValues[parStore->list.at(ix).res]);
    midiWorker->updateWaveForm(parStore->list.at(ix).waveForm);
    midiWorker->updateFrequency(lfoFreqValues[parStore->list.at(ix).freq]);
    freqBox->setCurrentIndex(parStore->list.at(ix).freq);
    resBox->setCurrentIndex(parStore->list.at(ix).res);
    waveFormBox->setCurrentIndex(parStore->list.at(ix).waveForm);
    loopBox->setCurrentIndex(parStore->list.at(ix).loopMode);
    updateLoop(parStore->list.at(ix).loopMode);
    updateWaveForm(parStore->list.at(ix).waveForm);
    if (!parStore->onlyPatternList.at(ix)) {
        //muteOut->setChecked(parStore->list.at(ix).muteOut);
        offset->setValue(parStore->list.at(ix).offs);
        chIn->setCurrentIndex(parStore->list.at(ix).chIn);
        updateChIn(parStore->list.at(ix).chIn);
        ccnumberInBox->setValue(parStore->list.at(ix).ccnumberIn);
        ccnumberBox->setValue(parStore->list.at(ix).ccnumber);
        channelOut->setCurrentIndex(parStore->list.at(ix).channelOut);
        updateChannelOut(parStore->list.at(ix).channelOut);
        setPortOut(parStore->list.at(ix).portOut);
        updatePortOut(parStore->list.at(ix).portOut);
        amplitude->setValue(parStore->list.at(ix).ampl);
    }
    midiWorker->setFramePtr(0);
}

void LfoWidget::copyParamsFrom(LfoWidget *fromWidget)
{
    int tmp;

    enableNoteOff->setChecked(fromWidget->enableNoteOff->isChecked());
    enableRestartByKbd->setChecked(fromWidget->enableRestartByKbd->isChecked());
    enableTrigByKbd->setChecked(fromWidget->enableTrigByKbd->isChecked());
    enableTrigLegato->setChecked(fromWidget->enableTrigLegato->isChecked());

    tmp = fromWidget->chIn->currentIndex();
    chIn->setCurrentIndex(tmp);
    updateChIn(tmp);
    tmp = fromWidget->channelOut->currentIndex();
    channelOut->setCurrentIndex(tmp);
    updateChannelOut(tmp);
    tmp = fromWidget->portOut->currentIndex();
    portOut->setCurrentIndex(tmp);
    updatePortOut(tmp);

    tmp = fromWidget->ccnumberInBox->value();
    ccnumberInBox->setValue(tmp);
    updateCcnumberIn(tmp);
    tmp = fromWidget->ccnumberBox->value();
    ccnumberBox->setValue(tmp);
    updateCcnumber(tmp);

    tmp = fromWidget->resBox->currentIndex();
    resBox->setCurrentIndex(tmp);
    updateRes(tmp);
    tmp = fromWidget->sizeBox->currentIndex();
    sizeBox->setCurrentIndex(tmp);
    updateSize(tmp);
    tmp = fromWidget->loopBox->currentIndex();
    loopBox->setCurrentIndex(tmp);
    updateLoop(tmp);
    tmp = fromWidget->freqBox->currentIndex();
    freqBox->setCurrentIndex(tmp);
    updateFreq(tmp);

    amplitude->setValue(fromWidget->amplitude->value());
    offset->setValue(fromWidget->offset->value());

    for (int l1 = 0; l1 < fromWidget->getMidiWorker()->maxNPoints; l1++) {
        midiWorker->customWave.replace(l1, fromWidget->getCustomWave().at(l1));
        midiWorker->muteMask.replace(l1, midiWorker->customWave.at(l1).muted);
    }
    midiControl->setCcList(fromWidget->midiControl->ccList);
    muteOutAction->setChecked(true);

    tmp = fromWidget->waveFormBox->currentIndex();
    waveFormBox->setCurrentIndex(tmp);
    updateWaveForm(tmp);
}

QVector<Sample> LfoWidget::getCustomWave()
{
    return midiWorker->customWave;
}

void LfoWidget::handleController(int ccnumber, int channel, int value)
{
    bool m;
    int min, max, sval;
    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        min = cclist.at(l2).min;
        max = cclist.at(l2).max;
        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            switch (cclist.at(l2).ID) {
                case 0: if (min == max) {
                            if (value == max) {
                                m = midiWorker->isMuted;
                                midiWorker->setMuted(!m);
                            }
                        }
                        else {
                            if (value == max) {
                                midiWorker->setMuted(false);
                            }
                            if (value == min) {
                                midiWorker->setMuted(true);
                            }
                        }
                break;

                case 1:
                        sval = min + ((double)value * (max - min) / 127);
                        midiWorker->updateAmplitude(sval);
                break;

                case 2:
                        sval = min + ((double)value * (max - min) / 127);
                        midiWorker->updateOffset(sval);
                break;
                case 3:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) waveFormBoxIndex = sval;
                break;
                case 4:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 14) freqBoxIndex = sval;
                break;
                case 5: if (min == max) {
                            if (value == max) {
                                m = midiWorker->recordMode;
                                setRecord(!m);
                                return;
                            }
                        }
                        else {
                            if (value == max) {
                                setRecord(true);
                            }
                            if (value == min) {
                                setRecord(false);
                            }
                        }
                break;
                case 6:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 9) resBoxIndex = sval;
                break;
                case 7:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 12) sizeBoxIndex = sval;
                break;
                case 8:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) midiWorker->curLoopMode = sval;
                break;
                case 9:
                        sval = min + ((double)value * (max - min) / 127);
                        if ((sval < parStore->list.count())
                                && (sval != parStore->activeStore)
                                && (sval != parStore->currentRequest)) {
                            parStore->requestDispState(sval, 2);
                            parStore->restoreRequest = sval;
                            parStore->restoreRunOnce = (parStore->jumpToList.at(sval) > -2);
                        }
                        else return;
                break;


                default:
                break;
            }
            needsGUIUpdate = true;
        }
    }
}

void LfoWidget::updateDisplay()
{
    QVector<Sample> data;

    parStore->updateDisplay(getFramePtr()/midiWorker->frameSize, midiWorker->reverse);

    if (midiWorker->dataChanged) {
        midiWorker->getData(&data);
        screen->updateData(data);
        cursor->updateNumbers(midiWorker->res, midiWorker->size);
        midiWorker->dataChanged = false;
    }
    screen->updateDraw();
    cursor->updateDraw();
    midiControl->update();

    if (!(needsGUIUpdate || midiWorker->needsGUIUpdate)) return;

    muteOutAction->setChecked(midiWorker->isMuted);
    screen->setMuted(midiWorker->isMuted);
    parStore->ndc->setMuted(midiWorker->isMuted);
    recordAction->setChecked(midiWorker->recordMode);
    resBox->setCurrentIndex(resBoxIndex);
    updateRes(resBoxIndex);
    sizeBox->setCurrentIndex(sizeBoxIndex);
    updateSize(sizeBoxIndex);
    freqBox->setCurrentIndex(freqBoxIndex);
    updateFreq(freqBoxIndex);
    loopBox->setCurrentIndex(midiWorker->curLoopMode);
    amplitude->setValue(midiWorker->amp);
    offset->setValue(midiWorker->offs);
    if (waveFormBoxIndex != waveFormBox->currentIndex()) {
        waveFormBox->setCurrentIndex(waveFormBoxIndex);
        updateWaveForm(waveFormBoxIndex);
    }
    needsGUIUpdate = false;
    midiWorker->needsGUIUpdate = false;
}
