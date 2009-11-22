#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QMessageBox>
#include <QMainWindow>
#include <QString>
#include <QTabWidget>
#include <QToolBar>

#include <alsa/asoundlib.h>

#include "logwidget.h"
#include "arpdata.h"
#include "passwidget.h"
#include "groovewidget.h"
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
    PassWidget *passWidget;
    GrooveWidget *grooveWidget;
    QTabWidget *tabWidget;
    LogWidget *logWidget;
    ArpData *arpData;
    QString lastDir, filename;
    QStringList patternNames, patternPresets;
    QDockWidget *logWindow, *grooveWindow, *passWindow;
    
    void chooseFile();
    bool isSave();
    void updateWindowTitle();
    bool saveFile();
    bool saveFileAs();
    bool isModified();

    void addArp(const QString&);
    void addLfo(const QString&);
    void addSeq(const QString&);
    void removeArp(int index);
    void removeLfo(int index);
    void removeSeq(int index);
    bool checkRcFile();
    void writeRcFile();
    void readRcFile();
    void checkIfLastModule();
    void checkIfFirstModule();
    void clear();
    
  protected:
    void closeEvent(QCloseEvent*);

  public:
    MainWindow(int p_portCount);
    ~MainWindow();
    QToolBar *runBox, *fileToolBar;
    QAction *runAction, *addArpAction, *removeArpAction, *renameArpAction;
    QAction *addLfoAction, *addSeqAction;
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
    void seqNew();
    void moduleRename();
    void moduleDelete();

    void helpAbout();
    void helpAboutQt();


    void updateTempo(int tempo);
    void updateRunQueue(bool on);
    void midiClockToggle(bool on);
    void resetQueue();
    void updatePatternPresets(QString n, QString p, int index);
};
  
#endif
