#include <QBoxLayout>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QStringList>
#include <QSpinBox>
#include <QTextStream>

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
#include "pixmaps/fileopen.xpm"
#include "pixmaps/filenew.xpm"
#include "pixmaps/filesave.xpm"
#include "pixmaps/filesaveas.xpm"
#include "pixmaps/filequit.xpm"

static const char FILEEXT[] = ".qma";

MainWindow::MainWindow(int p_portCount)
{
    filename = "";
    lastDir = QDir::homePath();

    arpData = new ArpData(this);
    arpData->registerPorts(p_portCount);
            
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

    passWidget = new PassWidget(p_portCount, this);
    passWindow = new QDockWidget(tr("Settings"), this);
    passWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    passWindow->setWidget(passWidget);
    passWindow->setObjectName("passWidget");
    passWindow->setVisible(false);
    passWindow->setFloating(true);
    passWindow->setGeometry(10, 10, 400, 200);
    addDockWidget(Qt::BottomDockWidgetArea, passWindow);


    connect(passWidget, SIGNAL(forwardToggled(bool)), 
            arpData->seqDriver, SLOT(setForwardUnmatched(bool)));
    connect(passWidget, SIGNAL(newMIDItpb(int)), 
            arpData->seqDriver, SLOT(updateMIDItpb(int)));
    connect(passWidget, SIGNAL(newPortUnmatched(int)), 
            arpData->seqDriver, SLOT(setPortUnmatched(int)));
    connect(passWidget, SIGNAL(midiMuteToggle(bool)), 
            arpData->seqDriver, SLOT(setMidiMutable(bool)));
    connect(passWidget, SIGNAL(newCnumber(int)), 
            arpData, SLOT(updateCCnumber(int)));

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

    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(100);
    tempoSpin->setKeyboardTracking(false);
    connect(tempoSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateTempo(int)));

    midiClockAction = new QAction(QIcon(midiclock_xpm), 
            tr("&Use incoming MIDI Clock"), this);
    connect(midiClockAction, SIGNAL(toggled(bool)), this,
            SLOT(midiClockToggle(bool)));

    midiClockAction->setCheckable(true);
    midiClockAction->setChecked(false);
    midiClockAction->setDisabled(true);
    runAction->setCheckable(true);
    runAction->setChecked(false);
    runAction->setDisabled(true);
    updateRunQueue(false);

    QAction* viewLogAction = logWindow->toggleViewAction();
    viewLogAction->setIcon(QIcon(eventlog_xpm));
    viewLogAction->setText(tr("&Event Log"));
    viewLogAction->setShortcut(QKeySequence(tr("Ctrl+H", "View|Event Log")));

    QAction* viewGrooveAction = grooveWindow->toggleViewAction();
    viewGrooveAction->setIcon(QIcon(groovetog_xpm));
    viewGrooveAction->setText(tr("&Groove Settings"));
    viewGrooveAction->setShortcut(QKeySequence(tr("Ctrl+G", "View|Groove")));

    QAction* viewSettingsAction = passWindow->toggleViewAction();
    viewSettingsAction->setIcon(QIcon(settings_xpm));
    viewSettingsAction->setText(tr("&Settings"));
    viewSettingsAction->setShortcut(QKeySequence(tr("Ctrl+P",
                    "View|Settings")));

    QMenuBar *menuBar = new QMenuBar; 
    QMenu *fileMenu = new QMenu(tr("&File"), this); 
    QMenu *viewMenu = new QMenu(tr("&View"), this);
    QMenu *arpMenu = new QMenu(tr("&Module"), this);
    QMenu *helpMenu = new QMenu(tr("&Help"), this);

    fileMenu->addAction(fileNewAction);
    fileMenu->addAction(fileOpenAction);
    fileMenu->addAction(fileSaveAction);
    fileMenu->addAction(fileSaveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(fileQuitAction);    

    viewMenu->addAction(viewLogAction);
    viewMenu->addAction(viewGrooveAction);
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
    if (checkRcFile()) readRcFile();
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
    connect(arpData->seqDriver, SIGNAL(nextStep(snd_seq_tick_time_t)),
            arpWidget->arpScreen, SLOT(updateArpScreen(snd_seq_tick_time_t)));
    connect(arpWidget, SIGNAL(patternChanged()), 
            this, SLOT(resetQueue()));
    connect(arpWidget, SIGNAL(presetsChanged(QString, QString, int)), 
            this, SLOT(updatePatternPresets(QString, QString, int)));
    connect(arpWidget, SIGNAL(arpRemove(int)), 
            this, SLOT(removeArp(int)));
    connect(arpWidget, SIGNAL(dockRename(QString, int)), 
            this, SLOT(renameDock(QString, int)));

    connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
            arpWidget->arpScreen, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
            arpWidget->arpScreen, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
            arpWidget->arpScreen, SLOT(setGrooveLength(int)));

    widgetID = arpData->arpWidgetCount();
    arpWidget->name = name;
    arpWidget->ID = widgetID;
    if (passWidget->compactStyle) arpWidget->setStyleSheet(COMPACT_STYLE);
    
    arpData->addArpWidget(arpWidget);
    arpData->seqDriver->sendGroove();
    arpData->seqDriver->setMidiMutable(passWidget->cbuttonCheck->isChecked());
    arpData->updateCCnumber(passWidget->cnumberSpin->value());
    
    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(arpWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    
    count = arpData->moduleWindowCount();
    arpWidget->parentDockID = count;
    if (count) tabifyDockWidget(arpData->moduleWindow(count - 1), moduleWindow);
    arpData->addModuleWindow(moduleWindow);
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
    connect(lfoWidget, SIGNAL(dockRename(QString, int)), 
            this, SLOT(renameDock(QString, int)));

    widgetID = arpData->lfoWidgetCount();
    lfoWidget->name = name;
    lfoWidget->ID = widgetID;
    if (passWidget->compactStyle) lfoWidget->setStyleSheet(COMPACT_STYLE);

    arpData->addLfoWidget(lfoWidget);

    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(lfoWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    
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
    connect(seqWidget, SIGNAL(dockRename(QString, int)), 
            this, SLOT(renameDock(QString, int)));
            
    widgetID = arpData->seqWidgetCount();
    seqWidget->name = name;
    seqWidget->ID = widgetID;
    if (passWidget->compactStyle) seqWidget->setStyleSheet(COMPACT_STYLE);
    
    arpData->addSeqWidget(seqWidget);
    
    QDockWidget *moduleWindow = new QDockWidget(name, this);
    moduleWindow->setFeatures(QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    moduleWindow->setWidget(seqWidget);
    moduleWindow->setObjectName(name);
    addDockWidget(Qt::TopDockWidgetArea, moduleWindow);
    
    count = arpData->moduleWindowCount();
    seqWidget->parentDockID = count;
    
    if (count) tabifyDockWidget(arpData->moduleWindow(count - 1), moduleWindow);
    arpData->addModuleWindow(moduleWindow);

    checkIfFirstModule();
}

void MainWindow::renameDock(QString name, int parentDockID) 
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
            tr("QMidiArp files")  + " (*" + FILEEXT + ")");
    if (fn.isEmpty())
        return;

    openFile(fn);
}

void MainWindow::openFile(const QString& fn)
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
    updateWindowTitle();

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
        qs2 = qs.section(' ', 1, 1);
        passWidget->cnumberSpin->setValue(qs2.toInt());
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
        if (qs.startsWith("Seq:")) c = 1;
        if (qs.startsWith("LFO:")) c = 2;
        if (qs.startsWith("Arp:")) c = 3;
        switch (c) {
            case 1:
                addSeq(qs);
                arpData->seqWidget(arpData->midiSeqCount() - 1)->readSeq(loadText);
            break;
            case 2:
                addLfo(qs);
                arpData->lfoWidget(arpData->midiLfoCount() - 1)->readLfo(loadText);
            break;
            case 3:
                addArp(qs);
                arpData->arpWidget(arpData->midiArpCount() - 1)->readArp(loadText);
            break;
            default:
                qs = "Arp: " + qs;
                addArp(qs);
                arpData->arpWidget(arpData->midiArpCount() - 1)->readArp(loadText);
            break;
        }
    }
    arpData->setModified(false);
    midiClockAction->setChecked(midiclocktmp);
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
    saveText << (int)passWidget->cbuttonCheck->isChecked();
    saveText << ' ' << passWidget->cnumberSpin->value() << '\n';
    saveText << "MIDI Clock\n";
    saveText << (int)arpData->seqDriver->use_midiclock << ' ';
    saveText << (int)arpData->seqDriver->midiclock_tpb << '\n';
    saveText << (int)arpData->seqDriver->forwardUnmatched;
    saveText << ' ' << arpData->seqDriver->portUnmatched << '\n';
    
    saveText << arpData->seqDriver->grooveTick;
    saveText << ' ' << arpData->seqDriver->grooveVelocity;
    saveText << ' ' << arpData->seqDriver->grooveLength << '\n';

    for (l1 = 0; l1 < arpData->moduleWindowCount(); l1++) {
        
        nameTest = arpData->moduleWindow(l1)->windowTitle();
        
        if (nameTest.startsWith('S')) {
            saveText << qPrintable(arpData->seqWidget(ns)->name) << '\n';
            arpData->seqWidget(ns)->writeSeq(saveText);
            ns++;
        } 
        if (nameTest.startsWith('L')) {
            saveText << qPrintable(arpData->lfoWidget(nl)->name) << '\n';
            arpData->lfoWidget(nl)->writeLfo(saveText);
            nl++;
        }
        if (nameTest.startsWith('A')) {
            saveText << qPrintable(arpData->arpWidget(na)->name) << '\n';
            arpData->arpWidget(na)->writeArp(saveText);
            na++;
        }
    }
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
    runAction->setChecked(on);
    arpData->seqDriver->setUseMidiClock(on);
    runAction->setDisabled(on);
    tempoSpin->setDisabled(on);
    addArpAction->setDisabled(on);
    addLfoAction->setDisabled(on);
    addSeqAction->setDisabled(on);
    fileOpenAction->setDisabled(on);
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
    QString qs, qs2;
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
        if (qs.startsWith('[')) break;
        qs2 = loadText.readLine();
        patternNames << qs;
        patternPresets << qs2;
    }
    if (qs.startsWith("[GUI")) {
        int i = 0;
        qs = loadText.readLine();
        passWidget->compactStyleCheck->setChecked(qs.section(' ', i, i).toInt());
        i++;
        passWindow->setVisible(qs.section(' ', i, i).toInt());
        i++;
        passWindow->setFloating(qs.section(' ', i, i).toInt());
        i++;
        passWindow->move(qs.section(' ', i, i).toInt(), 
                        qs.section(' ', i+1, i+1).toInt());
        i+=2;
        
        logWindow->setVisible(qs.section(' ', i, i).toInt());
        i++;       
        logWindow->setFloating(qs.section(' ', i, i).toInt());
        i++;       
        logWindow->move(qs.section(' ', i, i).toInt(), 
            qs.section(' ', i+1, i+1).toInt());
        i+=2;       
        logWindow->resize(qs.section(' ', i, i).toInt(), 
            qs.section(' ', i+1, i+1).toInt() );
        i+=2;       
        logWidget->enableLog->setChecked(qs.section(' ', i, i).toInt());
        i++;       
        logWidget->logMidiClock->setChecked(qs.section(' ', i, i).toInt());
        i++;
        
        grooveWindow->setVisible(qs.section(' ', i, i).toInt());
        i++;       
        grooveWindow->setFloating(qs.section(' ', i, i).toInt());
        i++;       
        grooveWindow->move(qs.section(' ', i, i).toInt(), 
            qs.section(' ', i+1, i+1).toInt());
        i+=2;       
        grooveWindow->resize(qs.section(' ', i, i+1).toInt(), 
            qs.section(' ', i+1, i+1).toInt());
        
        i+=2;       
        fileToolBar->setVisible(qs.section(' ', i, i).toInt());
        
        i++;       
        move(qs.section(' ', i, i).toInt(), qs.section(' ', i+1, i+1).toInt());
        i+=2;       
        resize(qs.section(' ', i, i).toInt(), qs.section(' ', i+1, i+1).toInt()); 
        
        qs = loadText.readLine();
        if (qs.startsWith("[Last")) {
            qs = loadText.readLine();
            lastDir = qs;
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
        writeText << qPrintable(patternNames.at(l1)) << endl;
        writeText << qPrintable(patternPresets.at(l1)) << endl;
    }
    writeText << "[GUI settings]" << endl
    << passWidget->compactStyle << ' '
    << passWindow->isVisible() << ' '
    << passWindow->isFloating() << ' '
    << passWindow->x() << ' ' << passWindow->y() << ' '
    
    << logWindow->isVisible() << ' '
    << logWindow->isFloating() << ' '
    << logWindow->x() << ' ' << logWindow->y() << ' '
    << logWindow->width() << ' ' << logWindow->height() << ' '
    
    << logWidget->enableLog->isChecked() << ' '
    << logWidget->logMidiClock->isChecked() << ' '
    
    << grooveWindow->isVisible() << ' '
    << grooveWindow->isFloating() << ' '
    << grooveWindow->x() << ' ' << grooveWindow->y() << ' '
    << grooveWindow->width() << ' ' << grooveWindow->height() << ' '
   
    << fileToolBar->isVisible() << ' '
    
    << x() << ' ' << y() << ' ' 
    << width() << ' ' << height() << endl;
    
    writeText << "[Last Dir]" << endl;
    writeText << lastDir << endl;
}

void MainWindow::updatePatternPresets(QString n, QString p, int index)
{
    if (index) {
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
    }
}

void MainWindow::checkIfFirstModule()
{
    if (arpData->moduleWindowCount() == 1) {
        midiClockAction->setEnabled(true);
        runAction->setEnabled(true);
    }
}
