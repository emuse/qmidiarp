/*!
 * @file lfowidget.cpp
 * @brief Implements the LfoWidget GUI class.
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
 */

#include <QBoxLayout>
#include <QGridLayout>
#include <QInputDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QStringList>

#include "midilfo.h"
#include "lfowidget.h"
#include "slider.h"
#include "lfoscreen.h"
#include "pixmaps/lfowavcp.xpm"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
#include "pixmaps/lfowsine.xpm"
#include "pixmaps/lfowsawup.xpm"
#include "pixmaps/lfowsawdn.xpm"
#include "pixmaps/lfowtri.xpm"
#include "pixmaps/lfowsquare.xpm"
#include "pixmaps/lfowcustm.xpm"
#include "pixmaps/seqrecord.xpm"
#include "config.h"


LfoWidget::LfoWidget(MidiLfo *p_midiWorker, int portCount, bool compactStyle,
    bool mutedAdd, QWidget *parent):
    QWidget(parent), midiWorker(p_midiWorker), modified(false)
{
    int l1;
    QStringList midiCCNames;
    midiCCNames << "MuteToggle" << "Amplitude" << "Offset" << "WaveForm" << "Frequency"
                << "RecordToggle"<< "Resolution"<< "Size" << "unknown";
    midiControl = new MidiControl(midiCCNames);

    // Management Buttons on the right top
    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

    cloneAction = new QAction(QIcon(lfowavcp_xpm), tr("&Clone..."), this);
    cloneAction->setToolTip(tr("Duplicate this LFO in muted state"));
    QToolButton *cloneButton = new QToolButton(this);
    cloneButton->setDefaultAction(cloneAction);
    connect(cloneAction, SIGNAL(triggered()), this, SLOT(moduleClone()));

    renameAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."), this);
    renameAction->setToolTip(tr("Rename this LFO"));
    QToolButton *renameButton = new QToolButton(this);
    renameButton->setDefaultAction(renameAction);
    connect(renameAction, SIGNAL(triggered()), this, SLOT(moduleRename()));

    deleteAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."), this);
    deleteAction->setToolTip(tr("Delete this LFO"));
    QToolButton *deleteButton = new QToolButton(this);
    deleteButton->setDefaultAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));

    manageBoxLayout->addStretch();
    manageBoxLayout->addWidget(cloneButton);
    manageBoxLayout->addWidget(renameButton);
    manageBoxLayout->addWidget(deleteButton);

    // Input group box on right top
    QGroupBox *inBox = new QGroupBox(tr("Input"), this);

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
    inBoxLayout->addWidget(chInLabel, 2, 0);
    inBoxLayout->addWidget(chIn, 2, 1);
    if (compactStyle) {
        inBoxLayout->setSpacing(1);
        inBoxLayout->setMargin(2);
    }

    inBox->setLayout(inBoxLayout);

    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);
    connect(muteOut, SIGNAL(toggled(bool)), this, SLOT(setMuted(bool)));
    muteLabel->setBuddy(muteOut);
    midiControl->addMidiLearnMenu(muteOut, 0);


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
    if (compactStyle) {
        outputLayout->setSpacing(1);
        outputLayout->setMargin(2);
    }

    portBox->setLayout(outputLayout);

    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
    inOutBoxLayout->addLayout(manageBoxLayout);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();

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
    waveFormBox->setToolTip(tr("Waveform Basis"));

    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
    midiControl->addMidiLearnMenu(waveFormBox, 3);


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
    freqBox->setToolTip(
            tr("Frequency (cycles/beat): Number of wave cycles produced every beat"));
    freqBox->setMinimumContentsLength(3);
    connect(freqBox, SIGNAL(activated(int)), this,
            SLOT(updateFreq(int)));
    midiControl->addMidiLearnMenu(freqBox, 4);

    QLabel *resBoxLabel = new QLabel(tr("&Resolution"),
            waveBox);
    resBox = new QComboBox(waveBox);
    resBoxLabel->setBuddy(resBox);
    names.clear();
    names << "1" << "2" << "4" << "8" << "16" << "32" << "64" << "96" << "192";
    resBox->insertItems(0, names);
    resBox->setCurrentIndex(4);
    resBox->setToolTip(
            tr("Resolution (events/beat): Number of events produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));
    midiControl->addMidiLearnMenu(resBox, 6);

    QLabel *sizeBoxLabel = new QLabel(tr("&Length"), waveBox);
    sizeBox = new QComboBox(waveBox);
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8"
            << "12" << "16" << "24" << "32" ;
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(0);
    sizeBox->setToolTip(tr("Length of LFO wave in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));
    midiControl->addMidiLearnMenu(sizeBox, 7);

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"), waveBox);
    recordAction = new QAction(QIcon(seqrecord_xpm), tr("Re&cord"), waveBox);
    recordAction->setToolTip(tr("Record incoming controller"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton(waveBox);
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
    midiControl->addMidiLearnMenu(recordButton, 5);

    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), waveBox);
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));
    midiControl->addMidiLearnMenu(amplitude, 1);


    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), waveBox);
    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));
    midiControl->addMidiLearnMenu(offset, 2);

    QVBoxLayout* sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(amplitude);
    sliderLayout->addWidget(offset);
    sliderLayout->addStretch();
    if (compactStyle) {
        sliderLayout->setSpacing(1);
        sliderLayout->setMargin(2);
    }

    QGridLayout *paramBoxLayout = new QGridLayout;
    paramBoxLayout->addWidget(recordButtonLabel, 0, 0);
    paramBoxLayout->addWidget(recordButton, 0, 1);
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
    waveBoxLayout->addLayout(paramBoxLayout, 1, 0);
    waveBoxLayout->addLayout(sliderLayout, 2, 0);
    if (compactStyle) {
        waveBoxLayout->setSpacing(1);
        waveBoxLayout->setMargin(2);
    }
    waveBox->setLayout(waveBoxLayout);

    muteOut->setChecked(mutedAdd);

    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(waveBox, 1);
    widgetLayout->addLayout(inOutBoxLayout, 0);

    setLayout(widgetLayout);
    updateAmp(64);

    lastMute = false;
}

LfoWidget::~LfoWidget()
{
}

MidiLfo *LfoWidget::getMidiWorker()
{
    return (midiWorker);
}

void LfoWidget::writeData(QXmlStreamWriter& xml)
{
    QByteArray tempArray;
    int l1;

    xml.writeStartElement(name.left(3));
    xml.writeAttribute("name", name.mid(name.indexOf(':') + 1));
        xml.writeStartElement("input");
            xml.writeTextElement("channel", QString::number(
                midiWorker->chIn));
            xml.writeTextElement("ccnumber", QString::number(
                midiWorker->ccnumberIn));
        xml.writeEndElement();

        xml.writeStartElement("output");
            xml.writeTextElement("muted", QString::number(
                midiWorker->isMuted));
            xml.writeTextElement("port", QString::number(
                midiWorker->portOut));
            xml.writeTextElement("channel", QString::number(
                midiWorker->channelOut));
            xml.writeTextElement("ccnumber", QString::number(
                midiWorker->ccnumber));
        xml.writeEndElement();

        xml.writeStartElement("waveParams");
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
        while (l1 < midiWorker->muteMask.count()) {
            tempArray.append(midiWorker->muteMask.at(l1));
            l1++;
        }
        xml.writeStartElement("muteMask");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiWorker->muteMask.count()) {
            tempArray.append(midiWorker->customWave.at(l1).value);
            l1++;
        }
        xml.writeStartElement("customWave");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        midiControl->writeData(xml);

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
                    muteOut->setChecked(xml.readElementText().toInt());
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
                if (xml.name() == "waveform")
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
                    midiWorker->muteMask.clear();
                    QByteArray tmpArray =
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        midiWorker->muteMask.append(tmpArray.at(l1));
                    }
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
                    midiWorker->customWave.clear();
                    QByteArray tmpArray =
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    int step = TPQN / midiWorker->res;
                    int lt = 0;
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        sample.value = tmpArray.at(l1);
                        sample.tick = lt;
                        sample.muted = midiWorker->muteMask.at(l1);
                        midiWorker->customWave.append(sample);
                        lt+=step;
                    }
                }
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "midiControllers")) {
            midiControl->readData(xml);
        }
        else skipXmlElement(xml);
    }
    waveFormBox->setCurrentIndex(wvtmp);
    updateWaveForm(wvtmp);
    modified = false;
}

void LfoWidget::skipXmlElement(QXmlStreamReader& xml)
{
    if (xml.isStartElement()) {
        qWarning("Unknown Element in XML File: %s",qPrintable(xml.name().toString()));
        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.isEndElement())
                break;

            if (xml.isStartElement()) {
                skipXmlElement(xml);
            }
        }
    }
}

void LfoWidget::readDataText(QTextStream& arpText)
{
    QString qs, qs2;
    int l1, lt, wvtmp;
    Sample sample;

    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    channelOut->setCurrentIndex(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    portOut->setCurrentIndex(qs2.toInt());
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
    if (qs == "MIDICC")
    {
        qs = arpText.readLine();
        while (qs != "EOCC") {
            qs2 = qs.section(' ', 0, 0);
            int controlID = qs2.toInt();
            qs2 = qs.section(' ', 1, 1);
            int ccnumber = qs2.toInt();
            qs2 = qs.section(' ', 2, 2);
            int channel = qs2.toInt();
            qs2 = qs.section(' ', 3, 3);
            int min = qs2.toInt();
            qs2 = qs.section(' ', 4, 4);
            int max = qs2.toInt();
            midiControl->appendMidiCC(controlID, ccnumber, channel, min, max);
            qs = arpText.readLine();
        }
    qs = arpText.readLine();
    }

    wvtmp = qs.toInt();

    // Read Mute Mask
    int step = TPQN / midiWorker->res;
    qs = arpText.readLine();
    if (qs.isEmpty() || (qs == "EOP")) return;
    qs2 = qs.section(' ', 0, 0);
    midiWorker->muteMask.clear();
    l1 = 0;
    while (qs2 !="EOM") {
        midiWorker->muteMask.append(qs2.toInt());
        l1++;
        if (!(l1%32)) qs = arpText.readLine();
        qs2 = qs.section(' ', l1%32, l1%32);
    }

    // Read Custom Waveform
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    midiWorker->customWave.clear();
    l1 = 0;
    lt = 0;
    while (qs2 !="EOW") {
        sample.value=qs2.toInt();
        sample.tick = lt;
        sample.muted = midiWorker->muteMask.at(l1);
        midiWorker->customWave.append(sample);
        lt+=step;
        l1++;
        if (!(l1%16)) qs = arpText.readLine();
        qs2 = qs.section(' ', l1%16, l1%16);
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

void LfoWidget::updateWaveForm(int val)
{
    if (val > 5) return;
    midiWorker->updateWaveForm(val);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    bool isCustom = (val == 5);
    if (isCustom) newCustomOffset();
    amplitude->setDisabled(isCustom);
    freqBox->setDisabled(isCustom);
    modified = true;
}

void LfoWidget::updateScreen(int val)
{
    if (midiWorker->isRecording) {
        midiWorker->getData(&data);
        screen->updateScreen(data);
    }
    else {
        screen->updateScreen(val);
    }
}

void LfoWidget::updateFreq(int val)
{
    if (val > 13) return;
    midiWorker->updateFrequency(lfoFreqValues[val]);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::updateRes(int val)
{
    if (val > 8) return;
    midiWorker->updateResolution(lfoResValues[val]);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::updateSize(int val)
{
    if (val > 11) return;
    midiWorker->updateSize(sizeBox->currentText().toInt());
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::updateAmp(int val)
{
    midiWorker->updateAmplitude(val);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::updateOffs(int val)
{
    midiWorker->updateOffset(val);
    midiWorker->getData(&data);
    screen->updateScreen(data);
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
    for (int l1 = 0; l1 < data.count() - 1; l1++) {
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
    screen->updateScreen(data);
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
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::mouseWheel(int step)
{
    int cv;
    cv = offset->value() + step;
    if ((cv < 127) && (cv > 0))
    offset->setValue(cv + step);
}

void LfoWidget::setMuted(bool on)
{
    midiWorker->setMuted(on);
    screen->setMuted(on);
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

void LfoWidget::moduleDelete()
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
    emit moduleRemove(ID);
}

void LfoWidget::moduleRename()
{
    QString newname, oldname;
    bool ok;

    oldname = name;

    newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);

    if (ok && !newname.isEmpty()) {
        name = "LFO:" + newname;
        emit dockRename(name, parentDockID);
    }
}

void LfoWidget::moduleClone()
{
        emit moduleClone(ID);
}

void LfoWidget::copyParamsFrom(LfoWidget *fromWidget)
{
    int tmp;

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
    tmp = fromWidget->freqBox->currentIndex();
    freqBox->setCurrentIndex(tmp);
    updateFreq(tmp);

    amplitude->setValue(fromWidget->amplitude->value());
    offset->setValue(fromWidget->offset->value());

    midiWorker->customWave = fromWidget->getCustomWave();
    midiWorker->muteMask.clear();
    for (int l1 = 0; l1 < midiWorker->customWave.count(); l1++) {
        midiWorker->muteMask.append(midiWorker->customWave.at(l1).muted);
    }
    midiControl->setCcList(fromWidget->midiControl->ccList);
    muteOut->setChecked(true);

    tmp = fromWidget->waveFormBox->currentIndex();
    waveFormBox->setCurrentIndex(tmp);
    updateWaveForm(tmp);
}

QVector<Sample> LfoWidget::getCustomWave()
{
    return midiWorker->customWave;
}
