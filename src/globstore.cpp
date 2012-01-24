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
    currentRequest = 0;
    switchAtBeat = 0;

    storeSignalMapper = new QSignalMapper(this);
    connect(storeSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(store(int)));
    restoreSignalMapper = new QSignalMapper(this);
    connect(restoreSignalMapper, SIGNAL(mapped(int)),
             this, SLOT(restore(int)));

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
    indicator = new Indicator(30, this);
    indicatorBox->setMinimumHeight(40);
    indicatorLayout->addWidget(indicator);
    indicatorLayout->setMargin(2);
    indicatorLayout->setSpacing(1);
    indicatorBox->setLayout(indicatorLayout);

    QVBoxLayout *timeModeLayout = new QVBoxLayout();
    timeModeLayout->addWidget(timeModeBox);
    timeModeLayout->addWidget(timeModuleBox);
    timeModeLayout->addWidget(switchAtBeatBox);
    timeModeLayout->addWidget(indicatorBox);
    timeModeLayout->setSpacing(0);
    timeModeLayout->addStretch();

    rowLayout = new QHBoxLayout();
    rowLayout->addLayout(timeModeLayout);

    QAction* removeStoreAction = new QAction(tr("&Remove"), this);
    QToolButton *removeStoreButton = new QToolButton(this);
    removeStoreButton->setDefaultAction(removeStoreAction);
    removeStoreButton->setFixedSize(20, 60);
    removeStoreButton->setArrowType (Qt::ArrowType(3));
    connect(removeStoreAction, SIGNAL(triggered()), this, SLOT(remove()));

    QVBoxLayout *rmStoreLayout = new QVBoxLayout();
    rmStoreLayout->addWidget(removeStoreButton);
    rmStoreLayout->addStretch();

    QHBoxLayout *centLayout = new QHBoxLayout();
    centLayout->addLayout(rowLayout);
    centLayout->addLayout(rmStoreLayout);
    centLayout->addStretch();
    add();

    setStyleSheet("QGroupBox { font: 8pt; }");
    setLayout(centLayout);
}

GlobStore::~GlobStore()
{
}

void GlobStore::store(int ix)
{
    emit globStore(ix);
    if (ix >= (widgetList.count() - 1)) {
        add();
    }
}

void GlobStore::restore(int ix)
{
    if (ix < (widgetList.count() - 1)) {
        setDispState(ix, 2);
        emit requestGlobRestore(ix);
    }
}

void GlobStore::add()
{
    QAction* restoreAction = new QAction(tr("&Restore"), this);
    QToolButton *restoreButton = new QToolButton(this);
    restoreButton->setDefaultAction(restoreAction);
    restoreButton->setFixedSize(40, 40);
    restoreAction->setText(QString::number(widgetList.count() + 1));
    //restoreButton->setStyleSheet("QToolButton { font: 22pt; background-color: rgba(50, 255, 50, 50%); }");
    restoreAction->setFont(QFont("Helvetica", 22));
    restoreAction->setDisabled(true);
    connect(restoreAction, SIGNAL(triggered()), restoreSignalMapper, SLOT(map()));
    restoreSignalMapper->setMapping(restoreAction, widgetList.count());

    QAction* storeAction = new QAction(tr("&Store"), this);
    QToolButton *storeButton = new QToolButton(this);
    storeButton->setDefaultAction(storeAction);
    storeButton->setFixedSize(40, 15);
    storeAction->setIcon(QIcon(filesave_xpm));
    connect(storeAction, SIGNAL(triggered()), storeSignalMapper, SLOT(map()));
    storeSignalMapper->setMapping(storeAction, widgetList.count());

    QWidget* globWidget = new QWidget(this);
    QVBoxLayout* globLayout = new QVBoxLayout;
    globLayout->setSpacing(0);
    globLayout->addWidget(restoreButton);
    globLayout->addWidget(storeButton);
    globLayout->addStretch();
    globWidget->setLayout(globLayout);

    rowLayout->addWidget(globWidget);
    if (widgetList.count()) {
        widgetList.last()->layout()->itemAt(0)->widget()->setEnabled(true);
    }
    widgetList.append(globWidget);
    updateTimeModule(0);

}

void GlobStore::remove(int ix)
{
    if (ix == -1) ix = widgetList.count() - 1;
    if (ix < 0) return;

    emit removeParStores(ix - 1);
    if (widgetList.count() > 1) {
        QWidget* globWidget = widgetList.takeAt(ix);
        delete globWidget;
    }
    widgetList.last()->layout()->itemAt(0)->widget()->setDisabled(true);
}

void GlobStore::updateTimeModule(int ix)
{
    (void)ix;
    emit updateGlobRestoreTimeModule(timeModuleBox->currentText());
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

void GlobStore::setDispState(int ix, int selected)
{
    if (selected == 1) {
        widgetList.at(activeStore)->layout()->itemAt(0)->widget()->
        setStyleSheet("QToolButton {  }");
        widgetList.at(ix)->layout()->itemAt(0)->widget()->
        setStyleSheet("QToolButton { background-color: rgba(50, 255, 50, 20%); }");
        activeStore = ix;
    }
    else if (selected == 2) {
        widgetList.at(currentRequest)->layout()->itemAt(0)->widget()->
        setStyleSheet("QToolButton {  }");
        widgetList.at(ix)->layout()->itemAt(0)->widget()->
        setStyleSheet("QToolButton { background-color: rgba(255, 255, 50, 20%); }");
        currentRequest = ix;
    }
    else {
        widgetList.at(currentRequest)->layout()->itemAt(0)->widget()->
        setStyleSheet("QToolButton {  }");
        widgetList.at(activeStore)->layout()->itemAt(0)->widget()->
        setStyleSheet("QToolButton {  }");
    }

}
