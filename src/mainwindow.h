#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QMessageBox>
#include <QMainWindow>
#include <QString>
#include <QToolBar>

#include <alsa/asoundlib.h>

#include "logwidget.h"
#include "arpdata.h"
#include "midicctable.h"
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

    static int sigpipe[2];
    QSpinBox *tempoSpin;
    PassWidget *passWidget;
    GrooveWidget *grooveWidget;
    LogWidget *logWidget;
    ArpData *arpData;
    MidiCCTable *midiCCTable;
    QString lastDir, filename;
    QStringList patternNames, patternPresets;
    QStringList recentFiles;
    QDockWidget *logWindow, *grooveWindow, *passWindow;
    
    void chooseFile();
    bool isSave();
    void updateWindowTitle();
    bool saveTextFile();
    bool saveFile();
    bool saveFileAs();
    bool isModified();
    bool seqEventLocked;

    void addArp(const QString&);
    void addLfo(const QString&);
    void addSeq(const QString&);

    void readFilePartGlobal(QXmlStreamReader& xml);
    void readFilePartModules(QXmlStreamReader& xml);
    void readFilePartGUI(QXmlStreamReader& xml);
    void addRecentlyOpenedFile(const QString &fn, QStringList &lst);
    void appendRecentlyOpenedFile(const QString &fn, QStringList &lst);
    bool checkRcFile();
    void writeRcFile();
    void readRcFile();
    void checkIfLastModule();
    void checkIfFirstModule();
    void clear();
    static void handleSignal(int);
    bool installSignalHandlers();
    
        
  protected:
    void closeEvent(QCloseEvent*);

  public:
    MainWindow(int p_portCount);
    ~MainWindow();
    QToolBar *controlToolBar, *fileToolBar;
    QAction *runAction, *addArpAction;
    QAction *addLfoAction, *addSeqAction;
    QAction *fileNewAction, *fileOpenAction, *fileSaveAction, *fileSaveAsAction;
    QAction *fileQuitAction;
    QAction *midiClockAction, *jackSyncAction;
    QMenu* fileRecentlyOpenedFiles;
    void openFile(const QString&);
    void skipXmlElement(QXmlStreamReader& xml);
    void openTextFile(const QString&);

  signals:  
    void newTempo(int);
    void runQueue(bool);

  public slots: 
    void fileNew();
    void fileOpen();
    void fileSave();
    void fileSaveAs();
    void recentFileActivated(QAction*);
    void setupRecentFilesMenu();
    void arpNew();
    void lfoNew();
    void seqNew();
    void renameDock(const QString& name, int index);
    void removeArp(int index);
    void removeLfo(int index);
    void removeSeq(int index);
    void helpAbout();
    void helpAboutQt();
    void updateTempo(int tempo);
    void updateRunQueue(bool on);
    void midiClockToggle(bool on);
    void jackSyncToggle(bool on);
    void setGUIforExtSync(bool on);
    void resetQueue();
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void showMidiCCDialog();
    void signalAction(int);
};
  
#endif
