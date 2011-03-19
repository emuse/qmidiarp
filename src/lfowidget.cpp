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


LfoWidget::LfoWidget(MidiLfo *p_midiWorker, int portCount, bool compactStyle, QWidget *parent):
    QWidget(parent), midiWorker(p_midiWorker), modified(false)
{
    // QSignalMappers allow identifying signal senders for MIDI learn/forget
    learnSignalMapper = new QSignalMapper(this);
    connect(learnSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(midiLearn(int)));

    forgetSignalMapper = new QSignalMapper(this);
    connect(forgetSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(midiForget(int)));

    // we need the cancel MIDI Learn action only once for all
    cancelMidiLearnAction = new QAction(tr("Cancel MIDI &Learning"), this);
    connect(cancelMidiLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnCancel()));
    cancelMidiLearnAction->setEnabled(false);

    midiCCNames << "MuteToggle" << "Amplitude" << "Offset" << "WaveForm" << "Frequency"
                << "RecordToggle" << "unknown";

    // Management Buttons on the right top
    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

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
    chIn = new QSpinBox(inBox);
    chIn->setRange(1, 16);
    chIn->setKeyboardTracking(false);
    chInLabel->setBuddy(chIn);
    connect(chIn, SIGNAL(valueChanged(int)), this, SLOT(updateChIn(int)));

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

    muteOut->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    connect(muteOut, SIGNAL(toggled(bool)), this, SLOT(setMuted(bool)));
    muteLabel->setBuddy(muteOut);

    QAction *muteLearnAction = new QAction(tr("MIDI &Learn"), this);
    muteOut->addAction(muteLearnAction);
    connect(muteLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(muteLearnAction, 0);

    QAction *muteForgetAction = new QAction(tr("MIDI &Forget"), this);
    muteOut->addAction(muteForgetAction);
    connect(muteForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(muteForgetAction, 0);

    muteOut->addAction(cancelMidiLearnAction);


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
    waveFormBox->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));

    QAction *waveFormBoxLearnAction = new QAction(tr("MIDI &Learn"), this);
    waveFormBox->addAction(waveFormBoxLearnAction);
    connect(waveFormBoxLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(waveFormBoxLearnAction, 3);

    QAction *waveFormBoxForgetAction = new QAction(tr("MIDI &Forget"), this);
    waveFormBox->addAction(waveFormBoxForgetAction);
    connect(waveFormBoxForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(waveFormBoxForgetAction, 3);

    waveFormBox->addAction(cancelMidiLearnAction);


    QLabel *freqBoxLabel = new QLabel(tr("&Frequency"),
            waveBox);
    freqBox = new QComboBox(waveBox);
    freqBoxLabel->setBuddy(freqBox);
    QStringList names;
    names << "1/4" << "1/2" << "3/4" << "1" << "2" << "3"
        << "4" << "5" << "6" << "7" << "8";
    freqBox->insertItems(0, names);
    freqBox->setCurrentIndex(3);
    freqBox->setToolTip(
            tr("Frequency (cycles/beat): Number of wave cycles produced every beat"));
    freqBox->setMinimumContentsLength(3);
    connect(freqBox, SIGNAL(activated(int)), this,
            SLOT(updateFreq(int)));

    freqBox->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));

    QAction *freqBoxLearnAction = new QAction(tr("MIDI &Learn"), this);
    freqBox->addAction(freqBoxLearnAction);
    connect(freqBoxLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(freqBoxLearnAction, 4);

    QAction *freqBoxForgetAction = new QAction(tr("MIDI &Forget"), this);
    freqBox->addAction(freqBoxForgetAction);
    connect(freqBoxForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(freqBoxForgetAction, 4);

    freqBox->addAction(cancelMidiLearnAction);

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

    QLabel *sizeBoxLabel = new QLabel(tr("&Length"), waveBox);
    sizeBox = new QComboBox(waveBox);
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(0);
    sizeBox->setToolTip(tr("Length of LFO wave in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"), waveBox);
    recordAction = new QAction(QIcon(seqrecord_xpm), tr("Re&cord"), waveBox);
    recordAction->setToolTip(tr("Record incoming controller"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton(waveBox);
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
    recordButton->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));

    QAction *recordLearnAction = new QAction(tr("MIDI &Learn"), this);
    recordButton->addAction(recordLearnAction);
    connect(recordLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(recordLearnAction, 5);

    QAction *recordForgetAction = new QAction(tr("MIDI &Forget"), this);
    recordButton->addAction(recordForgetAction);
    connect(recordForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(recordForgetAction, 5);

    recordButton->addAction(cancelMidiLearnAction);

    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), waveBox);

    amplitude->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));

    QAction *amplitudeLearnAction = new QAction(tr("MIDI &Learn"), this);
    amplitude->addAction(amplitudeLearnAction);
    connect(amplitudeLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(amplitudeLearnAction, 1);

    QAction *amplitudeForgetAction = new QAction(tr("MIDI &Forget"), this);
    amplitude->addAction(amplitudeForgetAction);
    connect(amplitudeForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(amplitudeForgetAction, 1);

    amplitude->addAction(cancelMidiLearnAction);


    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), waveBox);
    offset->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));

    QAction *offsetLearnAction = new QAction(tr("MIDI &Learn"), this);
    offset->addAction(offsetLearnAction);
    connect(offsetLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(offsetLearnAction, 2);

    QAction *offsetForgetAction = new QAction(tr("MIDI &Forget"), this);
    offset->addAction(offsetForgetAction);
    connect(offsetForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(offsetForgetAction, 2);

    offset->addAction(cancelMidiLearnAction);

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

    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(waveBox, 1);
    widgetLayout->addLayout(inOutBoxLayout, 0);

    setLayout(widgetLayout);
    updateAmp(64);

    ccList.clear();
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

        xml.writeStartElement("midiControllers");
        for (int l1 = 0; l1 < ccList.count(); l1++) {
            xml.writeStartElement("MIDICC");
            xml.writeAttribute("CtrlID", QString::number(ccList.at(l1).ID));
                xml.writeTextElement("ccnumber", QString::number(
                    ccList.at(l1).ccnumber));
                xml.writeTextElement("channel", QString::number(
                    ccList.at(l1).channel));
                xml.writeTextElement("min", QString::number(
                    ccList.at(l1).min));
                xml.writeTextElement("max", QString::number(
                    ccList.at(l1).max));
            xml.writeEndElement();
        }
        xml.writeEndElement();

    xml.writeEndElement();
}

void LfoWidget::writeDataText(QTextStream& arpText)
{
    int l1 = 0;
    arpText << midiWorker->channelOut << ' '
        << midiWorker->portOut << ' '
        << midiWorker->ccnumber << '\n';
    arpText << freqBox->currentIndex() << ' '
        << resBox->currentIndex() << ' '
        << sizeBox->currentIndex() << ' '
        << midiWorker->amp << ' '
        << midiWorker->offs << '\n';
    arpText << "MIDICC" << endl;
    for (int l1 = 0; l1 < ccList.count(); l1++) {
        arpText << ccList.at(l1).ID << ' '
                << ccList.at(l1).ccnumber << ' '
                << ccList.at(l1).channel << ' '
                << ccList.at(l1).min << ' '
                << ccList.at(l1).max << endl;
    }
    arpText << "EOCC" << endl;

    arpText << waveFormBox->currentIndex() << '\n';
    // Write Mute Mask
    while (l1 < midiWorker->muteMask.count()) {
        arpText << midiWorker->muteMask.at(l1) << ' ';
        l1++;
        if (!(l1 % 32)) arpText << "\n";
    }
    arpText << "EOM\n"; // End Of Mute
    // Write Custom Waveform
    l1 = 0;
    while (l1 < midiWorker->customWave.count()) {
        arpText << midiWorker->customWave.at(l1).value << ' ';
        l1++;
        if (!(l1 % 16)) arpText << "\n";
    }
    arpText << "EOW\n"; // End Of Wave
    modified = false;
}

void LfoWidget::readData(QXmlStreamReader& xml)
{
    int controlID, ccnumber, channel, min, max;
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
                if (xml.name() == "channel")
                    chIn->setValue(xml.readElementText().toInt() + 1);
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
                if (xml.name() == "channel")
                    channelOut->setValue(xml.readElementText().toInt() + 1);
                else if (xml.name() == "port")
                    portOut->setValue(xml.readElementText().toInt() + 1);
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
                    int step = TICKS_PER_QUARTER / midiWorker->res;
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
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.isStartElement() && (xml.name() == "MIDICC")) {
                    controlID = xml.attributes().value("CtrlID").toString().toInt();
                    ccnumber = -1;
                    channel = -1;
                    min = -1;
                    max = -1;
                    while (!xml.atEnd()) {
                        xml.readNext();
                        if (xml.isEndElement())
                            break;
                        if (xml.name() == "ccnumber")
                            ccnumber = xml.readElementText().toInt();
                        else if (xml.name() == "channel")
                            channel = xml.readElementText().toInt();
                        else if (xml.name() == "min")
                            min = xml.readElementText().toInt();
                        else if (xml.name() == "max")
                            max = xml.readElementText().toInt();
                        else skipXmlElement(xml);
                    }
                    if ((-1 < ccnumber) && (-1 < channel) && (-1 < min) && (-1 < max))
                        appendMidiCC(controlID, ccnumber, channel, min, max);
                    else qWarning("Controller data incomplete");
                }
                else skipXmlElement(xml);
            }
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
            appendMidiCC(controlID, ccnumber, channel, min, max);
            qs = arpText.readLine();
        }
    qs = arpText.readLine();
    }

    wvtmp = qs.toInt();

    // Read Mute Mask
    int step = TICKS_PER_QUARTER / midiWorker->res;
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

void LfoWidget::updateChIn(int value)
{
    midiWorker->chIn = value - 1;
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
    if (val > 10) return;
    midiWorker->updateFrequency(lfoFreqValues[val]);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::updateRes(int val)
{
    midiWorker->updateResolution(lfoResValues[val]);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void LfoWidget::updateSize(int val)
{
    midiWorker->updateSize(val + 1);
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
    portOut->setValue(value);
    modified = true;
}

void LfoWidget::setChannelOut(int value)
{
    channelOut->setValue(value);
    modified = true;
}

void LfoWidget::updatePortOut(int value)
{
    midiWorker->portOut = value - 1;
    modified = true;
}

void LfoWidget::updateChannelOut(int value)
{
    midiWorker->channelOut = value - 1;
    modified = true;
}

bool LfoWidget::isModified()
{
    return modified;
}

void LfoWidget::setModified(bool m)
{
    modified = m;
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

void LfoWidget::appendMidiCC(int controlID, int ccnumber, int channel, int min, int max)
{
    MidiCC midiCC;
    int l1 = 0;
    midiCC.name = midiCCNames.at(controlID);
    midiCC.ID = controlID;
    midiCC.ccnumber = ccnumber;
    midiCC.channel = channel;
    midiCC.min = min;
    midiCC.max = max;

    while ( (l1 < ccList.count()) &&
        ((controlID != ccList.at(l1).ID) ||
        (ccnumber != ccList.at(l1).ccnumber) ||
        (channel != ccList.at(l1).channel)) ) l1++;

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

void LfoWidget::removeMidiCC(int controlID, int ccnumber, int channel)
{
    for (int l1 = 0; l1 < ccList.count(); l1++) {
        if (ccList.at(l1).ID == controlID) {
            if (((ccList.at(l1).ccnumber == ccnumber)
                    && (ccList.at(l1).channel == channel))
                    || (0 > channel)) {
                ccList.remove(l1);
                l1--;
                qWarning("controller removed");
            }
        }
    }
    modified = true;
}

void LfoWidget::midiLearn(int controlID)
{
    emit setMidiLearn(parentDockID, ID, controlID);
    qWarning("Requesting Midi Learn for %s", qPrintable(midiCCNames.at(controlID)));
    cancelMidiLearnAction->setEnabled(true);
}

void LfoWidget::midiForget(int controlID)
{
    removeMidiCC(controlID, 0, -1);
}

void LfoWidget::midiLearnCancel()
{
    emit setMidiLearn(parentDockID, ID, -1);
    qWarning("Cancelling Midi Learn request");
    cancelMidiLearnAction->setEnabled(false);
}
