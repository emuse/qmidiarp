#include <QStringList>
#include <QSpinBox>
#include <QInputDialog>
#include <QFile>
#include <QIcon>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>
#include <QApplication>
#include <QCloseEvent>
#include <QMenu>
#include <QMenuBar>

#include <alsa/asoundlib.h>

#include "arpdata.h"
#include "logwidget.h"
#include "passwidget.h"
#include "groovewidget.h"
#include "mainwindow.h"
#include "arpwidget.h"
#include "arpscreen.h"

#include "pixmaps/qmidiarp2.xpm"
#include "pixmaps/arpadd.xpm"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
#include "pixmaps/play.xpm"


MainWindow::MainWindow(QString fileName, int p_portCount)
{
    checkRcFile();
    if (!fileName.isEmpty())
		{
		filename = fileName;
		lastDir = fileName;
	} 
		else
		{
		lastDir = QDir::homePath();
	}
    arpData = new ArpData(this);
    arpData->registerPorts(p_portCount);



    aboutWidget = new QMessageBox(this); 

    tabWidget = new QTabWidget(this);
    logWidget = new LogWidget(tabWidget);
    connect(arpData->seqDriver, SIGNAL(midiEvent(snd_seq_event_t *)), 
            logWidget, SLOT(appendEvent(snd_seq_event_t *)));
    tabWidget->addTab(logWidget, tr("Event Log"));

    passWidget = new PassWidget(p_portCount, tabWidget);


    connect(passWidget, SIGNAL(discardToggled(bool)), 
            arpData->seqDriver, SLOT(setDiscardUnmatched(bool)));
    connect(passWidget, SIGNAL(midiClockToggle(bool)), 
            this, SLOT(midiClockToggle(bool)));
    connect(passWidget, SIGNAL(newMIDItpb(int)), 
            arpData->seqDriver, SLOT(updateMIDItpb(int)));
    connect(passWidget, SIGNAL(newPortUnmatched(int)), 
            arpData->seqDriver, SLOT(setPortUnmatched(int)));

    connect(this, SIGNAL(newTempo(int)), 
            arpData->seqDriver, SLOT(setQueueTempo(int)));
    //CHECK: may be receiver is "arpData"
    connect(this, SIGNAL(runQueue(bool)), 
            arpData->seqDriver, SLOT(runQueue(bool)));				   
    tabWidget->addTab(passWidget, tr("Settings"));

    grooveWidget = new GrooveWidget(tabWidget);
    connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
            arpData->seqDriver, SLOT(setGrooveTick(int)));
    connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
            arpData->seqDriver, SLOT(setGrooveVelocity(int)));
    connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
            arpData->seqDriver, SLOT(setGrooveLength(int)));
    tabWidget->addTab(grooveWidget, tr("Groove"));			


    addArpAction = new QAction(QIcon(arpadd_xpm), tr("&New..."), this);
    addArpAction->setShortcut(QKeySequence(QMenu::tr("Ctrl+N", "Arp|New")));
    connect(addArpAction, SIGNAL(triggered()), this, SLOT(addArp()));

    renameArpAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."),
            this);
	renameArpAction->setShortcut(QKeySequence(QMenu::tr("Ctrl+R", "Arp|Rename")));
    connect(renameArpAction, SIGNAL(triggered()), this, SLOT(renameArp()));
    renameArpAction->setDisabled(true);

    removeArpAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."),
            this);
	removeArpAction->setShortcut(QKeySequence(QMenu::tr("Ctrl+Del", "Arp|Delete")));
    connect(removeArpAction, SIGNAL(triggered()), this, SLOT(removeArp()));
    removeArpAction->setDisabled(true);

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
    connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(updateTempo(int)));
	
    QMenuBar *menuBar = new QMenuBar; 
    QMenu *filePopup = new QMenu(QMenu::tr("&File"),this); 
    QMenu *arpPopup = new QMenu(QMenu::tr("&Arp"),this); 
    QMenu *aboutMenu = new QMenu(QMenu::tr("&Help"),this);

    filePopup->addAction(QMenu::tr("&Open..."), this, SLOT(load()),
					    QKeySequence(QKeySequence::Open));    
    filePopup->addAction(QMenu::tr("&Save"), this, SLOT(save()),
				    QKeySequence(QKeySequence::Save));    
    filePopup->addAction(QMenu::tr("Save &As..."), this, SLOT(saveAs()));
    filePopup->addSeparator();
    filePopup->addAction(QMenu::tr("&Quit"), qApp, SLOT(quit()),
					QKeySequence(QMenu::tr("Ctrl+Q", "File|Quit")));    

    arpPopup->addAction(addArpAction);
    arpPopup->addAction(renameArpAction);
    arpPopup->addAction(removeArpAction);

    aboutMenu->addAction(QMenu::tr("&About %1...").arg(PACKAGE), this,
            SLOT(displayAbout())); 
 
    runBox = new QToolBar(tr("&Control Toolbar"), this);
    runBox->addAction(addArpAction);
    runBox->addAction(renameArpAction);
    runBox->addAction(removeArpAction);
    runBox->addSeparator();
    runBox->addAction(runAction);
    runBox->addWidget(tempoSpin);
    runBox->setMaximumHeight(30);

    menuBar->addMenu(filePopup);
    menuBar->addMenu(arpPopup);
    menuBar->addMenu(aboutMenu);
	
	
    setMenuBar(menuBar);
	addToolBar(runBox);
    setCentralWidget(tabWidget);
	setWindowTitle(filename + " - "  PACKAGE);
    setWindowIcon(QPixmap(qmidiarp2_xpm));
	if (!filename.isEmpty()) load(filename);

    show();
}

MainWindow::~MainWindow()
{
}

void MainWindow::displayAbout()
{
    aboutWidget->about(this, tr("About %1").arg(PACKAGE), aboutText);
    aboutWidget->raise();
}

void MainWindow::addArp()
{
    QString name;
    bool ok;

    name = QInputDialog::getText(this, PACKAGE,
            tr("Add MIDI Arpeggiator"), QLineEdit::Normal,
           tr("Arp %1").arg(arpData->midiArpCount() + 1), &ok);
    if (ok && !name.isEmpty()) {
        addArp(name);
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
    arpData->addArpWidget(arpWidget);
    arpData->seqDriver->sendGroove();
    tabWidget->addTab(arpWidget, name);
    tabWidget->setCurrentWidget(arpWidget);
    arpWidget->arpName = name;
    removeArpAction->setEnabled(true);    
    renameArpAction->setEnabled(true);
    passWidget->mbuttonCheck->setEnabled(true);
    runAction->setEnabled(true);
}

void MainWindow::renameArp() {

    QString newname, oldname;
    bool ok;

    if (tabWidget->currentIndex() < 3) {
        return;
    }
    oldname = tabWidget->tabText(tabWidget->currentIndex());
    newname = QInputDialog::getText(this, PACKAGE,
            tr("New Name"), QLineEdit::Normal, oldname, &ok);

    if (ok && !newname.isEmpty()) {
        tabWidget->setTabText(tabWidget->currentIndex(), newname);
        ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentWidget();
        arpWidget->arpName = newname;
    }
}

void MainWindow::removeArp()
{
    QString qs;

    if (tabWidget->currentIndex() < 3) {
        return;
    } 
    ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentWidget();
    qs = tr("Remove \"%1\"?")
        .arg(tabWidget->tabText(tabWidget->currentIndex()));
    if (QMessageBox::question(0, PACKAGE, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
    arpData->removeMidiArp(arpWidget->getMidiArp());
    arpData->removeArpWidget(arpWidget);
    tabWidget->removeTab(tabWidget->currentIndex());
    if (arpData->midiArpCount() < 1) {  
        removeArpAction->setDisabled(true);
        renameArpAction->setDisabled(true);
        runAction->setDisabled(true);
        runAction->setChecked(false);
        passWidget->mbuttonCheck->setDisabled(true);
        passWidget->mbuttonCheck->setChecked(false);
    }
}

void MainWindow::removeArp(int index)
{
//    QString qs;

    ArpWidget *arpWidget = arpData->arpWidget(index);
    arpData->removeMidiArp(arpWidget->getMidiArp());
    arpData->removeArpWidget(arpWidget);
    tabWidget->removeTab(index + 3);
    if (arpData->midiArpCount() < 1) {
        removeArpAction->setDisabled(true);
        renameArpAction->setDisabled(true);
        runAction->setDisabled(true);
        runAction->setChecked(false);
        passWidget->mbuttonCheck->setDisabled(true);
        passWidget->mbuttonCheck->setChecked(false);
    }                      
}

void MainWindow::clear()
{
    while (arpData->midiArpCount()) {
        removeArp(arpData->midiArpCount() - 1);
    }
}

void MainWindow::load()
{
    filename =  QFileDialog::getOpenFileName(this,
            tr("Open arpeggiator file"), lastDir,
            tr("QMidiArp files (*.qma)"));
    if (filename.isEmpty()) {
        return;
    }
    lastDir = filename;
    clear();
    load(filename);
}

void MainWindow::load(const QString& name)
{
    QString qs, qs2;

    clear();
	
    QFile f(name);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, PACKAGE,
                tr("Could not read from file %1.").arg(name));
        return;
    }          
    QTextStream loadText(&f);
    qs = loadText.readLine();
    qs2 = qs.section(' ', 0, 0);
    passWidget->setDiscard(qs2.toInt());
    qs2 = qs.section(' ', 1, 1);
    passWidget->setPortUnmatched(qs2.toInt());
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
        addArp(qs);
        arpData->arpWidget(arpData->midiArpCount() - 1)->readArp(loadText);
    }
    tabWidget->setCurrentWidget(arpData->arpWidget(0));
}

void MainWindow::save()
{
    int l1;

    if (arpData->midiArpCount() < 1) return;

	if (lastDir.endsWith('/') || filename.isEmpty()) 
	{
				filename =  QFileDialog::getSaveFileName(this,
            tr("Save arpeggiator"), lastDir, tr("QMidiArp files") 
			+ " (*" + FILEEXT + ")");

	}
	if (filename.isEmpty()) return;
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, PACKAGE,
                tr("Could not write to file \"%1\".").arg(filename));
        return;
    }          

    QTextStream saveText(&f);
    saveText << (int)arpData->seqDriver->discardUnmatched;
    saveText << ' ' << arpData->seqDriver->portUnmatched << '\n';
    saveText << arpData->seqDriver->grooveTick;
    saveText << ' ' << arpData->seqDriver->grooveVelocity;
    saveText << ' ' << arpData->seqDriver->grooveLength << '\n';
    for (l1 = 0; l1 < arpData->arpWidgetCount(); l1++) {
        saveText << qPrintable(arpData->arpWidget(l1)->arpName) << '\n';
        arpData->arpWidget(l1)->writeArp(saveText);
    }
	lastDir = filename;
}

void MainWindow::saveAs()
{
    if (arpData->midiArpCount() < 1) {  
        return;
	}
		filename =  QFileDialog::getSaveFileName(this,
            tr("Save arpeggiator"), lastDir, tr("QMidiArp files") 
			+ " (*" + FILEEXT + ")");

    if (!filename.isEmpty()) {
        if (!filename.endsWith(FILEEXT))
            filename.append(FILEEXT);
		lastDir = filename;
		save();
				
		}
}


void MainWindow::updateTempo(int p_tempo)
{
    emit(newTempo(p_tempo));
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
        //qWarning("New qmidiarp resource file in home directory created");

        if (!f.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, PACKAGE,
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
            << "(012345)"
            << ">>(01234)0(01234)0"
            << ">>////(012345)\\ \\ \\ +(012345)"
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
