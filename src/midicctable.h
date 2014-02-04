/*!
 * @file midicctable.cpp
 * @brief Header for the MidiCCTable QDialog class
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2014 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef MIDICCTABLE_H
#define MIDICCTABLE_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include "engine.h"

/*! @brief QDialog class for managing MIDI controller mappings
 *
 * MidiCCTable uses a QTableWidget to display and modify the content of
 * all MIDI controllers attributed to QMidiArp's UI elements. It is
 * instantiated on program start by MainWindow and shown when the MainWindow
 * menu entry is selected. Calling the menu entry will also cause retrieval
 * and display of all modules' CCLists.
 */
class MidiCCTable : public QDialog
{
    Q_OBJECT

 public:

    MidiCCTable(Engine *p_engine, QWidget *parent);
    ~MidiCCTable();

 private:
    QTableWidget *midiCCTable;
    QPushButton *removeButton, *revertButton;
    void getCurrentControls();
    void fillControlRow(int nrows, MidiCC midiCC, int moduleID);
    Engine *engine;

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
