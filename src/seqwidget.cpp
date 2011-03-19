/*!
 * @file seqwidget.cpp
 * @brief Implements the SeqWidget GUI class.
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
#include "pixmaps/seqrecord.xpm"
#include "config.h"


SeqWidget::SeqWidget(MidiSeq *p_midiWorker, int portCount, bool compactStyle, QWidget *parent):
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

    midiCCNames << "MuteToggle" << "Velocity" << "NoteLength"
                << "RecordToggle" << "unknown";

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
    if (compactStyle) {
        inBoxLayout->setSpacing(1);
        inBoxLayout->setMargin(2);
    }

    inBox->setLayout(inBoxLayout);


    // Output group box on right bottom
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

    // group box for sequence setup
    QGroupBox *seqBox = new QGroupBox(tr("Sequence"), this);

    screen = new SeqScreen(this);
    screen->setToolTip(
        tr("Right button to mute points, left button to draw custom wave"));
    screen->setMinimumHeight(SEQSCREEN_MINIMUM_HEIGHT);
    connect(screen, SIGNAL(mouseMoved(double, double, int)), this,
            SLOT(mouseMoved(double, double, int)));
    connect(screen, SIGNAL(mousePressed(double, double, int)), this,
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

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"), seqBox);
    recordAction = new QAction(QIcon(seqrecord_xpm), tr("Re&cord"), seqBox);
    recordAction->setToolTip(tr("Record step by step"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton(seqBox);
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
    recordButton->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));

    QAction *recordLearnAction = new QAction(tr("MIDI &Learn"), this);
    recordButton->addAction(recordLearnAction);
    connect(recordLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(recordLearnAction, 3);

    QAction *recordForgetAction = new QAction(tr("MIDI &Forget"), this);
    recordButton->addAction(recordForgetAction);
    connect(recordForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(recordForgetAction, 3);

    recordButton->addAction(cancelMidiLearnAction);

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
    connect(velocity, SIGNAL(sliderMoved(int)), this,
            SLOT(updateVelocity(int)));


    QAction *velocityLearnAction = new QAction(tr("MIDI &Learn"), this);
    velocity->addAction(velocityLearnAction);
    connect(velocityLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(velocityLearnAction, 1);

    QAction *velocityForgetAction = new QAction(tr("MIDI &Forget"), this);
    velocity->addAction(velocityForgetAction);
    connect(velocityForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(velocityForgetAction, 1);

    velocity->addAction(cancelMidiLearnAction);


    notelength = new Slider(0, 127, 1, 16, 64, Qt::Horizontal,
            tr("N&ote Length"), seqBox);
    notelength->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    connect(notelength, SIGNAL(valueChanged(int)), this,
            SLOT(updateNoteLength(int)));

    QAction *noteLengthLearnAction = new QAction(tr("MIDI &Learn"), this);
    notelength->addAction(noteLengthLearnAction);
    connect(noteLengthLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(noteLengthLearnAction, 2);

    QAction *noteLengthForgetAction = new QAction(tr("MIDI &Forget"), this);
    notelength->addAction(noteLengthForgetAction);
    connect(noteLengthForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(noteLengthForgetAction, 2);

    notelength->addAction(cancelMidiLearnAction);

    transpose = new Slider(-24, 24, 1, 2, 0, Qt::Horizontal,
            tr("&Transpose"), seqBox);
    connect(transpose, SIGNAL(sliderMoved(int)), this,
            SLOT(updateTranspose(int)));


    QGridLayout* sliderLayout = new QGridLayout;
    sliderLayout->addWidget(copyToCustomButton, 0 , 0);
    sliderLayout->addWidget(velocity, 1, 0);
    sliderLayout->addWidget(notelength, 2, 0);
    sliderLayout->addWidget(transpose, 3, 0);
    sliderLayout->setRowStretch(4, 1);
    if (compactStyle) {
        sliderLayout->setSpacing(1);
        sliderLayout->setMargin(2);
    }

    QGridLayout *paramBoxLayout = new QGridLayout;
    paramBoxLayout->addWidget(waveFormBoxLabel, 0, 0);
    paramBoxLayout->addWidget(waveFormBox, 0, 1);
    paramBoxLayout->addWidget(recordButtonLabel, 1, 0);
    paramBoxLayout->addWidget(recordButton, 1, 1);
    paramBoxLayout->addWidget(resBoxLabel, 2, 0);
    paramBoxLayout->addWidget(resBox, 2, 1);
    paramBoxLayout->addWidget(sizeBoxLabel, 3, 0);
    paramBoxLayout->addWidget(sizeBox, 3, 1);
    paramBoxLayout->setRowStretch(4, 1);

    QGridLayout* seqBoxLayout = new QGridLayout;
    seqBoxLayout->addWidget(screen, 0, 0, 1, 2);
    seqBoxLayout->addLayout(paramBoxLayout, 1, 0);
    seqBoxLayout->addLayout(sliderLayout, 1, 1);
    if (compactStyle) {
        seqBoxLayout->setMargin(2);
        seqBoxLayout->setSpacing(1);
    }
    seqBox->setLayout(seqBoxLayout);

    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(seqBox, 1);
    widgetLayout->addLayout(inOutBoxLayout, 0);

    setLayout(widgetLayout);
    recordMode = false;
    updateVelocity(64);
    updateWaveForm(0);
    ccList.clear();
    lastMute = false;
}

SeqWidget::~SeqWidget()
{
}

MidiSeq *SeqWidget::getMidiWorker()
{
    return (midiWorker);
}

void SeqWidget::writeData(QXmlStreamWriter& xml)
{
    QByteArray tempArray;
    int l1;

    xml.writeStartElement(name.left(3));
    xml.writeAttribute("name", name.mid(name.indexOf(':') + 1));
        xml.writeStartElement("input");
            xml.writeTextElement("enableNote", QString::number(
                midiWorker->enableNoteIn));
            xml.writeTextElement("enableVelocity", QString::number(
                midiWorker->enableVelIn));
            xml.writeTextElement("channel", QString::number(
                midiWorker->chIn));
        xml.writeEndElement();

        xml.writeStartElement("output");
            xml.writeTextElement("port", QString::number(
                midiWorker->portOut));
            xml.writeTextElement("channel", QString::number(
                midiWorker->channelOut));
        xml.writeEndElement();

        xml.writeStartElement("seqParams");
            xml.writeTextElement("resolution", QString::number(
                resBox->currentIndex()));
            xml.writeTextElement("size", QString::number(
                sizeBox->currentIndex()));
            xml.writeTextElement("velocity", QString::number(
                midiWorker->vel));
            xml.writeTextElement("noteLength", QString::number(
                midiWorker->notelength));
            xml.writeTextElement("transp", QString::number(
                midiWorker->transp));
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
        xml.writeStartElement("sequence");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        xml.writeStartElement("midiControllers");
        for (int l1 = 0; l1 < ccList.count(); l1++) {
            xml.writeStartElement("MIDICC");
            xml.writeAttribute("CtrlID", QString::number(ccList.at(l1).ID));
                xml.writeTextElement("ccnumber", QString::number(ccList.at(l1).ccnumber));
                xml.writeTextElement("channel", QString::number(ccList.at(l1).channel));
                xml.writeTextElement("min", QString::number(ccList.at(l1).min));
                xml.writeTextElement("max", QString::number(ccList.at(l1).max));
            xml.writeEndElement();
        }
        xml.writeEndElement();

    xml.writeEndElement();
}

void SeqWidget::writeDataText(QTextStream& arpText)
{
    int l1 = 0;
    arpText << midiWorker->enableNoteIn << ' '
        << midiWorker->enableVelIn << ' '
        << midiWorker->chIn << '\n';
    arpText << midiWorker->channelOut << ' '
        << midiWorker->portOut << ' '
        << midiWorker->notelength << '\n';
    arpText << resBox->currentIndex() << ' '
        << sizeBox->currentIndex() << ' '
        << midiWorker->vel << ' '
        << midiWorker->transp << '\n';
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
    // Write Custom Sequence
    l1 = 0;
    while (l1 < midiWorker->customWave.count()) {
        arpText << midiWorker->customWave.at(l1).value << ' ';
        l1++;
        if (!(l1 % 16)) arpText << "\n";
    }
    arpText << "EOS\n"; // End Of Wave
    modified = false;
}

void SeqWidget::readData(QXmlStreamReader& xml)
{
    int controlID, ccnumber, channel, min, max;
    int tmp;
    int wvtmp = 0;
    Sample sample;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        else if (xml.isStartElement() && (xml.name() == "input")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "enableNote")
                    enableNoteIn->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "enableVelocity")
                    enableVelIn->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "channel")
                    chIn->setValue(xml.readElementText().toInt() + 1);
                else skipXmlElement(xml);
            }
        }

        else if (xml.isStartElement() && (xml.name() == "output")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "channel")
                    channelOut->setValue(xml.readElementText().toInt() + 1);
                else if (xml.name() == "port")
                    portOut->setValue(xml.readElementText().toInt() + 1);
                else skipXmlElement(xml);
            }
        }

        else if (xml.isStartElement() && (xml.name() == "seqParams")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
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
                else if (xml.name() == "velocity") {
                    tmp = xml.readElementText().toInt();
                    velocity->setValue(tmp);
                    updateVelocity(tmp);
                }
                else if (xml.name() == "noteLength") {
                    notelength->setValue(xml.readElementText().toInt() / 2);
                }
                else if (xml.name() == "transp") {
                    tmp = xml.readElementText().toInt();
                    transpose->setValue(tmp);
                    updateVelocity(tmp);
                }
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
        else if (xml.isStartElement() && (xml.name() == "sequence")) {
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

void SeqWidget::skipXmlElement(QXmlStreamReader& xml)
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

void SeqWidget::readDataText(QTextStream& arpText)
{
    QString qs, qs2;
    int l1, lt, wvtmp;
    Sample sample;

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
    while (qs2 !="EOS") {
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

void SeqWidget::loadWaveForms()
{
    waveForms << tr("Custom");
}

void SeqWidget::setEnableNoteIn(bool on)
{
    enableNoteIn->setChecked(on);
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

void SeqWidget::updateChIn(int value)
{
    midiWorker->chIn = value - 1;
    modified = true;
}

void SeqWidget::updateEnableNoteIn(bool on)
{
    midiWorker->enableNoteIn = on;
    modified = true;
}

void SeqWidget::updateEnableVelIn(bool on)
{
    midiWorker->enableVelIn = on;
    modified = true;
}

void SeqWidget::updateNoteLength(int val)
{
    midiWorker->notelength = val + val;
    modified = true;
}

void SeqWidget::updateWaveForm(int val)
{
    midiWorker->updateWaveForm(val);
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void SeqWidget::setRecord(bool on)
{
    recordMode = on;
    midiWorker->setRecordMode(on);
    screen->setRecordMode(on);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
    screen->updateScreen(data);
}

void SeqWidget::updateRes(int val)
{
    midiWorker->res = seqResValues[val];
    midiWorker->resizeAll();
    midiWorker->getData(&data);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
    screen->updateScreen(data);
    modified = true;
}

void SeqWidget::updateSize(int val)
{
    midiWorker->size = val + 1;
    midiWorker->resizeAll();
    midiWorker->getData(&data);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
    screen->updateScreen(data);
    modified = true;
}

void SeqWidget::updateVelocity(int val)
{
    midiWorker->vel = val;
    modified = true;
}

void SeqWidget::updateTranspose(int val)
{
    midiWorker->transp = val;
    modified = true;
}

void SeqWidget::processNote(int note, int vel)
{
    if (!recordMode) {
        if (enableNoteIn->isChecked()) transpose->setValue(note - 60);
        if (enableVelIn->isChecked()) velocity->setValue(vel);
    }
    else {
        midiWorker->getData(&data);
        screen->setCurrentRecStep(midiWorker->currentRecStep);
        screen->updateScreen(data);
    }
}

void SeqWidget::copyToCustom()
{
    midiWorker->copyToCustom();
    waveFormBox->setCurrentIndex(0);
    updateWaveForm(0);
    modified = true;
}

void SeqWidget::mouseMoved(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        midiWorker->setMutePoint(mouseX, lastMute);
    }
    else {
        midiWorker->setCustomWavePoint(mouseX, mouseY);
        screen->setCurrentRecStep(midiWorker->currentRecStep);
    }
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void SeqWidget::mousePressed(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        lastMute = midiWorker->toggleMutePoint(mouseX);
    } else {
        midiWorker->setCustomWavePoint(mouseX, mouseY);
        screen->setCurrentRecStep(midiWorker->currentRecStep);
    }
    midiWorker->getData(&data);
    screen->updateScreen(data);
    modified = true;
}

void SeqWidget::setMuted(bool on)
{
    midiWorker->setMuted(on);
    screen->setMuted(on);
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

void SeqWidget::updatePortOut(int value)
{
    midiWorker->portOut = value - 1;
    modified = true;
}

void SeqWidget::updateChannelOut(int value)
{
    midiWorker->channelOut = value - 1;
    modified = true;
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
    emit moduleRemove(ID);
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

void SeqWidget::appendMidiCC(int controlID, int ccnumber, int channel, int min, int max)
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

void SeqWidget::removeMidiCC(int controlID, int ccnumber, int channel)
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

void SeqWidget::midiLearn(int controlID)
{
    emit setMidiLearn(parentDockID, ID, controlID);
    qWarning("Requesting Midi Learn for %s", qPrintable(midiCCNames.at(controlID)));
    cancelMidiLearnAction->setEnabled(true);
}

void SeqWidget::midiForget(int controlID)
{
    removeMidiCC(controlID, 0, -1);
}

void SeqWidget::midiLearnCancel()
{
    emit setMidiLearn(parentDockID, ID, -1);
    qWarning("Cancelling Midi Learn request");
    cancelMidiLearnAction->setEnabled(false);
}
