#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QString>
#include <QBoxLayout>
#include <QMessageBox>
#include <QTabWidget>
#include <QToolBar>
#include <QMainWindow>
#include <QAction>


#include "arpwidget.h"
#include "logwidget.h"
#include "arpdata.h"
#include "passwidget.h"
#include "groovewidget.h"
#include "arpscreen.h"
#include "config.h"

const QString aboutText = PACKAGE_STRING "\n"
                          "(C) 2002-2003 Matthias Nagorni (SuSE AG Nuremberg)\n"
			  "(C) 2009 Frank Kober\n"
			  "(C) 2009 Guido Scholz\n\n"
                          PACKAGE " is licensed under the GPL.\n";

class MainWindow : public QMainWindow
{
  Q_OBJECT

  private:
      QSpinBox *tempoSpin;
      QMessageBox *aboutWidget;
      PassWidget *passWidget;
      GrooveWidget *grooveWidget;
      QTabWidget *tabWidget;
      LogWidget *logWidget;
      ArpData *arpData;
      ArpWidget *arpWidget;
      void addArp(const QString&);
      void removeArp(int index);
      void checkRcFile();
      QString lastDir, filename;

  public:
      MainWindow(QString fileName, int p_portCount);
      ~MainWindow();
      QToolBar *runBox;
      QAction *runAction, *addArpAction, *removeArpAction, *renameArpAction;
      void load(const QString&);

  signals:  
      void newTempo(int);
      void runQueue(bool);

  public slots: 
      void displayAbout();
      void addArp();
      void renameArp();
      void removeArp();
      void saveAs();
      void save();
      void load();
      void clear();
      void updateTempo(int tempo);
      void updateRunQueue(bool on);
      void midiClockToggle(bool on);
      void resetQueue();
};
  
#endif
