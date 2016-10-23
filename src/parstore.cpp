/**
 * @file parstore.cpp
 * @brief Implements the ParStore class.
 *
 * @section LICENSE
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

#include "main.h"
#include "parstore.h"
#include "storagebutton.h"

#include "pixmaps/filesave.xpm"

ParStore::ParStore(GlobStore *p_globStore, const QString &name,
            QAction *p_muteOutAction, QAction *p_deferChangesAction,
            QWidget *p_parent): globStore(p_globStore)
{
    setParent(p_parent);
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
    temp.dispVertIndex = 0;
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

    ndc = new Indicator(14, name.at(0));

    topButton = new QToolButton;
    topButton->setFont(QFont("Helvetica", 8));
    topButton->setText(name);
    topButton->setMinimumSize(QSize(75, 10));

    muteOutAction = p_muteOutAction;
    muteOut = new QToolButton;
    muteOut->setDefaultAction(muteOutAction);
    muteOut->setFont(QFont("Helvetica", 8));
    muteOut->setMinimumSize(QSize(10, 10));

    deferChangesAction = p_deferChangesAction;
    deferChanges = new QToolButton;
    deferChanges->setDefaultAction(deferChangesAction);
    deferChanges->setFont(QFont("Helvetica", 8));
    deferChanges->setMinimumSize(QSize(10, 10));

    QHBoxLayout *muteRowLayout = new QHBoxLayout;
    muteRowLayout->addStretch();
    muteRowLayout->addWidget(muteOut);
    muteRowLayout->addWidget(deferChanges);
    muteRowLayout->setMargin(0);
    muteRowLayout->setSpacing(0);

    QVBoxLayout *controlLayout = new QVBoxLayout;
    controlLayout->addWidget(topButton);
    controlLayout->addLayout(muteRowLayout);
    controlLayout->setMargin(0);
    controlLayout->setSpacing(0);

    QWidget *indicatorBox = new QWidget;
    QHBoxLayout *indicatorLayout = new QHBoxLayout;
    indicatorBox->setMinimumHeight(20);
    indicatorBox->setMinimumWidth(25);
    indicatorLayout->addWidget(ndc);
    indicatorLayout->setMargin(2);
    indicatorLayout->setSpacing(1);
    indicatorBox->setLayout(indicatorLayout);

    QFrame *topRow = new QFrame;
    QHBoxLayout *topRowLayout = new QHBoxLayout;
    topRowLayout->addWidget(indicatorBox);
    topRowLayout->addLayout(controlLayout);
    topRowLayout->setSpacing(0);
    topRowLayout->setMargin(0);
    topRow->setMinimumSize(QSize(48,46));;
    topRow->setFrameStyle(QFrame::StyledPanel);
    topRow->setLayout(topRowLayout);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(topRow);

    locContextMenu = new QMenu(this);

    QAction *storeHereAction = new QAction(tr("&Store here"), this);
    storeHereAction->setProperty("index", list.count());
    storeHereAction->setIcon(QPixmap(filesave_xpm));
    locContextMenu->addAction(storeHereAction);
    connect(storeHereAction, SIGNAL(triggered()), this, SLOT(mapStoreSignal()));

    QAction *onlyPatternAction = new QAction(tr("&Act on pattern only"), this);
    onlyPatternAction->setProperty("index", list.count());
    onlyPatternAction->setCheckable(true);
    locContextMenu->addAction(onlyPatternAction);
    connect(onlyPatternAction, SIGNAL(toggled(bool)), this, SLOT(updateOnlyPattern(bool)));

    jumpToIndexMenu = new QMenu(tr("When finished"), this);

    jumpToGroup = new QActionGroup(this);
    connect(jumpToGroup, SIGNAL(triggered(QAction *))
             , this, SLOT(mapJumpToGroup(QAction *)));

    jumpToIndexMenu->addAction(new QAction(tr("Stay here"), this));
    jumpToIndexMenu->actions().last()->setProperty("index", -2);
    jumpToIndexMenu->actions().last()->setActionGroup(jumpToGroup);
    jumpToIndexMenu->actions().last()->setCheckable(true);
    jumpToIndexMenu->actions().last()->setChecked(true);

    jumpToIndexMenu->addAction(new QAction(tr("Jump back"), this));
    jumpToIndexMenu->actions().last()->setProperty("index", -1);
    jumpToIndexMenu->actions().last()->setActionGroup(jumpToGroup);
    jumpToIndexMenu->actions().last()->setCheckable(true);

    jumpToIndexMenu->addSeparator()->setText(tr("Jump to:"));

    locContextMenu->addMenu(jumpToIndexMenu);

    for (int l1 = 0; l1 < list.size() - 1; l1++) addLocation();

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
            xml.writeTextElement("dispVertical", QString::number(list.at(ix).dispVertIndex));
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

            xml.writeTextElement("jumpTo", QString::number(jumpToList.at(ix)));
            xml.writeTextElement("onlyPattern", QString::number((int)onlyPatternList.at(ix)));

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
    int tmpjumpto = -2;
    int tmponlypattern = 0;

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
                    temp.dispVertIndex = xml.readElementText().toInt();
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
                else if (xml.name() == "jumpTo")
                    tmpjumpto = xml.readElementText().toInt();
                else if (xml.name() == "onlyPattern")
                    tmponlypattern = xml.readElementText().toInt();
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
            updateRunOnce(ix, tmpjumpto);
            onlyPatternList.replace(ix, tmponlypattern);
            ix++;
        }
    }
}

void ParStore::addLocation()
{
    StorageButton *toolButton = new StorageButton(this);
    toolButton->setText(QString::number(list.count()));
    toolButton->setProperty("index", list.count());
    connect(toolButton, SIGNAL(pressed()), this, SLOT(mapRestoreSignal()));

    toolButton->setContextMenuPolicy(Qt::ContextMenuPolicy(Qt::CustomContextMenu));
    connect(toolButton, SIGNAL(customContextMenuRequested(const QPoint &))
                    , this, SLOT(showLocContextMenu(const QPoint &)));

    jumpToIndexMenu->addAction(new QAction(QString::number(list.count()), this));
    jumpToIndexMenu->actions().last()->setActionGroup(jumpToGroup);
    jumpToIndexMenu->actions().last()->setProperty("index", list.count() - 1);
    jumpToIndexMenu->actions().last()->setCheckable(true);

    layout()->itemAt(0)->layout()->addWidget(toolButton);
    jumpToList.append(-2);
    onlyPatternList.append(false);
}

void ParStore::removeLocation(int ix)
{
    if (ix == -1) ix = list.count() - 1;

    list.removeAt(ix);
    QWidget *button = layout()->itemAt(0)->layout()->takeAt(ix + 1)->widget();
    QAction *action = jumpToIndexMenu->actions().at(ix + 3);
    delete button;
    delete action;
    jumpToList.removeAt(ix);
    onlyPatternList.removeAt(ix);

    for (int l1 = 0; l1 < jumpToList.count(); l1++) {
        if (jumpToList.at(l1) >= jumpToList.count()) updateRunOnce(l1, -2);
    }
}


void ParStore::setRestoreRequest(int ix)
{
    restoreRequest = ix;
    restoreRunOnce = (jumpToList.at(ix) > -2 );

    setDispState(ix, 2);
}

void ParStore::mapJumpToGroup(QAction *action)
{
    int choice = action->property("index").toInt();
    int location = sender()->property("index").toInt();

    updateRunOnce(location, choice);
}

void ParStore::updateRunOnce(int location, int choice)
{
    if (choice == -2) { //stay here
        jumpToList.replace(location, -2);
        setBGColorAt(location + 1, 0);
        ((StorageButton *)(layout()->itemAt(0)->layout()->itemAt(location + 1)
            ->widget()))->setSecondText("", 0);
    }
    else if (choice == -1) { //jump back to last
        jumpToList.replace(location, -1);
        setBGColorAt(location + 1, 3);
        ((StorageButton *)(layout()->itemAt(0)->layout()->itemAt(location + 1)
            ->widget()))->setSecondText("<- ", 1);
    }
    else if (choice >= 0) { //jump to location
        jumpToList.replace(location, choice);
        ((StorageButton *)(layout()->itemAt(0)->layout()->itemAt(location + 1)
            ->widget()))->setSecondText("-> "+QString::number(choice + 1), 2);
        setBGColorAt(location + 1, 3);
    }
}

void ParStore::setBGColorAt(int row, int color)
{
    QString styleSheet;

    if (color == 1)         //green
        styleSheet = "QToolButton { background-color: rgba(50, 255, 50, 40%); }";
    else if (color == 2)    //yellow
        styleSheet = "QToolButton { background-color: rgba(80, 255, 80, 10%); }";
    else if (color == 3)    //blueish
        styleSheet = "QToolButton { }";
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
    currentRequest = ix;
    setDispState(ix, 1);
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
            color = 3 * (jumpToList.at(l2 - 1) > -2);
            setBGColorAt(l2, color);
        }
        setBGColorAt(ix + 1, 1);
        activeStore = ix;
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

void ParStore::updateDisplay(int frame, bool reverse)
{
    ndc->updateDraw();

    if (needsGUIUpdate) {
        needsGUIUpdate = false;
        setDispState(dispReqIx, dispReqSelected);
    }

    if ((restoreRequest >= 0) && !frame) {
        int req = restoreRequest;
        setDispState(req, 1);
        emit restore(req);
        restoreRequest = -1;
        if (!restoreRunOnce) {
            oldRestoreRequest = req;
        }
    }

    if ((frame == 1)
            && (restoreRequest != oldRestoreRequest)
            && restoreRunOnce
            && !reverse) {
        if (jumpToList.at(activeStore) >= 0) {
            restoreRequest = jumpToList.at(activeStore);
            oldRestoreRequest = restoreRequest;
        }
        else {
            restoreRequest = oldRestoreRequest;
        }
        restoreRunOnce = (jumpToList.at(restoreRequest) > -2);
        setDispState(restoreRequest, 2);
    }

}

void ParStore::showLocContextMenu(const QPoint &pos)
{
    int senderlocation = sender()->property("index").toInt() - 1;
    int l1;

    for (l1 = 0; l1 < locContextMenu->actions().count(); l1++) {
        locContextMenu->actions().at(l1)->setProperty("index",
            senderlocation + 1);
    }
    for (l1 = 0; l1 < list.count(); l1++) {
        jumpToGroup->actions().at(l1 + 2)
                ->setDisabled(l1 == senderlocation);
    }

    locContextMenu->setProperty("index", senderlocation);
    jumpToGroup->setProperty("index", senderlocation);

    jumpToGroup->actions().at(jumpToList.at(senderlocation) + 2)
            ->setChecked(true);
    locContextMenu->actions().at(1)->setChecked(onlyPatternList.at(senderlocation));
    locContextMenu->popup(QWidget::mapToGlobal(pos));
}

void ParStore::updateOnlyPattern(bool on)
{
    onlyPatternList.replace(sender()->property("index").toInt() - 1, on);
}
