/*!
 * @file lfowidget.cpp
 * @brief Implements the LfoWidget GUI class.
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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
#include <cstdio>
#include <vector>

#include "lfowidget.h"

#include "pixmaps/lfowsine.xpm"
#include "pixmaps/lfowsawup.xpm"
#include "pixmaps/lfowsawdn.xpm"
#include "pixmaps/lfowtri.xpm"
#include "pixmaps/lfowsquare.xpm"
#include "pixmaps/lfowcustm.xpm"
#include "pixmaps/lfowflip.xpm"
#include "pixmaps/seqrecord.xpm"
#include "config.h"


#ifdef APPBUILD
LfoWidget::LfoWidget(MidiLfo *p_midiLfo, GlobStore *p_globStore,
    Prefs *p_prefs, bool inOutVisible, const QString& p_name):
    InOutBox(p_midiLfo, p_globStore, p_prefs, inOutVisible, p_name),
    midiLfo(p_midiLfo)
{
    bool compactStyle = p_prefs->compactStyle;
#else
LfoWidget::LfoWidget():
    InOutBox("LFO:"),
    midiLfo(NULL)
{
    bool compactStyle = true;
#endif

    // group box for wave setup
    QGroupBox *waveBox = new QGroupBox(tr("Wave"));

    screen = new LfoScreen(this);
    screen->setToolTip(
        tr("Right button to mute points\nLeft button to draw custom wave\nWheel to change offset"));
    screen->setMinimumHeight(80);

    connect(screen, SIGNAL(mouseEvent(double, double, int, int)), this,
            SLOT(mouseEvent(double, double, int, int)));
    connect(screen, SIGNAL(mouseWheel(int)), this,
            SLOT(mouseWheel(int)));

    cursor = new Cursor('L');

    QLabel *waveFormBoxLabel = new QLabel(tr("&Waveform"));
    waveFormBox = new QComboBox;
    waveFormBoxLabel->setBuddy(waveFormBox);
    //loadWaveForms();
    waveFormBox->addItem(QPixmap(lfowsine_xpm),"");
    waveFormBox->addItem(QPixmap(lfowsawup_xpm),"");
    waveFormBox->addItem(QPixmap(lfowtri_xpm),"");
    waveFormBox->addItem(QPixmap(lfowsawdn_xpm),"");
    waveFormBox->addItem(QPixmap(lfowsquare_xpm),"");
    waveFormBox->addItem(QPixmap(lfowcustm_xpm),"");
    waveFormBox->setCurrentIndex(0);
    waveFormBoxIndex = 0;
    waveFormBox->setToolTip(tr("Waveform Basis"));

    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("WaveForm", waveFormBox, LFO_WAVEFORM);
#endif

    QLabel *freqBoxLabel = new QLabel(tr("&Frequency"));
    freqBox = new QComboBox;
    freqBoxLabel->setBuddy(freqBox);
    QStringList names;
    names << "1/32" << "1/16" << "1/8" << "1/4"
        << "1/2" << "3/4" << "1" << "2" << "3"
        << "4" << "5" << "6" << "7" << "8";
    freqBox->insertItems(0, names);
    freqBoxIndex = 3;
    freqBox->setCurrentIndex(freqBoxIndex);
    freqBox->setToolTip(
            tr("Frequency (cycles/beat): Number of wave cycles produced every beat"));
    freqBox->setMinimumContentsLength(3);
    connect(freqBox, SIGNAL(activated(int)), this,
            SLOT(updateFreq(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Frequency", freqBox, LFO_FREQUENCY);
#endif
    QLabel *resBoxLabel = new QLabel(tr("&Resolution"));
    resBox = new QComboBox;
    resBoxLabel->setBuddy(resBox);
    names.clear();
    for (uint64_t i = 0; i < sizeof(lfoResValues)/sizeof(lfoResValues[0]); i++) {
        names << QString::number(lfoResValues[i]);
    }
    resBox->insertItems(0, names);
    resBoxIndex = 3;
    resBox->setCurrentIndex(resBoxIndex);
    resBox->setToolTip(
            tr("Resolution (events/beat): Number of events produced every beat"));
    resBox->setMinimumContentsLength(3);
    connect(resBox, SIGNAL(activated(int)), this,
            SLOT(updateRes(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Resolution", resBox, LFO_RESOLUTION);
#endif
    QLabel *sizeBoxLabel = new QLabel(tr("&Length"));
    sizeBox = new QComboBox;
    sizeBoxLabel->setBuddy(sizeBox);
    names.clear();
    for (uint64_t i = 0; i < sizeof(lfoSizeValues)/sizeof(lfoSizeValues[0]); i++) {
        names << QString::number(lfoSizeValues[i]);
    }
    sizeBox->insertItems(0, names);
    sizeBoxIndex = 3;
    sizeBox->setCurrentIndex(sizeBoxIndex);
    sizeBox->setToolTip(tr("Length of LFO wave in beats"));
    sizeBox->setMinimumContentsLength(3);
    connect(sizeBox, SIGNAL(activated(int)), this,
            SLOT(updateSize(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Size", sizeBox, LFO_SIZE);
#endif
    loopBox = new QComboBox;
    names.clear();
    names << "->_>" << " <_<-" << "->_<" << " >_<-" << "->_|" << " |_<-" << "RANDM";
    loopBox->insertItems(0, names);
    loopBox->setCurrentIndex(0);
    loopBox->setToolTip(tr("Loop, bounce or play once going forward or backward"));
    loopBox->setMinimumContentsLength(5);
    connect(loopBox, SIGNAL(activated(int)), this,
            SLOT(updateLoop(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("LoopMode", loopBox, LFO_LOOPMODE);
#endif

    flipWaveVerticalAction = new QAction(QPixmap(lfowflip_xpm),tr("&Flip"), this);
    flipWaveVerticalAction->setToolTip(tr("Do a vertical flip of the wave about its mid value"));
    connect(flipWaveVerticalAction, SIGNAL(triggered(bool)), this, SLOT(updateFlipWaveVertical()));

    QToolButton *flipWaveVerticalButton = new QToolButton;
    flipWaveVerticalButton->setDefaultAction(flipWaveVerticalAction);
    flipWaveVerticalButton->setFixedSize(20, 20);

    QLabel *recordButtonLabel = new QLabel(tr("Re&cord"));
    recordAction = new QAction(QPixmap(seqrecord_xpm), tr("Re&cord"), this);
    recordAction->setToolTip(tr("Record incoming controller"));
    recordAction->setCheckable(true);
    QToolButton *recordButton = new QToolButton;
    recordButton->setDefaultAction(recordAction);
    recordButtonLabel->setBuddy(recordButton);
    connect(recordAction, SIGNAL(toggled(bool)), this, SLOT(setRecord(bool)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("RecordToggle", recordButton, LFO_RECORD);
#endif
    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), this);
    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Amplitude", amplitude, LFO_AMPLITUDE);
#endif

    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), this);
    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Offset", offset, LFO_OFFSET);
#endif

    phase = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Phase"), this);
    connect(phase, SIGNAL(valueChanged(int)), this,
            SLOT(updatePhase(int)));
#ifdef APPBUILD
    midiControl->addMidiLearnMenu("Phase", phase, LFO_PHASE);
#endif


    QVBoxLayout* sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(amplitude);
    sliderLayout->addWidget(offset);
    sliderLayout->addWidget(phase);
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
    paramBoxLayout->addWidget(flipWaveVerticalButton, 0, 6);
    paramBoxLayout->setColumnStretch(7, 7);

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


    QHBoxLayout *widgetLayout = new QHBoxLayout;
    widgetLayout->addWidget(waveBox, 1);
    widgetLayout->addWidget(hideInOutBoxButton, 0);
    widgetLayout->addWidget(inOutBoxWidget, 0);

    setLayout(widgetLayout);
    updateAmp(64);
}

#ifdef APPBUILD
MidiLfo *LfoWidget::getMidiWorker()
{
    return (midiLfo);
}

void LfoWidget::writeData(QXmlStreamWriter& xml)
{
    QByteArray tempArray;
    int l1;

        writeCommonData(xml);
    
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
                midiLfo->amp));
            xml.writeTextElement("offset", QString::number(
                midiLfo->offs));
            xml.writeTextElement("phase", QString::number(
                midiLfo->phase));
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiLfo->maxNPoints) {
            tempArray.append(midiLfo->muteMask.at(l1));
            l1++;
        }
        xml.writeStartElement("muteMask");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

        tempArray.clear();
        l1 = 0;
        while (l1 < midiLfo->maxNPoints) {
            tempArray.append(midiLfo->customWave.at(l1).value);
            l1++;
        }
        xml.writeStartElement("customWave");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();

    xml.writeEndElement();
}

void LfoWidget::readData(QXmlStreamReader& xml, const QString& qmaxVersion)
{
    int tmp;
    int wvtmp = 0;
    Sample sample;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
        
        readCommonData(xml);

        if (xml.isStartElement() && (xml.name() == "waveParams")) {
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
                    if (qmaxVersion == "" && tmp < 9) {
                        tmp = mapOldLfoRes[tmp];
                    }
                    resBox->setCurrentIndex(tmp);
                    updateRes(tmp);
                }
                else if (xml.name() == "size") {
                    tmp = xml.readElementText().toInt();
                    if (qmaxVersion == "" && tmp < 12) {
                        tmp = mapOldLfoSize[tmp];
                    }
                    sizeBox->setCurrentIndex(tmp);
                    updateSize(tmp);
                }
                else if (xml.name() == "amplitude")
                    amplitude->setValue(xml.readElementText().toInt());
                else if (xml.name() == "offset")
                    offset->setValue(xml.readElementText().toInt());
                else if (xml.name() == "phase")
                    phase->setValue(xml.readElementText().toInt());
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
                        midiLfo->muteMask[l1] = tmpArray.at(l1);
                    }
                    midiLfo->maxNPoints = tmpArray.count();
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
                    int step = TPQN / midiLfo->res;
                    int lt = 0;
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        sample.value = tmpArray.at(l1);
                        sample.tick = lt;
                        sample.muted = midiLfo->muteMask[l1];
                        midiLfo->customWave[l1] = sample;
                        lt+=step;
                    }
                }
                else skipXmlElement(xml);
            }
        }
        else skipXmlElement(xml);
    }
    
    updateChIn(chIn->currentIndex());
    updateChannelOut(channelOut->currentIndex());
    updatePortOut(portOut->currentIndex());
    waveFormBox->setCurrentIndex(wvtmp);
    updateWaveForm(wvtmp);
    midiLfo->needsGUIUpdate = false;
    modified = false;
}
#endif

void LfoWidget::loadWaveForms()
{
    waveForms << tr("Sine") << tr("Saw up") << tr("Triangle")
        << tr("Saw down") << tr("Square") << tr("Custom");
}

void LfoWidget::updateWaveForm(int val)
{
    if (val > 5) return;
    waveFormBoxIndex = val;
    if (midiLfo) midiLfo->updateWaveForm(val);
    std::vector<Sample> sdata;
    if (midiLfo) midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    if (midiLfo) screen->updateData(data);
    bool isCustom = (val == 5);
    if (isCustom && midiLfo) midiLfo->newCustomOffset();
    amplitude->setDisabled(isCustom);
    freqBox->setDisabled(isCustom);
    phase->setDisabled(isCustom);
    modified = true;

}

void LfoWidget::updateFreq(int val)
{
    if ((uint64_t)val >= sizeof(lfoFreqValues)/sizeof(lfoFreqValues[0])) return;
    freqBoxIndex = val;
    modified = true;
    if (!midiLfo) return;
    midiLfo->updateFrequency(lfoFreqValues[val]);
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
}

void LfoWidget::updateRes(int val)
{
    if ((uint64_t)val >= sizeof(lfoResValues)/sizeof(lfoResValues[0])) return;
    resBoxIndex = val;
    modified = true;
    if (!midiLfo) return;
    midiLfo->updateResolution(lfoResValues[val]);
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
    if (waveFormBoxIndex == 5) midiLfo->newCustomOffset();
}

void LfoWidget::updateSize(int val)
{
    if ((uint64_t)val >= sizeof(lfoSizeValues)/sizeof(lfoSizeValues[0])) return;
    sizeBoxIndex = val;
    modified = true;
    if (!midiLfo) return;
    midiLfo->updateSize(sizeBox->currentText().toInt());
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
    if (waveFormBoxIndex == 5) midiLfo->newCustomOffset();
}

void LfoWidget::updateLoop(int val)
{
    if (val > 6) return;
    if (midiLfo) midiLfo->updateLoop(val);
    modified = true;
}

void LfoWidget::updateAmp(int val)
{
    modified = true;
    if (!midiLfo) return;
    midiLfo->updateAmplitude(val);
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
}

void LfoWidget::updateOffs(int val)
{
    modified = true;
    if (!midiLfo) return;
    midiLfo->updateOffset(val);
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
}

void LfoWidget::updatePhase(int val)
{
    modified = true;
    if (!midiLfo) return;
    midiLfo->updatePhase(val);
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
}

void LfoWidget::copyToCustom()
{
    if (midiLfo) midiLfo->copyToCustom();
    waveFormBox->setCurrentIndex(5);
    updateWaveForm(5);
    modified = true;
}

void LfoWidget::updateFlipWaveVertical()
{
    modified = true;
    if (!midiLfo) return;
    if (waveFormBox->currentIndex() != 5) copyToCustom();
    midiLfo->flipWaveVertical();
    std::vector<Sample> sdata;
    midiLfo->getData(&sdata);
    data=QVector<Sample>::fromStdVector(sdata);
    screen->updateData(data);
}

void LfoWidget::mouseEvent(double mouseX, double mouseY, int buttons, int pressed)
{
    if (!midiLfo) emit mouseSig(mouseX, mouseY, buttons, pressed);
    else midiLfo->mouseEvent(mouseX, mouseY, buttons, pressed);

    if ((buttons == 1) && (waveFormBox->currentIndex() != 5)) {
        waveFormBox->setCurrentIndex(5);
        updateWaveForm(5);
    }
    modified = true;
}

void LfoWidget::mouseWheel(int step)
{
    int cv;
    cv = offset->value() + step;
    if ((cv < 127) && (cv > 0))
    offset->setValue(cv + step);
}

void LfoWidget::setRecord(bool on)
{
    if (midiLfo) midiLfo->setRecordMode(on);
    screen->setRecordMode(on);
}

QVector<Sample> LfoWidget::getCustomWave()
{
    return QVector<Sample>::fromStdVector(midiLfo->customWave);
}

QVector<bool> LfoWidget::getMuteMask()
{
    return QVector<bool>::fromStdVector(midiLfo->muteMask);
}

#ifdef APPBUILD

void LfoWidget::doStoreParams(int ix)
{
    parStore->temp.ccnumberIn = ccnumberInBox->value();
    parStore->temp.ccnumber = ccnumberBox->value();
    parStore->temp.res = resBox->currentIndex();
    parStore->temp.size = sizeBox->currentIndex();
    parStore->temp.loopMode = loopBox->currentIndex();
    parStore->temp.freq = freqBox->currentIndex();
    parStore->temp.ampl = amplitude->value();
    parStore->temp.offs = offset->value();
    parStore->temp.phase = phase->value();
    parStore->temp.waveForm = waveFormBox->currentIndex();

    if (midiLfo) parStore->temp.wave = getCustomWave().mid(0, midiLfo->maxNPoints);
    if (midiLfo) parStore->temp.muteMask = getMuteMask().mid(0, midiLfo->maxNPoints);

    parStore->tempToList(ix);
}

void LfoWidget::doRestoreParams(int ix)
{
    midiLfo->applyPendingParChanges();
    if (parStore->list.at(ix).empty) return;
    for (int l1 = 0; l1 < parStore->list.at(ix).wave.count(); l1++) {
        midiLfo->customWave[l1] = parStore->list.at(ix).wave.at(l1);
        midiLfo->muteMask[l1] = parStore->list.at(ix).muteMask.at(l1);
    }
    sizeBox->setCurrentIndex(parStore->list.at(ix).size);
    midiLfo->updateSize(sizeBox->currentText().toInt());

    midiLfo->updateResolution(lfoResValues[parStore->list.at(ix).res]);
    midiLfo->updateWaveForm(parStore->list.at(ix).waveForm);
    midiLfo->updateFrequency(lfoFreqValues[parStore->list.at(ix).freq]);
    freqBox->setCurrentIndex(parStore->list.at(ix).freq);
    resBox->setCurrentIndex(parStore->list.at(ix).res);
    waveFormBox->setCurrentIndex(parStore->list.at(ix).waveForm);
    loopBox->setCurrentIndex(parStore->list.at(ix).loopMode);
    updateLoop(parStore->list.at(ix).loopMode);
    updateWaveForm(parStore->list.at(ix).waveForm);
    if (!parStore->onlyPatternList.at(ix)) {
        amplitude->setValue(parStore->list.at(ix).ampl);
        offset->setValue(parStore->list.at(ix).offs);
        phase->setValue(parStore->list.at(ix).phase);
        ccnumberInBox->setValue(parStore->list.at(ix).ccnumberIn);
        ccnumberBox->setValue(parStore->list.at(ix).ccnumber);
    }
    int frame = 0;
    //int frame = ( midiLfo->reverse ? midiLfo->nPoints : 0);
    midiLfo->setFramePtr(frame);
}

void LfoWidget::copyParamsFrom(LfoWidget *fromWidget)
{
    int tmp;

    enableNoteOff->setChecked(fromWidget->enableNoteOff->isChecked());
    enableRestartByKbd->setChecked(fromWidget->enableRestartByKbd->isChecked());
    enableTrigByKbd->setChecked(fromWidget->enableTrigByKbd->isChecked());
    enableTrigLegato->setChecked(fromWidget->enableTrigLegato->isChecked());

    for (int l1 = 0; l1 < 1; l1++) {
        tmp = fromWidget->indexIn[l1]->value();
        indexIn[l1]->setValue(tmp);
    }
    for (int l1 = 0; l1 < 1; l1++) {
        tmp = fromWidget->rangeIn[l1]->value();
        rangeIn[l1]->setValue(tmp);
    }

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
    tmp = fromWidget->ccnumberBox->value();
    ccnumberBox->setValue(tmp);

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
    phase->setValue(fromWidget->phase->value());

    for (int l1 = 0; l1 < fromWidget->getMidiWorker()->maxNPoints; l1++) {
        midiLfo->customWave[l1] = fromWidget->getCustomWave().at(l1);
        midiLfo->muteMask[l1] = midiLfo->customWave.at(l1).muted;
    }
    midiControl->setCcList(fromWidget->midiControl->ccList);
    muteOutAction->setChecked(true);

    tmp = fromWidget->waveFormBox->currentIndex();
    waveFormBox->setCurrentIndex(tmp);
    updateWaveForm(tmp);
}

void LfoWidget::handleController(int ccnumber, int channel, int value)
{
    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        int min = cclist.at(l2).min;
        int max = cclist.at(l2).max;
        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            int sval = 0;
            bool m = false;
            switch (cclist.at(l2).ID) {
                case MUTE_BUTTON: if (min == max) {
                            if (value == max) {
                                m = midiLfo->isMuted;
                                midiLfo->setMuted(!m);
                            }
                        }
                        else {
                            if (value == max) {
                                midiLfo->setMuted(false);
                            }
                            if (value == min) {
                                midiLfo->setMuted(true);
                            }
                        }
                break;

                case LFO_AMPLITUDE:
                        sval = min + ((double)value * (max - min) / 127);
                        midiLfo->updateAmplitude(sval);
                break;

                case LFO_OFFSET:
                        sval = min + ((double)value * (max - min) / 127);
                        midiLfo->updateOffset(sval);
                break;
                case LFO_WAVEFORM:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) waveFormBoxIndex = sval;
                break;
                case LFO_FREQUENCY:
                        sval = min + ((double)value * (max - min) / 127);
                        if ((uint64_t)sval < sizeof(lfoFreqValues)/sizeof(lfoFreqValues[0])) freqBoxIndex = sval;
                break;
                case LFO_RECORD: if (min == max) {
                            if (value == max) {
                                m = midiLfo->recordMode;
                                midiLfo->setRecordMode(!m);
                                return;
                            }
                        }
                        else {
                            if (value == max) {
                                midiLfo->setRecordMode(true);
                            }
                            if (value == min) {
                                midiLfo->setRecordMode(false);
                            }
                        }
                break;
                case LFO_RESOLUTION:
                        sval = min + ((double)value * (max - min) / 127);
                        if ((uint64_t)sval < sizeof(lfoResValues)/sizeof(lfoResValues[0])) resBoxIndex = sval;
                break;
                case LFO_SIZE:
                        sval = min + ((double)value * (max - min) / 127);
                        if ((uint64_t)sval < sizeof(lfoSizeValues)/sizeof(lfoSizeValues[0])) sizeBoxIndex = sval;
                break;
                case LFO_LOOPMODE:
                        sval = min + ((double)value * (max - min) / 127);
                        if (sval < 6) midiLfo->curLoopMode = sval;
                break;
                case PARAM_RESTORE:
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
                case LFO_PHASE:
                        sval = min + ((double)value * (max - min) / 127);
                        midiLfo->updatePhase(sval);
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
    std::vector<Sample> sdata;

    bool repetitionsFinished = (midiLfo->currentRepetition == 0);
    if (midiLfo->reverse) {
    repetitionsFinished = (midiLfo->currentRepetition >= midiLfo->nRepetitions - 1);
    }
    parStore->updateDisplay(getFramePtr(), 
        midiLfo->nPoints, repetitionsFinished, midiLfo->reverse);
    if (parStore->nRepList.count() > 0) {
        if (parStore->nRepList.at(parStore->activeStore) != midiLfo->nRepetitions) {
            updateNRep(parStore->nRepList.at(parStore->activeStore));
        }
    }
    if (midiLfo->dataChanged) {
        midiLfo->getData(&sdata);
        data = QVector<Sample>::fromStdVector(sdata);
        screen->updateData(data);
        cursor->updateNumbers(midiLfo->res, midiLfo->size);
        offset->setValue(midiLfo->offs);
        phase->setValue(midiLfo->phase);
        midiLfo->dataChanged = false;
    }
    screen->updateDraw();
    cursor->updateDraw();
    midiControl->update();

    if (!(needsGUIUpdate || midiLfo->needsGUIUpdate)) return;

    muteOut->setChecked(midiLfo->isMuted);
    screen->newGrooveValues(midiLfo->newGrooveTick, midiLfo->grooveVelocity,
                midiLfo->grooveLength);   
    screen->setMuted(midiLfo->isMuted);
    parStore->ndc->setMuted(midiLfo->isMuted);
    recordAction->setChecked(midiLfo->recordMode);
    screen->setRecordMode(midiLfo->recordMode);
    resBox->setCurrentIndex(resBoxIndex);
    updateRes(resBoxIndex);
    sizeBox->setCurrentIndex(sizeBoxIndex);
    updateSize(sizeBoxIndex);
    freqBox->setCurrentIndex(freqBoxIndex);
    updateFreq(freqBoxIndex);
    loopBox->setCurrentIndex(midiLfo->curLoopMode);
    amplitude->setValue(midiLfo->amp);
    offset->setValue(midiLfo->offs);
    phase->setValue(midiLfo->phase);
    if (waveFormBoxIndex != waveFormBox->currentIndex()) {
        waveFormBox->setCurrentIndex(waveFormBoxIndex);
        updateWaveForm(waveFormBoxIndex);
    }
    needsGUIUpdate = false;
    midiLfo->needsGUIUpdate = false;
}

#endif
