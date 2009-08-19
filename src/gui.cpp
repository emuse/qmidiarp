#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QBoxLayout>
#include <QSocketNotifier>
#include <QStringList>
#include <QSpinBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QIcon>
#include <QAction>
#include <QFileDialog>
#include <QToolButton>
#include <QToolBar>
#include <QTextStream>
#include <alsa/asoundlib.h>

#include "arpdata.h"
#include "logwidget.h"
#include "passwidget.h"
#include "groovewidget.h"
#include "gui.h"
#include "arpwidget.h"
#include "arpscreen.h"

#include "pixmaps/arpadd.xpm"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
#include "pixmaps/play.xpm"


Gui::Gui(int p_portCount, QWidget *parent) : QWidget(parent)
{
    checkRcFile();
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

    runBox = new QToolBar(this);

    addArpButton = new QToolButton(this);
    addArpAction = new QAction(QIcon(arpadd_xpm),tr("&Add Arp"), runBox);
    connect(addArpAction, SIGNAL(triggered()), this, SLOT(addArp()));
    addArpButton->setDefaultAction(addArpAction);

    renameArpButton = new QToolButton(this);
    renameArpAction = new QAction(QIcon(arprename_xpm),tr("&Rename Arp"), runBox);
    connect(renameArpAction, SIGNAL(triggered()), this, SLOT(renameArp()));
    renameArpButton->setDefaultAction(renameArpAction);
    renameArpAction->setDisabled(true);

    removeArpButton = new QToolButton(this);
    removeArpAction = new QAction(QIcon(arpremove_xpm),tr("&Remove Arp"), runBox);
    connect(removeArpAction, SIGNAL(triggered()), this, SLOT(removeArp()));
    removeArpButton->setDefaultAction(removeArpAction);
    removeArpAction->setDisabled(true);

    runButton = new QToolButton(this);
    runAction = new QAction(QIcon(play_xpm), tr("&Run"), this);
    connect(runAction, SIGNAL(toggled(bool)), this, SLOT(updateRunQueue(bool)));
    runButton->setDefaultAction(runAction);   
    runAction->setCheckable(true);
    runAction->setChecked(false);
	runAction->setDisabled(true);
	updateRunQueue(false);
	

    tempoSpin = new QSpinBox(runBox);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(100);
    connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(updateTempo(int)));

    runBox->addWidget(addArpButton);
    runBox->addWidget(renameArpButton);
    runBox->addWidget(removeArpButton);
    runBox->addSeparator();
    runBox->addWidget(runButton);
    runBox->addWidget(tempoSpin);
    runBox->setMaximumHeight(30);


    QVBoxLayout *guiBoxLayout = new QVBoxLayout;
    guiBoxLayout->addWidget(runBox);
    guiBoxLayout->addWidget(tabWidget);
    guiBoxLayout->setSpacing(2);
    guiBoxLayout->setMargin(2);

    setLayout(guiBoxLayout);
}

Gui::~Gui()
{
}

void Gui::displayAbout()
{
    aboutWidget->about(this, tr("About %1").arg(PACKAGE), aboutText);
    aboutWidget->raise();
}

void Gui::addArp()
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

void Gui::addArp(QString qs)
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
    tabWidget->addTab(arpWidget, qs);
    tabWidget->setCurrentWidget(arpWidget);
    arpWidget->arpName = qs;
    removeArpButton->setEnabled(true);    
    renameArpButton->setEnabled(true);
	passWidget->mbuttonCheck->setEnabled(true);
	runAction->setEnabled(true);
}

void Gui::renameArp() {

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

void Gui::removeArp()
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
        removeArpButton->setDisabled(true);
        renameArpButton->setDisabled(true);
		runAction->setDisabled(true);
		runAction->setChecked(false);
		passWidget->mbuttonCheck->setDisabled(true);
		passWidget->mbuttonCheck->setChecked(false);
    }
}

void Gui::removeArp(int index)
{
    QString qs;

    ArpWidget *arpWidget = arpData->arpWidget(index);
    arpData->removeMidiArp(arpWidget->getMidiArp());
    arpData->removeArpWidget(arpWidget);
    tabWidget->removeTab(tabWidget->currentIndex());
    if (arpData->midiArpCount() < 1) {
        removeArpButton->setDisabled(true);
        renameArpButton->setDisabled(true);
		runAction->setDisabled(true);
		runAction->setChecked(false);
		passWidget->mbuttonCheck->setDisabled(true);
		passWidget->mbuttonCheck->setChecked(false);
    }                      
}

void Gui::clear()
{
    while (arpData->midiArpCount()) {
        removeArp(arpData->midiArpCount() - 1);
    }
}

void Gui::load()
{
    QString filename =  QFileDialog::getOpenFileName(this,
            QString::null, "", tr("QMidiArp files (*.qma)"));
    if (filename == "") {
        return;
    }
    clear();
    load(filename);
}

void Gui::load(QString name)
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

void Gui::save()
{
    int l1;

    QString filename =  QFileDialog::getSaveFileName(this,
            QString::null, "", tr("QMidiArp files (*.qma)"));
    if (filename == "") {
        return;
    }
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
}

void Gui::updateTempo(int p_tempo)
{
    emit(newTempo(p_tempo));
}

void Gui::updateRunQueue(bool on)
{
    emit(runQueue(on));
}

void Gui::resetQueue()
{
    arpData->seqDriver->runQueue(runButton->isChecked());
}

void Gui::midiClockToggle(bool on)
{
    runAction->setChecked(on);
    arpData->seqDriver->setUseMidiClock(on);
    runAction->setDisabled(on);

}

void Gui::checkRcFile()
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
