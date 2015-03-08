/*!
 * @file passwidget.h
 * @brief Headers for the PassWidget UI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009 - 2015 <qmidiarp-devel@lists.sourceforge.net>
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
#ifndef PASSWIDGET_H
#define PASSWIDGET_H

#include <QComboBox>
#include <QDialog>
#include <QSpinBox>

#include "engine.h"

/*!
 * The PassWidget class is a small QDialog UI that allows defining some
 * global settings for QMidiArp. It is instantiated by MainWindow.
 * It is shown when the MainWindow::viewSettingsAction() is triggered.

 * @brief Preferences QDialog UI class. Instantiated by MainWindow.
 */
class PassWidget : public QDialog

{
  Q_OBJECT

  private:
    Engine *engine;
    bool modified;

  public:
    PassWidget(Engine* engine, int p_portcount, QWidget* parent=0);
    ~PassWidget();
    void setForward(bool on);
    void setPortUnmatched(int id);
    QCheckBox *cbuttonCheck, *compactStyleCheck, *mutedAddCheck;
    QCheckBox *forwardCheck;
    QComboBox *portUnmatchedSpin;
    bool compactStyle, mutedAdd;
    bool isModified() { return modified;};
    void setModified(bool on) { modified = on; };

  signals:
    void compactLayoutToggle(bool);

  public slots:
    void updateForward(bool on);
    void updatePortUnmatched(int);
    void updateControlSetting(bool);
    void updateCompactStyle(bool);
    void updateMutedAdd(bool);
};

#endif
