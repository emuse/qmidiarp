/*!
 * @file arpwidget.cpp
 * @brief Implements the ArpWidget GUI class.
 *
 *
 *      Copyright 2009 - 2016 <qmidiarp-devel@lists.sourceforge.net>
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

#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDir>

#include "midiarp.h"
#include "arpwidget.h"
#include "slider.h"
#include "arpscreen.h"
#include "config.h"

#include "pixmaps/editmodeon.xpm"
#include "pixmaps/latchmodeon.xpm"
#ifdef APPBUILD
#include "pixmaps/patternremove.xpm"
#include "pixmaps/patternstore.xpm"
#endif



#ifdef APPBUILD
ArpWidget::ArpWidget(MidiArp *p_midiArp, GlobStore *p_globStore,
    int portCount, bool compactStyle,
    bool mutedAdd, bool inOutVisible, const QString& p_name):
    InOutBox(p_midiArp, p_globStore, portCount, compactStyle, inOutVisible, p_name),
    midiArp(p_midiArp)
{
#else
ArpWidget::ArpWidget(
    bool compactStyle,
    bool mutedAdd, bool inOutVisible):
    InOutBox(compactStyle, inOutVisible, "Arp:"),
    midiArp(NULL)
{
#endif

    // group box for pattern setup
    QGroupBox *patternBox = new QGroupBox(tr("Pattern"));
    QVBoxLayout *patternBoxLayout = new QVBoxLayout;

    textEditButton = new QToolButton;
    textEditAction = new QAction(QPixmap(editmodeon_xpm),
            tr("&Edit Pattern"), this);
    connect(textEditAction, SIGNAL(toggled(bool)), this,
            SLOT(openTextEditWindow(bool)));
    textEditAction->setCheckable(true);
    textEditButton->setDefaultAction(textEditAction);

#ifdef APPBUILD
    textRemoveButton = new QToolButton;
    textRemoveAction = new QAction(QPixmap(patternremove_xpm),
            tr("&Remove Pattern"), this);
    connect(textRemoveAction, SIGNAL(triggered()), this,
            SLOT(removeCurrentPattern()));
    textRemoveButton->setDefaultAction(textRemoveAction);
    textRemoveAction->setEnabled(false);

    textStoreButton = new QToolButton;
    textStoreAction = new QAction(QPixmap(patternstore_xpm),
            tr("&Store Pattern"), this);
    connect(textStoreAction, SIGNAL(triggered()), this,
            SLOT(storeCurrentPattern()));
    textStoreAction->setEnabled(false);
    textStoreButton->setDefaultAction(textStoreAction);
#endif

    patternPresetBox = new QComboBox;
    loadPatternPresets();
    patternPresetBox->insertItems(0, patternNames);
    patternPresetBox->setCurrentIndex(0);
    patternPresetBoxIndex = 0;
    patternPresetBox->setToolTip(tr("Pattern preset"));
    patternPresetBox->setMinimumContentsLength(20);
    connect(patternPresetBox, SIGNAL(activated(int)), this,
            SLOT(selectPatternPreset(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("PresetSwitch", patternPresetBox, 1);
#endif

    repeatPatternThroughChord = new QComboBox;
    QStringList repeatPatternNames;
    repeatPatternNames << tr("Static") << tr("Up") << tr("Down") << tr("Random");
    repeatPatternThroughChord->insertItems(0, repeatPatternNames);
    repeatPatternThroughChord->setToolTip(tr("Repeat mode - This is how notes are sequenced\n"
                            "when a chord is pressed"));
    connect(repeatPatternThroughChord, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateRepeatPattern(int)));
    repeatPatternThroughChord->setCurrentIndex(1);

    octaveModeBox = new QComboBox;
    repeatPatternNames.clear();
    repeatPatternNames << tr("Static") << tr("Up") << tr("Down") << tr("Bounce");
    octaveModeBox->insertItems(0, repeatPatternNames);
    octaveModeBox->setToolTip(tr("Octave mode - The overall octave changes like this\n"
                            "once all pressed notes were played through"));
    connect(octaveModeBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateOctaveMode(int)));
    octaveModeBox->setCurrentIndex(0);

    octaveLowBox = new QComboBox;
    repeatPatternNames.clear();
    repeatPatternNames << "0" << "-1" << "-2" << "-3";
    octaveLowBox->insertItems(0, repeatPatternNames);
    octaveLowBox->setToolTip(tr("Low octave limit"));
    connect(octaveLowBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateOctaveLow(int)));
    octaveLowBox->setCurrentIndex(0);

    octaveHighBox = new QComboBox;
    repeatPatternNames.clear();
    repeatPatternNames << "0" << "1" << "2" << "3";
    octaveHighBox->insertItems(0, repeatPatternNames);
    octaveHighBox->setToolTip(tr("High octave limit"));
    connect(octaveHighBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(updateOctaveHigh(int)));
    octaveHighBox->setCurrentIndex(0);

    latchModeButton = new QToolButton;
    latchModeAction = new QAction(QPixmap(latchmodeon_xpm),
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
#ifdef APPBUILD
    patternPresetLayout->addWidget(textStoreButton);
    patternPresetLayout->addWidget(textRemoveButton);
#endif
    patternPresetLayout->addStretch(2);

    QHBoxLayout *modeLayout = new QHBoxLayout;
    if (compactStyle) {
        modeLayout->setMargin(2);
        modeLayout->setSpacing(1);
    }

    modeLayout->addWidget(muteOut);
    modeLayout->addWidget(deferChangesButton);
    modeLayout->addWidget(repeatPatternThroughChord);
    modeLayout->addWidget(octaveModeBox);
    modeLayout->addWidget(octaveLowBox);
    modeLayout->addWidget(octaveHighBox);
    modeLayout->addWidget(latchModeButton);
    modeLayout->addStretch(2);

    patternText = new QLineEdit;
    connect(patternText, SIGNAL(textChanged(const QString&)), this,
            SLOT(updateText(const QString&)));
    patternText->setHidden(true);
    patternText->setToolTip(
            tr("0..9  note played on keyboard, 0 is lowest\n"
            "( ) numbers in parenthesis are stacked to chords\n"
            "  + = -  octave up/reset/down\n"
            "  t = g  semitone up/reset/down\n"
            " < . > tempo up/reset/down\n"
            "  d h  note length up/down\n"
            "  / \\  velocity up/down\n"
            "   p   pause"));


    QWidget *screenBox = new QWidget;
    QHBoxLayout *screenBoxLayout = new QHBoxLayout;
    screen = new ArpScreen(this);
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
    randomBox = new QGroupBox(tr("Random"));
    QVBoxLayout *randomBoxLayout = new QVBoxLayout;

    randomTick = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("&Shift"), this);
    connect(randomTick, SIGNAL(valueChanged(int)), this,
            SLOT(updateRandomTickAmp(int)));

    randomVelocity = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("Vel&ocity"), this);
    connect(randomVelocity, SIGNAL(valueChanged(int)), this,
            SLOT(updateRandomVelocityAmp(int)));

    randomLength = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("&Length"), this);
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

    envelopeBox = new QGroupBox(tr("Envelope"));
    QVBoxLayout *envelopeBoxLayout = new QVBoxLayout;
    attackTime = new Slider(0, 20, 1, 1, 0, Qt::Horizontal,
            tr("&Attack (beats)"), this);
    connect(attackTime, SIGNAL(valueChanged(int)), this,
            SLOT(updateAttackTime(int)));
    releaseTime = new Slider(0, 20, 1, 1, 0, Qt::Horizontal,
            tr("&Release (beats)"), this);
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


    muteOutAction->setChecked(mutedAdd);

    QGridLayout *widgetLayout = new QGridLayout;
    widgetLayout->addWidget(patternBox, 0, 0);
    widgetLayout->addWidget(randomBox, 1, 0);
    widgetLayout->addWidget(envelopeBox, 2, 0);
    widgetLayout->addWidget(hideInOutBoxButton, 0, 1);
    widgetLayout->addWidget(inOutBoxWidget, 0, 2, 3, 1);
    widgetLayout->setRowStretch(3, 1);
    widgetLayout->setColumnStretch(0, 5);
    setLayout(widgetLayout);
    modified = false;
}


#ifdef APPBUILD
MidiArp *ArpWidget::getMidiWorker()
{
    return (midiArp);
}

void ArpWidget::writeData(QXmlStreamWriter& xml)
{
        writeCommonData(xml);

        xml.writeStartElement("pattern");
            xml.writeTextElement("pattern", QString::fromStdString(midiArp->pattern));
            xml.writeTextElement("repeatMode", QString::number(
                midiArp->repeatPatternThroughChord));
            xml.writeTextElement("octaveMode", QString::number(
                midiArp->octMode));
            xml.writeTextElement("octaveLow", QString::number(
                midiArp->octLow));
            xml.writeTextElement("octaveHigh", QString::number(
                midiArp->octHigh));
            xml.writeTextElement("latchMode", QString::number(
                latchModeAction->isChecked()));
        xml.writeEndElement();

        xml.writeStartElement("random");
            xml.writeTextElement("tick", QString::number(
                midiArp->randomTickAmp));
            xml.writeTextElement("velocity", QString::number(
                midiArp->randomVelocityAmp));
            xml.writeTextElement("length", QString::number(
                midiArp->randomLengthAmp));
        xml.writeEndElement();

        xml.writeStartElement("envelope");
            xml.writeTextElement("attack", QString::number(
                attackTime->value()));
            xml.writeTextElement("release", QString::number(
                releaseTime->value()));
        xml.writeEndElement();


    xml.writeEndElement();
}

void ArpWidget::readData(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        readCommonData(xml);

        if (xml.isStartElement() && (xml.name() == "pattern")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "pattern")
                    patternText->setText(xml.readElementText());
                else if (xml.name() == "repeatMode")
                    repeatPatternThroughChord->setCurrentIndex(xml.readElementText().toInt());
                else if (xml.name() == "octaveMode")
                    octaveModeBox->setCurrentIndex(xml.readElementText().toInt());
                else if (xml.name() == "octaveLow")
                    octaveLowBox->setCurrentIndex(-xml.readElementText().toInt());
                else if (xml.name() == "octaveHigh")
                    octaveHighBox->setCurrentIndex(xml.readElementText().toInt());
                else if (xml.name() == "latchMode")
                    latchModeAction->setChecked(xml.readElementText().toInt());
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
        else skipXmlElement(xml);
    }

    updateChIn(chIn->currentIndex());
    updateChannelOut(channelOut->currentIndex());
    updatePortOut(portOut->currentIndex());
    midiArp->needsGUIUpdate = false;
    modified = false;
}
#endif


void ArpWidget::updateText(const QString& newtext)
{
    patternPresetBox->setCurrentIndex(0);
    if (!midiArp) return;
    textRemoveAction->setEnabled(false);
    textStoreAction->setEnabled(true);
    midiArp->updatePattern(newtext.toStdString());
    screen->updateData(newtext, midiArp->minOctave,
                    midiArp->maxOctave, midiArp->minStepWidth,
                    midiArp->nSteps, midiArp->patternMaxIndex);

    modified = true;
}

void ArpWidget::selectPatternPreset(int val)
{
    if (val < patternPresets.count()) {
        if (val) {
            patternText->setText(patternPresets.at(val));
            if (!midiArp) return;
            patternPresetBox->setCurrentIndex(val);
            textStoreAction->setEnabled(false);
            textRemoveAction->setEnabled(true);
        }
        else {
            if (!midiArp) return;
            textRemoveAction->setEnabled(false);
        }
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
                tr("Could not read the pattern presets from the \n"
                   ".qmidiarprc resource file. To create this file \n"
                    "please just run the qmidiarp main application once."));
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
    if (midiArp) midiArp->repeatPatternThroughChord = val;
    modified = true;
}

void ArpWidget::updateOctaveMode(int val)
{
    if (midiArp) midiArp->updateOctaveMode(val);
    modified = true;
}

void ArpWidget::updateOctaveLow(int val)
{
    if (midiArp) midiArp->octLow = -val;
    modified = true;
}

void ArpWidget::updateOctaveHigh(int val)
{
    if (midiArp) midiArp->octHigh = val;
    modified = true;
}

void ArpWidget::updateRandomLengthAmp(int val)
{
    if (midiArp) midiArp->updateRandomLengthAmp(val);
    checkIfRandomSet();
    modified = true;
}

void ArpWidget::updateRandomTickAmp(int val)
{
    if (midiArp) midiArp->updateRandomTickAmp(val);
    checkIfRandomSet();
    modified = true;
}

void ArpWidget::updateRandomVelocityAmp(int val)
{
    if (midiArp) midiArp->updateRandomVelocityAmp(val);
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
    if (midiArp) midiArp->updateAttackTime(val);
    checkIfEnvelopeSet();
    modified = true;
}

void ArpWidget::updateReleaseTime(int val)
{
    if (midiArp) midiArp->updateReleaseTime(val);
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

void ArpWidget::setLatchMode(bool on)
{
    if (midiArp) midiArp->setLatchMode(on);
    modified = true;
}

#ifdef APPBUILD

void ArpWidget::doStoreParams(int ix)
{
    parStore->temp.attack = attackTime->value();
    parStore->temp.release = releaseTime->value();
    parStore->temp.rndTick = randomTick->value();
    parStore->temp.rndLen = randomLength->value();
    parStore->temp.rndVel = randomVelocity->value();
    parStore->temp.pattern = patternText->text();
    parStore->temp.repeatMode = repeatPatternThroughChord->currentIndex();
    parStore->tempToList(ix);
}

void ArpWidget::doRestoreParams(int ix)
{
    midiArp->applyPendingParChanges();
    if (parStore->list.at(ix).empty) return;
    patternText->setText(parStore->list.at(ix).pattern);
    repeatPatternThroughChord->setCurrentIndex(parStore->list.at(ix).repeatMode);
    updateRepeatPattern(parStore->list.at(ix).repeatMode);
    if (!parStore->onlyPatternList.at(ix)) {
        attackTime->setValue(parStore->list.at(ix).attack);
        releaseTime->setValue(parStore->list.at(ix).release);
        randomTick->setValue(parStore->list.at(ix).rndTick);
        randomLength->setValue(parStore->list.at(ix).rndLen);
        randomVelocity->setValue(parStore->list.at(ix).rndVel);
    }
    midiArp->advancePatternIndex(true);
}

void ArpWidget::handleController(int ccnumber, int channel, int value)
{
    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        int min = cclist.at(l2).min;
        int max = cclist.at(l2).max;

        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            int sval = 0;
            switch (cclist.at(l2).ID) {
                case 0: if (min == max) {
                            if (value == max) {
                                bool m = midiArp->isMuted;
                                midiArp->setMuted(!m);
                            }
                        }
                        else {
                            if (value == max) {
                                midiArp->setMuted(false);
                            }
                            if (value == min) {
                                midiArp->setMuted(true);
                            }
                        }
                break;
                case 1:
                        sval = min + ((double)value * (max - min) / 127);
                        patternPresetBoxIndex = sval;
                break;
                case 2:
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


void ArpWidget::updateDisplay()
{
    parStore->updateDisplay(getFramePtr() , false);

    screen->updateDraw();
    midiControl->update();

    if (!(needsGUIUpdate || midiArp->needsGUIUpdate)) return;

    muteOut->setChecked(midiArp->isMuted);
    screen->newGrooveValues(midiArp->newGrooveTick, midiArp->grooveVelocity,
                midiArp->grooveLength);    
    screen->setMuted(midiArp->isMuted);
    parStore->ndc->setMuted(midiArp->isMuted);
    if (patternPresetBoxIndex != patternPresetBox->currentIndex())
        selectPatternPreset(patternPresetBoxIndex);

    needsGUIUpdate = false;
    midiArp->needsGUIUpdate = false;
}

#endif
