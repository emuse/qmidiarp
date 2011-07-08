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

#include "seqwidget.h"

#include "pixmaps/seqrecord.xpm"


SeqWidget::SeqWidget(MidiSeq *p_midiWorker, int portCount, bool compactStyle,
    bool mutedAdd, QWidget *parent):
    QWidget(parent), midiWorker(p_midiWorker), modified(false)
{
    int l1;
    QStringList midiCCNames;
    midiCCNames << "MuteToggle" << "Velocity" << "NoteLength"
                << "RecordToggle" << "Resolution"<< "Size" << "unknown";
    midiControl = new MidiControl(midiCCNames);

    manageBox = new ManageBox("Seq:", true, this);

    // Display group box on right
    QGroupBox *dispBox = new QGroupBox(tr("Display"), this);

    QSignalMapper *dispSignalMapper = new QSignalMapper(this);
    QLabel *dispLabel[4];
    QString dispText[4] = {tr("&Full"), tr("&Upper"), tr("&Mid"), tr("&Lower")};
    QGridLayout *dispBoxLayout = new QGridLayout;

    for (int l1 = 0; l1 < 4; l1++) {
        dispLabel[l1] = new QLabel(dispText[l1],dispBox);
        dispVert[l1] = new QCheckBox(this);
        connect(dispVert[l1], SIGNAL(toggled(bool)), dispSignalMapper, SLOT(map()));
        dispSignalMapper->setMapping(dispVert[l1], l1);
        dispVert[l1]->setAutoExclusive(true);
        dispLabel[l1]->setBuddy(dispVert[l1]);
        dispBoxLayout->addWidget(dispLabel[l1], l1, 0);
        dispBoxLayout->addWidget(dispVert[l1], l1, 1);
    }

    dispVert[0]->setChecked(true);
    dispVertical = 0;
    connect(dispSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(updateDispVert(int)));

    QVBoxLayout* dispLayout = new QVBoxLayout;
    dispLayout->addLayout(dispBoxLayout);

    if (compactStyle) {
        dispLayout->setSpacing(1);
        dispLayout->setMargin(2);
    }

    dispBox->setLayout(dispLayout);

    // Input group box on right top
    QGroupBox *inBox = new QGroupBox(tr("Input"), this);

    QLabel *enableNoteInLabel = new QLabel(tr("&Note"),inBox);
    enableNoteIn = new QCheckBox(this);
    connect(enableNoteIn, SIGNAL(toggled(bool)), this, SLOT(updateEnableNoteIn(bool)));
    enableNoteInLabel->setBuddy(enableNoteIn);
    enableNoteIn->setToolTip(tr("Transpose the sequence following incoming notes"));

    QLabel *enableNoteOffLabel = new QLabel(tr("&Note Off"),inBox);
    enableNoteOff = new QCheckBox(this);
    connect(enableNoteOff, SIGNAL(toggled(bool)), this, SLOT(updateEnableNoteOff(bool)));
    enableNoteOffLabel->setBuddy(enableNoteOff);
    enableNoteOff->setToolTip(tr("Stop output when Note is released"));

    QLabel *enableVelInLabel = new QLabel(tr("&Velocity"),inBox);
    enableVelIn = new QCheckBox(this);
    connect(enableVelIn, SIGNAL(toggled(bool)), this, SLOT(updateEnableVelIn(bool)));
    enableVelInLabel->setBuddy(enableVelIn);
    enableVelIn->setToolTip(tr("Set sequence velocity to that of incoming notes"));

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

    QLabel *enableLoopLabel = new QLabel(tr("&Loop"),inBox);
    enableLoop = new QCheckBox(this);
    connect(enableLoop, SIGNAL(toggled(bool)), this, SLOT(updateEnableLoop(bool)));
    enableLoopLabel->setBuddy(enableLoop);
    enableLoop->setToolTip(tr("Play sequence as loop instead of a single run"));

    enableNoteIn->setChecked(true);
    enableNoteOff->setChecked(false);
    enableVelIn->setChecked(true);
    enableRestartByKbd->setChecked(false);
    enableTrigByKbd->setChecked(false);
    enableLoop->setChecked(true);

    QLabel *chInLabel = new QLabel(tr("&Channel"), inBox);
    chIn = new QComboBox(inBox);
    for (l1 = 0; l1 < 16; l1++) chIn->addItem(QString::number(l1 + 1));
    chInLabel->setBuddy(chIn);
    connect(chIn, SIGNAL(activated(int)), this, SLOT(updateChIn(int)));

    QGridLayout *inBoxLayout = new QGridLayout;

    inBoxLayout->addWidget(enableNoteInLabel, 0, 0);
    inBoxLayout->addWidget(enableNoteIn, 0, 1);
    inBoxLayout->addWidget(enableNoteOffLabel, 1, 0);
    inBoxLayout->addWidget(enableNoteOff, 1, 1);
    inBoxLayout->addWidget(enableVelInLabel, 2, 0);
    inBoxLayout->addWidget(enableVelIn, 2, 1);
    inBoxLayout->addWidget(enableRestartByKbdLabel, 3, 0);
    inBoxLayout->addWidget(enableRestartByKbd, 3, 1);
    inBoxLayout->addWidget(enableTrigByKbdLabel, 4, 0);
    inBoxLayout->addWidget(enableTrigByKbd, 4, 1);
    inBoxLayout->addWidget(enableLoopLabel, 5, 0);
    inBoxLayout->addWidget(enableLoop, 5, 1);
    inBoxLayout->addWidget(chInLabel, 6, 0);
    inBoxLayout->addWidget(chIn, 6, 1);
    if (compactStyle) {
        inBoxLayout->setSpacing(1);
        inBoxLayout->setMargin(2);
    }

    inBox->setLayout(inBoxLayout);


    // Output group box on right bottom
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);
    connect(muteOut, SIGNAL(toggled(bool)), this, SLOT(setMuted(bool)));
    muteLabel->setBuddy(muteOut);
    midiControl->addMidiLearnMenu(muteOut, 0);

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
    inOutBoxLayout->addWidget(manageBox);
    inOutBoxLayout->addWidget(dispBox);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();

    // group box for sequence setup
    QGroupBox *seqBox = new QGroupBox(tr("Sequence"), this);

    screen = new SeqScreen(this);
    screen->setToolTip(
        tr("Right button to mute points, left button to draw custom wave"));
    screen->setMinimumHeight(SEQSCR_MIN_H);
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
    midiControl->addMidiLearnMenu(recordButton, 3);

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
    midiControl->addMidiLearnMenu(resBox, 4);

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
    midiControl->addMidiLearnMenu(sizeBox, 5);

    //temporarily hide these elements until multiple patterns are implemented
    waveFormBox->setEnabled(false);
    waveFormBox->setVisible(false);
    waveFormBoxLabel->setVisible(false);


    velocity = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("Veloc&ity"), seqBox);
    connect(velocity, SIGNAL(valueChanged(int)), this,
            SLOT(updateVelocity(int)));
    midiControl->addMidiLearnMenu(velocity, 1);


    notelength = new Slider(0, 127, 1, 16, 64, Qt::Horizontal,
            tr("N&ote Length"), seqBox);
    connect(notelength, SIGNAL(valueChanged(int)), this,
            SLOT(updateNoteLength(int)));
    midiControl->addMidiLearnMenu(notelength, 2);

    transpose = new Slider(-24, 24, 1, 2, 0, Qt::Horizontal,
            tr("&Transpose"), seqBox);
    connect(transpose, SIGNAL(valueChanged(int)), this,
            SLOT(updateTranspose(int)));


    QGridLayout* sliderLayout = new QGridLayout;
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

    muteOut->setChecked(mutedAdd);

    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(seqBox, 1);
    widgetLayout->addLayout(inOutBoxLayout, 0);

    setLayout(widgetLayout);
    recordMode = false;
    updateVelocity(64);
    updateWaveForm(0);
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

    xml.writeStartElement(manageBox->name.left(3));
    xml.writeAttribute("name", manageBox->name.mid(manageBox->name.indexOf(':') + 1));

        xml.writeStartElement("display");
            xml.writeTextElement("vertical", QString::number(
                dispVertical));
        xml.writeEndElement();

        xml.writeStartElement("input");
            xml.writeTextElement("enableNote", QString::number(
                midiWorker->enableNoteIn));
            xml.writeTextElement("enableNoteOff", QString::number(
                midiWorker->enableNoteOff));
            xml.writeTextElement("enableVelocity", QString::number(
                midiWorker->enableVelIn));
            xml.writeTextElement("restartByKbd", QString::number(
                midiWorker->restartByKbd));
            xml.writeTextElement("trigByKbd", QString::number(
                midiWorker->trigByKbd));
            xml.writeTextElement("enableLoop", QString::number(
                midiWorker->enableLoop));
            xml.writeTextElement("channel", QString::number(
                midiWorker->chIn));
        xml.writeEndElement();

        xml.writeStartElement("output");
            xml.writeTextElement("muted", QString::number(
                midiWorker->isMuted));
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

        midiControl->writeData(xml);

    xml.writeEndElement();
}

void SeqWidget::readData(QXmlStreamReader& xml)
{
    int tmp;
    int wvtmp = 0;
    Sample sample;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        else if (xml.isStartElement() && (xml.name() == "display")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "vertical")
                    setDispVert(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "input")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "enableNote")
                    enableNoteIn->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "enableNoteOff")
                    enableNoteOff->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "enableVelocity")
                    enableVelIn->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "restartByKbd")
                    enableRestartByKbd->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "trigByKbd")
                    enableTrigByKbd->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "enableLoop")
                    enableLoop->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "channel") {
                    tmp = xml.readElementText().toInt();
                    chIn->setCurrentIndex(tmp);
                    updateChIn(tmp);
                }
                else skipXmlElement(xml);
            }
        }

        else if (xml.isStartElement() && (xml.name() == "output")) {
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
                    updateTranspose(tmp);
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
    chIn->setCurrentIndex(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    channelOut->setCurrentIndex(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    portOut->setCurrentIndex(qs2.toInt());
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
    chIn->setCurrentIndex(value);
    modified = true;
}

void SeqWidget::updateChIn(int value)
{
    midiWorker->chIn = value;
    modified = true;
}

void SeqWidget::updateEnableNoteIn(bool on)
{
    midiWorker->enableNoteIn = on;
    modified = true;
}

void SeqWidget::updateEnableNoteOff(bool on)
{
    midiWorker->enableNoteOff = on;
    modified = true;
}

void SeqWidget::updateEnableVelIn(bool on)
{
    midiWorker->enableVelIn = on;
    modified = true;
}

void SeqWidget::updateEnableRestartByKbd(bool on)
{
    midiWorker->restartByKbd = on;
    modified = true;
}

void SeqWidget::updateEnableTrigByKbd(bool on)
{
    midiWorker->trigByKbd = on;
    modified = true;
}

void SeqWidget::updateEnableLoop(bool on)
{
    midiWorker->enableLoop = on;
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
    screen->update();
}

void SeqWidget::updateRes(int val)
{
    if (val > 4) return;
    midiWorker->res = seqResValues[val];
    midiWorker->resizeAll();
    midiWorker->getData(&data);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
    screen->updateScreen(data);
    modified = true;
}

void SeqWidget::updateSize(int val)
{
    if (val > 7) return;
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
    portOut->setCurrentIndex(value);
    modified = true;
}

void SeqWidget::setChannelOut(int value)
{
    channelOut->setCurrentIndex(value);
    modified = true;
}

void SeqWidget::updatePortOut(int value)
{
    midiWorker->portOut = value;
    modified = true;
}

void SeqWidget::updateChannelOut(int value)
{
    midiWorker->channelOut = value;
    modified = true;
}

bool SeqWidget::isModified()
{
    return (modified || midiControl->isModified());
}

void SeqWidget::setModified(bool m)
{
    modified = m;
    midiControl->setModified(m);
}

void SeqWidget::setDispVert(int mode)
{
    dispVert[mode]->setChecked(true);
}

void SeqWidget::updateDispVert(int mode)
{
    int noct, baseoct;

    switch (mode) {
        case 0:
            noct = 4;
            baseoct = 3;
        break;
        case 1:
            noct = 2;
            baseoct = 5;
        break;
        case 2:
            noct = 2;
            baseoct = 4;
        break;
        case 3:
            noct = 2;
            baseoct = 3;
        break;
        default:
            noct = 4;
            baseoct = 3;
    }

    dispVertical = mode;
    midiWorker->nOctaves = noct;
    midiWorker->baseOctave = baseoct;
    screen->nOctaves = noct;
    screen->baseOctave = baseoct;
    screen->update();
    modified = true;
}

void SeqWidget::copyParamsFrom(SeqWidget *fromWidget)
{
    int tmp;
    setDispVert(fromWidget->dispVertical);
    enableNoteIn->setChecked(fromWidget->enableNoteIn->isChecked());
    enableNoteOff->setChecked(fromWidget->enableNoteOff->isChecked());
    enableVelIn->setChecked(fromWidget->enableVelIn->isChecked());
    enableRestartByKbd->setChecked(fromWidget->enableRestartByKbd->isChecked());
    enableTrigByKbd->setChecked(fromWidget->enableTrigByKbd->isChecked());
    enableLoop->setChecked(fromWidget->enableLoop->isChecked());

    tmp = fromWidget->chIn->currentIndex();
    chIn->setCurrentIndex(tmp);
    updateChIn(tmp);
    tmp = fromWidget->channelOut->currentIndex();
    channelOut->setCurrentIndex(tmp);
    updateChannelOut(tmp);
    tmp = fromWidget->portOut->currentIndex();
    portOut->setCurrentIndex(tmp);
    updatePortOut(tmp);

    tmp = fromWidget->resBox->currentIndex();
    resBox->setCurrentIndex(tmp);
    updateRes(tmp);
    tmp = fromWidget->sizeBox->currentIndex();
    sizeBox->setCurrentIndex(tmp);
    updateSize(tmp);

    tmp = fromWidget->velocity->value();
    updateVelocity(tmp);
    velocity->setValue(tmp);
    tmp = fromWidget->transpose->value();
    updateTranspose(tmp);
    transpose->setValue(tmp);

    notelength->setValue(fromWidget->notelength->value());
    midiWorker->customWave = fromWidget->getCustomWave();
    midiWorker->muteMask.clear();
    for (int l1 = 0; l1 < midiWorker->customWave.count(); l1++) {
        midiWorker->muteMask.append(midiWorker->customWave.at(l1).muted);
    }
    midiControl->setCcList(fromWidget->midiControl->ccList);
    muteOut->setChecked(true);
    updateWaveForm(0);
}

QVector<Sample> SeqWidget::getCustomWave()
{
    return midiWorker->customWave;
}
