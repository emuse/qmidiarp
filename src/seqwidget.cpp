/*!
 * @file seqwidget.cpp
 * @brief Implements the SeqWidget GUI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
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
#include <QGroupBox>
#include <QInputDialog>
#include <QMessageBox>

#include "seqwidget.h"

#include "pixmaps/seqrecord.xpm"


SeqWidget::SeqWidget(MidiSeq *p_midiWorker, GlobStore *p_globStore,
    int portCount, bool compactStyle,
    bool mutedAdd, bool inOutVisible, const QString& name, QWidget *parent):
    QWidget(parent), midiWorker(p_midiWorker),
    globStore(p_globStore), modified(false)
{
    int l1;

    midiControl = new MidiControl(this);

    parStore = new ParStore(globStore, name, this);
    midiControl->addMidiLearnMenu("Restore_"+name, parStore->topButton, 7);
    connect(parStore, SIGNAL(store(int, bool)),
             this, SLOT(storeParams(int, bool)));
    connect(parStore, SIGNAL(restore(int)),
             this, SLOT(restoreParams(int)));

    manageBox = new ManageBox("Seq:", true, this);

    // Display group box on right
    QGroupBox *dispBox = new QGroupBox(tr("Display"), this);

    QSignalMapper *dispSignalMapper = new QSignalMapper(this);
    QLabel *dispLabel[4];
    QString dispText[4] = {tr("&F"), tr("&U"), tr("&M"), tr("&L")};
    QString dispToolTip[4] = {tr("Full"), tr("Upper"), tr("Mid"), tr("Lower")};
    QGridLayout *dispBoxLayout = new QGridLayout;

    for (int l1 = 0; l1 < 4; l1++) {
        dispLabel[l1] = new QLabel(dispText[l1],dispBox);
        dispVert[l1] = new QCheckBox(this);
        connect(dispVert[l1], SIGNAL(toggled(bool)), dispSignalMapper, SLOT(map()));
        dispSignalMapper->setMapping(dispVert[l1], l1);
        dispVert[l1]->setAutoExclusive(true);
        dispLabel[l1]->setBuddy(dispVert[l1]);
        dispVert[l1]->setToolTip(dispToolTip[l1]);
        dispBoxLayout->addWidget(dispLabel[l1], 0, l1);
        dispBoxLayout->addWidget(dispVert[l1], 1, l1);
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

    QLabel *enableTrigLegatoLabel = new QLabel(tr("&Legato"),inBox);
    enableTrigLegato = new QCheckBox(this);
    connect(enableTrigLegato, SIGNAL(toggled(bool)), this, SLOT(updateTrigLegato(bool)));
    enableTrigLegatoLabel->setBuddy(enableTrigLegato);
    enableTrigLegato->setToolTip(tr("Retrigger / restart upon new legato note as well"));

    enableNoteIn->setChecked(true);
    enableNoteOff->setChecked(false);
    enableVelIn->setChecked(true);
    enableRestartByKbd->setChecked(false);
    enableTrigByKbd->setChecked(false);
    enableTrigLegato->setChecked(false);

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
    inBoxLayout->addWidget(enableTrigLegatoLabel, 5, 0);
    inBoxLayout->addWidget(enableTrigLegato, 5, 1);
    inBoxLayout->addWidget(chInLabel, 6, 0);
    inBoxLayout->addWidget(chIn, 6, 1);
    if (compactStyle) {
        inBoxLayout->setSpacing(1);
        inBoxLayout->setMargin(2);
    }

    inBox->setLayout(inBoxLayout);


    // Output group box on right bottom
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

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
    portBoxLayout->addWidget(portLabel, 0, 0);
    portBoxLayout->addWidget(portOut, 0, 1);
    portBoxLayout->addWidget(channelLabel, 1, 0);
    portBoxLayout->addWidget(channelOut, 1, 1);

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
    inOutBoxLayout->addWidget(dispBox);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();
    inOutBox = new QWidget(this);
    inOutBox->setLayout(inOutBoxLayout);
    inOutBox->setVisible(inOutVisible);

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

    cursor = new Cursor('S', this);

    muteOut = new QPushButton(tr("&Mute"),this);
    muteOut->setFont(QFont("Helvetica", 8));
    muteOut->setMinimumSize(QSize(35,10));
    muteOut->setCheckable(true);
    connect(muteOut, SIGNAL(toggled(bool)), this, SLOT(setMuted(bool)));
    midiControl->addMidiLearnMenu("MuteToggle", muteOut, 0);

    loopBox = new QComboBox(seqBox);
    QStringList names;
    names.clear();
    names << "->_>" << " <_<-" << "->_<" << " >_<-" << "->_|" << " |_<-";
    loopBox->insertItems(0, names);
    loopBox->setCurrentIndex(0);
    loopBox->setToolTip(tr("Loop, bounce or play once going forward or backward"));
    loopBox->setMinimumContentsLength(5);
    connect(loopBox, SIGNAL(activated(int)), this,
            SLOT(updateLoop(int)));
    midiControl->addMidiLearnMenu("LoopMode", loopBox, 6);

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"), seqBox);
    recordAction = new QAction(QIcon(seqrecord_xpm), tr("Re&cord"), seqBox);
    recordAction->setToolTip(tr("Record step by step"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton(seqBox);
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
    midiControl->addMidiLearnMenu("RecordToggle", recordButton, 3);

    QLabel *resBoxLabel = new QLabel(tr("&Resolution"),
            seqBox);
    resBox = new QComboBox(seqBox);
    resBoxLabel->setBuddy(resBox);
    names.clear();
    names << "1" << "2" << "4" << "8" << "16";
    resBox->insertItems(0, names);
    resBox->setCurrentIndex(2);
    resBoxIndex = 2;
    resBox->setToolTip(
            tr("Resolution (notes/beat): Number of notes produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));
    midiControl->addMidiLearnMenu("Resolution", resBox, 4);

    QLabel *sizeBoxLabel = new QLabel(tr("&Length"), seqBox);
    sizeBox = new QComboBox(seqBox);
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    names << "1" << "2" << "3" << "4" << "5" << "6" << "7" << "8";
    sizeBox->insertItems(0, names);
    sizeBox->setCurrentIndex(3);
    sizeBoxIndex = 3;
    sizeBox->setToolTip(tr("Length of Sequence in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));
    midiControl->addMidiLearnMenu("Size", sizeBox, 5);


    velocity = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("Veloc&ity"), seqBox);
    connect(velocity, SIGNAL(valueChanged(int)), this,
            SLOT(updateVelocity(int)));
    midiControl->addMidiLearnMenu("Velocity", velocity, 1);


    notelength = new Slider(0, 127, 1, 16, 64, Qt::Horizontal,
            tr("N&ote Length"), seqBox);
    connect(notelength, SIGNAL(valueChanged(int)), this,
            SLOT(updateNoteLength(int)));
    midiControl->addMidiLearnMenu("NoteLength", notelength, 2);

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
    paramBoxLayout->addWidget(loopBox, 0, 0, 1, 2);
    paramBoxLayout->addWidget(muteOut, 1, 0, 1, 2);
    paramBoxLayout->addWidget(recordButtonLabel, 2, 0);
    paramBoxLayout->addWidget(recordButton, 2, 1);
    paramBoxLayout->addWidget(resBoxLabel, 3, 0);
    paramBoxLayout->addWidget(resBox, 3, 1);
    paramBoxLayout->addWidget(sizeBoxLabel, 4, 0);
    paramBoxLayout->addWidget(sizeBox, 4, 1);
    paramBoxLayout->setRowStretch(5, 1);

    QGridLayout* seqBoxLayout = new QGridLayout;
    seqBoxLayout->addWidget(screen, 0, 0, 1, 2);
    seqBoxLayout->addWidget(cursor, 1, 0, 1, 2);
    seqBoxLayout->addLayout(paramBoxLayout, 2, 0);
    seqBoxLayout->addLayout(sliderLayout, 2, 1);
    if (compactStyle) {
        seqBoxLayout->setMargin(2);
        seqBoxLayout->setSpacing(0);
    }
    seqBox->setLayout(seqBoxLayout);

    muteOut->setChecked(mutedAdd);

    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(seqBox, 1);
    widgetLayout->addWidget(hideInOutBoxButton, 0);
    widgetLayout->addWidget(inOutBox, 0);

    setLayout(widgetLayout);
    recordMode = false;
    dataChanged=false;
    needsGUIUpdate=false;
    updateVelocity(64);
    updateWaveForm(0);
    lastMute = false;
}

SeqWidget::~SeqWidget()
{
    delete parStore;
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
    xml.writeAttribute("inOutVisible", QString::number(inOutBox->isVisible()));

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
            xml.writeTextElement("trigLegato", QString::number(
                midiWorker->trigLegato));
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
            xml.writeTextElement("loopmode", QString::number(
                loopBox->currentIndex()));
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
        xml.writeStartElement("sequence");
            xml.writeTextElement("data", tempArray.toHex());
            xml.writeTextElement("loopmarker", QString::number(
                getLoopMarker()));
        xml.writeEndElement();

        midiControl->writeData(xml);

        parStore->writeData(xml);

    xml.writeEndElement();
}

void SeqWidget::readData(QXmlStreamReader& xml)
{
    int tmp;
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
                else if (xml.name() == "trigLegato")
                    enableTrigLegato->setChecked(xml.readElementText().toInt());
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
                if (xml.name() == "loopmode") {
                    tmp = xml.readElementText().toInt();
                    loopBox->setCurrentIndex(tmp);
                    updateLoop(tmp);
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
        else if (xml.isStartElement() && (xml.name() == "sequence")) {
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
                    updateWaveForm(0);
                }
                else if (xml.name() == "loopmarker") {
                    tmp = xml.readElementText().toInt();
                    midiWorker->setLoopMarker(tmp);
                    screen->setLoopMarker(tmp);
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
    modified = false;
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

void SeqWidget::updateTrigLegato(bool on)
{
    midiWorker->trigLegato = on;
    modified = true;
}

void SeqWidget::updateNoteLength(int val)
{
    midiWorker->notelength = val + val;
    modified = true;
}

void SeqWidget::updateWaveForm(int val)
{
    (void)val;
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void SeqWidget::setRecord(bool on)
{
    recordMode = on;
    midiWorker->setRecordMode(on);
    screen->setRecordMode(on);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
}

void SeqWidget::updateRes(int val)
{
    if (val > 4) return;
    resBoxIndex = val;
    midiWorker->res = seqResValues[val];
    midiWorker->resizeAll();
    midiWorker->getData(&data);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
    screen->updateData(data);
    modified = true;
}

void SeqWidget::updateSize(int val)
{
    if (val > 7) return;
    sizeBoxIndex = val;
    midiWorker->size = val + 1;
    midiWorker->resizeAll();
    midiWorker->getData(&data);
    screen->setCurrentRecStep(midiWorker->currentRecStep);
    screen->updateData(data);
    modified = true;
}

void SeqWidget::updateLoop(int val)
{
    if (val > 5) return;
    midiWorker->updateLoop(val);
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
        if (enableNoteIn->isChecked()) updateTranspose(note - 60);
        if (enableVelIn->isChecked()) updateVelocity(vel);
        needsGUIUpdate = true;
    }
    else {
        screen->setCurrentRecStep(midiWorker->currentRecStep);
        dataChanged=true;
    }
}

void SeqWidget::mouseMoved(double mouseX, double mouseY, int buttons)
{
    if ((mouseX > 1) || (mouseX < 0) || (mouseY > 1)) return;
    if (buttons == 2) {
        midiWorker->setMutePoint(mouseX, lastMute);
    }
    else {
        midiWorker->setCustomWavePoint(mouseX, mouseY);
        screen->setCurrentRecStep(midiWorker->currentRecStep);
    }
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void SeqWidget::mousePressed(double mouseX, double mouseY, int buttons)
{
    if (mouseY < 0) {
        if (mouseX < 0) mouseX = 0;
        if (buttons == 2) mouseX = - mouseX;
        midiWorker->setLoopMarkerMouse(mouseX);
        screen->setLoopMarker(midiWorker->loopMarker);
        modified = true;
        return;
    }

    if ((mouseX > 1) || (mouseX < 0) || (mouseY > 1)) return;

    if (buttons == 2) {
        lastMute = midiWorker->toggleMutePoint(mouseX);
    } else {
        midiWorker->setCustomWavePoint(mouseX, mouseY);
        screen->setCurrentRecStep(midiWorker->currentRecStep);
    }
    midiWorker->getData(&data);
    screen->updateData(data);
    modified = true;
}

void SeqWidget::setInOutBoxVisible(bool on)
{
    inOutBox->setVisible(on);
    modified=true;
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

void SeqWidget::storeParams(int ix, bool empty)
{
    parStore->temp.empty = empty;
    parStore->temp.muteOut = muteOut->isChecked();
    parStore->temp.chIn = chIn->currentIndex();
    parStore->temp.channelOut = channelOut->currentIndex();
    parStore->temp.portOut = portOut->currentIndex();
    parStore->temp.res = resBox->currentIndex();
    parStore->temp.size = sizeBox->currentIndex();
    parStore->temp.loopMode = loopBox->currentIndex();
    parStore->temp.notelen = notelength->value();
    parStore->temp.transp = transpose->value();
    parStore->temp.vel = velocity->value();
    parStore->temp.dispVertical = dispVertical;
    parStore->temp.loopMode = loopBox->currentIndex();
    parStore->temp.wave = getCustomWave().mid(0, midiWorker->maxNPoints);
    parStore->temp.muteMask = midiWorker->muteMask.mid(0, midiWorker->maxNPoints);
    parStore->temp.loopMarker = getLoopMarker();

    parStore->tempToList(ix);
}

void SeqWidget::restoreParams(int ix)
{
    if (parStore->list.at(ix).empty) return;
    for (int l1 = 0; l1 < parStore->list.at(ix).wave.count(); l1++) {
        midiWorker->customWave.replace(l1, parStore->list.at(ix).wave.at(l1));
        midiWorker->muteMask.replace(l1, parStore->list.at(ix).muteMask.at(l1));
    }
    sizeBox->setCurrentIndex(parStore->list.at(ix).size);
    midiWorker->size = sizeBox->currentText().toInt();
    midiWorker->res = seqResValues[parStore->list.at(ix).res];
    midiWorker->resizeAll();
    midiWorker->setLoopMarker(parStore->list.at(ix).loopMarker);
    screen->setLoopMarker(parStore->list.at(ix).loopMarker);

    resBox->setCurrentIndex(parStore->list.at(ix).res);
    loopBox->setCurrentIndex(parStore->list.at(ix).loopMode);
    notelength->setValue(parStore->list.at(ix).notelen);
    transpose->setValue(parStore->list.at(ix).transp);
    velocity->setValue(parStore->list.at(ix).vel);
    setDispVert(parStore->list.at(ix).dispVertical);

    muteOut->setChecked(parStore->list.at(ix).muteOut);
    chIn->setCurrentIndex(parStore->list.at(ix).chIn);
    updateChIn(parStore->list.at(ix).chIn);
    channelOut->setCurrentIndex(parStore->list.at(ix).channelOut);
    updateChannelOut(parStore->list.at(ix).channelOut);
    setPortOut(parStore->list.at(ix).portOut);
    updatePortOut(parStore->list.at(ix).portOut);
    updateLoop(parStore->list.at(ix).loopMode);

    updateWaveForm(parStore->list.at(ix).waveForm);
    midiWorker->setCurrentIndex(0);
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

    tmp = fromWidget->resBox->currentIndex();
    resBox->setCurrentIndex(tmp);
    updateRes(tmp);
    tmp = fromWidget->sizeBox->currentIndex();
    sizeBox->setCurrentIndex(tmp);
    updateSize(tmp);
    tmp = fromWidget->loopBox->currentIndex();
    loopBox->setCurrentIndex(tmp);
    updateLoop(tmp);

    tmp = fromWidget->velocity->value();
    updateVelocity(tmp);
    velocity->setValue(tmp);
    tmp = fromWidget->transpose->value();
    updateTranspose(tmp);
    transpose->setValue(tmp);

    notelength->setValue(fromWidget->notelength->value());
    for (int l1 = 0; l1 < fromWidget->getMidiWorker()->maxNPoints; l1++) {
        midiWorker->customWave.replace(l1, fromWidget->getCustomWave().at(l1));
        midiWorker->muteMask.replace(l1, midiWorker->customWave.at(l1).muted);
    }
    tmp = fromWidget->getLoopMarker();
    midiWorker->setLoopMarker(tmp);
    screen->setLoopMarker(tmp);
    midiControl->setCcList(fromWidget->midiControl->ccList);
    muteOut->setChecked(true);
    updateWaveForm(0);
}

QVector<Sample> SeqWidget::getCustomWave()
{
    return midiWorker->customWave;
}

void SeqWidget::handleController(int ccnumber, int channel, int value)
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
                        midiWorker->vel = sval;
                break;

                case 2:
                        sval = min + ((double)value * (max - min) / 127);
                        midiWorker->notelength = sval + sval;
                break;

                case 3: if (min == max) {
                            if (value == max) {
                                m = recordMode;
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
                case 4:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 5) resBoxIndex = sval;
                break;
                case 5:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 8) sizeBoxIndex = sval;
                break;
                case 6:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) midiWorker->curLoopMode = sval;
                break;
                case 7:
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

void SeqWidget::updateDisplay()
{
    QVector<Sample> data;

    parStore->updateDisplay(getCurrentIndex(), midiWorker->reverse);


    if (dataChanged || midiWorker->dataChanged) {
        dataChanged=false;
        midiWorker->dataChanged=false;
        midiWorker->getData(&data);
        screen->updateData(data);
        cursor->updateNumbers(midiWorker->res, midiWorker->size);
    }
    screen->updateDraw();
    cursor->updateDraw();
    midiControl->update();

    if (!needsGUIUpdate) return;

    transpose->setValue(midiWorker->transp);
    notelength->setValue(midiWorker->notelength/2);
    velocity->setValue(midiWorker->vel);
    muteOut->setChecked(midiWorker->isMuted);
    recordAction->setChecked(recordMode);
    resBox->setCurrentIndex(resBoxIndex);
    updateRes(resBoxIndex);
    sizeBox->setCurrentIndex(sizeBoxIndex);
    updateSize(sizeBoxIndex);
    loopBox->setCurrentIndex(midiWorker->curLoopMode);

    needsGUIUpdate = false;
}
