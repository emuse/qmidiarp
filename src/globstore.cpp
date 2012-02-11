/*!
 * @file globstore.cpp
 * @brief Implements the GlobStore UI class.
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

    activeStore[0] = 0;
    activeStore[1] = 0;
    currentRequest[0] = 0;
    currentRequest[1] = 0;
    switchAtBeat = 0;

    storeSignalMapper = new QSignalMapper(this);
    connect(storeSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(storeAll(int)));

    timeModeBox = new QComboBox(this);
    timeModeBox->addItem(tr("End of"));
    timeModeBox->addItem(tr("After"));
    connect(timeModeBox, SIGNAL(activated(int)),
             this, SLOT(updateTimeModeBox(int)));

    switchAtBeatBox = new QComboBox(this);
    for (l1 = 0; l1 < 16; l1++) {
        switchAtBeatBox->addItem(QString::number(l1 + 1)+" beats");
    }
    switchAtBeatBox->hide();
    connect(switchAtBeatBox, SIGNAL(activated(int)),
             this, SLOT(updateSwitchAtBeat(int)));

    timeModuleBox = new QComboBox(this);
    connect(timeModuleBox, SIGNAL(activated(int)),
             this, SLOT(updateTimeModule(int)));

    QWidget *indicatorBox = new QWidget(this);
    QHBoxLayout *indicatorLayout = new QHBoxLayout;
    indicator = new Indicator(20, this);
    indicatorBox->setMinimumHeight(30);
    indicatorBox->setMinimumWidth(30);
    indicatorLayout->addWidget(indicator);
    indicatorLayout->setMargin(2);
    indicatorLayout->setSpacing(1);
    indicatorBox->setLayout(indicatorLayout);

    QHBoxLayout *timeModeLayout = new QHBoxLayout();
    timeModeLayout->addWidget(timeModeBox);
    timeModeLayout->addWidget(timeModuleBox);
    timeModeLayout->addWidget(switchAtBeatBox);
    timeModeLayout->addWidget(indicatorBox);
    timeModeLayout->setSpacing(0);
    timeModeLayout->addStretch();

    QHBoxLayout *upperRowLayout = new QHBoxLayout();
    upperRowLayout->addLayout(timeModeLayout);
    upperRowLayout->addStretch();

    QAction* removeStoreAction = new QAction(tr("&Remove"), this);
    QToolButton *removeStoreButton = new QToolButton(this);
    removeStoreButton->setDefaultAction(removeStoreAction);
    removeStoreButton->setFixedSize(60, 20);
    removeStoreButton->setArrowType (Qt::ArrowType(1));
    connect(removeStoreAction, SIGNAL(triggered()), this, SLOT(removeLocation()));

    QToolButton *toolButton = new QToolButton(this);
    toolButton->setText("Global");
    toolButton->setMinimumSize(QSize(60,30));
    midiControl->addMidiLearnMenu("GlobRestore", toolButton, 0);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(toolButton);

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
    secondRowLayout->addStretch();

    QVBoxLayout *centLayout = new QVBoxLayout();
    centLayout->addLayout(upperRowLayout);
    centLayout->addLayout(secondRowLayout);

    addLocation();

    setLayout(centLayout);
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
    emit globStore(ix);
}

void GlobStore::addLocation()
{
    QAction* storeAction = new QAction(tr("&Store"), this);
    QToolButton *storeButton = new QToolButton(this);
    storeButton->setDefaultAction(storeAction);
    storeButton->setFixedSize(15, 30);
    storeAction->setIcon(QIcon(filesave_xpm));
    connect(storeAction, SIGNAL(triggered()), storeSignalMapper, SLOT(map()));
    storeSignalMapper->setMapping(storeAction, widgetList.count());

    QAction* restoreAction = new QAction(tr("&Restore"), this);
    QToolButton *restoreButton = new QToolButton(this);
    restoreButton->setDefaultAction(restoreAction);
    restoreButton->setFixedSize(45, 30);
    restoreAction->setText(QString::number(widgetList.count() + 1));
    restoreAction->setFont(QFont("Helvetica", 20));
    restoreAction->setDisabled(true);
    restoreAction->setObjectName("-1");
    restoreAction->setProperty("index", widgetList.count() + 1);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(mapRestoreSignal()));

    QWidget* globWidget = new QWidget(this);
    QHBoxLayout* globLayout = new QHBoxLayout;
    globLayout->setSpacing(0);
    globLayout->setMargin(0);
    globLayout->addWidget(storeButton);
    globLayout->addWidget(restoreButton);
    globWidget->setLayout(globLayout);

    indivButtonLayout->itemAt(0)->layout()->itemAt(0)->layout()->addWidget(globWidget);

    for (int l1 = 0; l1 < timeModuleBox->count(); l1++) {
        QToolButton *toolButton = new QToolButton(this);
        toolButton->setText(QString::number(widgetList.count()));
        toolButton->setMinimumSize(QSize(100, 30));
        toolButton->setObjectName(QString::number(l1));
        toolButton->setProperty("index", widgetList.count());
        connect(toolButton, SIGNAL(pressed()), this, SLOT(mapRestoreSignal()));

        indivButtonLayout->itemAt(l1 + 1)
            ->layout()->itemAt(0)->layout()->addWidget(toolButton);
    }

    if (widgetList.count()) {
        widgetList.last()->layout()->itemAt(1)->widget()->setEnabled(true);
    }
    widgetList.append(globWidget);
}

void GlobStore::removeLocation(int ix)
{
    if (ix == -1) ix = widgetList.count() - 1;
    if (ix < 0) return;

    emit removeParStores(ix - 1);
    if (widgetList.count() > 1) {
        delete widgetList.takeAt(ix);
        for (int l1 = 1; l1 <= timeModuleBox->count(); l1++) {
            delete indivButtonLayout->itemAt(l1)->layout()->itemAt(0)
                    ->layout()->itemAt(ix)->widget();
        }
    }
    widgetList.last()->layout()->itemAt(1)->widget()->setDisabled(true);
    updateTimeModule(0);
}

void GlobStore::updateTimeModule(int ix)
{
    (void)ix;
    emit updateGlobRestoreTimeModule(timeModuleBox->currentIndex());
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
}

void GlobStore::updateSwitchAtBeat(int ix)
{
    switchAtBeat = ix;
}

void GlobStore::setDispState(int ix, int selected, int windowIndex)
{
    int start, end;
    int l1, l2;

    if (windowIndex < 0) {
        start = 0;
        end = timeModuleBox->count();
    }
    else {
        start = windowIndex + 1;
        end = start;
    }

    if (selected == 1) {
        for (l1 = start; l1 <= end; l1++) {
            for (l2 = 1; l2 <= widgetList.count(); l2++) {
                setBGColorAt(l1, l2 - 1, 0);
            }
            setBGColorAt(l1, ix + 1, 1);
        }
        activeStore[0] = windowIndex;
        activeStore[1] = ix;
    }
    else if (selected == 2) {
        if ((currentRequest[0] != activeStore[0]) ||
            (currentRequest[1] != activeStore[1])) {
            setBGColorAt(currentRequest[0] + 1, currentRequest[1] + 1, 0);
        }
        for (l1 = start; l1 <= end; l1++) {
            setBGColorAt(l1, ix + 1, 2);
        }
        currentRequest[0] = windowIndex;
        currentRequest[1] = ix;
    }
}

void GlobStore::setBGColorAt(int column, int row, int color)
{
    QString styleSheet;

    if (color == 1)         //green
        styleSheet = "QToolButton { background-color: rgba(50, 255, 50, 30%); }";
    else if (color == 2)    //yellow
        styleSheet = "QToolButton { background-color: rgba(255, 255, 50, 30%); }";
    else                    //no color
        styleSheet = "QToolButton { }";

    indivButtonLayout->itemAt(column)->layout()->itemAt(0)->layout()
                ->itemAt(row)->widget()->setStyleSheet(styleSheet);
}

void GlobStore::addModule(const QString& name)
{
    timeModuleBox->addItem(name);

    int count = timeModuleBox->count();

    QToolButton *toolButton = new QToolButton(this);
    toolButton->setText(name);
    toolButton->setObjectName(QString::number(count - 1));
    toolButton->setMinimumSize(QSize(100,30));
    midiControl->addMidiLearnMenu("Restore_"+name, toolButton, count);

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(toolButton);


    for (int l1 = 0; l1 < widgetList.size() - 1; l1++) {
        QToolButton *toolButton = new QToolButton(this);
        toolButton->setText(QString::number(l1 + 1));
        toolButton->setMinimumSize(QSize(100, 30));
        toolButton->setObjectName(QString::number(count - 1));
        toolButton->setProperty("index", l1 + 1);
        connect(toolButton, SIGNAL(pressed()), this, SLOT(mapRestoreSignal()));

        buttonLayout->addWidget(toolButton);
    }

    QVBoxLayout *columnLayout = new QVBoxLayout;
    columnLayout->addLayout(buttonLayout);
    columnLayout->addStretch();

    indivButtonLayout->addLayout(columnLayout);
    updateTimeModule(timeModuleBox->currentIndex());
}

void GlobStore::removeModule(int ix)
{
    int l1, l2;
    if (ix < 0) return;

    timeModuleBox->setCurrentIndex(0);
    if (timeModuleBox->count()) updateTimeModule(0);
    timeModuleBox->removeItem(ix);

    midiControl->removeMidiCC(ix + 1, 0, -1);
    midiControl->names.removeAt(ix + 1);
    midiControl->names.append("");

    for (l1 = 0; l1 < widgetList.size(); l1++) {
        delete indivButtonLayout->itemAt(ix + 1)->layout()->itemAt(0)
                ->layout()->takeAt(0)->widget();
    }
    delete indivButtonLayout->takeAt(ix + 1)->layout();

    // decrement button indices and midiControl indices
    // to fill gap of removed module
    for (l2 = ix + 1; l2 < timeModuleBox->count() + 1; l2++) {
        for (l1 = 0; l1 < widgetList.size(); l1++) {
            QWidget *toolButton = indivButtonLayout->itemAt(l2)
            ->layout()->itemAt(0)->layout()->itemAt(l1)->widget();

            if (!l1) midiControl->changeMapping(toolButton, l2);
            toolButton->setObjectName(QString::number(l2 - 1));
        }
    }

    if (!timeModuleBox->count())
        while (widgetList.count() > 1) {
            removeLocation(-1);
        }
}

void GlobStore::mapRestoreSignal()
{
    int moduleID = sender()->objectName().toInt();
    int ix = sender()->property("index").toInt();

    if (moduleID >= 0) {
        timeModuleBox->setCurrentIndex(moduleID);
        updateTimeModule(moduleID);
    }
    emit requestRestore(moduleID, ix - 1);
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
}

void GlobStore::handleController(int ccnumber, int channel, int value)
{
    if (value >= widgetList.count() - 1) return;

    QVector<MidiCC> cclist= midiControl->ccList;

    for (int l2 = 0; l2 < cclist.count(); l2++) {
        if ((ccnumber == cclist.at(l2).ccnumber) &&
            (channel == cclist.at(l2).channel)) {
            if (cclist.at(l2).ID > timeModuleBox->count()) return;
            emit requestRestore(cclist.at(l2).ID - 1, value);
            timeModuleBox->setCurrentIndex(cclist.at(l2).ID - 1);
        }
    }
}
