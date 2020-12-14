/*!
 * @file prefswidget.cpp
 * @brief Implements the PrefsWidget UI class.
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
 *
 */
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

#include "prefswidget.h"

PrefsWidget::PrefsWidget(Engine *p_engine, Prefs *p_prefs, QWidget *parent)
            : QDialog(parent)
{
    int l1;
    prefs = p_prefs;
    engine = p_engine;

    forwardCheck = new QCheckBox(this);
    forwardCheck->setText(tr("&Forward unmatched events to port"));
    forwardCheck->setChecked(false);
    QObject::connect(forwardCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateForward(bool)));

    portUnmatchedSpin = new QComboBox(this);
    portUnmatchedSpin->setDisabled(true);
    for (l1 = 0; l1 < p_prefs->portCount; l1++) portUnmatchedSpin->addItem(QString::number(l1 + 1));
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

    mutedAddCheck = new QCheckBox(this);
    mutedAddCheck->setText(tr("&Add new modules in muted state"));
    QObject::connect(mutedAddCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateMutedAdd(bool)));

    storeMuteStateCheck = new QCheckBox(this);
    storeMuteStateCheck->setText(tr("&Store and restore mute state of modules"));
    QObject::connect(storeMuteStateCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateStoreMuteState(bool)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *prefsWidgetLayout = new QVBoxLayout;
    prefsWidgetLayout->addLayout(portBoxLayout);
    prefsWidgetLayout->addWidget(cbuttonCheck);
    prefsWidgetLayout->addWidget(compactStyleCheck);
    prefsWidgetLayout->addWidget(mutedAddCheck);
    prefsWidgetLayout->addWidget(storeMuteStateCheck);
    prefsWidgetLayout->addWidget(buttonBox);
    prefsWidgetLayout->addStretch();

    setLayout(prefsWidgetLayout);
    setModal(true);
    setWindowTitle(tr("Settings - ") + APP_NAME);
}

PrefsWidget::~PrefsWidget()
{
}

void PrefsWidget::updateForward(bool on)
{
    engine->driver->setForwardUnmatched(on);
    portUnmatchedSpin->setDisabled(!on);
    modified = true;
}

void PrefsWidget::updatePortUnmatched(int id)
{
    engine->driver->setPortUnmatched(id);
    prefs->portUnmatched = id;
    modified = true;
}

void PrefsWidget::setForward(bool on)
{
    forwardCheck->setChecked(on);
    prefs->forwardUnmatched = on;
}

void PrefsWidget::setPortUnmatched(int id)
{
    portUnmatchedSpin->setCurrentIndex(id);
    prefs->portUnmatched = id;
    updatePortUnmatched(id);
}

void PrefsWidget::updateControlSetting(bool on)
{
    engine->setMidiControllable(on);
    prefs->midiControllable = on;    
    modified = true;
}

void PrefsWidget::updateCompactStyle(bool on)
{
    engine->setCompactStyle(on);
    prefs->compactStyle = on;    
}

void PrefsWidget::updateStoreMuteState(bool on)
{
    prefs->storeMuteState = on;
}

void PrefsWidget::updateMutedAdd(bool on)
{
    prefs->mutedAdd = on;
}
