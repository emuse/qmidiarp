/*!
 * @file mainwindow.cpp
 * @brief Implements the MainWindow top-level UI class.
 *
 *
 *      Copyright 2009 - 2023 <qmidiarp-devel@lists.sourceforge.net>
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
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QPixmap>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMetaType>
#include <QSocketNotifier>
#include <QStringList>
#include <QSpinBox>
#include <QStyle>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cerrno>   // for errno
#include <csignal>  // for sigaction()
#include <cstring>  // for strerror()
#include <unistd.h> // for pipe()

#include <iostream>

#include "mainwindow.h"

#include "pixmaps/qmidiarp2.xpm"
#include "pixmaps/arpadd.xpm"
#include "pixmaps/lfoadd.xpm"
#include "pixmaps/seqadd.xpm"
#include "pixmaps/settings.xpm"
#include "pixmaps/eventlog.xpm"
#include "pixmaps/globtog.xpm"
#include "pixmaps/groovetog.xpm"
#include "pixmaps/play.xpm"
#include "pixmaps/midiclock.xpm"
#include "pixmaps/jacktr.xpm"
#include "pixmaps/fileopen.xpm"
#include "pixmaps/filenew.xpm"
#include "pixmaps/filesave.xpm"
#include "pixmaps/filesaveas.xpm"
#include "pixmaps/filequit.xpm"
#include "pixmaps/iopanelshow.xpm"
#include "pixmaps/iopanelhide.xpm"
#include "pixmaps/midicontrol.xpm"


static const char FILEEXT[] = ".qmax";

int MainWindow::sigpipe[2];
#ifdef NSM
nsm_client_t *MainWindow::nsm = 0;
#endif

MainWindow::MainWindow(int p_portCount, bool p_alsamidi, char *execName)
{
#ifndef NSM
(void)execName;
#endif
    jackFailed = false;
    filename = "";
    lastDir = QDir::homePath();
    alsaMidi = p_alsamidi;

    grooveWidget = new GrooveWidget;
    grooveWindow = new QDockWidget(tr("Groove"));
    grooveWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    grooveWindow->setWidget(grooveWidget);
    grooveWindow->setObjectName("grooveWidget");
    grooveWindow->setVisible(true);

    globStore = new GlobStore(this);
    globStoreWindow = new QDockWidget(tr("Global Store"));
    globStoreWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    globStoreWindow->setWidget(globStore);
    globStoreWindow->setObjectName("globStore");
    globStoreWindow->setVisible(true);

#ifdef NSM
    const char *nsm_url = getenv( "NSM_URL" );

    if ( nsm_url )
    {
        nsm = nsm_new();

        nsm_set_open_callback( nsm, cb_nsm_open, this );
        nsm_set_save_callback( nsm, cb_nsm_save, this );

        if ( 0 == nsm_init_thread( nsm, nsm_url ) )
        {
            connect(this, SIGNAL(nsmOpenFile(const QString &)), this,
                    SLOT(openFile(const QString &)));
            nsm_send_announce( nsm, APP_NAME, ":switch:", execName );
            nsm_thread_start(nsm);
        }
        else
        {
            nsm_free( nsm );
            nsm = 0;
        }
    }
#else
    bool nsm = 0;
#endif

    engine = new Engine(globStore, grooveWidget, p_portCount, alsaMidi, this);
    
    if (alsaMidi) {
        connect(engine->jackSync, SIGNAL(j_shutdown()), this, SLOT(jackShutdown()));
    }
    else {
        connect(engine->driver, SIGNAL(jsEvent(int)), this, SLOT(jsAction(int)));
        connect(engine->driver, SIGNAL(j_shutdown()), this, SLOT(jackShutdown()));
        if (!nsm && engine->driver->callJack(p_portCount, PACKAGE)) jackFailed = true;
    }
    connect(engine, SIGNAL(tempoUpdated(double)), this,
            SLOT(displayTempo(double)));

    connect(globStore, SIGNAL(store(int)), engine,
            SLOT(store(int)));
    connect(globStore, SIGNAL(requestRestore(int)), engine,
            SLOT(requestRestore(int)));
    connect(globStore, SIGNAL(updateGlobRestoreTimeModule(int)), engine,
            SLOT(updateGlobRestoreTimeModule(int)));
    connect(globStore, SIGNAL(removeParStores(int)), engine,
            SLOT(removeParStores(int)));

    midiCCTable = new MidiCCTable(engine, this);

    logWidget = new LogWidget(this);
    logWindow = new QDockWidget(tr("Event Log"), this);
    logWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    logWindow->setWidget(logWidget);
    logWindow->setObjectName("logWidget");
    qRegisterMetaType<MidiEvent>("MidiEvent");
    connect(engine, SIGNAL(midiEventReceived(MidiEvent, int)),
            logWidget, SLOT(appendEvent(MidiEvent, int)));

    connect(logWidget, SIGNAL(sendLogEvents(bool)),
            engine, SLOT(setSendLogEvents(bool)));

    addDockWidget(Qt::BottomDockWidgetArea, globStoreWindow);
    addDockWidget(Qt::BottomDockWidgetArea, grooveWindow);
    addDockWidget(Qt::BottomDockWidgetArea, logWindow);
    
    prefs = new Prefs;
    
    prefs->portCount = p_portCount;
    
    prefsWidget = new PrefsWidget(engine, prefs, this);

    addArpAction = new QAction(QPixmap(arpadd_xpm), tr("&New Arp..."), this);
    addArpAction->setShortcut(QKeySequence(tr("Ctrl+A", "Module|New Arp")));
    addArpAction->setToolTip(tr("Add new arpeggiator to tab bar"));
    connect(addArpAction, SIGNAL(triggered()), this, SLOT(arpNew()));

    addLfoAction = new QAction(QPixmap(lfoadd_xpm), tr("&New LFO..."), this);
    addLfoAction->setShortcut(QKeySequence(tr("Ctrl+L", "Module|New LFO")));
    addLfoAction->setToolTip(tr("Add new LFO to tab bar"));
    connect(addLfoAction, SIGNAL(triggered()), this, SLOT(lfoNew()));

    addSeqAction = new QAction(QPixmap(seqadd_xpm), tr("&New Sequencer..."), this);
    addSeqAction->setShortcut(QKeySequence(tr("Ctrl+T", "Module|New Sequencer")));
    addSeqAction->setToolTip(tr("Add new Sequencer to tab bar"));
    connect(addSeqAction, SIGNAL(triggered()), this, SLOT(seqNew()));


    fileNewAction = new QAction(QPixmap(filenew_xpm), tr("&New"), this);
    fileNewAction->setShortcut(QKeySequence(QKeySequence::New));
    fileNewAction->setToolTip(tr("Create new QMidiArp session"));
    connect(fileNewAction, SIGNAL(triggered()), this, SLOT(fileNew()));

    fileOpenAction = new QAction(QPixmap(fileopen_xpm), tr("&Open..."), this);
    fileOpenAction->setShortcut(QKeySequence(QKeySequence::Open));
    fileOpenAction->setToolTip(tr("Open QMidiArp file"));
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));

    fileSaveAction = new QAction(QPixmap(filesave_xpm), tr("&Save"), this);
    fileSaveAction->setShortcut(QKeySequence(QKeySequence::Save));
    fileSaveAction->setToolTip(tr("Save current QMidiArp session"));
    connect(fileSaveAction, SIGNAL(triggered()), this, SLOT(fileSave()));
    fileSaveAction->setDisabled(true);

    fileSaveAsAction = new QAction(QPixmap(filesaveas_xpm), tr("Save &as..."),
            this);
    fileSaveAsAction->setToolTip(
            tr("Save current QMidiArp session with new name"));
    connect(fileSaveAsAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));
    fileSaveAsAction->setDisabled(true);

    fileQuitAction = new QAction(QPixmap(filequit_xpm), tr("&Quit"), this);
    fileQuitAction->setShortcut(QKeySequence(tr("Ctrl+Q", "File|Quit")));
    fileQuitAction->setToolTip(tr("Quit application"));
    connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(close()));

    runAction = new QAction(QPixmap(play_xpm), tr("&Run with internal clock"), this);
    connect(runAction, SIGNAL(toggled(bool)), this, SLOT(updateTransportStatus(bool)));
    runAction->setCheckable(true);
    runAction->setChecked(false);
    runAction->setDisabled(true);

    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(120);
    tempoSpin->setKeyboardTracking(false);
    tempoSpin->setToolTip(tr("Tempo of internal clock"));
    connect(tempoSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateTempo(int)));
    engine->midiControl->addMidiLearnMenu("Tempo", tempoSpin, 0);

    midiClockAction = new QAction(QPixmap(midiclock_xpm),
            tr("&Use incoming MIDI Clock"), this);
    midiClockAction->setCheckable(true);
    midiClockAction->setChecked(false);
    midiClockAction->setDisabled(true);
    connect(midiClockAction, SIGNAL(toggled(bool)), this,
            SLOT(midiClockToggle(bool)));


    jackSyncAction = new QAction(QPixmap(jacktr_xpm),
            tr("&Connect to Jack Transport"), this);
    jackSyncAction->setCheckable(true);
    connect(jackSyncAction, SIGNAL(toggled(bool)), this,
            SLOT(jackSyncToggle(bool)));
    jackSyncAction->setChecked(false);
    jackSyncAction->setDisabled(true);

    updateTransportStatus(false);

    showAllIOAction = new QAction(tr("&Show all IO panels"), this);
    showAllIOAction->setIcon(QPixmap(iopanelshow_xpm));
    connect(showAllIOAction, SIGNAL(triggered()), this, SLOT(showIO()));
    showAllIOAction->setDisabled(true);

    hideAllIOAction = new QAction(tr("&Hide all IO panels"), this);
    hideAllIOAction->setIcon(QPixmap(iopanelhide_xpm));
    connect(hideAllIOAction, SIGNAL(triggered()), this, SLOT(hideIO()));
    hideAllIOAction->setDisabled(true);

    QAction* viewLogAction = logWindow->toggleViewAction();
    viewLogAction->setIcon(QPixmap(eventlog_xpm));
    viewLogAction->setText(tr("&Event Log"));
    viewLogAction->setShortcut(QKeySequence(tr("Ctrl+H", "View|Event Log")));

    QAction* viewGrooveAction = grooveWindow->toggleViewAction();
    viewGrooveAction->setIcon(QPixmap(groovetog_xpm));
    viewGrooveAction->setText(tr("&Groove Settings"));
    viewGrooveAction->setShortcut(QKeySequence(tr("Ctrl+G", "View|Groove")));

    QAction* viewSettingsAction = new QAction(tr("&Settings"), this);
    viewSettingsAction->setIcon(QPixmap(settings_xpm));
    viewSettingsAction->setShortcut(QKeySequence(tr("Ctrl+P",
                    "View|Settings")));
    connect(viewSettingsAction, SIGNAL(triggered()), prefsWidget, SLOT(show()));

    QAction* viewGlobAction = globStoreWindow->toggleViewAction();
    viewGlobAction->setIcon(QPixmap(globtog_xpm));
    viewGlobAction->setText(tr("&Global Store"));
    viewGlobAction->setShortcut(QKeySequence(tr("Ctrl+$",
                    "View|GlobalStore")));

    QMenuBar *menuBar = new QMenuBar;
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QMenu *viewMenu = new QMenu(tr("&View"), this);
    QMenu *arpMenu = new QMenu(tr("Mod&ule"), this);
    QMenu *helpMenu = new QMenu(tr("&Help"), this);

    fileMenu->addAction(fileNewAction);
    fileMenu->addAction(fileOpenAction);

    fileRecentlyOpenedFiles = fileMenu->addMenu(tr("&Recently opened files"));

    fileMenu->addAction(fileSaveAction);
    fileMenu->addAction(fileSaveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(fileQuitAction);
    connect(fileMenu, SIGNAL(aboutToShow()), this,
        SLOT(setupRecentFilesMenu()));
    connect(fileRecentlyOpenedFiles, SIGNAL(triggered(QAction*)), this,
        SLOT(recentFileActivated(QAction*)));

    viewMenu->addAction(showAllIOAction);
    viewMenu->addAction(hideAllIOAction);
    viewMenu->addAction(viewLogAction);
    viewMenu->addAction(viewGrooveAction);
    viewMenu->addAction(viewGlobAction);
    viewMenu->addAction(QPixmap(midicontrol_xpm), tr("&MIDI Controllers..."),
            this, SLOT(showMidiCCDialog()))
            ->setShortcut(QKeySequence(tr("Ctrl+M", "View|MidiControllers")));
    viewMenu->addAction(viewSettingsAction);

    arpMenu->addAction(addArpAction);
    arpMenu->addAction(addLfoAction);
    arpMenu->addAction(addSeqAction);
    arpMenu->addSeparator();

    helpMenu->addAction(tr("&About %1...").arg(APP_NAME), this,
            SLOT(helpAbout()));
    helpMenu->addAction(tr("&About Qt..."), this,
            SLOT(helpAboutQt()));

    fileToolBar = new QToolBar(tr("&File Toolbar"), this);
    fileToolBar->addAction(fileNewAction);
    fileToolBar->addAction(fileOpenAction);
    fileToolBar->addAction(fileSaveAction);
    fileToolBar->addAction(fileSaveAsAction);
    fileToolBar->setObjectName("fileToolBar");
    fileToolBar->setMaximumHeight(30);
    fileToolBar->setMinimumWidth(150);
    connect(fileToolBar, SIGNAL(orientationChanged(Qt::Orientation)), this,
            SLOT(ftb_update_orientation(Qt::Orientation)));

    controlToolBar = new QToolBar(tr("&Control Toolbar"), this);
    controlToolBar->addAction(showAllIOAction);
    controlToolBar->addAction(hideAllIOAction);
    controlToolBar->addSeparator();
    controlToolBar->addAction(viewLogAction);
    controlToolBar->addAction(viewGrooveAction);
    controlToolBar->addAction(viewGlobAction);
    controlToolBar->addSeparator();
    controlToolBar->addAction(addArpAction);
    controlToolBar->addAction(addLfoAction);
    controlToolBar->addAction(addSeqAction);
    controlToolBar->addSeparator();
    controlToolBar->addWidget(tempoSpin);
    controlToolBar->addAction(runAction);
    controlToolBar->addAction(midiClockAction);
    controlToolBar->addAction(jackSyncAction);
    controlToolBar->setObjectName("controlToolBar");
    controlToolBar->setMaximumHeight(30);
    controlToolBar->setMinimumWidth(460);
    connect(controlToolBar, SIGNAL(orientationChanged(Qt::Orientation)), this,
            SLOT(ctb_update_orientation(Qt::Orientation)));

    menuBar->addMenu(fileMenu);
    menuBar->addMenu(viewMenu);
    menuBar->addMenu(arpMenu);
    menuBar->addMenu(helpMenu);

    setMenuBar(menuBar);
    addToolBar(fileToolBar);
    addToolBar(controlToolBar);

    setWindowIcon(QPixmap(qmidiarp2_xpm));

    setCentralWidget(new QWidget(this));
    setDockNestingEnabled(true);
    updateWindowTitle();

    if (checkRcFile())
        readRcFile();

    prefsWidget->setModified(false);

    if (!installSignalHandlers())
        qWarning("%s", "Signal handlers not installed!");

    if (!jackFailed || alsaMidi) show();

#ifdef NSM
    if (nsm && nsm_is_active(nsm))
    {
        fileNewAction->setText(tr("Clear"));
        fileNewAction->setToolTip(tr("Clear QMidiArp session"));
        fileOpenAction->setText(tr("Import file..."));
        fileOpenAction->setToolTip(tr("Import QMidiArp file to NSM session"));
        fileSaveAsAction->setText(tr("Export session..."));
        fileSaveAsAction->setToolTip(tr("Export QMidiArp NSM session to file"));
        fileRecentlyOpenedFiles->setDisabled(true);
    }
#endif
}

MainWindow::~MainWindow()
{
    clear();
}

void MainWindow::updateWindowTitle()
{
    if (filename.isEmpty())
        setWindowTitle(QString("%1 (%2)")
                .arg(APP_NAME)
                .arg(engine->getClientId()));
    else
        setWindowTitle(QString("%1 - %2  (%3)")
                .arg(filename)
                .arg(APP_NAME)
                .arg(engine->getClientId()));
}

void MainWindow::helpAbout()
{
    QMessageBox::about(this, tr("About %1").arg(APP_NAME), ABOUTMSG);
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::arpNew()
{
    QString name = QString::number(engine->moduleWidgetCount('A') + 1);
    addArp("Arp:"+name);
}

void MainWindow::lfoNew()
{
    QString name = QString::number(engine->moduleWidgetCount('L') + 1);
    addLfo("LFO:"+name);
}

void MainWindow::seqNew()
{
    QString name = QString::number(engine->moduleWidgetCount('S') + 1);
    addSeq("Seq:"+name);
}

void MainWindow::addLfo(const QString& p_name, bool fromfile, 
                        ModuleWidget *clonefrom, bool inOutVisible)
{

    MidiLfo *midiWorker = new MidiLfo();
    LfoWidget *moduleWidget = new LfoWidget(midiWorker, globStore,
            prefs, inOutVisible, p_name);
    
    addModule(moduleWidget, midiWorker, fromfile, clonefrom);
}

void MainWindow::addSeq(const QString& p_name, bool fromfile, 
                        ModuleWidget *clonefrom, bool inOutVisible)
{

    MidiSeq *midiWorker = new MidiSeq();
    SeqWidget *moduleWidget = new SeqWidget(midiWorker, globStore,
            prefs, inOutVisible, p_name);
    
    addModule(moduleWidget, midiWorker, fromfile, clonefrom);
}

void MainWindow::addArp(const QString& p_name, bool fromfile, 
                        ModuleWidget *clonefrom, bool inOutVisible)
{
    (void)clonefrom;
    
    MidiArp *midiWorker = new MidiArp();
    ArpWidget *moduleWidget = new ArpWidget(midiWorker, globStore,
            prefs, inOutVisible, p_name);
    
    addModule(moduleWidget, midiWorker, fromfile, nullptr);
}

void MainWindow::addModule(ModuleWidget *moduleWidget, MidiWorker *midiWorker, bool fromfile, 
                        ModuleWidget *clonefrom)
{

    connect(moduleWidget, SIGNAL(removeModule()), this, SLOT(removeModule()));
    connect(moduleWidget, SIGNAL(cloneModule()), this, SLOT(cloneModule()));
    connect(moduleWidget, SIGNAL(dockRename(const QString&, int)),
            engine, SLOT(renameDock(const QString&, int)));
    connect(moduleWidget->midiControl, SIGNAL(setMidiLearn(int, int)),
            engine, SLOT(setMidiLearn(int, int)));

    if (clonefrom != nullptr) {
        moduleWidget->copyParamsFrom(clonefrom);
    }

    int widgetID = engine->moduleWidgetCount();
    moduleWidget->setID(widgetID);

    moduleWidget->parStore->engineRunning = engine->status;

    // if the module is added at a time when global stores are already
    // present we fill up the new global parameter storage list with dummies
    // and tag them empty
    if (!fromfile) for (int l1 = 0; l1 < (globStore->widgetList.count() - 1); l1++) {
        moduleWidget->storeParams(l1, true);
    }
    if (clonefrom != nullptr) {
        midiWorker->reverse = clonefrom->getReverse();
        midiWorker->setNextTick(clonefrom->getNextTick());
    }
    else if (engine->moduleWidgetCount())
        midiWorker->setNextTick(engine->moduleWidget(0)->getNextTick());


    appendDock(moduleWidget, widgetID);

    engine->addModuleWidget(moduleWidget);
    globStore->addModule(moduleWidget->name);

    connect(moduleWidget->parStore->topButton, SIGNAL(pressed())
            , moduleWidget->parent(), SLOT(raise()));

    checkIfFirstModule();
}

void MainWindow::cloneModule()
{
    ModuleWidget *clonefrom = (ModuleWidget *)sender();
    QString name = clonefrom->name + "_0";
    
    if (name.at(0) == 'L')
        addLfo(name, false, clonefrom);
    else if (name.at(0) == 'S')
        addSeq(name, false, clonefrom);
    else if (name.at(0) == 'A')
        addArp(name, false, clonefrom);
}

void MainWindow::appendDock(ModuleWidget *moduleWidget, int count)
{
    QDockWidget *moduleWindow = new QDockWidget(moduleWidget->name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(moduleWidget);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    
    if (prefs->compactStyle) moduleWidget->setStyleSheet(COMPACT_STYLE);
    
    if (count) {
        QDockWidget *lastWindow = (QDockWidget *)(engine->moduleWidget(count - 1)->parent());
        tabifyDockWidget(lastWindow, moduleWindow);
    }
    
    moduleWindow->setObjectName(moduleWindow->windowTitle());
    moduleWindow->show();
    moduleWindow->raise();
}

void MainWindow::removeModule()
{
    ModuleWidget *moduleWidget = (ModuleWidget *)sender();
    int ID = moduleWidget->ID;

    globStore->removeModule(ID);
    engine->removeModuleWidget(moduleWidget);
    
    engine->updateIDs(ID);
    checkIfLastModule();
}

void MainWindow::clear()
{
    updateTransportStatus(false);
    jackSyncToggle(false);

    for (int l1 = globStore->widgetList.count() - 1; l1 > 0; l1--) {
        globStore->removeLocation(l1);
    }
    globStore->setDispState(0, 0);
    globStore->midiControl->ccList.clear();
    while (engine->moduleWidgetCount()) {
        globStore->removeModule(0);
        engine->removeModuleWidget(engine->moduleWidget(0));
    }
    checkIfLastModule();
    grooveWidget->midiControl->ccList.clear();

}

void MainWindow::fileNew()
{
    if (isSave()) {
        clear();
#ifdef NSM
        if (nsm) {
            filename = configFile;
        }
        else {
            filename = "";
        }
#else
        filename = "";
#endif
        updateWindowTitle();
        engine->setModified(false);
    }
}

void MainWindow::fileOpen()
{
    if (isSave()) {
        chooseFile();
#ifdef NSM
        if (nsm) {
            filename = configFile;
            updateWindowTitle();
        }
#endif
    }
}

void MainWindow::chooseFile()
{
    QString fn =  QFileDialog::getOpenFileName(this,
            tr("Open arpeggiator file"), lastDir,
            tr("QMidiArp XML files")  + " (*" + FILEEXT + ")");
    if (fn.isEmpty())
        return;

    if (fn.endsWith(FILEEXT))
        openFile(fn);
}

void MainWindow::openFile(const QString& fn)
{
    QString qmaxVersion = "";

    lastDir = fn.left(fn.lastIndexOf('/'));

    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
#ifdef NSM
        if (nsm && nsm_is_active(nsm)) {
            filename = fn;
            //updateWindowTitle();
            return;
        }
        else {
#endif
            QMessageBox::warning(this, APP_NAME,
                tr("Could not read from file '%1'.").arg(fn));
            return;
#ifdef NSM
        }
#endif
    }

    clear();
    filename = fn;
    updateWindowTitle();

    QXmlStreamReader xml(&f);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.isEndElement())
                break;

            if (xml.name() != "session") {
                xml.raiseError(tr("Not a QMidiArp xml file."));
                QMessageBox::warning(this, APP_NAME,
                    tr("This is not a valid xml file for ")+APP_NAME);
                return;
            }
            if (xml.attributes().hasAttribute("qMaxVersion")) {
                qmaxVersion = xml.attributes().value("qMaxVersion").toString();
            }
            while (!xml.atEnd()) {
                xml.readNext();

                if (xml.isEndElement())
                    break;

                if ((xml.isStartElement()) && (xml.name() == "global"))
                    readFilePartGlobal(xml);
                else if (xml.isStartElement() && (xml.name() == "modules"))
                    readFilePartModules(xml, qmaxVersion);
                else if (xml.isStartElement() && (xml.name() == "GUI"))
                    readFilePartGUI(xml);
                else if (xml.isStartElement() && (xml.name() == "globalstorage"))
                    globStore->readData(xml);
                else skipXmlElement(xml);
            }
        }
        else skipXmlElement(xml);
    }

    addRecentlyOpenedFile(filename, recentFiles);
    engine->setModified(false);
}

void MainWindow::readFilePartGlobal(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement()) {
            break;
        }
        if (xml.name() == "tempo") {
            tempoSpin->setValue(xml.readElementText().toInt());
        }
        if (xml.isStartElement() && (xml.name() == "settings")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "midiControlEnabled")
                    prefsWidget->cbuttonCheck->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "midiClockEnabled") {
                        bool tmp = xml.readElementText().toInt();
                        if (alsaMidi) midiClockAction->setChecked(tmp);
                    }
                else if (xml.name() == "jackSyncEnabled") {
                        bool tmp = xml.readElementText().toInt();
                        jackSyncAction->setChecked(tmp);
                    }
                else if (xml.name() == "forwardUnmatched")
                    prefsWidget->setForward(xml.readElementText().toInt());
                else if (xml.name() == "storeMuteState") {
                        bool tmp = xml.readElementText().toInt();
                        prefsWidget->storeMuteStateCheck->setChecked(tmp);
                    }
                else if (xml.name() == "forwardPort")
                    prefsWidget->setPortUnmatched(xml.readElementText().toInt());
                else if (xml.name() == "outputMidiClock")
                    prefsWidget->setOutputMidiClock(xml.readElementText().toInt());
                else if (xml.name() == "midiClockPort")
                    prefsWidget->setPortMidiClock(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "groove"))
            grooveWidget->readData(xml);
        else if (xml.isStartElement() && (xml.name() == "midiControllers"))
            engine->midiControl->readData(xml);
        else skipXmlElement(xml);
    }
    prefsWidget->setModified(false);
}

void MainWindow::readFilePartModules(QXmlStreamReader& xml, const QString& qmaxVersion)
{
    while (!xml.atEnd()) {
        bool iovis = true;
        xml.readNext();

        if (xml.isEndElement())
            break;

        if (!xml.isStartElement()) {
            skipXmlElement(xml);
            continue;
        }
        if (xml.attributes().hasAttribute("inOutVisible"))
            iovis = xml.attributes().value("inOutVisible").toString().toInt();
            
        QString name = xml.name() + ":" + xml.attributes().value("name").toString();
        if (xml.name() == "Arp")
            addArp(name, true, nullptr, iovis);
        else if (xml.name() == "LFO")
            addLfo(name, true, nullptr, iovis);
        else if (xml.name() == "Seq")
            addSeq(name, true, nullptr, iovis);            

        engine->moduleWidget(-1)->readData(xml, qmaxVersion);
        
        if (engine->moduleWidgetCount() == 1) {
            for (int l1 = 0; l1 < engine->moduleWidget(0)->parStore->list.count(); l1++) {
                globStore->addLocation();
            }
        }
    }
}

void MainWindow::readFilePartGUI(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
        if (xml.name() == "windowState") {
            restoreState(QByteArray::fromHex(
            xml.readElementText().toLatin1()));
        }
        else skipXmlElement(xml);
    }
}

void MainWindow::skipXmlElement(QXmlStreamReader& xml)
{
    if (xml.isStartElement()) {
        qWarning("Unknown Element in XML File: %s",qPrintable(xml.name().toString()));
        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.isEndElement())
                break;

            if (xml.isStartElement()) {
                skipXmlElement(xml);
            }
        }
    }
}

void MainWindow::fileSave()
{
    if (filename.isEmpty())
        saveFileAs();
    else
        saveFile();
}

bool MainWindow::saveFile()
{
    QFile f(filename);
    QString nameTest;

    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, APP_NAME,
                tr("Could not write to file '%1'.").arg(filename));
        return false;
    }
    QXmlStreamWriter xml(&f);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();
    xml.writeDTD("<!DOCTYPE qmidiarpSession>");
    xml.writeStartElement("session");
    xml.writeAttribute("version", PACKAGE_VERSION);
    xml.writeAttribute("qMaxVersion", "1.1");
    xml.writeAttribute("name", filename.mid(filename.lastIndexOf('/') + 1,
                    filename.count() - filename.lastIndexOf('/') - 6));

    xml.writeStartElement("global");

        xml.writeTextElement("tempo", QString::number(tempoSpin->value()));

        xml.writeStartElement("settings");
            xml.writeTextElement("midiControlEnabled",
                QString::number((int)prefsWidget->cbuttonCheck->isChecked()));
            xml.writeTextElement("midiClockEnabled",
                QString::number((int)midiClockAction->isChecked()));
            xml.writeTextElement("jackSyncEnabled",
                QString::number((int)jackSyncAction->isChecked()));
            xml.writeTextElement("forwardUnmatched",
                QString::number((int)prefsWidget->forwardCheck->isChecked()));
            xml.writeTextElement("forwardPort",
                QString::number(prefsWidget->portUnmatchedSpin->currentIndex()));
            xml.writeTextElement("outputMidiClock",
                QString::number((int)prefsWidget->outputMidiClockCheck->isChecked()));
            xml.writeTextElement("midiClockPort",
                QString::number(prefsWidget->portMidiClockSpin->currentIndex()));
            xml.writeTextElement("storeMuteState",
                QString::number(prefsWidget->storeMuteStateCheck->isChecked()));
        xml.writeEndElement();

        grooveWidget->writeData(xml);
        engine->midiControl->writeData(xml);

    xml.writeEndElement();

    xml.writeStartElement("modules");

    for (int l1 = 0; l1 < engine->moduleWidgetCount(); l1++)
            engine->moduleWidget(l1)->writeData(xml);

    xml.writeEndElement();

    xml.writeStartElement("GUI");
        xml.writeTextElement("windowState", saveState().toHex());
    xml.writeEndElement();

    globStore->writeData(xml);

    xml.writeEndElement();
    xml.writeEndDocument();


    engine->setModified(false);
    return true;
}

void MainWindow::fileSaveAs()
{
    saveFileAs();
#ifdef NSM
    if (nsm) {
        filename = configFile;
        updateWindowTitle();
    }
#endif
}

bool MainWindow::saveFileAs()
{
    bool result = false;

    QString fn =  QFileDialog::getSaveFileName(this,
            tr("Save arpeggiator"), lastDir, tr("QMidiArp files")
            + " (*" + FILEEXT + ")");

    if (!fn.isEmpty()) {
        if (!fn.endsWith(FILEEXT))
            fn.append(FILEEXT);
        lastDir = fn.left(fn.lastIndexOf('/'));

        filename = fn;
        updateWindowTitle();
        result = saveFile();
    }
    return result;
}

bool MainWindow::isSave()
{
    bool result = false;
    QString queryStr;

    if (isModified()) {
        if (filename.isEmpty())
            queryStr = tr("Unnamed file was changed.\nSave changes?");
        else
            queryStr = tr("File '%1' was changed.\n"
                    "Save changes?").arg(filename);

        QMessageBox::StandardButton choice = QMessageBox::warning(this,
                tr("Save changes"), queryStr,
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);

        switch (choice) {
            case QMessageBox::Yes:
                if (filename.isEmpty())
                    result = saveFileAs();
                else
                    result = saveFile();
                break;
            case QMessageBox::No:
                result = true;
                break;
            case QMessageBox::Cancel:
            default:
                break;
        }
    }
    else
        result = true;

    return result;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
#ifdef NSM
    if (nsm) {
        writeRcFile();
        e->accept();
    } else
#endif
    if (isSave()) {
        writeRcFile();
        e->accept();
    }
    else
        e->ignore();
}

bool MainWindow::isModified()
{
    return (engine->isModified() || prefsWidget->isModified());
}

void MainWindow::updateTempo(int p_tempo)
{
    if (!midiClockAction->isChecked())
        engine->setTempo(p_tempo);
}

void MainWindow::displayTempo(double p_tempo)
{
    tempoSpin->setValue(p_tempo);
}

void MainWindow::updateTransportStatus(bool on)
{
    engine->setStatus(on);
    //if (alsaMidi) tempoSpin->setDisabled(on);
}

void MainWindow::midiClockToggle(bool on)
{
    if (on) jackSyncAction->setChecked(false);
    engine->setUseMidiClock(on);
    setGUIforExtSync(on);
}

void MainWindow::jackSyncToggle(bool on)
{
    if (on) midiClockAction->setChecked(false);
    setGUIforExtSync(on);
    engine->setUseJackTransport(on);
}

void MainWindow::showIO()
{
    engine->showAllIOPanels(true);
}

void MainWindow::hideIO()
{
    engine->showAllIOPanels(false);
    resize(10, 10);
}

void MainWindow::jackShutdown()
{
    if (!alsaMidi) {
        QMessageBox::warning(this, PACKAGE,
                tr("JACK has shut down or could not be started, but you are trying\n"
                   "to run QMidiArp with JACK MIDI backend.\n\n"
                    "Alternatively you can use the ALSA MIDI backend \n"
                    "by calling qmidiarp -a"));
    }
    else {
        engine->setStatus(false);
        jackSyncAction->setChecked(false);
    }
}

void MainWindow::setGUIforExtSync(bool on)
{
    runAction->setDisabled(on);
    tempoSpin->setDisabled(on);
}

bool MainWindow::checkRcFile()
{
    QDir qmahome = QDir(QDir::homePath());
    bool retval = true;
    if (!qmahome.exists(QMARCNAME)) {

        patternNames
            <<  "                         "
            <<  "Simple 4"
            <<  "Simple 8"
            <<  "Simple 16"
            <<  "Simple 32"
            <<  "Chord 8"
            <<  "Chord+Bass 16"
            <<  "Chord Oct 16 A"
            <<  "Chord Oct 16 B"
            <<  "Chord Oct 16 C"
            <<  "Fast Chords1"
            <<  "Fast Chords2"
            <<  "Fast Chords3"
            <<  "Chords/Glissando 16";

        patternPresets
            << ""
            << "0"
            << ">0"
            << ">>0"
            << ">>>0"
            << ">(0123456789)"
            << ">>(01234)0(01234)0"
            << ">>////(0123456789)\\ \\ \\ +(0123456789)"
            << ">>///0\\ \\ \\ 0+////0\\ \\ \\ \\ -00+0-00+0-00+0-00+0-0"
            << ">>///0\\ \\ \\ 0+////(0123)\\ \\ \\ \\ -00+(1234)-00+0-00+0-00+0-0"
            << ">>////(0123456789)\\ \\ \\ +(0123456789) ////hh(0123456789)ddd\\ \\ \\ ////(0123456789)"
            << ">>(0123456)p+(0123456)-(01234)(234567)(56789)"
            << ">>////(0123456789)\\ \\ \\ +(0123456789) ////hh(0123456789)ddd\\ \\ \\ ////(0123456789)-\\(0123456789)\\ \\ \\ (0123456789) ////hh+(0123456789)dd\\ \\ \\ ////-(0123456789)"
            << "d(012)>h(123)>d(012)<d(234)>hh(23)(42)(12)(43)>d012342";

        writeRcFile();
        retval = false;
    }
    return retval;
}

void MainWindow::readRcFile()
{
    QString qs;
    QStringList value;

    QDir qmahome = QDir(QDir::homePath());
    QString qmarcpath = qmahome.filePath(QMARCNAME);
    QFile f(qmarcpath);

    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, PACKAGE,
                tr("Could not read from resource file"));
        return;
    }
    QTextStream loadText(&f);
    patternNames.clear();
    patternPresets.clear();

    while (!loadText.atEnd()) {
        qs = loadText.readLine();

        if (qs.startsWith('#')) {
            value.clear();
            value = qs.split('%');
            if ((value.at(0) == "#Pattern") && (value.count() > 2)) {
                patternNames << value.at(1);
                patternPresets << value.at(2);
            }
            else if ((value.at(0) == "#CompactStyle"))
                prefsWidget->compactStyleCheck->setChecked(value.at(1).toInt());
            else if ((value.at(0) == "#MutedAdd"))
                prefsWidget->mutedAddCheck->setChecked(value.at(1).toInt());
            else if ((value.at(0) == "#StoreMuteState"))
                prefsWidget->storeMuteStateCheck->setChecked(value.at(1).toInt());
            else if ((value.at(0) == "#EnableLog"))
                logWidget->enableLog->setChecked(value.at(1).toInt());
            else if ((value.at(0) == "#LogMidiClock"))
                logWidget->logMidiClock->setChecked(value.at(1).toInt());
            else if ((value.at(0) == "#GUIState"))
                restoreState(QByteArray::fromHex(value.at(1).toUtf8()));
            else if ((value.at(0) == "#LastDir"))
                lastDir = value.at(1);
            else if ((value.at(0) == "#RecentFile"))
                recentFiles << value.at(1);
        }
    }
}

void MainWindow::writeRcFile()
{
    int l1;

    QDir qmahome = QDir(QDir::homePath());
    QString qmarcpath = qmahome.filePath(QMARCNAME);
    QFile f(qmarcpath);

    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, PACKAGE,
                tr("Could not write to resource file"));
        return;
    }
    QTextStream writeText(&f);

    for (l1 = 0; l1 < patternNames.count(); l1++)
    {
        writeText << "#Pattern%";
        writeText << qPrintable(patternNames.at(l1)) << "%";
        writeText << qPrintable(patternPresets.at(l1)) << "\n";
    }

    writeText << "#CompactStyle%";
    writeText << prefs->compactStyle << "\n";
    writeText << "#MutedAdd%";
    writeText << prefs->mutedAdd << "\n";
    writeText << "#StoreMuteState%";
    writeText << prefs->storeMuteState << "\n";
    writeText << "#EnableLog%";
    writeText << logWidget->enableLog->isChecked() << "\n";
    writeText << "#LogMidiClock%";
    writeText << logWidget->logMidiClock->isChecked() << "\n";
    writeText << "#GUIState%";
    writeText << saveState().toHex() << "\n";

    writeText << "#LastDir%";
    writeText << lastDir << "\n";

    // save recently opened files (all recent files code taken from AMS)
    if (recentFiles.count() > 0) {
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            writeText << "#RecentFile%";
            writeText << *it << "\n";
        }
    }
}

void MainWindow::setupRecentFilesMenu()
{
    fileRecentlyOpenedFiles->clear();

    if (recentFiles.count() > 0) {
        if (!midiClockAction->isChecked()) fileRecentlyOpenedFiles->setEnabled(true);
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            fileRecentlyOpenedFiles->addAction(*it);
        }
    }
    else {
        fileRecentlyOpenedFiles->setEnabled(false);
    }
#ifdef NSM
    if (nsm && nsm_is_active(nsm))
      fileRecentlyOpenedFiles->setEnabled(false);
#endif
}

void MainWindow::recentFileActivated(QAction *action)
{
    if (!action->text().isEmpty()) {
        if (isSave())
            openFile(action->text().remove('&'));
    }
}

void MainWindow::addRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    if (lst.contains(fi.absoluteFilePath()))
        return;
    if (lst.count() >= 6 )
        lst.removeLast();

    lst.prepend(fi.absoluteFilePath());
}

void MainWindow::updatePatternPresets(const QString& n, const QString& p,
        int index)
{
    if (index > 0) {
        patternNames.removeAt(index);
        patternPresets.removeAt(index);

    } else {
        patternNames.append(n);
        patternPresets.append(p);
    }
    engine->updatePatternPresets(n, p, index);
    writeRcFile();
}

void MainWindow::checkIfLastModule()
{
    if (!engine->moduleWidgetCount()) {
        runAction->setDisabled(true);
        runAction->setChecked(false);
        midiClockAction->setDisabled(true);
        midiClockAction->setChecked(false);
        jackSyncAction->setDisabled(true);
        jackSyncAction->setChecked(false);
        fileSaveAction->setDisabled(true);
        fileSaveAsAction->setDisabled(true);
        showAllIOAction->setDisabled(true);
        hideAllIOAction->setDisabled(true);
    }
}

void MainWindow::checkIfFirstModule()
{
    if (engine->moduleWidgetCount() == 1) {
        if (alsaMidi) midiClockAction->setEnabled(true);
        jackSyncAction->setEnabled(true);
        fileSaveAction->setEnabled(true);
        fileSaveAsAction->setEnabled(true);
        showAllIOAction->setEnabled(true);
        hideAllIOAction->setEnabled(true);
        runAction->setEnabled(!(midiClockAction->isChecked()
                                || jackSyncAction->isChecked()));
    }
}

void MainWindow::showMidiCCDialog()
{
    midiCCTable->revert();
    midiCCTable->show();
}

void MainWindow::handleSignal(int sig)
{
    if (write(sigpipe[1], &sig, sizeof(sig)) == -1) {
        qWarning("write() failed: %s", std::strerror(errno));
    }
}

bool MainWindow::installSignalHandlers()
{
#ifdef SIGUSR1
    /*install pipe to forward received system signals*/
    if (pipe(sigpipe) < 0) {
        qWarning("pipe() failed: %s", std::strerror(errno));
        return false;
    }

    /*install notifier to handle pipe messages*/
    QSocketNotifier* signalNotifier = new QSocketNotifier(sigpipe[0],
            QSocketNotifier::Read, this);
    connect(signalNotifier, SIGNAL(activated(int)),
            this, SLOT(signalAction(int)));

    /*install signal handlers*/
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = handleSignal;

    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        qWarning("sigaction() failed: %s", std::strerror(errno));
        return false;
    }

    if (sigaction(SIGINT, &action, NULL) == -1) {
        qWarning("sigaction() failed: %s", std::strerror(errno));
        return false;
    }
#ifdef NSM
    if (nsm && nsm_is_active(nsm)) {
        if (sigaction(SIGTERM, &action, NULL) == -1) {
            qWarning("sigaction() failed: %s", std::strerror(errno));
            return false;
        }
    }
#endif

#endif
    return true;
}

void MainWindow::signalAction(int fd)
{
#ifndef SIGUSR1
    (void)fd;
#else
    int message;

    if (read(fd, &message, sizeof(message)) == -1) {
        qWarning("read() failed: %s", std::strerror(errno));
        return;
    }

    switch (message) {
        case SIGUSR1:
            fileSave();
            break;

        case SIGINT:
#ifdef NSM
        case SIGTERM:
#endif
            close();
            break;

        default:
            qWarning("Unexpected signal received: %d", message);
            break;
    }
#endif
}

void MainWindow::jsAction(int evtype)
{
    if (!evtype) {
        filename = engine->driver->jsFilename;
        qWarning("JACK Session request to save");
        lastDir = filename.left(filename.lastIndexOf('/'));
        updateWindowTitle();
        bool result = saveFile();
        if (!result) qWarning("Warning: JACK Session File save failed");
    }
    else if (evtype == 1)
    {
        close();
    }
}

void MainWindow::ctb_update_orientation(Qt::Orientation orient)
{
    if (orient == Qt::Vertical) {
        controlToolBar->setMinimumHeight(controlToolBar->iconSize().height() * 15);
        if (fileToolBar->orientation() == Qt::Vertical)
            fileToolBar->setMinimumWidth(controlToolBar->minimumWidth());
    }
    else {
        controlToolBar->setMinimumHeight(0);
        if (fileToolBar->orientation() == Qt::Vertical)
            fileToolBar->setMinimumHeight(controlToolBar->minimumHeight());
    }

}

void MainWindow::ftb_update_orientation(Qt::Orientation orient)
{
    if (orient == Qt::Vertical) {
        fileToolBar->setMinimumHeight(fileToolBar->iconSize().height() * 7);
    }
    else {
        fileToolBar->setMinimumHeight(0);
    }
}

#ifdef NSM
int MainWindow::cb_nsm_open(const char *name, const char *display_name, const char *client_id, char **out_msg, void *userdata)
{
    return ((MainWindow *)userdata)->nsm_open(name, display_name, client_id, out_msg);
}

int MainWindow::cb_nsm_save ( char **out_msg, void *userdata )
{
    return ((MainWindow *)userdata)->nsm_save(out_msg);
}

int MainWindow::nsm_open(const char *name, const char *display_name, const char *client_id, char **out_msg)
{
    qWarning("QMidiArp: NSM asks us to open file %s", name);
    (void)out_msg;
    (void)display_name;

    configFile = name;
    if (!alsaMidi) {
        engine->driver->callJack(-1);
        engine->driver->callJack(engine->getPortCount(), client_id);
    }
    else {
        engine->driver->setClientNameSuffix(client_id);
    }
    configFile.append(".qmax");
    emit nsmOpenFile(configFile);
    return ERR_OK;
}

int MainWindow::nsm_save(char **out_msg)
{
    (void)out_msg;

    int err = ERR_OK;
    if (!saveFile()) err = ERR_GENERAL;
    return err;
}

#endif
