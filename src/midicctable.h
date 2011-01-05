/*
 *      midicctable.h
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
 */

#ifndef MIDICCTABLE_H
#define MIDICCTABLE_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include "arpdata.h"

class MidiCCTable : public QDialog
{
    Q_OBJECT

 public:

    MidiCCTable(ArpData *p_arpData, QWidget *parent);
    ~MidiCCTable();

 private:
    QTableWidget *midiCCTable;
    QPushButton *removeButton, *revertButton;
    void getCurrentControls();
    void fillControlRow(int nrows, MidiCC midiCC, int moduleID);
    ArpData *arpData;
    
 public slots:
    void accept();
    void reject();
    void itemChanged(QTableWidgetItem *item);
    void closeEvent(QCloseEvent *e);
    void revert();
    void apply();
    void removeCurrent();
};

#endif
