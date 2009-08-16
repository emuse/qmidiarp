#ifndef GUI_H
#define GUI_H

#include <QString>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QList>
#include <QMessageBox>
#include <QTabWidget>
#include <QToolButton>
#include <QToolBar>

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

class Gui : public QWidget
{
  Q_OBJECT

  private:
      QSpinBox *tempoSpin;
      QToolButton *runButton, *addArpButton, *removeArpButton, *renameArpButton;
      QToolBar *runBox;
      QAction *runAction, *addArpAction, *removeArpAction, *renameArpAction;
      QMessageBox *aboutWidget;
      PassWidget *passWidget;
      GrooveWidget *grooveWidget;
      QTabWidget *tabWidget;
      LogWidget *logWidget;
      ArpData *arpData;
      ArpWidget *arpWidget;


  public:
      Gui(int p_portCount, QWidget* parent=0);
      ~Gui();


  signals:  
      void newTempo(int);
      void runQueue(bool);


  public slots: 
      void displayAbout();
      void addArp();
      void addArp(QString qs);
      void renameArp();
      void removeArp();
      void removeArp(int index);
      void save();
      void load();
      void load(QString name);
      void clear();
      void updateTempo(int tempo);
      void updateRunQueue(bool on);
      void midiClockToggle(bool on);
      void resetQueue();
      void checkRcFile();
};
  
#endif
