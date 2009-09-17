#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

static const char ABOUTMSG[] = APP_NAME " " PACKAGE_VERSION "\n"
                          "(C) 2002-2003 Matthias Nagorni (SuSE AG Nuremberg)\n"
			  "(C) 2009 Frank Kober\n"
			  "(C) 2009 Guido Scholz\n\n"
                          APP_NAME " is licensed under the GPL.\n";

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
    void updateWindowTitle();
    QString lastDir, filename;

  public:
    MainWindow(int p_portCount);
    ~MainWindow();
    QToolBar *runBox;
    QAction *runAction, *addArpAction, *removeArpAction, *renameArpAction;
	QAction *midiClockAction, *fileOpenAction;
    void openFile(const QString&);

  signals:  
    void newTempo(int);
    void runQueue(bool);

  public slots: 
	void helpAbout();
    void helpAboutQt();
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
