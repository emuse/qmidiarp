/*!
 * @file globstore.cpp
 * @brief Implements the GlobStore UI class.
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
 *
 */
#include <QLabel>
#include <QToolButton>

#include "globstore.h"
#include "main.h"
#include "pixmaps/filesave.xpm"

GlobStore::GlobStore(QWidget *parent)
            : QWidget(parent)
{
    int l1;

    midiControl = new MidiControl(this);
    midiControl->ID = -2;
    midiControl->parentDockID = -2;

    activeStore = 0;
    currentRequest = 0;
    switchAtBeat = 0;

    storeSignalMapper = new QSignalMapper(this);
    connect(storeSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(storeAll(int)));

    timeModeBox = new QComboBox;
    timeModeBox->addItem(tr("End of"));
    timeModeBox->addItem(tr("After"));
    connect(timeModeBox, SIGNAL(activated(int)),
             this, SLOT(updateTimeModeBox(int)));

    switchAtBeatBox = new QComboBox;
    for (l1 = 0; l1 < 16; l1++) {
        switchAtBeatBox->addItem(QString::number(l1 + 1)+" beats");
    }
    switchAtBeatBox->hide();
    connect(switchAtBeatBox, SIGNAL(activated(int)),
             this, SLOT(updateSwitchAtBeat(int)));

    timeModuleBox = new QComboBox;
    timeModuleBox->setCurrentIndex(0);
    connect(timeModuleBox, SIGNAL(activated(int)),
             this, SLOT(updateTimeModule(int)));

    QWidget *indicatorBox = new QWidget;
    QHBoxLayout *indicatorLayout = new QHBoxLayout;
    indicator = new Indicator(20, ' ');
    indicatorBox->setMinimumHeight(30);
    indicatorBox->setMinimumWidth(30);
    indicatorLayout->addWidget(indicator);
    indicatorLayout->setMargin(2);
    indicatorLayout->setSpacing(1);
    indicatorBox->setLayout(indicatorLayout);

    QHBoxLayout *timeModeLayout = new QHBoxLayout;
    timeModeLayout->addWidget(timeModeBox);
    timeModeLayout->addWidget(timeModuleBox);
    timeModeLayout->addWidget(switchAtBeatBox);
    timeModeLayout->addWidget(indicatorBox);
    timeModeLayout->setSpacing(0);
    timeModeLayout->addStretch();

    QHBoxLayout *upperRowLayout = new QHBoxLayout;
    upperRowLayout->addLayout(timeModeLayout);
    upperRowLayout->addStretch();

    QAction* removeStoreAction = new QAction(tr("&Remove"), this);
    QToolButton *removeStoreButton = new QToolButton;
    removeStoreButton->setDefaultAction(removeStoreAction);
    removeStoreButton->setFixedSize(50, 20);
    removeStoreButton->setArrowType (Qt::ArrowType(1));
    connect(removeStoreAction, SIGNAL(triggered()), this, SLOT(removeLocation()));

    QToolButton *toolButton = new QToolButton;
    toolButton->setText("Global");
    toolButton->setMinimumSize(QSize(48,40));
    midiControl->addMidiLearnMenu("GlobRestore", toolButton, 0);
    QFrame *topRow = new QFrame;

    QHBoxLayout *topRowLayout = new QHBoxLayout;
    topRowLayout->addWidget(toolButton);
    topRowLayout->setSpacing(0);
    topRowLayout->setMargin(0);
    topRow->setFrameStyle(QFrame::StyledPanel);
    topRow->setMinimumSize(QSize(48,46));;
    topRow->setLayout(topRowLayout);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(topRow);

    QVBoxLayout *columnLayout = new QVBoxLayout;
    columnLayout->addLayout(buttonLayout);
    columnLayout->addWidget(removeStoreButton);
    columnLayout->addStretch();

    indivButtonLayout = new QHBoxLayout;
    indivButtonLayout->setSpacing(0);
    indivButtonLayout->setMargin(0);
    indivButtonLayout->addLayout(columnLayout);
    indivButtonLayout->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *secondRowLayout = new QHBoxLayout;
    secondRowLayout->addLayout(indivButtonLayout);
    secondRowLayout->addStretch(1);

    QVBoxLayout *centLayout = new QVBoxLayout;
    centLayout->addLayout(upperRowLayout);
    centLayout->addLayout(secondRowLayout);

    addLocation();

    setLayout(centLayout);
    schedRestoreVal = 0;
    schedRestore = false;
    dispReqIx = 0;
    dispReqSelected = 0;
    needsGUIUpdate = false;
    modified = false;
}

GlobStore::~GlobStore()
{
}

void GlobStore::storeAll(int ix)
{
    if (!timeModuleBox->count()) return;
    if (ix >= (widgetList.count() - 1)) {
        addLocation();
    }
    emit store(ix);
}

void GlobStore::addLocation()
{
    QAction* storeAction = new QAction(tr("&Store"), this);
    QToolButton *storeButton = new QToolButton;
    storeButton->setDefaultAction(storeAction);
    storeButton->setFixedSize(15, 25);
    storeAction->setIcon(QPixmap(filesave_xpm));
    connect(storeAction, SIGNAL(triggered()), storeSignalMapper, SLOT(map()));
    storeSignalMapper->setMapping(storeAction, widgetList.count());

    QAction* restoreAction = new QAction(tr("&Restore"), this);
    QToolButton *restoreButton = new QToolButton;
    restoreButton->setDefaultAction(restoreAction);
    restoreButton->setFixedSize(45, 25);
    restoreAction->setText(QString::number(widgetList.count() + 1));
    restoreAction->setFont(QFont("Helvetica", 18));
    restoreAction->setDisabled(true);
    restoreAction->setObjectName("-1");
    restoreAction->setProperty("index", widgetList.count() + 1);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(mapRestoreSignal()));

    QWidget* globWidget = new QWidget;
    QHBoxLayout* globLayout = new QHBoxLayout;
    globLayout->setSpacing(0);
    globLayout->setMargin(0);
    globLayout->addWidget(storeButton);
    globLayout->addWidget(restoreButton);
    globWidget->setLayout(globLayout);

    indivButtonLayout->itemAt(0)->layout()->itemAt(0)->layout()->addWidget(globWidget);

    if (widgetList.count()) {
        widgetList.last()->layout()->itemAt(1)->widget()->setEnabled(true);
    }
    widgetList.append(globWidget);
    modified = true;
}

void GlobStore::removeLocation(int ix)
{
    if (ix == -1) ix = widgetList.count() - 1;
    if (ix < 1) return;

    emit removeParStores(ix - 1);
    if (widgetList.count() > 1) delete widgetList.takeAt(ix);
    widgetList.last()->layout()->itemAt(1)->widget()->setDisabled(true);
    if (timeModuleBox->count()) updateTimeModule(0);
    modified = true;
}

void GlobStore::updateTimeModule(int ix)
{
    (void)ix;
    emit updateGlobRestoreTimeModule(timeModuleBox->currentIndex());
    modified = true;
}

void GlobStore::updateTimeModeBox(int ix)
{
    if (ix == 0) {
        switchAtBeatBox->hide();
        timeModuleBox->show();
    }
    else if (ix == 1) {
        timeModuleBox->hide();
        switchAtBeatBox->show();
        indicator->updatePercent(0);
    }
    modified = true;
}

void GlobStore::updateSwitchAtBeat(int ix)
{
    switchAtBeat = ix;
    modified = true;
}

void GlobStore::requestDispState(int ix, int selected)
{
    dispReqIx = ix;
    dispReqSelected = selected;
    needsGUIUpdate = true;
}

void GlobStore::setDispState(int ix, int selected)
{
    int start, end;
    int l1, l2;

    start = 1;
    end = timeModuleBox->count();

    if (selected == 1) {
        for (l1 = start; l1 <= end; l1++) {
            for (l2 = 1; l2 < widgetList.count(); l2++) {
                setBGColorAt(l1, l2 , 0);
            }
            setBGColorAt(l1, ix + 1, 1);
        }
        activeStore = ix;
    }
    else if (selected == 2) {
        for (l1 = start; l1 <= end; l1++) {
            setBGColorAt(l1, ix + 1, 2);
            if (currentRequest != activeStore) {
                setBGColorAt(l1, currentRequest + 1, 0);
            }
        }
        currentRequest = ix;
    }
}

void GlobStore::setBGColorAt(int column, int row, int color)
{
    QString styleSheet;

    if (color == 1)         //green
        styleSheet = "QToolButton { background-color: rgba(50, 255, 50, 40%); }";
    else if (color == 2)    //yellow
        styleSheet = "QToolButton { background-color: rgba(80, 255, 80, 10%); }";
    else if (color == 3)    //blueish
        styleSheet = "QToolButton { background-color: rgba(50, 255, 255, 10%); }";
    else                    //no color
        styleSheet = "QToolButton { }";

    indivButtonLayout->itemAt(column)->widget()->layout()->itemAt(0)->layout()
                ->itemAt(row)->widget()->setStyleSheet(styleSheet);
}

void GlobStore::addModule(const QString& name)
{
    timeModuleBox->addItem(name);
    updateTimeModule(timeModuleBox->currentIndex());
}

void GlobStore::removeModule(int ix)
{
    if (ix < 0) return;

    timeModuleBox->setCurrentIndex(0);
    if (timeModuleBox->count()) updateTimeModule(0);
    timeModuleBox->removeItem(ix);

    if (!timeModuleBox->count())
        while (widgetList.count() > 1) {
            removeLocation(-1);
        }
}

void GlobStore::mapRestoreSignal()
{
    int ix = sender()->property("index").toInt();

    emit requestRestore(ix - 1);
}

void GlobStore::mapStoreSignal()
{
    int ix = sender()->property("index").toInt();

    emit store(ix - 1);
}

void GlobStore::readData(QXmlStreamReader& xml)
{
    int tmp;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
        if (xml.name() == "timeMode") {
            tmp =xml.readElementText().toInt();
            timeModeBox->setCurrentIndex(tmp);
            updateTimeModeBox(tmp);
        }
        else if (xml.name() == "switchAtBeat") {
            tmp =xml.readElementText().toInt();
            switchAtBeatBox->setCurrentIndex(tmp);
            updateSwitchAtBeat(tmp);
        }
        else if (xml.name() == "timeModule") {
            tmp =xml.readElementText().toInt();
            timeModuleBox->setCurrentIndex(tmp);
            updateTimeModule(tmp);
        }
        else if (xml.isStartElement() && (xml.name() == "midiControllers")) {
            midiControl->readData(xml);
        }
        else skipXmlElement(xml);
    }
    modified = false;
}

void GlobStore::writeData(QXmlStreamWriter& xml)
{
    xml.writeStartElement("globalstorage");
        xml.writeTextElement("timeMode",
            QString::number(timeModeBox->currentIndex()));
        xml.writeTextElement("switchAtBeat",
            QString::number(switchAtBeatBox->currentIndex()));
        xml.writeTextElement("timeModule",
            QString::number(timeModuleBox->currentIndex()));

        midiControl->writeData(xml);
    xml.writeEndElement();
    modified = false;
}

void GlobStore::handleController(int ccnumber, int channel, int value)
{
    if (!midiControl->ccList.count()) return;

    MidiCC midiCC = midiControl->ccList.at(0);
    int sval, min, max;

    if ((ccnumber != midiCC.ccnumber) || (channel != midiCC.channel)) return;

    min = midiCC.min;
    max = midiCC.max;
    sval = min + ((double)value * (max - min) / 127);
    if ((sval < widgetList.count() - 1)
            && (sval != activeStore)
            && (sval != currentRequest)) {
        schedRestoreVal = sval;
        schedRestore = true;
    }
}

void GlobStore::updateDisplay()
{
    indicator->updateDraw();
    midiControl->update();
    if (schedRestore) {
        schedRestore = false;
        emit requestRestore(schedRestoreVal);
    }

    if (!needsGUIUpdate) return;
    needsGUIUpdate = false;
    setDispState(dispReqIx, dispReqSelected);
}
