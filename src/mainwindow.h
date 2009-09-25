#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMainWindow>
#include <QTabWidget>
#include <QToolBar>
#include <alsa/asoundlib.h>


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
//    ArpWidget *arpWidget;
    QString lastDir, filename;

    void chooseFile();
    bool isSave();
    void updateWindowTitle();
    bool saveFile();
    bool saveFileAs();
	bool isModified();

    void addArp(const QString&);
    void addLfo(const QString&);
    void removeArp(int index);
    void removeLfo(int index);
    void checkRcFile();
    void clear();

	
  protected:
    void closeEvent(QCloseEvent*);

  public:
    MainWindow(int p_portCount);
    ~MainWindow();
    QToolBar *runBox;
    QAction *runAction, *addArpAction, *removeArpAction, *renameArpAction;
	QAction *addLfoAction;
    QAction *fileNewAction, *fileOpenAction, *fileSaveAction, *fileSaveAsAction;
    QAction *fileQuitAction;
	QAction *midiClockAction;
    void openFile(const QString&);

  signals:  
    void newTempo(int);
    void runQueue(bool);

  public slots: 
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void arpNew();
	void lfoNew();
    void moduleRename();
    void moduleDelete();

	void helpAbout();
    void helpAboutQt();


    void updateTempo(int tempo);
    void updateRunQueue(bool on);
    void midiClockToggle(bool on);
    void resetQueue();
};
  
#endif
