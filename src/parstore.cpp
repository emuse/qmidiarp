/**
 * @file parstore.cpp
 * @brief Implements the ParStore class.
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

#include "main.h"
#include "parstore.h"

ParStore::ParStore(GlobStore *p_globStore, const QString &name, QWidget* parent):
            QWidget(parent), globStore(p_globStore)
{
    // when temp.empty is true, restoring from that set is ignored
    temp.empty = false;
    temp.muteOut = false;
    temp.res = 1;
    temp.size = 0;
    temp.loopMode = 0;
    temp.waveForm = 0;
    temp.portOut = 0;
    temp.channelOut = 0;
    temp.chIn = 0;
    temp.wave.clear();
    temp.muteMask.clear();
    /* LFO Modules */
    temp.ccnumber = -1;
    temp.ccnumberIn = 0;
    temp.freq = 0;
    temp.ampl = 0;
    temp.offs = 0;
    /* Seq Modules */
    temp.loopMarker = 0;
    temp.notelen = 0;
    temp.vel = 0;
    temp.transp = 0;
    temp.dispVertical = 0;
    /* Arp Modules */
    temp.indexIn0 = 0;
    temp.indexIn1 = 0;
    temp.rangeIn0 = 0;
    temp.rangeIn1 = 0;
    temp.attack = 0;
    temp.release = 0;
    temp.repeatMode = 0;
    temp.rndTick = 0;
    temp.rndLen = 0;
    temp.rndVel = 0;
    temp.pattern = "";
    list.clear();

    ndc = new Indicator(14, name.at(0), this);

    topButton = new QToolButton(this);
    topButton->setFont(QFont("Helvetica", 8));
    topButton->setText(name);
    topButton->setMinimumSize(QSize(75, 30));

    QWidget *indicatorBox = new QWidget(this);
    QHBoxLayout *indicatorLayout = new QHBoxLayout;
    indicatorBox->setMinimumHeight(20);
    indicatorBox->setMinimumWidth(25);
    indicatorLayout->addWidget(ndc);
    indicatorLayout->setMargin(2);
    indicatorLayout->setSpacing(1);
    indicatorBox->setLayout(indicatorLayout);

    QWidget *topRow = new QWidget(this);
    QHBoxLayout *topRowLayout = new QHBoxLayout;
    topRowLayout->addWidget(indicatorBox);
    topRowLayout->addWidget(topButton);
    topRowLayout->setSpacing(0);
    topRowLayout->setMargin(0);
    topRow->setLayout(topRowLayout);

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->addWidget(topRow);


    for (int l1 = 0; l1 < list.size() - 1; l1++) {
        QToolButton *toolButton = new QToolButton(this);
        toolButton->setText(QString::number(l1 + 1));
        toolButton->setFixedSize(QSize(100, 25));
        toolButton->setProperty("index", l1 + 1);
        connect(toolButton, SIGNAL(pressed()), globStore, SLOT(mapRestoreSignal()));

        toolButton->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
        QAction *storeHereAction = new QAction(tr("&Store here"), this);
        storeHereAction->setProperty("index", l1 + 1);
        toolButton->addAction(storeHereAction);
        connect(storeHereAction, SIGNAL(triggered()), globStore, SLOT(mapStoreSignal()));

        QAction *setRunOnceAction = new QAction(tr("&Run once and return"), this);
        setRunOnceAction->setProperty("index", l1 + 1);
        setRunOnceAction->setCheckable(true);
        toolButton->addAction(setRunOnceAction);
        connect(setRunOnceAction, SIGNAL(toggled(bool)), this, SLOT(updateRunOnce(bool)));

        buttonLayout->addWidget(toolButton);
    }

    QVBoxLayout *columnLayout = new QVBoxLayout;
    columnLayout->addLayout(buttonLayout);
    columnLayout->addStretch();
    columnLayout->setMargin(0);
    columnLayout->setSpacing(0);
    setLayout(columnLayout);

    globStore->indivButtonLayout->addWidget(this);


    restoreRequest = -1;
    oldRestoreRequest = 0;
    restoreRunOnce = false;
    activeStore = 0;
    currentRequest = 0;
    dispReqIx = 0;
    dispReqSelected = 0;
    needsGUIUpdate = false;
}

ParStore::~ParStore()
{
}

void ParStore::writeData(QXmlStreamWriter& xml)
{
    int l1;
    QByteArray tempArray;

    xml.writeStartElement("globalStores");

    for (int ix = 0; ix < list.size(); ix++) {
        xml.writeStartElement("parStore");
        xml.writeAttribute("ID", QString::number(ix));
            xml.writeTextElement("empty", QString::number(list.at(ix).empty));
            xml.writeTextElement("muteOut", QString::number(list.at(ix).muteOut));
            xml.writeTextElement("res", QString::number(list.at(ix).res));
            xml.writeTextElement("size", QString::number(list.at(ix).size));
            xml.writeTextElement("loopMode", QString::number(list.at(ix).loopMode));
            xml.writeTextElement("waveForm", QString::number(list.at(ix).waveForm));
            xml.writeTextElement("portOut", QString::number(list.at(ix).portOut));
            xml.writeTextElement("channelOut", QString::number(list.at(ix).channelOut));
            xml.writeTextElement("chIn", QString::number(list.at(ix).chIn));
            xml.writeTextElement("ccnumber", QString::number(list.at(ix).ccnumber));
            xml.writeTextElement("ccnumberIn", QString::number(list.at(ix).ccnumberIn));
            xml.writeTextElement("freq", QString::number(list.at(ix).freq));
            xml.writeTextElement("ampl", QString::number(list.at(ix).ampl));
            xml.writeTextElement("offs", QString::number(list.at(ix).offs));
            xml.writeTextElement("loopMarker", QString::number(list.at(ix).loopMarker));
            xml.writeTextElement("notelen", QString::number(list.at(ix).notelen));
            xml.writeTextElement("vel", QString::number(list.at(ix).vel));
            xml.writeTextElement("dispVertical", QString::number(list.at(ix).dispVertical));
            xml.writeTextElement("transp", QString::number(list.at(ix).transp));
            xml.writeTextElement("indexIn0", QString::number(list.at(ix).indexIn0));
            xml.writeTextElement("indexIn1", QString::number(list.at(ix).indexIn1));
            xml.writeTextElement("rangeIn0", QString::number(list.at(ix).rangeIn0));
            xml.writeTextElement("rangeIn1", QString::number(list.at(ix).rangeIn1));
            xml.writeTextElement("attack", QString::number(list.at(ix).attack));
            xml.writeTextElement("release", QString::number(list.at(ix).release));
            xml.writeTextElement("repeatMode", QString::number(list.at(ix).repeatMode));
            xml.writeTextElement("rndTick", QString::number(list.at(ix).rndTick));
            xml.writeTextElement("rndLen", QString::number(list.at(ix).rndLen));
            xml.writeTextElement("rndVel", QString::number(list.at(ix).rndVel));
            xml.writeTextElement("pattern", list.at(ix).pattern);

            tempArray.clear();
            l1 = 0;
            while (l1 < list.at(ix).muteMask.count()) {
                tempArray.append(list.at(ix).muteMask.at(l1));
                l1++;
            }
            xml.writeStartElement("muteMask");
                xml.writeTextElement("data", tempArray.toHex());
            xml.writeEndElement();

            tempArray.clear();
            l1 = 0;
            while (l1 < list.at(ix).wave.count()) {
                tempArray.append(list.at(ix).wave.at(l1).value);
                l1++;
            }
            xml.writeStartElement("wave");
                xml.writeTextElement("data", tempArray.toHex());
            xml.writeEndElement();
        xml.writeEndElement();
    }
    xml.writeEndElement();
}

void ParStore::readData(QXmlStreamReader& xml)
{
    int ix = 0;
    int step = 0;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        if (xml.isStartElement() && (xml.name() == "parStore")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "empty")
                    temp.empty = xml.readElementText().toInt();
                else if (xml.name() == "muteOut")
                    temp.muteOut = xml.readElementText().toInt();
                else if (xml.name() == "res")
                    temp.res = xml.readElementText().toInt();
                else if (xml.name() == "size")
                    temp.size = xml.readElementText().toInt();
                else if (xml.name() == "loopMode")
                    temp.loopMode = xml.readElementText().toInt();
                else if (xml.name() == "waveForm")
                    temp.waveForm = xml.readElementText().toInt();
                else if (xml.name() == "portOut")
                    temp.portOut = xml.readElementText().toInt();
                else if (xml.name() == "channelOut")
                    temp.channelOut = xml.readElementText().toInt();
                else if (xml.name() == "chIn")
                    temp.chIn = xml.readElementText().toInt();
                else if (xml.name() == "ccnumber")
                    temp.ccnumber = xml.readElementText().toInt();
                else if (xml.name() == "ccnumberIn")
                    temp.ccnumberIn = xml.readElementText().toInt();
                else if (xml.name() == "freq")
                    temp.freq = xml.readElementText().toInt();
                else if (xml.name() == "ampl")
                    temp.ampl = xml.readElementText().toInt();
                else if (xml.name() == "offs")
                    temp.offs = xml.readElementText().toInt();
                else if (xml.name() == "vel")
                    temp.vel = xml.readElementText().toInt();
                else if (xml.name() == "dispVertical")
                    temp.dispVertical = xml.readElementText().toInt();
                else if (xml.name() == "transp")
                    temp.transp = xml.readElementText().toInt();
                else if (xml.name() == "notelen")
                    temp.notelen = xml.readElementText().toInt();
                else if (xml.name() == "loopMarker")
                    temp.loopMarker = xml.readElementText().toInt();
                else if (xml.name() == "indexIn0")
                    temp.indexIn0 = xml.readElementText().toInt();
                else if (xml.name() == "indexIn1")
                    temp.indexIn1 = xml.readElementText().toInt();
                else if (xml.name() == "rangeIn0")
                    temp.rangeIn0 = xml.readElementText().toInt();
                else if (xml.name() == "rangeIn1")
                    temp.rangeIn1 = xml.readElementText().toInt();
                else if (xml.name() == "attack")
                    temp.attack = xml.readElementText().toInt();
                else if (xml.name() == "release")
                    temp.release = xml.readElementText().toInt();
                else if (xml.name() == "repeatMode")
                    temp.repeatMode = xml.readElementText().toInt();
                else if (xml.name() == "rndTick")
                    temp.rndTick = xml.readElementText().toInt();
                else if (xml.name() == "rndLen")
                    temp.rndLen = xml.readElementText().toInt();
                else if (xml.name() == "rndVel")
                    temp.rndVel = xml.readElementText().toInt();
                else if (xml.name() == "pattern")
                    temp.pattern = xml.readElementText();
                else if (xml.isStartElement() && (xml.name() == "muteMask")) {
                    while (!xml.atEnd()) {
                        xml.readNext();
                        if (xml.isEndElement())
                            break;
                        if (xml.isStartElement() && (xml.name() == "data")) {
                            temp.muteMask.clear();
                            QByteArray tmpArray =
                                    QByteArray::fromHex(xml.readElementText().toLatin1());
                            for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                                temp.muteMask.append(tmpArray.at(l1));
                            }
                        }
                        else skipXmlElement(xml);
                    }
                }
                else if (xml.isStartElement() && (xml.name() == "wave")) {
                    while (!xml.atEnd()) {
                        xml.readNext();
                        if (xml.isEndElement())
                            break;
                        if (xml.isStartElement() && (xml.name() == "data")) {
                            temp.wave.clear();
                            QByteArray tmpArray =
                                    QByteArray::fromHex(xml.readElementText().toLatin1());

                            if (temp.ccnumberIn >= 0)
                                step = TPQN / lfoResValues[temp.res];
                            else
                                step = TPQN / seqResValues[temp.res];

                            int lt = 0;
                            Sample sample;
                            for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                                sample.value = tmpArray.at(l1);
                                sample.tick = lt;
                                sample.muted = temp.muteMask.at(l1);
                                temp.wave.append(sample);
                                lt+=step;
                            }
                        }
                        else skipXmlElement(xml);
                    }
                }
                else skipXmlElement(xml);
            }
            tempToList(ix);
            ix++;
        }
    }
}

void ParStore::addLocation()
{
        QToolButton *toolButton = new QToolButton(this);
        toolButton->setText(QString::number(list.count()));
        toolButton->setFixedSize(QSize(100, 25));
        toolButton->setProperty("index", list.count());
        connect(toolButton, SIGNAL(pressed()), this, SLOT(mapRestoreSignal()));

        toolButton->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::ActionsContextMenu));
        QAction *storeHereAction = new QAction(tr("&Store here"), this);
        storeHereAction->setProperty("index", list.count());
        toolButton->addAction(storeHereAction);
        connect(storeHereAction, SIGNAL(triggered()), this, SLOT(mapStoreSignal()));

        QAction *setRunOnceAction = new QAction(tr("&Run once and return"), this);
        setRunOnceAction->setProperty("index", list.count());
        setRunOnceAction->setCheckable(true);
        toolButton->addAction(setRunOnceAction);
        connect(setRunOnceAction, SIGNAL(toggled(bool)), this, SLOT(updateRunOnce(bool)));

        layout()->itemAt(0)->layout()->addWidget(toolButton);
        runOnceList.append(false);
}

void ParStore::removeLocation(int ix)
{
    if (ix == -1) ix = list.count() - 1;

    list.removeAt(ix);
    QWidget *button = new QWidget(this);
    button = layout()->itemAt(0)->layout()->takeAt(ix + 1)->widget();
    delete button;
    runOnceList.removeAt(ix);
}


void ParStore::setRestoreRequest(int ix)
{
    restoreRequest = ix;
    restoreRunOnce = runOnceList.at(ix);

    setDispState(ix, 2);
}

void ParStore::updateRunOnce(bool on)
{
    int ix = sender()->property("index").toInt();

    runOnceList.replace(ix - 1, on);
    setBGColorAt(ix, 3*on);
}

void ParStore::setBGColorAt(int row, int color)
{
    QString styleSheet;

    if (color == 1)         //green
        styleSheet = "QToolButton { background-color: rgba(50, 255, 50, 30%); }";
    else if (color == 2)    //yellow
        styleSheet = "QToolButton { background-color: rgba(255, 255, 50, 30%); }";
    else if (color == 3)    //blueish
        styleSheet = "QToolButton { background-color: rgba(50, 255, 255, 10%); }";
    else                    //no color
        styleSheet = "QToolButton { }";

    layout()->itemAt(0)->layout()->itemAt(row)->widget()->setStyleSheet(styleSheet);
}

void ParStore::tempToList(int ix)
{
    if (ix >= list.size()) {
        list.append(temp);
        addLocation();
    }
    else {
        list.replace(ix, temp);
    }
    setDispState(ix + 1, 1);
}

void ParStore::mapRestoreSignal()
{
    int ix = sender()->property("index").toInt();

    setRestoreRequest(ix - 1);
}

void ParStore::mapStoreSignal()
{
    int ix = sender()->property("index").toInt();

    emit store(ix - 1, false);
}

void ParStore::setDispState(int ix, int selected)
{
    int l2;
    int color;

    if (selected == 1) {
        for (l2 = 1; l2 <= list.count(); l2++) {
            color = 3 * (layout()->itemAt(0)->layout()
                    ->itemAt(l2)->widget()
                    ->actions().at(1)->isChecked());
            setBGColorAt(l2, color);
        }
        setBGColorAt(ix, 1);
        activeStore = ix - 1;
    }
    else if (selected == 2) {
        setBGColorAt(ix + 1, 2);
        if (currentRequest != activeStore) {
            setBGColorAt(currentRequest + 1, 0);
        }
        currentRequest = ix;
    }
}

void ParStore::requestDispState(int ix, int selected)
{
    dispReqIx = ix;
    dispReqSelected = selected;
    needsGUIUpdate = true;
}

void ParStore::updateDisplay()
{
    ndc->updateDraw();

    if (!needsGUIUpdate) return;
    needsGUIUpdate = false;
    setDispState(dispReqIx, dispReqSelected);
}
