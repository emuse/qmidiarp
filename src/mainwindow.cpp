#include <QStringList>
#include <QSpinBox>
#include <QInputDialog>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>

#include "mainwindow.h"

#include "pixmaps/qmidiarp2.xpm"
#include "pixmaps/arpadd.xpm"
#include "pixmaps/lfoadd.xpm"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
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
    checkRcFile();
    filename = "";
    lastDir = QDir::homePath();

    arpData = new ArpData(this);
    arpData->registerPorts(p_portCount);

    tabWidget = new QTabWidget(this);

    logWidget = new LogWidget(this);
    QDockWidget *logWindow = new QDockWidget(tr("Event Log"), this);
    logWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    logWindow->setWidget(logWidget);;
    addDockWidget(Qt::BottomDockWidgetArea, logWindow);
    connect(arpData->seqDriver, SIGNAL(midiEvent(snd_seq_event_t *)), 
            logWidget, SLOT(appendEvent(snd_seq_event_t *)));

    passWidget = new PassWidget(p_portCount, this);
    QDockWidget *passWindow = new QDockWidget(tr("Settings"), this);
    passWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    passWindow->setWidget(passWidget);
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
            arpData->seqDriver, SLOT(updateCnumber(int)));

    connect(this, SIGNAL(runQueue(bool)), 
            arpData->seqDriver, SLOT(runQueue(bool)));				   

    grooveWidget = new GrooveWidget(this);
    QDockWidget *grooveWindow = new QDockWidget(tr("Groove"), this);
    grooveWindow->setFeatures(QDockWidget::DockWidgetClosable
            | QDockWidget::DockWidgetMovable
            | QDockWidget::DockWidgetFloatable);
    grooveWindow->setWidget(grooveWidget);;
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

    renameArpAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."), this);
    renameArpAction->setShortcut(QKeySequence(tr("Ctrl+R", "Module|Rename")));
    renameArpAction->setToolTip(tr("Rename this module"));
    connect(renameArpAction, SIGNAL(triggered()), this, SLOT(moduleRename()));
    renameArpAction->setDisabled(true);

    removeArpAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."), this);
    removeArpAction->setShortcut(QKeySequence(tr("Ctrl+Del", "Module|Delete")));
    removeArpAction->setToolTip(tr("Delete this module"));
    connect(removeArpAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));
    removeArpAction->setDisabled(true);

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
    updateRunQueue(false);

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

    QAction* viewLogAction = logWindow->toggleViewAction();
    viewLogAction->setText(tr("&Event Log"));
    viewLogAction->setShortcut(QKeySequence(tr("Ctrl+L", "View|Event Log")));

    QAction* viewGrooveAction = grooveWindow->toggleViewAction();
    viewGrooveAction->setText(tr("&Groove Settings"));
    viewGrooveAction->setShortcut(QKeySequence(tr("Ctrl+G", "View|Groove")));

    QAction* viewSettingsAction = passWindow->toggleViewAction();
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
    viewMenu->addAction(viewSettingsAction);
    viewMenu->addAction(viewGrooveAction);

    arpMenu->addAction(addArpAction);
    arpMenu->addAction(addLfoAction);
    arpMenu->addSeparator();
    arpMenu->addAction(renameArpAction);
    arpMenu->addAction(removeArpAction);

    helpMenu->addAction(tr("&About %1...").arg(PACKAGE), this,
            SLOT(helpAbout())); 
    helpMenu->addAction(tr("&About Qt..."), this,
            SLOT(helpAboutQt()));

    QToolBar *fileToolBar = new QToolBar(tr("&File Toolbar"), this);
    fileToolBar->addAction(fileNewAction);    
    fileToolBar->addAction(fileOpenAction);    
    fileToolBar->addAction(fileSaveAction);    
    fileToolBar->addAction(fileSaveAsAction);
    fileToolBar->setMaximumHeight(30);

    runBox = new QToolBar(tr("&Control Toolbar"), this);
    runBox->addAction(addArpAction);
    runBox->addAction(addLfoAction);
    runBox->addSeparator();
    runBox->addAction(renameArpAction);
    runBox->addAction(removeArpAction);
    runBox->addSeparator();
    runBox->addAction(runAction);
    runBox->addWidget(tempoSpin);
    runBox->addAction(midiClockAction);
    runBox->setMaximumHeight(30);

    menuBar->addMenu(fileMenu);
    menuBar->addMenu(viewMenu);
    menuBar->addMenu(arpMenu);
    menuBar->addMenu(helpMenu);

    setMenuBar(menuBar);
    addToolBar(fileToolBar);
    addToolBar(runBox);
    setCentralWidget(tabWidget);
    setWindowIcon(QPixmap(qmidiarp2_xpm));
    updateWindowTitle();
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

void MainWindow::addArp(const QString& name)
{
    MidiArp *midiArp = new MidiArp();
    arpData->addMidiArp(midiArp);   
    ArpWidget *arpWidget = new ArpWidget(midiArp,
            arpData->getPortCount(), tabWidget);
    connect(arpData->seqDriver, SIGNAL(nextStep(snd_seq_tick_time_t)),
            arpWidget->arpScreen, SLOT(updateArpScreen(snd_seq_tick_time_t)));
    connect(arpWidget, SIGNAL(patternChanged()), 
            this, SLOT(resetQueue()));
    connect(midiArp, SIGNAL(toggleMute()), arpWidget->muteOut, SLOT(toggle()));

    connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
            arpWidget->arpScreen, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
            arpWidget->arpScreen, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
            arpWidget->arpScreen, SLOT(setGrooveLength(int)));

    arpData->addArpWidget(arpWidget);
    arpData->seqDriver->sendGroove();
    arpData->seqDriver->setMidiMutable(passWidget->cbuttonCheck->isChecked());
    arpData->seqDriver->updateCnumber(passWidget->cnumberSpin->value());
    tabWidget->addTab(arpWidget, name);
    tabWidget->setCurrentWidget(arpWidget);
    arpWidget->arpName = name;
    removeArpAction->setEnabled(true);    
    renameArpAction->setEnabled(true);
    midiClockAction->setEnabled(true);
    runAction->setEnabled(true);
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

void MainWindow::addLfo(const QString& name)
{
    MidiLfo *midiLfo = new MidiLfo();
    arpData->addMidiLfo(midiLfo);   
    LfoWidget *lfoWidget = new LfoWidget(midiLfo,
            arpData->getPortCount(), tabWidget);
    connect(midiLfo, SIGNAL(toggleMute()), lfoWidget->muteOut, SLOT(toggle()));

    arpData->addLfoWidget(lfoWidget);
    arpData->seqDriver->sendGroove();
    arpData->seqDriver->setMidiMutable(passWidget->cbuttonCheck->isChecked());
    arpData->seqDriver->updateCnumber(passWidget->cnumberSpin->value());
    tabWidget->addTab(lfoWidget, name);
    tabWidget->setCurrentWidget(lfoWidget);
    lfoWidget->lfoName = name;
    removeArpAction->setEnabled(true);    
    renameArpAction->setEnabled(true);
    midiClockAction->setEnabled(true);
    runAction->setEnabled(true);
}

void MainWindow::moduleRename() {

    QString newname, oldname;
    bool ok;

    oldname = tabWidget->tabText(tabWidget->currentIndex());

    if (oldname.startsWith("LFO:")) {
        newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);
        if (ok && !newname.isEmpty()) {
            tabWidget->setTabText(tabWidget->currentIndex(), "LFO:"+newname);
            LfoWidget *lfoWidget = (LfoWidget *)tabWidget->currentWidget();
            lfoWidget->lfoName = "LFO:"+newname;
        }
    } else {
        if (!oldname.startsWith("Arp:")) oldname = "";
        newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);
        if (ok && !newname.isEmpty()) {
            tabWidget->setTabText(tabWidget->currentIndex(), "Arp:"+newname);
            ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentWidget();
            arpWidget->arpName = "Arp:"+newname;
        }
    }
}

void MainWindow::moduleDelete()
{
    QString qs;

    qs = tr("Remove \"%1\"?")
        .arg(tabWidget->tabText(tabWidget->currentIndex()));
    if (QMessageBox::question(0, APP_NAME, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
    if (tabWidget->tabText(tabWidget->currentIndex()).startsWith("A")) {
        ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentWidget();
        arpData->removeMidiArp(arpWidget->getMidiArp());
        arpData->removeArpWidget(arpWidget);
    } else {
        LfoWidget *lfoWidget = (LfoWidget *)tabWidget->currentWidget();
        arpData->removeMidiLfo(lfoWidget->getMidiLfo());
        arpData->removeLfoWidget(lfoWidget);
    }

    tabWidget->removeTab(tabWidget->currentIndex());
    if (!arpData->midiArpCount() && !arpData->midiLfoCount()) {  
        removeArpAction->setDisabled(true);
        renameArpAction->setDisabled(true);
        runAction->setDisabled(true);
        runAction->setChecked(false);
        midiClockAction->setDisabled(true);
        midiClockAction->setChecked(false);
    }
}

void MainWindow::removeArp(int index)
{
    ArpWidget *arpWidget = arpData->arpWidget(index);
    arpData->removeMidiArp(arpWidget->getMidiArp());
    arpData->removeArpWidget(arpWidget);
    tabWidget->removeTab(index);
    if (!arpData->midiArpCount() && !arpData->midiLfoCount()) {  
        removeArpAction->setDisabled(true);
        renameArpAction->setDisabled(true);
        runAction->setDisabled(true);
        runAction->setChecked(false);
        midiClockAction->setDisabled(true);
        midiClockAction->setChecked(false);
    }
    delete arpWidget;
}

void MainWindow::removeLfo(int index)
{
    LfoWidget *lfoWidget = arpData->lfoWidget(index);
    arpData->removeMidiLfo(lfoWidget->getMidiLfo());
    arpData->removeLfoWidget(lfoWidget);
    tabWidget->removeTab(index);
    if (!arpData->midiArpCount() && !arpData->midiLfoCount()) {  
        removeArpAction->setDisabled(true);
        renameArpAction->setDisabled(true);
        runAction->setDisabled(true);
        runAction->setChecked(false);
        midiClockAction->setDisabled(true);
        midiClockAction->setChecked(false);
    }
    delete lfoWidget;
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
        //check for full name to give "good chances" to load old files
        if (qs.startsWith("LFO:")) {
            addLfo(qs);
            arpData->lfoWidget(arpData->midiLfoCount() - 1)->readLfo(loadText);
        } else if (!loadText.atEnd()) {
            if (!qs.startsWith("Arp:"))
                qs = "Arp: " + qs;
            addArp(qs);
            arpData->arpWidget(arpData->midiArpCount() - 1)->readArp(loadText);
        }
    }
    arpData->setModified(false);
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
    int na = 0;
    int nl = 0;
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
    saveText <<	(int)passWidget->cbuttonCheck->isChecked();
    saveText <<	' ' << passWidget->cnumberSpin->value() << '\n';
    saveText << (int)arpData->seqDriver->forwardUnmatched;
    saveText << ' ' << arpData->seqDriver->portUnmatched << '\n';
    saveText << arpData->seqDriver->grooveTick;
    saveText << ' ' << arpData->seqDriver->grooveVelocity;
    saveText << ' ' << arpData->seqDriver->grooveLength << '\n';

    for (l1 = 0; l1 < tabWidget->count(); l1++) {
        if (tabWidget->tabText(l1).startsWith('L')) {
            saveText << qPrintable(arpData->lfoWidget(nl)->lfoName) << '\n';
            arpData->lfoWidget(nl)->writeLfo(saveText);
            nl++;
        } else {
            saveText << qPrintable(arpData->arpWidget(na)->arpName) << '\n';
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
    if (isSave())
        e->accept();
    else 
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
}

void MainWindow::resetQueue()
{
    arpData->seqDriver->runQueue(runAction->isChecked());
}

void MainWindow::midiClockToggle(bool on)
{
    runAction->setChecked(on);
    arpData->seqDriver->setUseMidiClock(on);
    runAction->setDisabled(on);
    tempoSpin->setDisabled(on);
    removeArpAction->setDisabled(on);    
    renameArpAction->setDisabled(on);
    addArpAction->setDisabled(on);
    addLfoAction->setDisabled(on);
    fileOpenAction->setDisabled(on);
}

void MainWindow::checkRcFile()
{
    QString qs2;
    int l1;
    QStringList defaultPatternNames, defaultPatterns;

    QDir qmahome = QDir(QDir::homePath());
    if (!qmahome.exists(QMARCNAME)) {
        QString qmarcpath = qmahome.filePath(QMARCNAME);
        QFile f(qmarcpath);

        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::warning(this, APP_NAME,
                    tr("Could not write to resource file"));
            return;
        }

        defaultPatternNames
            <<	"                         "
            <<  "Simple 4"   
            <<	"Simple 8"   
            <<	"Simple 16"  
            <<	"Simple 32"  
            << 	"Chord 8"    
            << 	"Chord+Bass 16"   
            <<	"Chord Oct 16 A"  
            <<	"Chord Oct 16 B"  
            << 	"Chord Oct 16 C"  
            << 	"Chords/Glissando 16";

        defaultPatterns
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

        QTextStream writeText(&f);
        for (l1 = 0; l1 < defaultPatterns.count(); l1++) {
            writeText << defaultPatternNames.at(l1) << '\n';
            writeText << defaultPatterns.at(l1) << '\n';
        }
    }
}
