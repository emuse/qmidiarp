/*!
 * @file arpwidget.cpp
 * @brief Implements the ArpWidget GUI class.
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
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>

#include "midiarp.h"
#include "arpwidget.h"
#include "slider.h"
#include "arpscreen.h"
#include "config.h"

#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
#include "pixmaps/editmodeon.xpm"
#include "pixmaps/patternremove.xpm"
#include "pixmaps/patternstore.xpm"
#include "pixmaps/latchmodeon.xpm"



ArpWidget::ArpWidget(MidiArp *p_midiWorker, int portCount, bool compactStyle, QWidget *parent)
: QWidget(parent), midiWorker(p_midiWorker), modified(false)
{
    int l1;

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

    midiCCNames << "MuteToggle" << "PresetSwitch" << "unknown";


    // Management Buttons on the right top
    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

    renameAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."), this);
    renameAction->setToolTip(tr("Rename this Arp"));
    QToolButton *renameButton = new QToolButton(this);
    renameButton->setDefaultAction(renameAction);
    connect(renameAction, SIGNAL(triggered()), this, SLOT(moduleRename()));

    deleteAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."), this);
    deleteAction->setToolTip(tr("Delete this Arp"));
    QToolButton *deleteButton = new QToolButton(this);
    deleteButton->setDefaultAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));

    manageBoxLayout->addStretch();
    manageBoxLayout->addWidget(renameButton);
    manageBoxLayout->addWidget(deleteButton);

    // Input group box on left side
    QGroupBox *inBox = new QGroupBox(tr("Input"), this);

    QLabel *chInLabel = new QLabel(tr("&Channel"), inBox);
    chIn = new QComboBox(inBox);
    for (l1 = 0; l1 < 16; l1++) chIn->addItem(QString::number(l1 + 1));
    chInLabel->setBuddy(chIn);
    connect(chIn, SIGNAL(activated(int)), this, SLOT(updateChIn(int)));

    inputFilterBox = new QGroupBox(tr("Note Filter"));
    indexInLabel = new QLabel(tr("&Note"), inputFilterBox);
    indexIn[0] = new QSpinBox(inputFilterBox);
    indexIn[1] = new QSpinBox(inputFilterBox);
    indexInLabel->setBuddy(indexIn[0]);
    indexIn[0]->setRange(0, 127);
    indexIn[1]->setRange(0, 127);
    indexIn[1]->setValue(127);
    indexIn[0]->setKeyboardTracking(false);
    indexIn[1]->setKeyboardTracking(false);
    connect(indexIn[0], SIGNAL(valueChanged(int)), this,
            SLOT(updateIndexIn(int)));
    connect(indexIn[1], SIGNAL(valueChanged(int)), this,
            SLOT(updateIndexIn(int)));

    rangeInLabel = new QLabel(tr("&Velocity"), inputFilterBox);
    rangeIn[0] = new QSpinBox(inputFilterBox);
    rangeIn[1] = new QSpinBox(inputFilterBox);
    rangeInLabel->setBuddy(rangeIn[0]);
    rangeIn[0]->setRange(0, 127);
    rangeIn[1]->setRange(0, 127);
    rangeIn[1]->setValue(127);
    rangeIn[0]->setKeyboardTracking(false);
    rangeIn[1]->setKeyboardTracking(false);
    connect(rangeIn[0], SIGNAL(valueChanged(int)), this,
            SLOT(updateRangeIn(int)));
    connect(rangeIn[1], SIGNAL(valueChanged(int)), this,
            SLOT(updateRangeIn(int)));


    QGridLayout *inputFilterBoxLayout = new QGridLayout;
    inputFilterBoxLayout->addWidget(indexInLabel, 0, 0);
    inputFilterBoxLayout->addWidget(indexIn[0], 0, 1);
    inputFilterBoxLayout->addWidget(indexIn[1], 0, 2);
    inputFilterBoxLayout->addWidget(rangeInLabel, 1, 0);
    inputFilterBoxLayout->addWidget(rangeIn[0], 1, 1);
    inputFilterBoxLayout->addWidget(rangeIn[1], 1, 2);
    inputFilterBoxLayout->setMargin(2);
    inputFilterBoxLayout->setSpacing(2);
    inputFilterBox->setCheckable(true);
    connect(inputFilterBox, SIGNAL(toggled(bool)), this,
            SLOT(setInputFilterVisible(bool)));
    inputFilterBox->setChecked(false);
    inputFilterBox->setFlat(true);
    inputFilterBox->setLayout(inputFilterBoxLayout);

    QGridLayout *inBoxLayout = new QGridLayout;
    inBoxLayout->addWidget(chInLabel, 0, 0);
    inBoxLayout->addWidget(chIn, 0, 1);
    inBoxLayout->addWidget(inputFilterBox, 1, 0, 1, 2);
    if (compactStyle) {
        inBoxLayout->setMargin(2);
        inBoxLayout->setSpacing(1);
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
    if (compactStyle) {
        portBoxLayout->setMargin(2);
        portBoxLayout->setSpacing(1);
    }
    portBox->setLayout(portBoxLayout);


    // Layout for left/right placements of in/out group boxes
    QVBoxLayout *inOutBoxLayout = new QVBoxLayout();
    inOutBoxLayout->addLayout(manageBoxLayout);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();

    // group box for pattern setup
    QGroupBox *patternBox = new QGroupBox(tr("Pattern"), this);
    QVBoxLayout *patternBoxLayout = new QVBoxLayout;

    textEditButton = new QToolButton(this);
    textEditAction = new QAction(QIcon(editmodeon_xpm),
            tr("&Edit Pattern"), this);
    connect(textEditAction, SIGNAL(toggled(bool)), this,
            SLOT(openTextEditWindow(bool)));
    textEditAction->setCheckable(true);
    textEditButton->setDefaultAction(textEditAction);

    textRemoveButton = new QToolButton(this);
    textRemoveAction = new QAction(QIcon(patternremove_xpm),
            tr("&Remove Pattern"), this);
    connect(textRemoveAction, SIGNAL(triggered()), this,
            SLOT(removeCurrentPattern()));
    textRemoveButton->setDefaultAction(textRemoveAction);
    textRemoveAction->setEnabled(false);

    textStoreButton = new QToolButton(this);
    textStoreAction = new QAction(QIcon(patternstore_xpm),
            tr("&Store Pattern"), this);
    connect(textStoreAction, SIGNAL(triggered()), this,
            SLOT(storeCurrentPattern()));
    textStoreAction->setEnabled(false);
    textStoreButton->setDefaultAction(textStoreAction);

    patternPresetBox = new QComboBox(patternBox);
    loadPatternPresets();
    patternPresetBox->insertItems(0, patternNames);
    patternPresetBox->setCurrentIndex(0);
    patternPresetBox->setToolTip(tr("Pattern preset"));
    patternPresetBox->setMinimumContentsLength(20);
    connect(patternPresetBox, SIGNAL(activated(int)), this,
            SLOT(selectPatternPreset(int)));
    patternPresetBox->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));

    QAction *presetSwitchLearnAction = new QAction(tr("MIDI &Learn"), this);
    patternPresetBox->addAction(presetSwitchLearnAction);
    connect(presetSwitchLearnAction, SIGNAL(triggered()), learnSignalMapper, SLOT(map()));
    learnSignalMapper->setMapping(presetSwitchLearnAction, 1);

    QAction *presetSwitchForgetAction = new QAction(tr("MIDI &Forget"), this);
    patternPresetBox->addAction(presetSwitchForgetAction);
    connect(presetSwitchForgetAction, SIGNAL(triggered()), forgetSignalMapper, SLOT(map()));
    forgetSignalMapper->setMapping(presetSwitchForgetAction, 1);

    patternPresetBox->addAction(cancelMidiLearnAction);

    repeatPatternThroughChord = new QComboBox(patternBox);
    QStringList repeatPatternNames;
    repeatPatternNames << tr("Static") << tr("Up") << tr("Down");
    repeatPatternThroughChord->insertItems(0, repeatPatternNames);
    repeatPatternThroughChord->setToolTip(tr("Repeat mode"));
    connect(repeatPatternThroughChord, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateRepeatPattern(int)));
    repeatPatternThroughChord->setCurrentIndex(1);

    triggerMode = new QComboBox(patternBox);
    QStringList triggerModeNames;
    triggerModeNames << tr("No trigger") << tr("Kbd restart") << tr("Kbd trigger");
    triggerMode->insertItems(0, triggerModeNames);
    triggerMode->setToolTip(tr("Trigger Mode"));
    connect(triggerMode, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateTriggerMode(int)));
    triggerMode->setCurrentIndex(0);

    latchModeButton = new QToolButton(this);
    latchModeAction = new QAction(QIcon(latchmodeon_xpm),
            tr("&Latch Mode"), this);
    connect(latchModeAction, SIGNAL(toggled(bool)), this,
            SLOT(setLatchMode(bool)));
    latchModeAction->setCheckable(true);
    latchModeButton->setDefaultAction(latchModeAction);

    QHBoxLayout *patternPresetLayout = new QHBoxLayout;
    if (compactStyle) {
        patternPresetLayout->setMargin(2);
        patternPresetLayout->setSpacing(1);
    }
    patternPresetLayout->addWidget(patternPresetBox);
    patternPresetLayout->addWidget(textEditButton);
    patternPresetLayout->addWidget(textStoreButton);
    patternPresetLayout->addWidget(textRemoveButton);
    patternPresetLayout->addStretch(2);

    QHBoxLayout *modeLayout = new QHBoxLayout;
    if (compactStyle) {
        modeLayout->setMargin(2);
        modeLayout->setSpacing(1);
    }

    modeLayout->addWidget(repeatPatternThroughChord);
    modeLayout->addWidget(triggerMode);
    modeLayout->addWidget(latchModeButton);
    modeLayout->addStretch(2);

    patternText = new QLineEdit(patternBox);
    connect(patternText, SIGNAL(textChanged(const QString&)), this,
            SLOT(updateText(const QString&)));
    patternText->setHidden(true);
    patternText->setToolTip(
            tr("0..9  note played on keyboard, 0 is lowest\n"
            "( ) numbers in parenthesis are stacked to chords\n"
            "  + = -  octave up/reset/down\n"
            " < . > tempo up/reset/down\n"
            "  d h  note length up/down\n"
            "  / \\  velocity up/down\n"
            "   p   pause"));


    QWidget *screenBox = new QWidget(patternBox);
    QHBoxLayout *screenBoxLayout = new QHBoxLayout;
    screen = new ArpScreen(patternBox);
    screenBox->setMinimumHeight(80);
    screenBoxLayout->addWidget(screen);
    screenBoxLayout->setMargin(2);
    screenBoxLayout->setSpacing(1);
    screenBox->setLayout(screenBoxLayout);

    patternBoxLayout->addWidget(screenBox);
    patternBoxLayout->addLayout(patternPresetLayout);
    patternBoxLayout->addWidget(patternText);
    patternBoxLayout->addLayout(modeLayout);
    if (compactStyle) {
        patternBoxLayout->setMargin(2);
        patternBoxLayout->setSpacing(1);
    }
    patternBox->setLayout(patternBoxLayout);

    // group box for random settings
    randomBox = new QGroupBox(tr("Random"), this);
    QVBoxLayout *randomBoxLayout = new QVBoxLayout;

    randomTick = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("&Shift"), randomBox);
    connect(randomTick, SIGNAL(valueChanged(int)), this,
            SLOT(updateRandomTickAmp(int)));

    randomVelocity = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("Vel&ocity"), randomBox);
    connect(randomVelocity, SIGNAL(valueChanged(int)), this,
            SLOT(updateRandomVelocityAmp(int)));

    randomLength = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("&Length"), randomBox);
    connect(randomLength, SIGNAL(valueChanged(int)), this,
            SLOT(updateRandomLengthAmp(int)));

    randomBoxLayout->addWidget(randomTick);
    randomBoxLayout->addWidget(randomVelocity);
    randomBoxLayout->addWidget(randomLength);
    randomBoxLayout->addStretch();
    if (compactStyle) {
        randomBoxLayout->setSpacing(1);
        randomBoxLayout->setMargin(2);
    }
    randomBox->setCheckable(true);
    connect(randomBox, SIGNAL(toggled(bool)), this,
            SLOT(setRandomVisible(bool)));
    randomBox->setChecked(false);
    randomBox->setFlat(true);
    randomBox->setLayout(randomBoxLayout);

    envelopeBox = new QGroupBox(tr("Envelope"), this);
    QVBoxLayout *envelopeBoxLayout = new QVBoxLayout;
    attackTime = new Slider(0, 20, 1, 1, 0, Qt::Horizontal,
            tr("&Attack (s)"), envelopeBox);
    connect(attackTime, SIGNAL(valueChanged(int)), this,
            SLOT(updateAttackTime(int)));
    releaseTime = new Slider(0, 20, 1, 1, 0, Qt::Horizontal,
            tr("&Release (s)"), envelopeBox);
    connect(releaseTime, SIGNAL(valueChanged(int)), this,
            SLOT(updateReleaseTime(int)));

    envelopeBoxLayout->addWidget(attackTime);
    envelopeBoxLayout->addWidget(releaseTime);
    envelopeBoxLayout->addStretch();
    if (compactStyle) {
        envelopeBoxLayout->setSpacing(1);
        envelopeBoxLayout->setMargin(2);
    }
    envelopeBox->setCheckable(true);
    connect(envelopeBox, SIGNAL(toggled(bool)), this,
            SLOT(setEnvelopeVisible(bool)));
    envelopeBox->setChecked(false);
    envelopeBox->setFlat(true);
    envelopeBox->setLayout(envelopeBoxLayout);

    QGridLayout *widgetLayout = new QGridLayout;
    widgetLayout->addWidget(patternBox, 0, 0);
    widgetLayout->addWidget(randomBox, 1, 0);
    widgetLayout->addWidget(envelopeBox, 2, 0);
    widgetLayout->addLayout(inOutBoxLayout, 0, 1, 3, 1);
    widgetLayout->setRowStretch(3, 1);
    widgetLayout->setColumnStretch(0, 5);
    setLayout(widgetLayout);
    ccList.clear();
}

ArpWidget::~ArpWidget()
{
}

MidiArp *ArpWidget::getMidiWorker()
{
    return (midiWorker);
}

void ArpWidget::updateChIn(int value)
{
    midiWorker->chIn = value;
}

void ArpWidget::updateIndexIn(int value)
{
    if (indexIn[0] == sender()) {
        midiWorker->indexIn[0] = value;
    } else {
        midiWorker->indexIn[1] = value;
    }
    checkIfInputFilterSet();
}

void ArpWidget::updateRangeIn(int value)
{
    if (rangeIn[0] == sender()) {
        midiWorker->rangeIn[0] = value;
    } else {
        midiWorker->rangeIn[1] = value;
    }
    checkIfInputFilterSet();
}

void ArpWidget::checkIfInputFilterSet()
{
    if (((indexIn[1]->value() - indexIn[0]->value()) < 127)
            || ((rangeIn[1]->value() - rangeIn[0]->value()) < 127)) {
        inputFilterBox->setFlat(false);
        inputFilterBox->setTitle(tr("Note Filter - ACTIVE"));
    }
    else {
        inputFilterBox->setFlat(true);
        inputFilterBox->setTitle(tr("Note Filter"));
    }
}

void ArpWidget::writeData(QXmlStreamWriter& xml)
{
    xml.writeStartElement(name.left(3));
    xml.writeAttribute("name", name.mid(name.indexOf(':') + 1));
        xml.writeStartElement("pattern");
            xml.writeTextElement("pattern", midiWorker->pattern);
            xml.writeTextElement("repeatMode", QString::number(
                midiWorker->repeatPatternThroughChord));
            xml.writeTextElement("triggerMode", QString::number(
                triggerMode->currentIndex()));
            xml.writeTextElement("latchMode", QString::number(
                latchModeAction->isChecked()));
        xml.writeEndElement();

        xml.writeStartElement("input");
            xml.writeTextElement("channel", QString::number(
                midiWorker->chIn));
            xml.writeTextElement("indexMin", QString::number(
                midiWorker->indexIn[0]));
            xml.writeTextElement("indexMax", QString::number(
                midiWorker->indexIn[1]));
            xml.writeTextElement("rangeMin", QString::number(
                midiWorker->rangeIn[0]));
            xml.writeTextElement("rangeMax", QString::number(
                midiWorker->rangeIn[1]));
        xml.writeEndElement();

        xml.writeStartElement("output");
            xml.writeTextElement("port", QString::number(
                midiWorker->portOut));
            xml.writeTextElement("channel", QString::number(
                midiWorker->channelOut));
        xml.writeEndElement();

        xml.writeStartElement("random");
            xml.writeTextElement("tick", QString::number(
                midiWorker->randomTickAmp));
            xml.writeTextElement("velocity", QString::number(
                midiWorker->randomVelocityAmp));
            xml.writeTextElement("length", QString::number(
                midiWorker->randomLengthAmp));
        xml.writeEndElement();

        xml.writeStartElement("envelope");
            xml.writeTextElement("attack", QString::number(
                attackTime->value()));
            xml.writeTextElement("release", QString::number(
                releaseTime->value()));
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

void ArpWidget::readData(QXmlStreamReader& xml)
{
    int controlID, ccnumber, channel, min, max;
    int tmp;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        if (xml.isStartElement() && (xml.name() == "pattern")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "pattern")
                    patternText->setText(xml.readElementText());
                else if (xml.name() == "repeatMode")
                    repeatPatternThroughChord->setCurrentIndex(xml.readElementText().toInt());
                else if (xml.name() == "triggerMode")
                    triggerMode->setCurrentIndex(xml.readElementText().toInt());
                else if (xml.name() == "latchMode")
                    latchModeAction->setChecked(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }

        else if (xml.isStartElement() && (xml.name() == "input")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "channel") {
                    tmp = xml.readElementText().toInt();
                    chIn->setCurrentIndex(tmp);
                    updateChIn(tmp);
                }
                else if (xml.name() == "indexMin")
                    indexIn[0]->setValue(xml.readElementText().toInt());
                else if (xml.name() == "indexMax")
                    indexIn[1]->setValue(xml.readElementText().toInt());
                else if (xml.name() == "rangeMin")
                    rangeIn[0]->setValue(xml.readElementText().toInt());
                else if (xml.name() == "rangeMax")
                    rangeIn[1]->setValue(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "output")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "channel") {
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
        else if (xml.isStartElement() && (xml.name() == "random")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "tick")
                    randomTick->setValue(xml.readElementText().toInt());
                else if (xml.name() == "velocity")
                    randomVelocity->setValue(xml.readElementText().toInt());
                else if (xml.name() == "length")
                    randomLength->setValue(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "envelope")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "attack")
                    attackTime->setValue(xml.readElementText().toInt());
                else if (xml.name() == "release")
                    releaseTime->setValue(xml.readElementText().toInt());
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
    modified = false;
}

void ArpWidget::skipXmlElement(QXmlStreamReader& xml)
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

void ArpWidget::readDataText(QTextStream& arpText)
{
    QString qs, qs2;
    MidiCC midiCC;
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    chIn->setCurrentIndex(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    repeatPatternThroughChord->setCurrentIndex(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    indexIn[0]->setValue(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    indexIn[1]->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    rangeIn[0]->setValue(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    rangeIn[1]->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    channelOut->setCurrentIndex(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    portOut->setCurrentIndex(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    randomTick->setValue(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    randomVelocity->setValue(qs2.toInt());
    qs2 = qs.section(' ', 2, 2);
    randomLength->setValue(qs2.toInt());
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
    if (qs == "Envelope")
    {
        qs = arpText.readLine();
        qs2 = qs.section(' ', 0, 0);
        attackTime->setValue(qs2.toInt());
        qs2 = qs.section(' ', 1, 1);
        releaseTime->setValue(qs2.toInt());
        qs = arpText.readLine();
    }
    while (!arpText.atEnd()) {
        qs2 = arpText.readLine();

        if (qs2.contains("EOP", Qt::CaseSensitivity(TRUE))) {
            break;
        }
        qs += '\n' + qs2;
    }
    patternText->setText(qs);
    modified = false;
}

void ArpWidget::setChIn(int value)
{
    chIn->setCurrentIndex(value);
    modified = true;
}

void ArpWidget::setIndexIn(int index, int value)
{
    indexIn[index]->setValue(value);
    modified = true;
}

void ArpWidget::setRangeIn(int index, int value)
{
    rangeIn[index]->setValue(value);
    modified = true;
}

void ArpWidget::updateText(const QString& newtext)
{
    patternPresetBox->setCurrentIndex(0);
    textRemoveAction->setEnabled(false);
    textStoreAction->setEnabled(true);
    screen->updateScreen(newtext);
    midiWorker->updatePattern(newtext);
    modified = true;
}

void ArpWidget::selectPatternPreset(int val)
{
    if (val < patternPresets.count()) {
        if (val) {
            patternText->setText(patternPresets.at(val));
            patternPresetBox->setCurrentIndex(val);
            textStoreAction->setEnabled(false);
            textRemoveAction->setEnabled(true);
        } else
            textRemoveAction->setEnabled(false);
        modified = true;
    }
}

void ArpWidget::loadPatternPresets()
{
    QString qs;
    QStringList value;

    QDir qmahome = QDir(QDir::homePath());
    QString qmarcpath = qmahome.filePath(QMARCNAME);
    QFile f(qmarcpath);

    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, PACKAGE,
                tr("Could not read from resource file"));
        return;
    }
    QTextStream loadText(&f);
    patternNames.clear();
    patternPresets.clear();

    while (!loadText.atEnd()) {
        qs = loadText.readLine();
        if (qs.startsWith('#')) {
            value.clear();
            value = qs.split('%');
            if ((value.at(0) == "#Pattern") && (value.count() > 2)) {
                patternNames << value.at(1);
                patternPresets << value.at(2);
            }
        }
    }
}

void ArpWidget::updateRepeatPattern(int val)
{
    midiWorker->repeatPatternThroughChord = val;
    modified = true;
}

void ArpWidget::updateTriggerMode(int val)
{
    midiWorker->updateTriggerMode(val);
    modified = true;
}

void ArpWidget::updateRandomLengthAmp(int val)
{
    midiWorker->updateRandomLengthAmp(val);
    checkIfRandomSet();
    modified = true;
}

void ArpWidget::updateRandomTickAmp(int val)
{
    midiWorker->updateRandomTickAmp(val);
    checkIfRandomSet();
    modified = true;
}

void ArpWidget::updateRandomVelocityAmp(int val)
{
    midiWorker->updateRandomVelocityAmp(val);
    checkIfRandomSet();
    modified = true;
}

void ArpWidget::checkIfRandomSet()
{
    if (randomLength->value() || randomTick->value()
                || randomVelocity->value()) {
        randomBox->setFlat(false);
        randomBox->setTitle(tr("Random - ACTIVE"));
    }
    else {
        randomBox->setFlat(true);
        randomBox->setTitle(tr("Random"));
    }
}

void ArpWidget::updateAttackTime(int val)
{
    midiWorker->updateAttackTime(val);
    checkIfEnvelopeSet();
    modified = true;
}

void ArpWidget::updateReleaseTime(int val)
{
    midiWorker->updateReleaseTime(val);
    checkIfEnvelopeSet();
    modified = true;
}

void ArpWidget::checkIfEnvelopeSet()
{
    if (attackTime->value() || releaseTime->value()) {
        envelopeBox->setFlat(false);
        envelopeBox->setTitle(tr("Envelope - ACTIVE"));
    }
    else {
        envelopeBox->setFlat(true);
        envelopeBox->setTitle(tr("Envelope"));
    }
}

void ArpWidget::openTextEditWindow(bool on)
{
    patternText->setHidden(!on);
}

void ArpWidget::storeCurrentPattern()
{
    QString qs;
    bool ok;

    qs = QInputDialog::getText(this, tr("%1: Store pattern").arg(PACKAGE),
            tr("New pattern"), QLineEdit::Normal, tr("Arp pattern"), &ok);

    if (ok && !qs.isEmpty()) {

        emit presetsChanged(qs, patternText->text(), 0);
        patternPresetBox->setCurrentIndex(patternNames.count() - 1);
        textRemoveAction->setEnabled(true);
    }
}

void ArpWidget::updatePatternPresets(const QString& n, const QString& p, int index)
{
    if (index) {
       if (index == patternPresetBox->currentIndex()) {
            patternPresetBox->setCurrentIndex(0);
            textRemoveAction->setEnabled(false);
        }
        patternNames.removeAt(index);
        patternPresets.removeAt(index);
        patternPresetBox->removeItem(index);
    } else {
        patternNames.append(n);
        patternPresets.append(p);
        patternPresetBox->addItem(n);
    }
}

void ArpWidget::removeCurrentPattern()
{
    QString qs;

    int currentIndex = patternPresetBox->currentIndex();
    if (currentIndex < 1) {
        return;
    }

    qs = tr("Remove \"%1\"?").arg(patternPresetBox->currentText());

    if (QMessageBox::question(0, PACKAGE, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }

    emit presetsChanged("", "", currentIndex);
}

void ArpWidget::setInputFilterVisible(bool on)
{
    rangeIn[0]->setVisible(on);
    rangeIn[1]->setVisible(on);
    rangeInLabel->setVisible(on);
    indexIn[0]->setVisible(on);
    indexIn[1]->setVisible(on);
    indexInLabel->setVisible(on);
}

void ArpWidget::setRandomVisible(bool on)
{
    randomTick->setVisible(on);
    randomVelocity->setVisible(on);
    randomLength->setVisible(on);
}

void ArpWidget::setEnvelopeVisible(bool on)
{
    attackTime->setVisible(on);
    releaseTime->setVisible(on);
}

void ArpWidget::setMuted(bool on)
{
    midiWorker->setMuted(on);
    screen->setMuted(on);
}

void ArpWidget::setLatchMode(bool on)
{
    midiWorker->setLatchMode(on);
    modified = true;
}

void ArpWidget::setPortOut(int value)
{
    portOut->setCurrentIndex(value);
    modified = true;
}

void ArpWidget::setChannelOut(int value)
{
    channelOut->setCurrentIndex(value);
    modified = true;
}

void ArpWidget::updatePortOut(int value)
{
    midiWorker->portOut = value;
    modified = true;
}

void ArpWidget::updateChannelOut(int value)
{
    midiWorker->channelOut = value;
    modified = true;
}

bool ArpWidget::isModified()
{
    return modified;
}

void ArpWidget::setModified(bool m)
{
    modified = m;
}

void ArpWidget::moduleDelete()
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

void ArpWidget::moduleRename()
{
    QString newname, oldname;
    bool ok;

    oldname = name;

    newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);

    if (ok && !newname.isEmpty()) {
        name = "Arp:" + newname;
        emit dockRename(name, parentDockID);
    }
}

void ArpWidget::appendMidiCC(int controlID, int ccnumber, int channel, int min, int max)
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

void ArpWidget::removeMidiCC(int controlID, int ccnumber, int channel)
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

void ArpWidget::midiLearn(int controlID)
{
    emit setMidiLearn(parentDockID, ID, controlID);
    qWarning("Requesting Midi Learn for %s", qPrintable(midiCCNames.at(controlID)));
    cancelMidiLearnAction->setEnabled(true);
}

void ArpWidget::midiForget(int controlID)
{
    removeMidiCC(controlID, 0, -1);
}

void ArpWidget::midiLearnCancel()
{
    emit setMidiLearn(parentDockID, ID, -1);
    qWarning("Cancelling Midi Learn request");
    cancelMidiLearnAction->setEnabled(false);
}
