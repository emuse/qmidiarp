/*
 *      midicctable.cpp
 *      
 *      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
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


#include <QVector>
#include "midicctable.h"
#include "mainwindow.h"

MidiCCTable::MidiCCTable(ArpData *arpData, QWidget *parent) : QDialog(parent)
{
    QTableWidget *midiCCTable = new QTableWidget(this);
    
    midiCCTable->clear();
    midiCCTable->setRowCount(arpData->moduleWindowCount()*10);
    midiCCTable->setColumnCount(4);
    
    getCurrentControls(arpData, midiCCTable);
	setModal(true);
	resize(300, 200);
    setWindowTitle(tr("MIDI Controllers - ") + APP_NAME);
    show();
}

MidiCCTable::~MidiCCTable()
{
}

void MidiCCTable::getCurrentControls(ArpData * arpData, QTableWidget * midiCCTable)
{
    QVector<MidiCC> ccList;
    int l1, l2, parentDockID;
    int nrows = 0;
    
    for (l1 = 0; l1 < arpData->arpWidgetCount(); l1++) {
        ccList = arpData->arpWidget(l1)->ccList;
        parentDockID = arpData->arpWidget(l1)->parentDockID;
        
        for (l2 = 0; l2 < arpData->arpWidget(l1)->ccList.count(); l2++) {
        
			midiCCTable->setVerticalHeaderItem(parentDockID + l2,
					new QTableWidgetItem(arpData->arpWidget(l1)->name));
					
	        midiCCTable->setItem(parentDockID + l2, 0, 
					new QTableWidgetItem(ccList.at(l2).name));
	        midiCCTable->setItem(parentDockID + l2, 1, 
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).ccnumber)));
	        midiCCTable->setItem(parentDockID + l2, 2,
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).min)));
	        midiCCTable->setItem(parentDockID + l2, 3,
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).max)));
			
			midiCCTable->setRowHeight(parentDockID + l2, 20);
			nrows++;
		}
    }
    
    for (l1 = 0; l1 < arpData->lfoWidgetCount(); l1++) {
        ccList = arpData->lfoWidget(l1)->ccList;
        parentDockID = arpData->lfoWidget(l1)->parentDockID;
        
        for (l2 = 0; l2 < arpData->lfoWidget(l1)->ccList.count(); l2++) {
        
			midiCCTable->setVerticalHeaderItem(parentDockID + l2,
					new QTableWidgetItem(arpData->lfoWidget(l1)->name));
					
	        midiCCTable->setItem(parentDockID + l2, 0, 
					new QTableWidgetItem(ccList.at(l2).name));
	        midiCCTable->setItem(parentDockID + l2, 1, 
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).ccnumber)));
	        midiCCTable->setItem(parentDockID + l2, 2,
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).min)));
	        midiCCTable->setItem(parentDockID + l2, 3,
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).max)));
			midiCCTable->setRowHeight(parentDockID + l2, 20);
			nrows++;
		}
    }
    
    for (l1 = 0; l1 < arpData->seqWidgetCount(); l1++) {
        ccList = arpData->seqWidget(l1)->ccList;
        parentDockID = arpData->seqWidget(l1)->parentDockID;
        
        for (l2 = 0; l2 < arpData->seqWidget(l1)->ccList.count(); l2++) {
        
			midiCCTable->setVerticalHeaderItem(parentDockID + l2,
					new QTableWidgetItem(arpData->seqWidget(l1)->name));
					
	        midiCCTable->setItem(parentDockID + l2, 0, 
					new QTableWidgetItem(ccList.at(l2).name));
	        midiCCTable->setItem(parentDockID + l2, 1, 
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).ccnumber)));
	        midiCCTable->setItem(parentDockID + l2, 2,
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).min)));
	        midiCCTable->setItem(parentDockID + l2, 3,
					new QTableWidgetItem(QString("%1").arg(ccList.at(l2).max)));
			midiCCTable->setRowHeight(parentDockID + l2, 20);
			nrows++;
		}
    }

	midiCCTable->setHorizontalHeaderItem(0,
			new QTableWidgetItem(tr("Control")));
	midiCCTable->setColumnWidth(0, 80);
	
	midiCCTable->setHorizontalHeaderItem(1,
			new QTableWidgetItem(tr("CC#")));
	midiCCTable->setColumnWidth(1, 30);
	
	midiCCTable->setHorizontalHeaderItem(2,
			new QTableWidgetItem(tr("min")));
	midiCCTable->setColumnWidth(2, 30);
	
	midiCCTable->setHorizontalHeaderItem(3,
			new QTableWidgetItem(tr("max")));
	midiCCTable->setColumnWidth(3, 30);
	
	midiCCTable->setRowCount(nrows);
}
