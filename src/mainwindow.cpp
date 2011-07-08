/*!
 * @file mainwindow.cpp
 * @brief Implements the MainWindow top-level UI class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
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
#include <QBoxLayout>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMetaType>
#include <QSocketNotifier>
#include <QStringList>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <cerrno>   // for errno
#include <csignal>  // for sigaction()
#include <cstring>  // for strerror()
#include <unistd.h> // for pipe()


#include "mainwindow.h"

#include "pixmaps/qmidiarp2.xpm"
#include "pixmaps/arpadd.xpm"
#include "pixmaps/lfoadd.xpm"
#include "pixmaps/seqadd.xpm"
#include "pixmaps/settings.xpm"
#include "pixmaps/eventlog.xpm"
#include "pixmaps/groovetog.xpm"
#include "pixmaps/play.xpm"
#include "pixmaps/midiclock.xpm"
#include "pixmaps/jacktr.xpm"
#include "pixmaps/fileopen.xpm"
#include "pixmaps/filenew.xpm"
#include "pixmaps/filesave.xpm"
#include "pixmaps/filesaveas.xpm"
#include "pixmaps/filequit.xpm"


static const char FILEEXT[] = ".qmax";

int MainWindow::sigpipe[2];

MainWindow::MainWindow(int p_portCount)
{
    filename = "";
    lastDir = QDir::homePath();

    grooveWidget = new GrooveWidget(this);
    grooveWindow = new QDockWidget(tr("Groove"), this);
    grooveWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    grooveWindow->setWidget(grooveWidget);;
    grooveWindow->setObjectName("grooveWidget");
    grooveWindow->setVisible(true);
    addDockWidget(Qt::BottomDockWidgetArea, grooveWindow);

    engine = new Engine(grooveWidget, p_portCount, this);

    midiCCTable = new MidiCCTable(engine, this);

    logWidget = new LogWidget(this);
    logWindow = new QDockWidget(tr("Event Log"), this);
    logWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    logWindow->setWidget(logWidget);
    logWindow->setObjectName("logWidget");
    addDockWidget(Qt::BottomDockWidgetArea, logWindow);
    qRegisterMetaType<MidiEvent>("MidiEvent");
    connect(engine->seqDriver, SIGNAL(midiEvent(MidiEvent)),
            logWidget, SLOT(appendEvent(MidiEvent)));

    connect(logWidget, SIGNAL(sendLogEvents(bool)),
            engine->seqDriver, SLOT(setSendLogEvents(bool)));

    passWidget = new PassWidget(engine, p_portCount, this);

    connect(this, SIGNAL(runQueue(bool)),
            engine->seqDriver, SLOT(setQueueStatus(bool)));

    addArpAction = new QAction(QIcon(arpadd_xpm), tr("&New Arp..."), this);
    addArpAction->setShortcut(QKeySequence(tr("Ctrl+A", "Module|New Arp")));
    addArpAction->setToolTip(tr("Add new arpeggiator to tab bar"));
    connect(addArpAction, SIGNAL(triggered()), this, SLOT(arpNew()));

    addLfoAction = new QAction(QIcon(lfoadd_xpm), tr("&New LFO..."), this);
    addLfoAction->setShortcut(QKeySequence(tr("Ctrl+L", "Module|New LFO")));
    addLfoAction->setToolTip(tr("Add new LFO to tab bar"));
    connect(addLfoAction, SIGNAL(triggered()), this, SLOT(lfoNew()));

    addSeqAction = new QAction(QIcon(seqadd_xpm), tr("&New Sequencer..."), this);
    addSeqAction->setShortcut(QKeySequence(tr("Ctrl+T", "Module|New Sequencer")));
    addSeqAction->setToolTip(tr("Add new Sequencer to tab bar"));
    connect(addSeqAction, SIGNAL(triggered()), this, SLOT(seqNew()));


    fileNewAction = new QAction(QIcon(filenew_xpm), tr("&New"), this);
    fileNewAction->setShortcut(QKeySequence(QKeySequence::New));
    fileNewAction->setToolTip(tr("Create new arpeggiator file"));
    connect(fileNewAction, SIGNAL(triggered()), this, SLOT(fileNew()));

    fileOpenAction = new QAction(QIcon(fileopen_xpm), tr("&Open..."), this);
    fileOpenAction->setShortcut(QKeySequence(QKeySequence::Open));
    fileOpenAction->setToolTip(tr("Open arpeggiator file"));
    connect(fileOpenAction, SIGNAL(triggered()), this, SLOT(fileOpen()));

    fileSaveAction = new QAction(QIcon(filesave_xpm), tr("&Save"), this);
    fileSaveAction->setShortcut(QKeySequence(QKeySequence::Save));
    fileSaveAction->setToolTip(tr("Save current arpeggiator file"));
    connect(fileSaveAction, SIGNAL(triggered()), this, SLOT(fileSave()));

    fileSaveAsAction = new QAction(QIcon(filesaveas_xpm), tr("Save &as..."),
            this);
    fileSaveAsAction->setToolTip(
            tr("Save current arpeggiator file with new name"));
    connect(fileSaveAsAction, SIGNAL(triggered()), this, SLOT(fileSaveAs()));

    fileQuitAction = new QAction(QIcon(filequit_xpm), tr("&Quit"), this);
    fileQuitAction->setShortcut(QKeySequence(tr("Ctrl+Q", "File|Quit")));
    fileQuitAction->setToolTip(tr("Quit application"));
    connect(fileQuitAction, SIGNAL(triggered()), this, SLOT(close()));

    runAction = new QAction(QIcon(play_xpm), tr("&Run with internal clock"), this);
    connect(runAction, SIGNAL(toggled(bool)), this, SLOT(updateRunQueue(bool)));
    runAction->setCheckable(true);
    runAction->setChecked(false);
    runAction->setDisabled(true);

    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(100);
    tempoSpin->setKeyboardTracking(false);
    tempoSpin->setToolTip(tr("Tempo of internal clock"));
    connect(tempoSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateTempo(int)));

    midiClockAction = new QAction(QIcon(midiclock_xpm),
            tr("&Use incoming MIDI Clock"), this);
    midiClockAction->setCheckable(true);
    midiClockAction->setChecked(false);
    midiClockAction->setDisabled(true);
    connect(midiClockAction, SIGNAL(toggled(bool)), this,
            SLOT(midiClockToggle(bool)));


    jackSyncAction = new QAction(QIcon(jacktr_xpm),
            tr("&Connect to Jack Transport"), this);
    jackSyncAction->setCheckable(true);
    jackSyncAction->setChecked(false);
    jackSyncAction->setDisabled(true);
    connect(jackSyncAction, SIGNAL(toggled(bool)), this,
            SLOT(jackSyncToggle(bool)));
    connect(engine->seqDriver, SIGNAL(jackShutdown(bool)),
            jackSyncAction, SLOT(setChecked(bool)));


    updateRunQueue(false);

    QAction* viewLogAction = logWindow->toggleViewAction();
    viewLogAction->setIcon(QIcon(eventlog_xpm));
    viewLogAction->setText(tr("&Event Log"));
    viewLogAction->setShortcut(QKeySequence(tr("Ctrl+H", "View|Event Log")));

    QAction* viewGrooveAction = grooveWindow->toggleViewAction();
    viewGrooveAction->setIcon(QIcon(groovetog_xpm));
    viewGrooveAction->setText(tr("&Groove Settings"));
    viewGrooveAction->setShortcut(QKeySequence(tr("Ctrl+G", "View|Groove")));

    QAction* viewSettingsAction = new QAction(tr("&Settings"), this);
    viewSettingsAction->setIcon(QIcon(settings_xpm));
    viewSettingsAction->setShortcut(QKeySequence(tr("Ctrl+P",
                    "View|Settings")));
    connect(viewSettingsAction, SIGNAL(triggered()), passWidget, SLOT(show()));

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

    viewMenu->addAction(viewLogAction);
    viewMenu->addAction(viewGrooveAction);
    viewMenu->addAction(viewSettingsAction);
    viewMenu->addAction(tr("&MIDI Controllers..."),
            this, SLOT(showMidiCCDialog()));

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
    connect(fileToolBar, SIGNAL(orientationChanged(Qt::Orientation)), this,
            SLOT(ftb_update_orientation(Qt::Orientation)));

    controlToolBar = new QToolBar(tr("&Control Toolbar"), this);
    controlToolBar->addAction(viewLogAction);
    controlToolBar->addAction(viewGrooveAction);
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
    QWidget *centWidget = new QWidget(this);
    setCentralWidget(centWidget);
    setDockNestingEnabled(true);
    updateWindowTitle();

    if (checkRcFile())
        readRcFile();

    if (!installSignalHandlers())
        qWarning("%s", "Signal handlers not installed!");

    show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateWindowTitle()
{
    if (filename.isEmpty())
        setWindowTitle(QString("%1 (%2)")
                .arg(APP_NAME)
                .arg(engine->getAlsaClientId()));
    else
        setWindowTitle(QString("%1 - %2  (%3)")
                .arg(filename)
                .arg(APP_NAME)
                .arg(engine->getAlsaClientId()));
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
    QString name;
    bool ok;

    name = QInputDialog::getText(this, APP_NAME,
            tr("Add MIDI Arpeggiator"), QLineEdit::Normal,
            tr("%1").arg(engine->midiArpCount() + 1), &ok);
    if (ok && !name.isEmpty()) {
        addArp("Arp:"+name);
    }
}

void MainWindow::lfoNew()
{
    QString name;
    bool ok;

    name = QInputDialog::getText(this, APP_NAME,
            tr("Add MIDI LFO"), QLineEdit::Normal,
            tr("%1").arg(engine->midiLfoCount() + 1), &ok);
    if (ok && !name.isEmpty()) {
        addLfo("LFO:"+name);
    }
}

void MainWindow::seqNew()
{
    QString name;
    bool ok;

    name = QInputDialog::getText(this, APP_NAME,
            tr("Add Step Sequencer"), QLineEdit::Normal,
            tr("%1").arg(engine->midiSeqCount() + 1), &ok);
    if (ok && !name.isEmpty()) {
        addSeq("Seq:"+name);
    }
}

void MainWindow::addArp(const QString& name)
{
    int count, widgetID;
    MidiArp *midiWorker = new MidiArp();
    engine->addMidiArp(midiWorker);
    ArpWidget *moduleWidget = new ArpWidget(midiWorker,
            engine->getPortCount(), passWidget->compactStyle,
            passWidget->mutedAdd, this);
    connect(midiWorker, SIGNAL(nextStep(int)),
            moduleWidget->screen, SLOT(updateScreen(int)));
    connect(moduleWidget, SIGNAL(presetsChanged(const QString&, const
                    QString&, int)),
            this, SLOT(updatePatternPresets(const QString&, const
                    QString&, int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleRemove(int)),
            this, SLOT(removeArp(int)));
    connect(moduleWidget->manageBox, SIGNAL(dockRename(const QString&, int)),
            this, SLOT(renameDock(const QString&, int)));
    connect(moduleWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            engine, SLOT(setMidiLearn(int, int, int)));

    connect(grooveWidget, SIGNAL(newGrooveTick(int)),
            moduleWidget->screen, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)),
            moduleWidget->screen, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)),
            moduleWidget->screen, SLOT(setGrooveLength(int)));

    widgetID = engine->arpWidgetCount();
    moduleWidget->manageBox->name = name;
    moduleWidget->manageBox->ID = widgetID;
    moduleWidget->midiControl->ID = widgetID;

    engine->addArpWidget(moduleWidget);
    engine->sendGroove();

    count = engine->moduleWindowCount();
    moduleWidget->manageBox->parentDockID = count;
    moduleWidget->midiControl->parentDockID = count;
    appendDock(moduleWidget, name, count);

    checkIfFirstModule();
}

void MainWindow::addLfo(const QString& name)
{
    int widgetID, count;
    MidiLfo *midiWorker = new MidiLfo();
    engine->addMidiLfo(midiWorker);
    LfoWidget *moduleWidget = new LfoWidget(midiWorker,
            engine->getPortCount(), passWidget->compactStyle,
            passWidget->mutedAdd, this);
    connect(midiWorker, SIGNAL(nextStep(int)),
            moduleWidget, SLOT(updateScreen(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleRemove(int)),
            this, SLOT(removeLfo(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleClone(int)), this, SLOT(cloneLfo(int)));
    connect(moduleWidget->manageBox, SIGNAL(dockRename(const QString&, int)),
            this, SLOT(renameDock(const QString&, int)));
    connect(moduleWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            engine, SLOT(setMidiLearn(int, int, int)));

    widgetID = engine->lfoWidgetCount();
    moduleWidget->manageBox->name = name;
    moduleWidget->manageBox->ID = widgetID;
    moduleWidget->midiControl->ID = widgetID;

    engine->addLfoWidget(moduleWidget);
    count = engine->moduleWindowCount();
    moduleWidget->manageBox->parentDockID = count;
    moduleWidget->midiControl->parentDockID = count;
    appendDock(moduleWidget, name, count);

    checkIfFirstModule();
}

void MainWindow::addSeq(const QString& name)
{
    int widgetID, count;
    MidiSeq *midiWorker = new MidiSeq();
    engine->addMidiSeq(midiWorker);
    SeqWidget *moduleWidget = new SeqWidget(midiWorker,
            engine->getPortCount(), passWidget->compactStyle,
            passWidget->mutedAdd, this);
    connect(midiWorker, SIGNAL(nextStep(int)),
            moduleWidget->screen, SLOT(updateScreen(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleRemove(int)), this, SLOT(removeSeq(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleClone(int)), this, SLOT(cloneSeq(int)));
    connect(moduleWidget->manageBox, SIGNAL(dockRename(const QString&, int)),
            this, SLOT(renameDock(const QString&, int)));
    connect(moduleWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            engine, SLOT(setMidiLearn(int, int, int)));
    connect(midiWorker, SIGNAL(noteEvent(int, int)),
            moduleWidget, SLOT(processNote(int, int)));

    widgetID = engine->seqWidgetCount();
    moduleWidget->manageBox->name = name;
    moduleWidget->manageBox->ID = widgetID;
    moduleWidget->midiControl->ID = widgetID;

    engine->addSeqWidget(moduleWidget);
    count = engine->moduleWindowCount();
    moduleWidget->manageBox->parentDockID = count;
    moduleWidget->midiControl->parentDockID = count;
    appendDock(moduleWidget, name, count);

    checkIfFirstModule();
}

void MainWindow::cloneLfo(int ID)
{
    int widgetID, count;
    MidiLfo *midiWorker = new MidiLfo();
    engine->addMidiLfo(midiWorker);
    LfoWidget *moduleWidget = new LfoWidget(midiWorker,
            engine->getPortCount(), passWidget->compactStyle,
            passWidget->mutedAdd, this);
    connect(midiWorker, SIGNAL(nextStep(int)),
            moduleWidget, SLOT(updateScreen(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleRemove(int)),
            this, SLOT(removeLfo(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleClone(int)), this, SLOT(cloneLfo(int)));
    connect(moduleWidget->manageBox, SIGNAL(dockRename(const QString&, int)),
            this, SLOT(renameDock(const QString&, int)));
    connect(moduleWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            engine, SLOT(setMidiLearn(int, int, int)));

    widgetID = engine->lfoWidgetCount();
    moduleWidget->manageBox->name = engine->lfoWidget(ID)->manageBox->name + "_0";
    moduleWidget->manageBox->ID = widgetID;
    moduleWidget->midiControl->ID = widgetID;

    moduleWidget->copyParamsFrom(engine->lfoWidget(ID));

    engine->addLfoWidget(moduleWidget);
    count = engine->moduleWindowCount();
    moduleWidget->manageBox->parentDockID = count;
    moduleWidget->midiControl->parentDockID = count;
    appendDock(moduleWidget, moduleWidget->manageBox->name, count);

    checkIfFirstModule();
}

void MainWindow::cloneSeq(int ID)
{
    int widgetID, count;
    QString name;
    MidiSeq *midiWorker = new MidiSeq();

    engine->addMidiSeq(midiWorker);
    SeqWidget *moduleWidget = new SeqWidget(midiWorker,
            engine->getPortCount(), passWidget->compactStyle,
            passWidget->mutedAdd, this);
    connect(midiWorker, SIGNAL(nextStep(int)),
            moduleWidget->screen, SLOT(updateScreen(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleRemove(int)), this, SLOT(removeSeq(int)));
    connect(moduleWidget->manageBox, SIGNAL(moduleClone(int)), this, SLOT(cloneSeq(int)));
    connect(moduleWidget->manageBox, SIGNAL(dockRename(const QString&, int)),
            this, SLOT(renameDock(const QString&, int)));
    connect(moduleWidget->midiControl, SIGNAL(setMidiLearn(int, int, int)),
            engine, SLOT(setMidiLearn(int, int, int)));
    connect(midiWorker, SIGNAL(noteEvent(int, int)),
            moduleWidget, SLOT(processNote(int, int)));

    widgetID = engine->seqWidgetCount();
    moduleWidget->manageBox->name = engine->seqWidget(ID)->manageBox->name + "_0";
    moduleWidget->manageBox->ID = widgetID;
    moduleWidget->midiControl->ID = widgetID;

    moduleWidget->copyParamsFrom(engine->seqWidget(ID));

    engine->addSeqWidget(moduleWidget);
    count = engine->moduleWindowCount();
    moduleWidget->manageBox->parentDockID = count;
    moduleWidget->midiControl->parentDockID = count;
    appendDock(moduleWidget, moduleWidget->manageBox->name, count);
}

void MainWindow::appendDock(QWidget *moduleWidget, const QString &name, int count)
{
    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(moduleWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    if (passWidget->compactStyle) moduleWindow->setStyleSheet(COMPACT_STYLE);

    if (count) tabifyDockWidget(engine->moduleWindow(count - 1), moduleWindow);
    engine->addModuleWindow(moduleWindow);
}

void MainWindow::renameDock(const QString& name, int parentDockID)
{
    engine->moduleWindow(parentDockID)->setWindowTitle(name);
    engine->setModified(true);
}

void MainWindow::removeArp(int index)
{
    int parentDockID;
    ArpWidget *arpWidget = engine->arpWidget(index);

    parentDockID = arpWidget->manageBox->parentDockID;
    QDockWidget *dockWidget = engine->moduleWindow(parentDockID);

    engine->removeMidiArp(arpWidget->getMidiWorker());
    engine->removeArpWidget(arpWidget);
    delete arpWidget;
    engine->removeModuleWindow(dockWidget);
    engine->updateIDs(parentDockID);
    checkIfLastModule();
}

void MainWindow::removeLfo(int index)
{
    int parentDockID;
    LfoWidget *lfoWidget = engine->lfoWidget(index);

    parentDockID = lfoWidget->manageBox->parentDockID;
    QDockWidget *dockWidget = engine->moduleWindow(parentDockID);

    engine->removeMidiLfo(lfoWidget->getMidiWorker());
    engine->removeLfoWidget(lfoWidget);
    delete lfoWidget;
    engine->removeModuleWindow(dockWidget);
    engine->updateIDs(parentDockID);
    checkIfLastModule();
}

void MainWindow::removeSeq(int index)
{
    int parentDockID;
    SeqWidget *seqWidget = engine->seqWidget(index);

    parentDockID = seqWidget->manageBox->parentDockID;
    QDockWidget *dockWidget = engine->moduleWindow(parentDockID);

    engine->removeMidiSeq(seqWidget->getMidiWorker());
    engine->removeSeqWidget(seqWidget);
    delete seqWidget;
    engine->removeModuleWindow(dockWidget);
    engine->updateIDs(parentDockID);
    checkIfLastModule();
}

void MainWindow::clear()
{
    updateRunQueue(false);
    jackSyncToggle(false);

    while (engine->midiArpCount()) {
        removeArp(engine->midiArpCount() - 1);
    }

    while (engine->midiLfoCount()) {
        removeLfo(engine->midiLfoCount() - 1);
    }

    while (engine->midiSeqCount()) {
        removeSeq(engine->midiSeqCount() - 1);
    }
}

void MainWindow::fileNew()
{
    if (isSave()) {
        clear();
        filename = "";
        updateWindowTitle();
        engine->setModified(false);
    }
}

void MainWindow::fileOpen()
{
    if (isSave())
        chooseFile();
}

void MainWindow::chooseFile()
{
    QString fn =  QFileDialog::getOpenFileName(this,
            tr("Open arpeggiator file"), lastDir,
            tr("QMidiArp XML files")  + " (*" + FILEEXT + ");;"
            + tr("Old QMidiArp files") + " (*.qma)");
    if (fn.isEmpty())
        return;

    if (fn.endsWith(".qma"))
        openTextFile(fn);

    else if (fn.endsWith(FILEEXT))
        openFile(fn);
}

void MainWindow::openFile(const QString& fn)
{

    lastDir = fn.left(fn.lastIndexOf('/'));

    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, APP_NAME,
                tr("Could not read from file '%1'.").arg(fn));
        return;
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
            while (!xml.atEnd()) {
                xml.readNext();

                if (xml.isEndElement())
                    break;

                if ((xml.isStartElement()) && (xml.name() == "global"))
                    readFilePartGlobal(xml);
                else if (xml.isStartElement() && (xml.name() == "modules"))
                    readFilePartModules(xml);
                else if (xml.isStartElement() && (xml.name() == "GUI"))
                    readFilePartGUI(xml);
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
                    passWidget->cbuttonCheck->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "midiClockEnabled")
                        midiClockAction->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "jackSyncEnabled")
                        jackSyncAction->setChecked(xml.readElementText().toInt());
                else if (xml.name() == "forwardUnmatched")
                    passWidget->setForward(xml.readElementText().toInt());
                else if (xml.name() == "forwardPort")
                    passWidget->setPortUnmatched(xml.readElementText().toInt());
                else skipXmlElement(xml);
            }
        }
        else if (xml.isStartElement() && (xml.name() == "groove")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "tick")
                    grooveWidget->grooveTick->setValue(xml.readElementText().toInt());
                else if (xml.name() == "velocity")
                    grooveWidget->grooveVelocity->setValue(xml.readElementText().toInt());
                else if (xml.name() == "length")
                    grooveWidget->grooveLength->setValue(xml.readElementText().toInt());
                else if (xml.isStartElement() && (xml.name() == "midiControllers")) {
                    grooveWidget->midiControl->readData(xml);
                }
                else skipXmlElement(xml);
            }
        }
        else skipXmlElement(xml);
    }
}

void MainWindow::readFilePartModules(QXmlStreamReader& xml)
{
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;
        if (xml.isStartElement() && (xml.name() == "Arp")) {
            addArp("Arp:" + xml.attributes().value("name").toString());
            engine->arpWidget(engine->midiArpCount() - 1)
                    ->readData(xml);
        }
        else if (xml.isStartElement() && (xml.name() == "LFO")) {
            addLfo("LFO:" + xml.attributes().value("name").toString());
            engine->lfoWidget(engine->midiLfoCount() - 1)
                    ->readData(xml);
        }
        else if (xml.isStartElement() && (xml.name() == "Seq")) {
            addSeq("Seq:" + xml.attributes().value("name").toString());
            engine->seqWidget(engine->midiSeqCount() - 1)
                    ->readData(xml);
        }
        else skipXmlElement(xml);
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

void MainWindow::openTextFile(const QString& fn)
{
    QString line, qs, qs2;
    bool midiclocktmp = false;
    int c = 0;

    lastDir = fn.left(fn.lastIndexOf('/'));

    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, APP_NAME,
                tr("Could not read from file '%1'.").arg(fn));
        return;
    }

    clear();
    filename = fn;

    QTextStream loadText(&f);
    qs = loadText.readLine();
    if (qs == "Tempo")
    {
        qs = loadText.readLine();
        tempoSpin->setValue(qs.toInt());
        qs = loadText.readLine();
    }
    if (qs == "MIDI Control")
    {
        qs = loadText.readLine();
        qs2 = qs.section(' ', 0, 0);
        passWidget->cbuttonCheck->setChecked(qs2.toInt());
        qs = loadText.readLine();
    }
    if (qs == "MIDI Clock")
    {
        qs = loadText.readLine();
        qs2 = qs.section(' ', 0, 0);
        midiclocktmp = qs2.toInt();
        qs = loadText.readLine();
    }
    qs2 = qs.section(' ', 0, 0);
    passWidget->setForward(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    passWidget->setPortUnmatched(qs2.toInt() + 1);
    qs = loadText.readLine();
    qs2 = qs.section(' ', 0, 0);

    grooveWidget->grooveTick->setValue(qs2.toInt());
    //  engine->seqDriver->setGrooveTick(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    grooveWidget->grooveVelocity->setValue(qs2.toInt());
    //  engine->seqDriver->setGrooveVelocity(qs2.toInt());
    qs2 = qs.section(' ', 2, 2);
    grooveWidget->grooveLength->setValue(qs2.toInt());
    //  engine->seqDriver->setGrooveLength(qs2.toInt());

    while (!loadText.atEnd()) {
        qs = loadText.readLine();
        if (qs.startsWith("GUI"))
            break;
        if (qs.startsWith("Seq:"))
            c = 1;
        if (qs.startsWith("LFO:"))
            c = 2;
        if (qs.startsWith("Arp:"))
            c = 3;

        switch (c) {
            case 1:
                addSeq(qs);
                engine->seqWidget(engine->midiSeqCount() - 1)->readDataText(loadText);
            break;
            case 2:
                addLfo(qs);
                engine->lfoWidget(engine->midiLfoCount() - 1)->readDataText(loadText);
            break;
            case 3:
                addArp(qs);
                engine->arpWidget(engine->midiArpCount() - 1)->readDataText(loadText);
            break;
            default:
                qs = "Arp: " + qs;
                addArp(qs);
                engine->arpWidget(engine->midiArpCount() - 1)->readDataText(loadText);
            break;
        }
    }

    if (qs.startsWith("GUI")) {
        qs = loadText.readLine();
        QByteArray array = QByteArray::fromHex(qs.toLatin1());
        restoreState(array);
    }

    midiClockAction->setChecked(midiclocktmp);

    filename.append("x");
    QMessageBox::warning(this, APP_NAME,
            tr("The QMidiArp text file was imported. If you save this file, \
it will be saved using the newer xml format under the name\n '%1'.").arg(filename));

    updateWindowTitle();
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
    int l1;
    int ns = 0;
    int nl = 0;
    int na = 0;

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
    xml.writeAttribute("name", filename.mid(filename.lastIndexOf('/') + 1,
                    filename.count() - filename.lastIndexOf('/') - 6));

    xml.writeStartElement("global");

        xml.writeTextElement("tempo", QString::number(tempoSpin->value()));

        xml.writeStartElement("settings");
            xml.writeTextElement("midiControlEnabled",
                QString::number((int)passWidget->cbuttonCheck->isChecked()));
            xml.writeTextElement("midiClockEnabled",
                QString::number((int)engine->seqDriver->use_midiclock));
            xml.writeTextElement("jackSyncEnabled",
                QString::number((int)engine->seqDriver->use_jacksync));
            xml.writeTextElement("forwardUnmatched",
                QString::number((int)engine->seqDriver->forwardUnmatched));
            xml.writeTextElement("forwardPort",
                QString::number(engine->seqDriver->portUnmatched));
        xml.writeEndElement();

        xml.writeStartElement("groove");
            xml.writeTextElement("tick",
                QString::number(engine->grooveTick));
            xml.writeTextElement("velocity",
                QString::number(engine->grooveVelocity));
            xml.writeTextElement("length",
                QString::number(engine->grooveLength));
            grooveWidget->midiControl->writeData(xml);
        xml.writeEndElement();

    xml.writeEndElement();

    xml.writeStartElement("modules");

    for (l1 = 0; l1 < engine->moduleWindowCount(); l1++) {

        nameTest = engine->moduleWindow(l1)->objectName();

        if (nameTest.startsWith('S')) {
            engine->seqWidget(ns)->writeData(xml);
            ns++;
        }
        if (nameTest.startsWith('L')) {
            engine->lfoWidget(nl)->writeData(xml);
            nl++;
        }
        if (nameTest.startsWith('A')) {
            engine->arpWidget(na)->writeData(xml);
            na++;
        }
    }

    xml.writeEndElement();

    xml.writeStartElement("GUI");
        xml.writeTextElement("windowState", saveState().toHex());
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();


    engine->setModified(false);
    return true;
}

void MainWindow::fileSaveAs()
{
    saveFileAs();
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
    if (isSave()) {
        writeRcFile();
        e->accept();
    } else
        e->ignore();
}

bool MainWindow::isModified()
{
    return engine->isModified();
}

void MainWindow::updateTempo(int p_tempo)
{
    engine->seqDriver->setQueueTempo(p_tempo);
    engine->setModified(true);
}

void MainWindow::updateRunQueue(bool on)
{
    emit(runQueue(on));
    tempoSpin->setDisabled(on);
}

void MainWindow::resetQueue()
{
    engine->seqDriver->setQueueStatus(engine->seqDriver->runArp);
}

void MainWindow::midiClockToggle(bool on)
{
    if (on) jackSyncAction->setChecked(false);
    engine->seqDriver->setUseMidiClock(on);
    setGUIforExtSync(on);
}

void MainWindow::jackSyncToggle(bool on)
{
    if (on) midiClockAction->setChecked(false);
    setGUIforExtSync(on);
    engine->seqDriver->setUseJackTransport(on);
}

void MainWindow::setGUIforExtSync(bool on)
{
    runAction->setDisabled(on);
    tempoSpin->setDisabled(on);
    addArpAction->setDisabled(on);
    addLfoAction->setDisabled(on);
    addSeqAction->setDisabled(on);
    fileOpenAction->setDisabled(on);
    fileRecentlyOpenedFiles->setDisabled(on);
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
                passWidget->compactStyleCheck->setChecked(value.at(1).toInt());
            else if ((value.at(0) == "#MutedAdd"))
                passWidget->mutedAddCheck->setChecked(value.at(1).toInt());
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
        writeText << qPrintable(patternPresets.at(l1)) << endl;
    }

    writeText << "#CompactStyle%";
    writeText << passWidget->compactStyle << endl;
    writeText << "#MutedAdd%";
    writeText << passWidget->mutedAdd << endl;
    writeText << "#EnableLog%";
    writeText << logWidget->enableLog->isChecked() << endl;
    writeText << "#LogMidiClock%";
    writeText << logWidget->logMidiClock->isChecked() << endl;
    writeText << "#GUIState%";
    writeText << saveState().toHex() << endl;

    writeText << "#LastDir%";
    writeText << lastDir << endl;

    // save recently opened files (all recent files code taken from AMS)
    if (recentFiles.count() > 0) {
        QStringList::Iterator it = recentFiles.begin();
        for (; it != recentFiles.end(); ++it) {
            writeText << "#RecentFile%";
            writeText << *it << endl;
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
    } else {
        fileRecentlyOpenedFiles->setEnabled(false);
    }
}

void MainWindow::recentFileActivated(QAction *action)
{
    if (!action->text().isEmpty()) {
        if (isSave())
            openFile(action->text());
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

void MainWindow::appendRecentlyOpenedFile(const QString &fn, QStringList &lst)
{
    QFileInfo fi(fn);
    if (lst.contains(fi.absoluteFilePath()))
        return;
    if (lst.count() >= 6 )
        lst.removeFirst();

    lst.append(fi.absoluteFilePath());
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
    if (!engine->moduleWindowCount()) {
        runAction->setDisabled(true);
        runAction->setChecked(false);
        midiClockAction->setDisabled(true);
        midiClockAction->setChecked(false);
        jackSyncAction->setDisabled(true);
        jackSyncAction->setChecked(false);
    }
}

void MainWindow::checkIfFirstModule()
{
    if (engine->moduleWindowCount() == 1) {
        midiClockAction->setEnabled(true);
        jackSyncAction->setEnabled(true);
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

    return true;
}

void MainWindow::signalAction(int fd)
{
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
            close();
            break;

        default:
            qWarning("Unexpected signal received: %d", message);
            break;
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
