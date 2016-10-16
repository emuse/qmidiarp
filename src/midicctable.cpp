/*!
 * @file midicctable.cpp
 * @brief Implementation of the MidiCCTable QDialog class
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
 *
 */


#include <QVector>
#include <QDialogButtonBox>
#include "midicctable.h"
#include "mainwindow.h"

MidiCCTable::MidiCCTable(Engine *p_engine, QWidget *parent) : QDialog(parent)
{
    engine = p_engine;
    QGridLayout *boxLayout = new QGridLayout(this);
    midiCCTable = new QTableWidget(this);

    midiCCTable->clear();
    midiCCTable->setRowCount(engine->moduleWindowCount()*10);
    midiCCTable->setColumnCount(7);
    midiCCTable->setColumnHidden(5, true);
    midiCCTable->setColumnHidden(6, true);
    connect(midiCCTable, SIGNAL(itemChanged(QTableWidgetItem*)),
                    this, SLOT(itemChanged(QTableWidgetItem*)));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                    | QDialogButtonBox::Cancel);
    removeButton = new QPushButton(tr("Re&move"), this);
    revertButton = new QPushButton(tr("Re&vert"), this);
    buttonBox->addButton(removeButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(revertButton, QDialogButtonBox::ResetRole);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeCurrent()));
    connect(revertButton, SIGNAL(clicked()), this, SLOT(revert()));

    boxLayout->addWidget(midiCCTable, 0, 0);
    boxLayout->addWidget(buttonBox, 1, 0);

    setLayout(boxLayout);
    setModal(true);
    setWindowTitle(tr("MIDI Controllers - ") + APP_NAME);
}

MidiCCTable::~MidiCCTable()
{
}

void MidiCCTable::getCurrentControls()
{
    QVector<MidiCC> ccList;
    int l1, l2;
    int nrows = 0;

    midiCCTable->clear();

    ccList = engine->midiControl->ccList;

    for (l2 = 0; l2 < engine->midiControl->ccList.count(); l2++) {

        midiCCTable->setVerticalHeaderItem(nrows,
                new QTableWidgetItem("Engine"));

        fillControlRow(nrows, ccList.at(l2), -1);
        nrows++;
    }

    ccList = engine->globStoreWidget->midiControl->ccList;

    for (l2 = 0; l2 < engine->globStoreWidget->midiControl->ccList.count(); l2++) {

        midiCCTable->setVerticalHeaderItem(nrows,
                new QTableWidgetItem("Param Store"));

        fillControlRow(nrows, ccList.at(l2), -1);
        nrows++;
    }

    ccList = engine->grooveWidget->midiControl->ccList;

    for (l2 = 0; l2 < engine->grooveWidget->midiControl->ccList.count(); l2++) {

        midiCCTable->setVerticalHeaderItem(nrows,
                new QTableWidgetItem("Groove"));

        fillControlRow(nrows, ccList.at(l2), -1);
        nrows++;
    }

    for (l1 = 0; l1 < engine->arpWidgetCount(); l1++) {
        ccList = engine->arpWidget(l1)->midiControl->ccList;

        for (l2 = 0; l2 < engine->arpWidget(l1)->midiControl->ccList.count(); l2++) {

            midiCCTable->setVerticalHeaderItem(nrows,
                    new QTableWidgetItem(engine->arpWidget(l1)->name));

            fillControlRow(nrows, ccList.at(l2), engine->arpWidget(l1)->ID);
            nrows++;
        }
    }

    for (l1 = 0; l1 < engine->lfoWidgetCount(); l1++) {
        ccList = engine->lfoWidget(l1)->midiControl->ccList;

        for (l2 = 0; l2 < engine->lfoWidget(l1)->midiControl->ccList.count(); l2++) {

            midiCCTable->setVerticalHeaderItem(nrows,
                    new QTableWidgetItem(engine->lfoWidget(l1)->name));

            fillControlRow(nrows, ccList.at(l2), engine->lfoWidget(l1)->ID);
            nrows++;
        }
    }

    for (l1 = 0; l1 < engine->seqWidgetCount(); l1++) {
        ccList = engine->seqWidget(l1)->midiControl->ccList;

        for (l2 = 0; l2 < engine->seqWidget(l1)->midiControl->ccList.count(); l2++) {

            midiCCTable->setVerticalHeaderItem(nrows,
                    new QTableWidgetItem(engine->seqWidget(l1)->name));

            fillControlRow(nrows, ccList.at(l2), engine->seqWidget(l1)->ID);
            nrows++;
        }
    }

    midiCCTable->setHorizontalHeaderItem(0,
            new QTableWidgetItem(tr("Control")));
    midiCCTable->setColumnWidth(0, 100);

    midiCCTable->setHorizontalHeaderItem(1,
            new QTableWidgetItem(tr("CC#")));
    midiCCTable->setColumnWidth(1, 30);

    midiCCTable->setHorizontalHeaderItem(2,
            new QTableWidgetItem(tr("Ch")));
    midiCCTable->setColumnWidth(2, 30);

    midiCCTable->setHorizontalHeaderItem(3,
            new QTableWidgetItem(tr("min")));
    midiCCTable->setColumnWidth(3, 40);

    midiCCTable->setHorizontalHeaderItem(4,
            new QTableWidgetItem(tr("max")));
    midiCCTable->setColumnWidth(4, 40);

    midiCCTable->setRowCount(nrows);
}

void MidiCCTable::accept()
{
    apply();
    close();
}

void MidiCCTable::apply()
{
    int ccnumber, channel, min, max, ctrlID, moduleID;
    int l1;
    QChar moduleType;

    engine->midiControl->ccList.clear();
    engine->globStoreWidget->midiControl->ccList.clear();
    engine->grooveWidget->midiControl->ccList.clear();

    for (l1 = 0; l1 < engine->arpWidgetCount(); l1++)
            engine->arpWidget(l1)->midiControl->ccList.clear();
    for (l1 = 0; l1 < engine->lfoWidgetCount(); l1++)
            engine->lfoWidget(l1)->midiControl->ccList.clear();
    for (l1 = 0; l1 < engine->seqWidgetCount(); l1++)
            engine->seqWidget(l1)->midiControl->ccList.clear();

    for (l1 = 0; l1 < midiCCTable->rowCount(); l1++) {
        ccnumber = midiCCTable->item(l1, 1)->text().toInt();
        channel = midiCCTable->item(l1, 2)->text().toInt() - 1;
        min = midiCCTable->item(l1, 3)->text().toInt();
        max = midiCCTable->item(l1, 4)->text().toInt();
        ctrlID = midiCCTable->item(l1, 5)->text().toInt();
        moduleID = midiCCTable->item(l1, 6)->text().toInt();
        moduleType = midiCCTable->verticalHeaderItem(l1)->text().at(0);

        switch (moduleType.toLatin1()) {
            case 'E':
                    engine->midiControl
                    ->appendMidiCC(ctrlID, ccnumber, channel, min, max);
            break;
            case 'P':
                    engine->globStoreWidget->midiControl
                    ->appendMidiCC(ctrlID, ccnumber, channel, min, max);
            break;
            case 'G':
                    engine->grooveWidget->midiControl
                    ->appendMidiCC(ctrlID, ccnumber, channel, min, max);
            break;
            case 'A':
                    engine->arpWidget(moduleID)->midiControl
                    ->appendMidiCC(ctrlID, ccnumber, channel, min, max);
            break;
            case 'L':
                    engine->lfoWidget(moduleID)->midiControl
                    ->appendMidiCC(ctrlID, ccnumber, channel, min, max);
            break;
            case 'S':
                    engine->seqWidget(moduleID)->midiControl
                    ->appendMidiCC(ctrlID, ccnumber, channel, min, max);
            break;
        }
    }
}

void MidiCCTable::reject()
{
    close();
}

void MidiCCTable::closeEvent(QCloseEvent *e)
{
    midiCCTable->clear();
    e->accept();
}

void MidiCCTable::itemChanged(QTableWidgetItem *item)
{
    int test, row, comp;
    QChar moduleType;

    row = midiCCTable->currentRow();
    if (row < 0) return;

    test = item->text().toInt();
    moduleType = midiCCTable->verticalHeaderItem(row)->text().at(0);

    switch (midiCCTable->currentColumn()) {
        case 1: // CC Number
                if (test > 127) item->setText("127");
                if (test < 1) item->setText("0");
        break;
        case 2: // CC Channel
                if (test > 16) item->setText("16");
                if (test < 2) item->setText("1");
        break;
        case 3: // min
                    comp = midiCCTable->item(row, 4)->text().toInt();
                    if (test > comp) item->setText(QString::number(comp));
                    if (moduleType == 'E') {
                        if (test < 11) item->setText("10");
                    }
                    else {
                        if (test < 1) item->setText("0");
                    }
        break;
        case 4: // max
                    comp = midiCCTable->item(row, 3)->text().toInt();
                    if (moduleType == 'E') {
                        if (test > 400) item->setText("400");
                    }
                    else {
                        if (test > 127) item->setText("127");
                    }
                    if (test < comp) item->setText(QString::number(comp));
        break;
        default:
        break;
    }
}
void MidiCCTable::fillControlRow(int row, MidiCC midiCC, int moduleID)
{
    QTableWidgetItem *nameItem = new QTableWidgetItem(midiCC.name);
    nameItem->setFlags(Qt::ItemFlags(Qt::ItemIsSelectable
                                    |Qt::ItemIsEnabled));
    nameItem->setBackground(QBrush(QColor(200,200,200)));
    midiCCTable->setItem(row, 0, nameItem);
    midiCCTable->setItem(row, 1,
            new QTableWidgetItem(QString::number(midiCC.ccnumber)));
    midiCCTable->setItem(row, 2,
            new QTableWidgetItem(QString::number(midiCC.channel + 1)));
    midiCCTable->setItem(row, 3,
            new QTableWidgetItem(QString::number(midiCC.min)));
    midiCCTable->setItem(row, 4,
            new QTableWidgetItem(QString::number(midiCC.max)));
    midiCCTable->setItem(row, 5,
            new QTableWidgetItem(QString::number(midiCC.ID)));
    midiCCTable->setItem(row, 6,
            new QTableWidgetItem(QString::number(moduleID)));

    midiCCTable->setRowHeight(row, 20);
}

void MidiCCTable::revert()
{
    midiCCTable->setRowCount(engine->moduleWindowCount()*10);
    getCurrentControls();
}

void MidiCCTable::removeCurrent()
{
    midiCCTable->removeRow(midiCCTable->currentRow());
}
