#include <QBoxLayout>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
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

    arpData = new ArpData(this);
    arpData->registerPorts(p_portCount);
                
    midiCCTable = new MidiCCTable(arpData, this);
    
    logWidget = new LogWidget(this);
    logWindow = new QDockWidget(tr("Event Log"), this);
    logWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    logWindow->setWidget(logWidget);
    logWindow->setObjectName("logWidget");
    addDockWidget(Qt::BottomDockWidgetArea, logWindow);
    connect(arpData->seqDriver, SIGNAL(midiEvent(snd_seq_event_t *)), 
            logWidget, SLOT(appendEvent(snd_seq_event_t *)));

    passWidget = new PassWidget(arpData, p_portCount, this);
   
    connect(this, SIGNAL(runQueue(bool)), 
            arpData->seqDriver, SLOT(runQueue(bool)));                                
    grooveWidget = new GrooveWidget(this);
    grooveWindow = new QDockWidget(tr("Groove"), this);
    grooveWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    grooveWindow->setWidget(grooveWidget);;
    grooveWindow->setObjectName("grooveWidget");
    grooveWindow->setVisible(true);
    addDockWidget(Qt::BottomDockWidgetArea, grooveWindow);
    connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
            arpData->seqDriver, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
            arpData->seqDriver, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
            arpData->seqDriver, SLOT(setGrooveLength(int)));

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

    runAction = new QAction(QIcon(play_xpm), tr("&Run"), this);
    connect(runAction, SIGNAL(toggled(bool)), this, SLOT(updateRunQueue(bool)));
    runAction->setCheckable(true);
    runAction->setChecked(false);
    runAction->setDisabled(true);

    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(100);
    tempoSpin->setKeyboardTracking(false);
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
    connect(arpData->seqDriver, SIGNAL(jackShutdown(bool)), 
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
    QMenu *arpMenu = new QMenu(tr("&Module"), this);
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

    controlToolBar = new QToolBar(tr("&Control Toolbar"), this);
    controlToolBar->addAction(viewLogAction);
    controlToolBar->addAction(viewGrooveAction);
    controlToolBar->addSeparator();
    controlToolBar->addAction(addArpAction);
    controlToolBar->addAction(addLfoAction);
    controlToolBar->addAction(addSeqAction);
    controlToolBar->addSeparator();
    controlToolBar->addAction(runAction);
    controlToolBar->addWidget(tempoSpin);
    controlToolBar->addAction(midiClockAction);
    controlToolBar->addAction(jackSyncAction);
    controlToolBar->setObjectName("controlToolBar");
    controlToolBar->setMaximumHeight(30);

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
    updateWindowTitle();
    seqEventLocked = false;

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
                .arg(arpData->getAlsaClientId()));
    else
        setWindowTitle(QString("%1 - %2  (%3)")
                .arg(filename)
                .arg(APP_NAME)
                .arg(arpData->getAlsaClientId()));
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
            tr("%1").arg(arpData->midiArpCount() + 1), &ok);
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
            tr("%1").arg(arpData->midiLfoCount() + 1), &ok);
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
            tr("%1").arg(arpData->midiSeqCount() + 1), &ok);
    if (ok && !name.isEmpty()) {
        addSeq("Seq:"+name);
    }
}

void MainWindow::addArp(const QString& name)
{
    int count, widgetID;
    MidiArp *midiArp = new MidiArp();
    arpData->addMidiArp(midiArp);   
    ArpWidget *arpWidget = new ArpWidget(midiArp,
            arpData->getPortCount(), passWidget->compactStyle, this);
    // passing compactStyle property was necessary because stylesheet
    // seems to have no effect on layout spacing/margin
    connect(arpData->seqDriver, SIGNAL(nextStep(int)),
            arpWidget->arpScreen, SLOT(updateArpScreen(int)));
    connect(arpWidget, SIGNAL(presetsChanged(const QString&, const
                    QString&, int)), 
            this, SLOT(updatePatternPresets(const QString&, const
                    QString&, int)));
    connect(arpWidget, SIGNAL(arpRemove(int)), 
            this, SLOT(removeArp(int)));
    connect(arpWidget, SIGNAL(dockRename(const QString&, int)), 
            this, SLOT(renameDock(const QString&, int)));
    connect(arpWidget, SIGNAL(setMidiLearn(int, int, int)), 
            arpData, SLOT(setMidiLearn(int, int, int)));

    connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
            arpWidget->arpScreen, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
            arpWidget->arpScreen, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
            arpWidget->arpScreen, SLOT(setGrooveLength(int)));
            
    widgetID = arpData->arpWidgetCount();
    arpWidget->name = name;
    arpWidget->ID = widgetID;
    
    arpData->addArpWidget(arpWidget);
    arpData->seqDriver->sendGroove();
    
    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(arpWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    if (passWidget->compactStyle) moduleWindow->setStyleSheet(COMPACT_STYLE);
    
    count = arpData->moduleWindowCount();
    arpWidget->parentDockID = count;
    if (count) tabifyDockWidget(arpData->moduleWindow(count - 1), moduleWindow);
    arpData->addModuleWindow(moduleWindow);
    moduleWindow->show();
    checkIfFirstModule();
}

void MainWindow::addLfo(const QString& name)
{
    int count, widgetID;
    MidiLfo *midiLfo = new MidiLfo();
    arpData->addMidiLfo(midiLfo);   
    LfoWidget *lfoWidget = new LfoWidget(midiLfo,
            arpData->getPortCount(), passWidget->compactStyle, this);
    connect(lfoWidget, SIGNAL(lfoRemove(int)), 
            this, SLOT(removeLfo(int)));
    connect(lfoWidget, SIGNAL(dockRename(const QString&, int)), 
            this, SLOT(renameDock(const QString&, int)));
    connect(lfoWidget, SIGNAL(setMidiLearn(int, int, int)), 
            arpData, SLOT(setMidiLearn(int, int, int)));

    widgetID = arpData->lfoWidgetCount();
    lfoWidget->name = name;
    lfoWidget->ID = widgetID;

    arpData->addLfoWidget(lfoWidget);

    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(lfoWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    if (passWidget->compactStyle) moduleWindow->setStyleSheet(COMPACT_STYLE);
    
    count = arpData->moduleWindowCount();
    lfoWidget->parentDockID = count;
    if (count) tabifyDockWidget(arpData->moduleWindow(count - 1), moduleWindow);
    arpData->addModuleWindow(moduleWindow);
    checkIfFirstModule();
}

void MainWindow::addSeq(const QString& name)
{
    int count, widgetID;
    MidiSeq *midiSeq = new MidiSeq();
    arpData->addMidiSeq(midiSeq);   
    SeqWidget *seqWidget = new SeqWidget(midiSeq,
            arpData->getPortCount(), passWidget->compactStyle, this);
    connect(seqWidget, SIGNAL(seqRemove(int)), this, SLOT(removeSeq(int)));
    connect(seqWidget, SIGNAL(dockRename(const QString&, int)), 
            this, SLOT(renameDock(const QString&, int)));
    connect(seqWidget, SIGNAL(setMidiLearn(int, int, int)),
            arpData, SLOT(setMidiLearn(int, int, int)));
    connect(arpData->seqDriver, SIGNAL(noteEvent(int, int)),
            seqWidget, SLOT(processNote(int, int)));
            
    widgetID = arpData->seqWidgetCount();
    seqWidget->name = name;
    seqWidget->ID = widgetID;
    
    arpData->addSeqWidget(seqWidget);
    
    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(seqWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    if (passWidget->compactStyle) moduleWindow->setStyleSheet(COMPACT_STYLE);

    count = arpData->moduleWindowCount();
    seqWidget->parentDockID = count;
    if (count) tabifyDockWidget(arpData->moduleWindow(count - 1), moduleWindow);
    arpData->addModuleWindow(moduleWindow);

    checkIfFirstModule();
}

void MainWindow::renameDock(const QString& name, int parentDockID) 
{
    arpData->moduleWindow(parentDockID)->setWindowTitle(name);
    arpData->setModified(true);
}

void MainWindow::removeArp(int index)
{
    int parentDockID;
    ArpWidget *arpWidget = arpData->arpWidget(index);
    
    parentDockID = arpWidget->parentDockID;
    QDockWidget *dockWidget = arpData->moduleWindow(parentDockID);
    
    arpData->removeMidiArp(arpWidget->getMidiArp());
    arpData->removeArpWidget(arpWidget);
    delete arpWidget;
    arpData->removeModuleWindow(dockWidget);
    arpData->updateIDs(parentDockID);
    checkIfLastModule();
}

void MainWindow::removeLfo(int index)
{
    int parentDockID;
    LfoWidget *lfoWidget = arpData->lfoWidget(index);
    
    parentDockID = lfoWidget->parentDockID;
    QDockWidget *dockWidget = arpData->moduleWindow(parentDockID);
    
    arpData->removeMidiLfo(lfoWidget->getMidiLfo());
    arpData->removeLfoWidget(lfoWidget);
    delete lfoWidget;
    arpData->removeModuleWindow(dockWidget);
    arpData->updateIDs(parentDockID);
    checkIfLastModule();
}

void MainWindow::removeSeq(int index)
{
    int parentDockID;
    SeqWidget *seqWidget = arpData->seqWidget(index);
    
    parentDockID = seqWidget->parentDockID;
    QDockWidget *dockWidget = arpData->moduleWindow(parentDockID);
    
    arpData->removeMidiSeq(seqWidget->getMidiSeq());
    arpData->removeSeqWidget(seqWidget);
    delete seqWidget;
    arpData->removeModuleWindow(dockWidget);
    arpData->updateIDs(parentDockID);
    checkIfLastModule();
}

void MainWindow::clear()
{
    updateRunQueue(false);
    jackSyncToggle(false);
    
    while (arpData->midiArpCount()) {
        removeArp(arpData->midiArpCount() - 1);
    }

    while (arpData->midiLfoCount()) {
        removeLfo(arpData->midiLfoCount() - 1);
    }
    
    while (arpData->midiSeqCount()) {
        removeSeq(arpData->midiSeqCount() - 1);
    }
}

void MainWindow::fileNew()
{
    if (isSave()) {
        clear();
        filename = "";
        updateWindowTitle();
        arpData->setModified(false);
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
    arpData->setModified(false);    
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
                else if (xml.name() == "midiClockRate")
                    passWidget->mtpbSpin->setValue(xml.readElementText().toInt());
                else if (xml.name() == "forwardUnmatched")
                    passWidget->setForward(xml.readElementText().toInt());
                else if (xml.name() == "forwardPort")
                    passWidget->setPortUnmatched(xml.readElementText().toInt() + 1);
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
            arpData->arpWidget(arpData->midiArpCount() - 1)
                    ->readArp(xml);
        }   
        else if (xml.isStartElement() && (xml.name() == "LFO")) {
            addLfo("LFO:" + xml.attributes().value("name").toString());
            arpData->lfoWidget(arpData->midiLfoCount() - 1)
                    ->readLfo(xml);
        }   
        else if (xml.isStartElement() && (xml.name() == "Seq")) {
            addSeq("Seq:" + xml.attributes().value("name").toString());
            arpData->seqWidget(arpData->midiSeqCount() - 1)
                    ->readSeq(xml);
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
        qs2 = qs.section(' ', 1, 1);
        passWidget->mtpbSpin->setValue(qs2.toInt());
        qs = loadText.readLine();
    }
    qs2 = qs.section(' ', 0, 0);
    passWidget->setForward(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    passWidget->setPortUnmatched(qs2.toInt() + 1);
    qs = loadText.readLine();
    qs2 = qs.section(' ', 0, 0);

    grooveWidget->grooveTick->setValue(qs2.toInt());
    //  arpData->seqDriver->setGrooveTick(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    grooveWidget->grooveVelocity->setValue(qs2.toInt());
    //  arpData->seqDriver->setGrooveVelocity(qs2.toInt());
    qs2 = qs.section(' ', 2, 2);
    grooveWidget->grooveLength->setValue(qs2.toInt());
    //  arpData->seqDriver->setGrooveLength(qs2.toInt());
 
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
                arpData->seqWidget(arpData->midiSeqCount() - 1)->readSeqText(loadText);
            break;
            case 2:
                addLfo(qs);
                arpData->lfoWidget(arpData->midiLfoCount() - 1)->readLfoText(loadText);
            break;
            case 3:
                addArp(qs);
                arpData->arpWidget(arpData->midiArpCount() - 1)->readArpText(loadText);
            break;
            default:
                qs = "Arp: " + qs;
                addArp(qs);
                arpData->arpWidget(arpData->midiArpCount() - 1)->readArpText(loadText);
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
                QString::number((int)arpData->seqDriver->use_midiclock));
            xml.writeTextElement("jackSyncEnabled", 
                QString::number((int)arpData->seqDriver->use_jacksync));
            xml.writeTextElement("midiClockRate", 
                QString::number(arpData->seqDriver->midiclock_tpb));
            xml.writeTextElement("forwardUnmatched", 
                QString::number((int)arpData->seqDriver->forwardUnmatched));
            xml.writeTextElement("forwardPort", 
                QString::number(arpData->seqDriver->portUnmatched));
        xml.writeEndElement();
        
        xml.writeStartElement("groove");
            xml.writeTextElement("tick", 
                QString::number(arpData->seqDriver->grooveTick));
            xml.writeTextElement("velocity", 
                QString::number(arpData->seqDriver->grooveVelocity));
            xml.writeTextElement("length", 
                QString::number(arpData->seqDriver->grooveLength));
        xml.writeEndElement();
        
    xml.writeEndElement();
   
    xml.writeStartElement("modules");
  
    for (l1 = 0; l1 < arpData->moduleWindowCount(); l1++) {
        
        nameTest = arpData->moduleWindow(l1)->objectName();
        
        if (nameTest.startsWith('S')) {
            arpData->seqWidget(ns)->writeSeq(xml);
            ns++;
        } 
        if (nameTest.startsWith('L')) {
            arpData->lfoWidget(nl)->writeLfo(xml);
            nl++;
        }
        if (nameTest.startsWith('A')) {
            arpData->arpWidget(na)->writeArp(xml);
            na++;
        }
    }
    
    xml.writeEndElement();
    
    xml.writeStartElement("GUI");    
        xml.writeTextElement("windowState", saveState().toHex());
    xml.writeEndElement();
    
    xml.writeEndElement();
    xml.writeEndDocument();
    

    arpData->setModified(false);
    return true;
}

bool MainWindow::saveTextFile()
{
    int l1;
    int ns = 0;
    int nl = 0;
    int na = 0;
    
    QString nameTest;
    QFile f(filename);

    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, APP_NAME,
                tr("Could not write to file '%1'.").arg(filename));
        return false;
    }
    
    
    QTextStream saveText(&f);
    saveText << "Tempo\n";   
    saveText << tempoSpin->value() << '\n';    
    saveText << "MIDI Control\n";
    saveText << (int)passWidget->cbuttonCheck->isChecked() << '\n';
    saveText << "MIDI Clock\n";
    saveText << (int)arpData->seqDriver->use_midiclock << ' ';
    saveText << (int)arpData->seqDriver->midiclock_tpb << '\n';
    saveText << (int)arpData->seqDriver->forwardUnmatched;
    saveText << ' ' << arpData->seqDriver->portUnmatched << '\n';
    
    saveText << arpData->seqDriver->grooveTick;
    saveText << ' ' << arpData->seqDriver->grooveVelocity;
    saveText << ' ' << arpData->seqDriver->grooveLength << '\n';

    for (l1 = 0; l1 < arpData->moduleWindowCount(); l1++) {
        
        nameTest = arpData->moduleWindow(l1)->objectName();
        
        if (nameTest.startsWith('S')) {
            saveText << qPrintable(arpData->seqWidget(ns)->name) << '\n';
            arpData->seqWidget(ns)->writeSeqText(saveText);
            ns++;
        } 
        if (nameTest.startsWith('L')) {
            saveText << qPrintable(arpData->lfoWidget(nl)->name) << '\n';
            arpData->lfoWidget(nl)->writeLfoText(saveText);
            nl++;
        }
        if (nameTest.startsWith('A')) {
            saveText << qPrintable(arpData->arpWidget(na)->name) << '\n';
            arpData->arpWidget(na)->writeArpText(saveText);
            na++;
        }
    }
    saveText << "GUI Settings\n";
    saveText << saveState().toHex();
    arpData->setModified(false);
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
    return arpData->isModified();
}

void MainWindow::updateTempo(int p_tempo)
{
    arpData->seqDriver->setQueueTempo(p_tempo);
    arpData->setModified(true);
}

void MainWindow::updateRunQueue(bool on)
{
    emit(runQueue(on));
    tempoSpin->setDisabled(on);
}

void MainWindow::resetQueue()
{
    arpData->seqDriver->runQueue(arpData->seqDriver->runArp);
}

void MainWindow::midiClockToggle(bool on)
{
    if (on) jackSyncAction->setChecked(false);
    arpData->seqDriver->setUseMidiClock(on);
    setGUIforExtSync(on);
}

void MainWindow::jackSyncToggle(bool on)
{
    if (on) midiClockAction->setChecked(false);
    setGUIforExtSync(on);
    arpData->seqDriver->setUseJackTransport(on);
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
    arpData->updatePatternPresets(n, p, index);
    writeRcFile();
}

void MainWindow::checkIfLastModule()
{
    if (!arpData->moduleWindowCount()) {
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
    if (arpData->moduleWindowCount() == 1) {
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

/* Handler for system signals (SIGUSR1, SIGINT...)
 * Write a message to the pipe and leave as soon as possible
 */
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

/* Slot to give response to the incoming pipe message */
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
