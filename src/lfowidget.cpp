/*
 *      lfowidget.cpp
 *      
 *      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
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
#include "pixmaps/lfowavcp.xpm"
#include "pixmaps/lfowsine.xpm"
#include "pixmaps/lfowsawup.xpm"
#include "pixmaps/lfowsawdn.xpm"
#include "pixmaps/lfowtri.xpm"
#include "pixmaps/lfowsquare.xpm"
#include "pixmaps/lfowcustm.xpm"
#include "config.h"


LfoWidget::LfoWidget(MidiLfo *p_midiLfo, int portCount, bool compactStyle, QWidget *parent):
    QWidget(parent), midiLfo(p_midiLfo), modified(false)
{
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

    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);

    cancelMidiLearnAction = new QAction(tr("Cancel MIDI &Learning"), this);
    connect(cancelMidiLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnCancel()));
    cancelMidiLearnAction->setEnabled(false);

    muteOut->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    
    QAction *muteLearnAction = new QAction(tr("MIDI &Learn"), this);
    muteOut->addAction(muteLearnAction);
    connect(muteLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnMute()));
    QAction *muteForgetAction = new QAction(tr("MIDI &Forget"), this);
    muteOut->addAction(muteForgetAction);
    connect(muteForgetAction, SIGNAL(triggered()), this, SLOT(midiForgetMute()));

    muteOut->addAction(cancelMidiLearnAction);

    
    connect(muteOut, SIGNAL(toggled(bool)), midiLfo, SLOT(muteLfo(bool)));
    muteLabel->setBuddy(muteOut);

    QLabel *ccnumberLabel = new QLabel(tr("&MIDI CC#"), portBox);
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

    portBox->setLayout(outputLayout);

    // group box for wave setup
    QGroupBox *waveBox = new QGroupBox(tr("Wave"), this);

    lfoScreen = new LfoScreen(this); 
    lfoScreen->setToolTip(
        tr("Right button to mute points\nLeft button to draw custom wave\nWheel to change offset"));
    lfoScreen->setMinimumHeight(80);
    connect(lfoScreen, SIGNAL(lfoMouseMoved(double, double, int)), this,
            SLOT(mouseMoved(double, double, int)));
    connect(lfoScreen, SIGNAL(lfoMousePressed(double, double, int)), this,
            SLOT(mousePressed(double, double, int)));
    connect(lfoScreen, SIGNAL(lfoWheel(int)), this,
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
    //waveFormBox->setMinimumContentsLength(8);
    connect(waveFormBox, SIGNAL(activated(int)), this,
            SLOT(updateWaveForm(int)));

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
    
    copyToCustomButton = new QToolButton(this); 
    copyToCustomAction = new QAction( QIcon(lfowavcp_xpm),
            tr("&Copy to custom wave"), this);
    connect(copyToCustomAction, SIGNAL(triggered()), this,
            SLOT(copyToCustom()));
    copyToCustomButton->setDefaultAction(copyToCustomAction);
    copyToCustomAction->setEnabled(true);

    amplitude = new Slider(0, 127, 1, 8, 64, Qt::Horizontal,
            tr("&Amplitude"), waveBox);

    amplitude->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    
    QAction *amplitudeLearnAction = new QAction(tr("MIDI &Learn"), this);
    amplitude->addAction(amplitudeLearnAction);
    connect(amplitudeLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnAmp()));
    QAction *amplitudeForgetAction = new QAction(tr("MIDI &Forget"), this);
    amplitude->addAction(amplitudeForgetAction);
    connect(amplitudeForgetAction, SIGNAL(triggered()), this, SLOT(midiForgetAmp()));
    
    amplitude->addAction(cancelMidiLearnAction);

    connect(amplitude, SIGNAL(valueChanged(int)), this,
            SLOT(updateAmp(int)));

    offset = new Slider(0, 127, 1, 8, 0, Qt::Horizontal,
            tr("&Offset"), waveBox);

    offset->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
    
    QAction *offsetLearnAction = new QAction(tr("MIDI &Learn"), this);
    offset->addAction(offsetLearnAction);
    connect(offsetLearnAction, SIGNAL(triggered()), this, SLOT(midiLearnOffs()));
    QAction *offsetForgetAction = new QAction(tr("MIDI &Forget"), this);
    offset->addAction(offsetForgetAction);
    connect(offsetForgetAction, SIGNAL(triggered()), this, SLOT(midiForgetOffs()));
    
    offset->addAction(cancelMidiLearnAction);

    connect(offset, SIGNAL(valueChanged(int)), this,
            SLOT(updateOffs(int)));
    
    QVBoxLayout* sliderLayout = new QVBoxLayout;
    sliderLayout->addWidget(copyToCustomButton);
    sliderLayout->addWidget(amplitude);
    sliderLayout->addWidget(offset);
    sliderLayout->addStretch();
    if (compactStyle) {
        sliderLayout->setSpacing(0);
        sliderLayout->setMargin(1);
    }

    QGridLayout *paramBoxLayout = new QGridLayout;
    paramBoxLayout->addWidget(waveFormBoxLabel, 0, 0);
    paramBoxLayout->addWidget(waveFormBox, 0, 1);
    paramBoxLayout->addWidget(freqBoxLabel, 1, 0);
    paramBoxLayout->addWidget(freqBox, 1, 1);
    paramBoxLayout->addWidget(resBoxLabel, 2, 0);
    paramBoxLayout->addWidget(resBox, 2, 1);
    paramBoxLayout->addWidget(sizeBoxLabel, 3, 0);
    paramBoxLayout->addWidget(sizeBox, 3, 1);
    paramBoxLayout->setRowStretch(4, 1);
    
    if (compactStyle) {
        paramBoxLayout->setSpacing(0);
        paramBoxLayout->setMargin(1);
    }
           
    QGridLayout* waveBoxLayout = new QGridLayout;
    waveBoxLayout->addWidget(lfoScreen, 0, 0, 1, 2);
    waveBoxLayout->addLayout(paramBoxLayout, 1, 0);
    waveBoxLayout->addLayout(sliderLayout, 1, 1);
    waveBox->setLayout(waveBoxLayout);
    
    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
    inOutBoxLayout->addLayout(manageBoxLayout);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();

    QHBoxLayout *lfoWidgetLayout = new QHBoxLayout;
    lfoWidgetLayout->addWidget(waveBox, 1);
    lfoWidgetLayout->addLayout(inOutBoxLayout, 0);

    setLayout(lfoWidgetLayout);
    updateAmp(64);
    
    ccList.clear();
}

LfoWidget::~LfoWidget()
{
}

MidiLfo *LfoWidget::getMidiLfo()
{
    return (midiLfo);
}

void LfoWidget::writeLfo(QXmlStreamWriter& xml)
{
    QByteArray tempArray;
    int l1;
    
    xml.writeStartElement(name.left(3));
    xml.writeAttribute("name", name.mid(name.indexOf(':') + 1));
        xml.writeStartElement("output");
            xml.writeTextElement("port", QString::number(
                midiLfo->portOut));
            xml.writeTextElement("channel", QString::number(
                midiLfo->channelOut));
            xml.writeTextElement("ccnumber", QString::number(
                midiLfo->ccnumber));
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
                midiLfo->amp));
            xml.writeTextElement("offset", QString::number(
                midiLfo->offs));
        xml.writeEndElement();
      
        tempArray.clear();
        l1 = 0;
        while (l1 < midiLfo->muteMask.count()) {
            tempArray.append(midiLfo->muteMask.at(l1));
            l1++;
        }
        xml.writeStartElement("muteMask");
            xml.writeTextElement("data", tempArray.toHex());
        xml.writeEndElement();
        
        tempArray.clear();
        l1 = 0;
        while (l1 < midiLfo->muteMask.count()) {
            tempArray.append(midiLfo->customWave.at(l1).value);
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

void LfoWidget::writeLfoText(QTextStream& arpText)
{
    int l1 = 0;
    arpText << midiLfo->channelOut << ' ' 
        << midiLfo->portOut << ' '
        << midiLfo->ccnumber << '\n';
    arpText << freqBox->currentIndex() << ' '
        << resBox->currentIndex() << ' '
        << sizeBox->currentIndex() << ' '
        << midiLfo->amp << ' '
        << midiLfo->offs << '\n';
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
    while (l1 < midiLfo->muteMask.count()) {
        arpText << midiLfo->muteMask.at(l1) << ' ';
        l1++;
        if (!(l1 % 32)) arpText << "\n";
    }
    arpText << "EOM\n"; // End Of Mute
    // Write Custom Waveform
    l1 = 0;
    while (l1 < midiLfo->customWave.count()) {
        arpText << midiLfo->customWave.at(l1).value << ' ';
        l1++;
        if (!(l1 % 16)) arpText << "\n";
    }
    arpText << "EOW\n"; // End Of Wave
    modified = false;
}
                                  
void LfoWidget::readLfo(QXmlStreamReader& xml)
{
    int ctrlID, ccnumber, channel, min, max;
    int wvtmp = 0;
    LfoSample lfoSample;
    
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
            
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
                else if (xml.name() == "frequency")
                    freqBox->setCurrentIndex(xml.readElementText().toInt());
                else if (xml.name() == "resolution") {
                    int tmp = xml.readElementText().toInt();
                    resBox->setCurrentIndex(tmp);
                    updateRes(tmp);
                }
                else if (xml.name() == "size")
                    sizeBox->setCurrentIndex(xml.readElementText().toInt());
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
                    midiLfo->muteMask.clear();
                    QByteArray tmpArray = 
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        midiLfo->muteMask.append(tmpArray.at(l1));
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
                    midiLfo->customWave.clear();
                    QByteArray tmpArray = 
                            QByteArray::fromHex(xml.readElementText().toLatin1());
                    int step = TICKS_PER_QUARTER / midiLfo->res;
                    int lt = 0;
                    for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                        lfoSample.value = tmpArray.at(l1);
                        lfoSample.tick = lt;
                        lfoSample.muted = midiLfo->muteMask.at(l1);
                        midiLfo->customWave.append(lfoSample);
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
                    ctrlID = xml.attributes().value("CtrlID").toString().toInt();
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
                        appendMidiCC(ctrlID, ccnumber, channel, min, max);
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
 
void LfoWidget::readLfoText(QTextStream& arpText)
{
    QString qs, qs2;
    int l1, lt, wvtmp;
    LfoSample lfoSample;
    
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
            int ctrlID = qs2.toInt();
            qs2 = qs.section(' ', 1, 1);
            int ccnumber = qs2.toInt();
            qs2 = qs.section(' ', 2, 2);
            int channel = qs2.toInt();
            qs2 = qs.section(' ', 3, 3);
            int min = qs2.toInt();
            qs2 = qs.section(' ', 4, 4);
            int max = qs2.toInt();
            appendMidiCC(ctrlID, ccnumber, channel, min, max);
            qs = arpText.readLine();
        }
    qs = arpText.readLine();
    }

    wvtmp = qs.toInt();
    
    // Read Mute Mask
    int step = TICKS_PER_QUARTER / midiLfo->res;
    qs = arpText.readLine();
    if (qs.isEmpty() || (qs == "EOP")) return;
    qs2 = qs.section(' ', 0, 0);
    midiLfo->muteMask.clear();
    l1 = 0;
    while (qs2 !="EOM") {
        // TODO: the following line produces a pointer deref warning, why?
        midiLfo->muteMask.append(qs2.toInt());
        l1++;
        if (!(l1%32)) qs = arpText.readLine();
        qs2 = qs.section(' ', l1%32, l1%32);
    }
    
    // Read Custom Waveform
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0);
    midiLfo->customWave.clear();
    l1 = 0;
    lt = 0;
    while (qs2 !="EOW") {
        lfoSample.value=qs2.toInt();
        lfoSample.tick = lt;
        lfoSample.muted = midiLfo->muteMask.at(l1);
        midiLfo->customWave.append(lfoSample);
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
    midiLfo->portOut = value - 1;
    modified = true;
}

void LfoWidget::updateChannelOut(int value)
{
    midiLfo->channelOut = value - 1;
    modified = true;
}

void LfoWidget::updateCcnumber(int val)
{
    midiLfo->ccnumber = val;
    modified = true;
}

void LfoWidget::updateWaveForm(int val)
{
    midiLfo->updateWaveForm(val);
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    bool isCustom = (val == 5);
    if (isCustom) newCustomOffset();
    amplitude->setDisabled(isCustom);
    freqBox->setDisabled(isCustom);
    copyToCustomAction->setDisabled(isCustom);
    modified = true;
}

void LfoWidget::updateFreq(int val)
{
    midiLfo->freq = lfoFreqValues[val];
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateRes(int val)
{
    midiLfo->res = lfoResValues[val];
    midiLfo->resizeAll();
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateSize(int val)
{
    midiLfo->size = val + 1;
    midiLfo->resizeAll();
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateAmp(int val)
{
    midiLfo->amp = val;
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::updateOffs(int val)
{
    if (waveFormBox->currentIndex() == 5) {
        midiLfo->updateCustomWaveOffset(val);
    }
    midiLfo->offs = val;
    midiLfo->getData(&lfoData);
    lfoScreen->updateScreen(lfoData);
    modified = true;
}

void LfoWidget::copyToCustom()
{
    midiLfo->copyToCustom();
    waveFormBox->setCurrentIndex(5);
    updateWaveForm(5);
    modified = true;
}

void LfoWidget::newCustomOffset()
{
    int min = 127;
    int value;
    for (int l1 = 0; l1 < lfoData.count() - 1; l1++) {
        value = lfoData.at(l1).value;
        if (value < min) min = value;
    }
    midiLfo->cwmin = min;
    offset->setValue(min);
}

void LfoWidget::mouseMoved(double mouseX, double mouseY, int buttons)
{
    if ((buttons == 1) && (waveFormBox->currentIndex() == 5)) {
        midiLfo->setCustomWavePoint(mouseX, mouseY);
        midiLfo->getData(&lfoData);
        lfoScreen->updateScreen(lfoData);
        newCustomOffset();
        modified = true;
    }
}

void LfoWidget::mousePressed(double mouseX, double mouseY, int buttons)
{
    if (buttons == 2) {
        midiLfo->toggleMutePoint(mouseX);
        midiLfo->getData(&lfoData);
        lfoScreen->updateScreen(lfoData);
        modified = true;
    } 
    else {
        if (waveFormBox->currentIndex() == 5) {
            midiLfo->setCustomWavePoint(mouseX, mouseY);
            midiLfo->getData(&lfoData);
            lfoScreen->updateScreen(lfoData);
            newCustomOffset();
            modified = true;
        }
    }
}

void LfoWidget::mouseWheel(int step)
{
    int cv;
    cv = offset->value() + step;
    if ((cv < 127) && (cv > 0))
    offset->setValue(cv + step);
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
    emit lfoRemove(ID);
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

void LfoWidget::appendMidiCC(int ctrlID, int ccnumber, int channel, int min, int max)
{
    MidiCC midiCC;
    int l1 = 0;
    switch (ctrlID) {
        case 0: midiCC.name = "MuteToggle";
        break;
        case 1: midiCC.name = "Amplitude";
        break;
        case 2: midiCC.name = "Offset";
        break;
        default: midiCC.name = "Unknown";
    }
    midiCC.ID = ctrlID;
    midiCC.ccnumber = ccnumber;
    midiCC.channel = channel;
    midiCC.min = min;
    midiCC.max = max;
    
    while ( (l1 < ccList.count()) && 
        ((ctrlID != ccList.at(l1).ID) || 
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

void LfoWidget::removeMidiCC(int ctrlID, int ccnumber, int channel)
{
    for (int l1 = 0; l1 < ccList.count(); l1++) {
        if (ccList.at(l1).ID == ctrlID) {
            if (((ccList.at(l1).ccnumber == ccnumber)
                    && (ccList.at(l1).channel == channel)) 
                    || (0 > ccnumber)) {
                ccList.remove(l1);
                l1--;
                qWarning("controller removed");
            }
        }
    }
    modified = true;
}

void LfoWidget::midiLearnMute()
{
    emit setMidiLearn(parentDockID, ID, 0);
    qWarning("Requesting Midi Learn for MuteToggle");
    cancelMidiLearnAction->setEnabled(true);
}

void LfoWidget::midiForgetMute()
{
    removeMidiCC(0, 0, -1);
}

void LfoWidget::midiLearnOffs()
{
    emit setMidiLearn(parentDockID, ID, 2);
    qWarning("Requesting Midi Learn for Offset");
    cancelMidiLearnAction->setEnabled(true);
}

void LfoWidget::midiForgetOffs()
{
    removeMidiCC(2, 0, -1);
}

void LfoWidget::midiLearnAmp()
{
    emit setMidiLearn(parentDockID, ID, 1);
    qWarning("Requesting Midi Learn for Amplitude");
    cancelMidiLearnAction->setEnabled(true);
}

void LfoWidget::midiForgetAmp()
{
    removeMidiCC(1, 0, -1);
}

void LfoWidget::midiLearnCancel()
{
    emit setMidiLearn(parentDockID, ID, -1);
    qWarning("Cancelling Midi Learn request");
    cancelMidiLearnAction->setEnabled(false);
}
