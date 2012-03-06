/*!
 * @file managebox.cpp
 * @brief Implements the ManageBox QWidget UI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
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
#include <QBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QLabel>
#include <QToolButton>

#include "managebox.h"

#include "pixmaps/lfowavcp.xpm"
#include "pixmaps/seqwavcp.xpm"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"

#include "config.h"

ManageBox::ManageBox(const QString & nameprefix, bool enable_clone, QWidget *parent)
            : QWidget(parent)
{

    namePrefix = nameprefix;
    // Management Buttons on the right top
    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

    QToolButton *cloneButton = new QToolButton(this);
    if (enable_clone) {
        if (namePrefix.startsWith('S')) {
            cloneAction = new QAction(QIcon(seqwavcp_xpm), tr("&Clone..."), this);
        }
        else {
            cloneAction = new QAction(QIcon(lfowavcp_xpm), tr("&Clone..."), this);
        }
        cloneAction->setToolTip(tr("Duplicate this Module in muted state"));
        cloneButton->setDefaultAction(cloneAction);
        connect(cloneAction, SIGNAL(triggered()), this, SLOT(moduleClone()));
    }
    else cloneButton->hide();
    renameAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."), this);
    renameAction->setToolTip(tr("Rename this Module"));
    QToolButton *renameButton = new QToolButton(this);
    renameButton->setDefaultAction(renameAction);
    connect(renameAction, SIGNAL(triggered()), this, SLOT(moduleRename()));

    deleteAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."), this);
    deleteAction->setToolTip(tr("Delete this Module"));
    QToolButton *deleteButton = new QToolButton(this);
    deleteButton->setDefaultAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));

    manageBoxLayout->addStretch();
    manageBoxLayout->addWidget(cloneButton);
    manageBoxLayout->addWidget(renameButton);
    manageBoxLayout->addWidget(deleteButton);

    setLayout(manageBoxLayout);
}

ManageBox::~ManageBox()
{
}

void ManageBox::moduleDelete()
{
    QString qs;
    qs = tr("Delete \"%1\"?")
        .arg(name);
    if (QMessageBox::question(0, APP_NAME, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
    emit moduleRemove(ID);
}

void ManageBox::moduleRename()
{
    QString newname, oldname;
    bool ok;

    oldname = name;

    newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);

    if (ok && !newname.isEmpty()) {
        name = namePrefix + newname;
        emit dockRename(name, parentDockID);
    }
}

void ManageBox::moduleClone()
{
        emit moduleClone(ID);
}
