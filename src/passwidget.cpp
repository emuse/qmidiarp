/*!
 * @file passwidget.cpp
 * @brief Implements the PassWidget UI class.
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
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

#include "passwidget.h"

PassWidget::PassWidget(Engine *p_engine, int p_portcount, QWidget *parent)
            : QDialog(parent)
{
    int l1;

    engine = p_engine;

    forwardCheck = new QCheckBox(this);
    forwardCheck->setText(tr("&Forward unmatched events to port"));
    forwardCheck->setChecked(false);
    QObject::connect(forwardCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateForward(bool)));

    portUnmatchedSpin = new QComboBox(this);
    portUnmatchedSpin->setDisabled(true);
    for (l1 = 0; l1 < p_portcount; l1++) portUnmatchedSpin->addItem(QString::number(l1 + 1));
    QObject::connect(portUnmatchedSpin, SIGNAL(activated(int)), this,
            SLOT(updatePortUnmatched(int)));

    QHBoxLayout *portBoxLayout = new QHBoxLayout;
    portBoxLayout->addWidget(forwardCheck);
    portBoxLayout->addStretch(1);
    portBoxLayout->addWidget(portUnmatchedSpin);

    cbuttonCheck = new QCheckBox(this);
    cbuttonCheck->setText(tr("&Modules controllable by MIDI controller"));
    cbuttonCheck->setChecked(true);
    QObject::connect(cbuttonCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateControlSetting(bool)));

    compactStyleCheck = new QCheckBox(this);
    compactStyleCheck->setText(tr("&Compact module layout style"));
    QObject::connect(compactStyleCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateCompactStyle(bool)));
    compactStyle = false;

    mutedAddCheck = new QCheckBox(this);
    mutedAddCheck->setText(tr("&Add new modules in muted state"));
    QObject::connect(mutedAddCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateMutedAdd(bool)));
    mutedAdd = false;

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *passWidgetLayout = new QVBoxLayout;
    passWidgetLayout->addLayout(portBoxLayout);
    passWidgetLayout->addWidget(cbuttonCheck);
    passWidgetLayout->addWidget(compactStyleCheck);
    passWidgetLayout->addWidget(mutedAddCheck);
    passWidgetLayout->addWidget(buttonBox);
    passWidgetLayout->addStretch();

    setLayout(passWidgetLayout);
    setModal(true);
    setWindowTitle(tr("Settings - ") + APP_NAME);
}

PassWidget::~PassWidget()
{
}

void PassWidget::updateForward(bool on)
{
    engine->driver->setForwardUnmatched(on);
    portUnmatchedSpin->setDisabled(!on);
    modified = true;
}

void PassWidget::updatePortUnmatched(int id)
{
    engine->driver->setPortUnmatched(id);
    modified = true;
}

void PassWidget::setForward(bool on)
{
    forwardCheck->setChecked(on);
}

void PassWidget::setPortUnmatched(int id)
{
    portUnmatchedSpin->setCurrentIndex(id);
    updatePortUnmatched(id);
}

void PassWidget::updateControlSetting(bool on)
{
    engine->setMidiControllable(on);
    modified = true;
}

void PassWidget::updateCompactStyle(bool on)
{
    compactStyle = on;
    engine->setCompactStyle(on);
}

void PassWidget::updateMutedAdd(bool on)
{
    mutedAdd = on;
}
