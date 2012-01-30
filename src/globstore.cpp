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
#include <QAction>
#include <QLabel>
#include <QToolButton>

#include "globstore.h"
#include "pixmaps/filesave.xpm"

GlobStore::GlobStore(QWidget *parent)
            : QGroupBox(tr("Global Storage"), parent)
{
    int l1;
    activeStore = 0;
    activeSingleStore[0] = 0;
    activeSingleStore[1] = 0;
    currentRequest = 0;
    currentSingleRequest[0] = 0;
    currentSingleRequest[1] = 0;
    switchAtBeat = 0;

    storeSignalMapper = new QSignalMapper(this);
    connect(storeSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(storeAll(int)));
    restoreSignalMapper = new QSignalMapper(this);
    connect(restoreSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(restoreAll(int)));

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

    setStyleSheet("QGroupBox { font: 8pt; }");
    setLayout(centLayout);
}

GlobStore::~GlobStore()
{
}

void GlobStore::storeAll(int ix)
{
    if (!timeModuleBox->count()) return;
    emit globStore(ix);
    if (ix >= (widgetList.count() - 1)) {
        addLocation();
    }
}

void GlobStore::restoreAll(int ix)
{
    if (ix < (widgetList.count() - 1)) {
        setDispState(ix, 2);
        emit requestGlobRestore(ix);
    }
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
    connect(restoreAction, SIGNAL(triggered()), restoreSignalMapper, SLOT(map()));
    restoreSignalMapper->setMapping(restoreAction, widgetList.count());

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
    updateTimeModule(0);
}

void GlobStore::removeLocation(int ix)
{
    if (ix == -1) ix = widgetList.count() - 1;
    if (ix < 0) return;

    emit removeParStores(ix - 1);
    if (widgetList.count() > 1) {
        QWidget* globWidget = widgetList.takeAt(ix);
        delete globWidget;
        for (int l1 = 1; l1 <= timeModuleBox->count(); l1++) {
            delete indivButtonLayout->itemAt(l1)->layout()->itemAt(0)
                    ->layout()->itemAt(ix)->widget();
        }
    }
    widgetList.last()->layout()->itemAt(1)->widget()->setDisabled(true);
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
        if (selected == 1) {
            for (l1 = start; l1 <= end; l1++) {
                for (l2 = 0; l2 < widgetList.count(); l2++) {
                    setBGColorAt(l1, l2, (l2 == ix + 1));
                }
            }
            activeStore = ix;
        }
        else if (selected == 2) {
            for (l1 = start; l1 <= end; l1++) {
                if (currentRequest != activeStore) setBGColorAt(l1, currentRequest + 1, 0);
                setBGColorAt(l1, ix + 1, 2);
            }
            currentRequest = ix;
        }
        else {
        }
    }
    else {
        if (selected == 1) {
            for (l1 = 1; l1 <= widgetList.count(); l1++) {
                setBGColorAt(windowIndex + 1, l1 - 1, 0);
            }
            setBGColorAt(windowIndex + 1, ix + 1, 1);
            activeSingleStore[0] = windowIndex;
            activeSingleStore[1] = ix;
        }
        else if (selected == 2) {
            if ((currentSingleRequest[0] != activeSingleStore[0]) ||
                (currentSingleRequest[1] != activeSingleStore[1])) {
                setBGColorAt(currentSingleRequest[0] + 1, currentSingleRequest[1] + 1, 0);
            }
            setBGColorAt(windowIndex + 1, ix + 1, 2);
            currentSingleRequest[0] = windowIndex;
            currentSingleRequest[1] = ix;
        }
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

    QToolButton *toolButton = new QToolButton(this);
    toolButton->setText(name);
    toolButton->setMinimumSize(QSize(100,30));

    QVBoxLayout *buttonLayout = new QVBoxLayout;
    buttonLayout->addWidget(toolButton);


    for (int l1 = 0; l1 < widgetList.size() - 1; l1++) {
        QToolButton *toolButton = new QToolButton(this);
        toolButton->setText(QString::number(l1 + 1));
        toolButton->setMinimumSize(QSize(100, 30));
        toolButton->setObjectName(QString::number(timeModuleBox->count() - 1));
        toolButton->setProperty("index", l1 + 1);
        connect(toolButton, SIGNAL(pressed()), this, SLOT(mapRestoreSignal()));

        buttonLayout->addWidget(toolButton);
    }

    QVBoxLayout *columnLayout = new QVBoxLayout;
    columnLayout->addLayout(buttonLayout);
    columnLayout->addStretch();

    indivButtonLayout->addLayout(columnLayout);
}

void GlobStore::removeModule(int ix)
{
    int l1, l2;
    if (ix < 0) return;

    timeModuleBox->removeItem(ix);
    timeModuleBox->setCurrentIndex(0);
    if (ix > 0) updateTimeModule(0);

    for (l1 = 0; l1 < widgetList.size(); l1++) {
        delete indivButtonLayout->itemAt(ix + 1)->layout()->itemAt(0)
                ->layout()->takeAt(0)->widget();
    }
    delete indivButtonLayout->takeAt(ix + 1)->layout();

    // decrement button indices to fill gap of removed module
    for (l2 = ix; l2 < timeModuleBox->count() + 1; l2++) {
        for (l1 = 1; l1 < widgetList.size(); l1++) {
            indivButtonLayout->itemAt(l2)->layout()->itemAt(0)
                    ->layout()->itemAt(l1)->widget()
                        ->setObjectName(QString::number(l2 - 1));
        }
    }
}

void GlobStore::mapRestoreSignal()
{
    int moduleID = sender()->objectName().toInt();
    int ix = sender()->property("index").toInt();

    setDispState(ix - 1, 2, moduleID);
    emit requestSingleRestore(moduleID, ix - 1);

    timeModuleBox->setCurrentIndex(moduleID);
    // qWarning("sending single restore module %d index %d", moduleID, ix - 1);

}
