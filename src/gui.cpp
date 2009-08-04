#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qspinbox.h>
#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <alsa/asoundlib.h>
#include "arpdata.h"
#include "logwidget.h"
#include "passwidget.h"
#include "groovewidget.h"
#include "gui.h"

Gui::Gui(int p_portCount, QWidget *parent) : QWidget(parent) {

QVBoxLayout *guiBoxLayout = new QVBoxLayout;
  arpData = new ArpData(this);
  arpData->registerPorts(p_portCount);
  aboutWidget = new QMessageBox(this); 
  tabWidget = new QTabWidget(this);
  logWidget = new LogWidget(tabWidget);
  QObject::connect(arpData->seqDriver, SIGNAL(midiEvent(snd_seq_event_t *)), 
                   logWidget, SLOT(appendEvent(snd_seq_event_t *)));
  tabWidget->addTab(logWidget, "Event Log");
    	
  passWidget = new PassWidget(p_portCount, tabWidget);
   
  QObject::connect(passWidget, SIGNAL(discardToggled(bool)), 
                   arpData->seqDriver, SLOT(setDiscardUnmatched(bool)));
  QObject::connect(passWidget, SIGNAL(midiClockToggle(bool)), 
				   arpData->seqDriver, SLOT(setUseMidiClock(bool)));
  QObject::connect(passWidget, SIGNAL(newMIDItpb(int)), 
				   arpData->seqDriver, SLOT(updateMIDItpb(int)));
  QObject::connect(passWidget, SIGNAL(newPortUnmatched(int)), 
                   arpData->seqDriver, SLOT(setPortUnmatched(int)));
				   
  QObject::connect(this, SIGNAL(newTempo(int)), 
                   arpData->seqDriver, SLOT(setQueueTempo(int)));
  QObject::connect(this, SIGNAL(runQueue(bool)), 
                   arpData->seqDriver, SLOT(runQueue(bool)));
  				   
  tabWidget->addTab(passWidget, "Settings");
 
grooveWidget = new GrooveWidget(tabWidget);
  QObject::connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
                   arpData->seqDriver, SLOT(setGrooveTick(int)));
  QObject::connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
                   arpData->seqDriver, SLOT(setGrooveVelocity(int)));
  QObject::connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
                   arpData->seqDriver, SLOT(setGrooveLength(int)));
tabWidget->addTab(grooveWidget, "Groove");			

QWidget *arpButtonBox = new QWidget(this);
QHBoxLayout *arpButtonBoxLayout = new QHBoxLayout;
  
  addArpButton = new QPushButton("Add Arp", arpButtonBox);
  QObject::connect(addArpButton, SIGNAL(clicked()), this, SLOT(addArp()));
  
  renameArpButton = new QPushButton("Rename Arp", arpButtonBox);
  QObject::connect(renameArpButton, SIGNAL(clicked()), this, SLOT(renameArp()));
  renameArpButton->setDisabled(true);
 
  removeArpButton = new QPushButton("Remove Arp", arpButtonBox);  
  QObject::connect(removeArpButton, SIGNAL(clicked()), this, SLOT(removeArp()));
  removeArpButton->setDisabled(true);

QWidget *runBox = new QWidget(this);
  QHBoxLayout *runBoxLayout = new QHBoxLayout;
  QLabel *runQueueLabel = new QLabel("Run Arpeggiator Queue ", runBox);
  QCheckBox *runQueueCheck = new QCheckBox(runBox);
  runQueueCheck->setChecked(true);
  QObject::connect(runQueueCheck, SIGNAL(toggled(bool)), this, SLOT(updateRunQueue(bool)));
  
  QLabel *tempoLabel = new QLabel("Tempo (bpm) ", runBox);
  tempoSpin = new QSpinBox(runBox);
  tempoSpin->setRange(10,400);
  tempoSpin->setValue(100);
  QObject::connect(tempoSpin, SIGNAL(valueChanged(int)), this, SLOT(updateTempo(int)));
  runBoxLayout->addWidget(runQueueLabel);
  runBoxLayout->addWidget(runQueueCheck);
  runBoxLayout->addWidget(tempoLabel);
  runBoxLayout->addWidget(tempoSpin);
  runBox->setLayout(runBoxLayout);


arpButtonBoxLayout->addWidget(addArpButton);
arpButtonBoxLayout->addWidget(renameArpButton);
arpButtonBoxLayout->addWidget(removeArpButton);
arpButtonBox->setLayout(arpButtonBoxLayout);

guiBoxLayout->addWidget(runBox);

guiBoxLayout->addWidget(tabWidget);
guiBoxLayout->addWidget(arpButtonBox);
guiBoxLayout->setSpacing(2);
guiBoxLayout->setMargin(2);

setLayout(guiBoxLayout);
}

Gui::~Gui() {

}

void Gui::displayAbout() {
 
    aboutWidget->about(this, "About QMidiArp", aboutText);
    aboutWidget->raise();
}

void Gui::addArp() {

  QString qs, qs2;
  bool ok;

  qs2.sprintf("Arp %d", arpData->midiArpCount() + 1);
 
  qs = QInputDialog::getText(this, "QMidiArp: Add MIDI Arp", "Add MIDI Arp",
                                          QLineEdit::Normal,
                                          qs2, &ok);
 if (ok && !qs.isEmpty()) {addArp(qs);}
}

void Gui::addArp(QString qs) {

  
  MidiArp *midiArp = new MidiArp();
  arpData->addMidiArp(midiArp);   
  ArpWidget *arpWidget = new ArpWidget(midiArp, arpData->getPortCount(), tabWidget);
  arpData->addArpWidget(arpWidget);
  arpData->seqDriver->sendGroove();
  tabWidget->addTab(arpWidget, qs);
  tabWidget->setCurrentWidget(arpWidget);
  arpWidget->arpName = qs;
  removeArpButton->setEnabled(true);    
  renameArpButton->setEnabled(true);
}

void Gui::renameArp() {

  QString qs, qs2;
  bool ok;
  if (tabWidget->currentIndex() < 3) {return;}
  qs2 = tabWidget->tabText(tabWidget->currentIndex());
  qs = QInputDialog::getText(this, "QMidiArp: Rename Arp", "New Name",
                                          QLineEdit::Normal,
                                          qs2, &ok);
  if (ok && !qs.isEmpty()) {										  									 
  tabWidget->setTabText(tabWidget->currentIndex(), qs);                                
  ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentWidget();
  arpWidget->arpName = qs;
}
}

void Gui::removeArp() {

  QString qs;

  if (tabWidget->currentIndex() < 3) {
    return;
  } 
  ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentWidget();
  qs.sprintf("Remove %s ?", qPrintable(tabWidget->tabText(tabWidget->currentIndex())));
  if (QMessageBox::question(0, "QMidiArp", qs, QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton)
      == QMessageBox::No) {
    return;
  }
  arpData->removeMidiArp(arpWidget->getMidiArp());
  arpData->removeArpWidget(arpWidget);
  tabWidget->removeTab(tabWidget->currentIndex());
  if (arpData->midiArpCount() < 1) {  
    removeArpButton->setDisabled(true);
	renameArpButton->setDisabled(true);
  }
}

void Gui::removeArp(int index) {

  QString qs;
  
  ArpWidget *arpWidget = arpData->arpWidget(index);
  arpData->removeMidiArp(arpWidget->getMidiArp());
  arpData->removeArpWidget(arpWidget);
  tabWidget->removeTab(tabWidget->currentIndex());
  if (arpData->midiArpCount() < 1) {
    removeArpButton->setDisabled(true);
	renameArpButton->setDisabled(true);
  }                      
}

void Gui::clear() {

  while (arpData->midiArpCount()) {
    removeArp(arpData->midiArpCount() - 1);
  }
}

void Gui::load() {

  QString qs, qs2; 
  QString selfile =  QFileDialog::getOpenFileName(this, QString::null, "", "QMidiArp files (*.qma)");
  if (selfile == "") {
    return;
  }
  clear();
  qs = selfile;
  load(qs);
  
}

void Gui::load(QString name) {

   QString qs, qs2;
  
  clear();
  QFile f(name);
  if (!f.open(QIODevice::ReadOnly)) {
    qs2.sprintf("Could not read from file %s.", qPrintable(qs));
    QMessageBox::information(this, "QMidiArp", qs2);
    return;
  }          
  QTextStream loadText(&f);
  QRegExp sep(" ");
  qs = loadText.readLine();
  qs2 = qs.section(sep, 0, 0);
  passWidget->setDiscard(qs2.toInt());
  qs2 = qs.section(sep, 1, 1);
  passWidget->setPortUnmatched(qs2.toInt());
  qs = loadText.readLine();
  qs2 = qs.section(sep, 0, 0);
  
  grooveWidget->grooveTick->setValue(qs2.toInt());
//  arpData->seqDriver->setGrooveTick(qs2.toInt());
  qs2 = qs.section(sep, 1, 1);
  grooveWidget->grooveVelocity->setValue(qs2.toInt());
//  arpData->seqDriver->setGrooveVelocity(qs2.toInt());
  qs2 = qs.section(sep, 2, 2);
  grooveWidget->grooveLength->setValue(qs2.toInt());
//  arpData->seqDriver->setGrooveLength(qs2.toInt());
  while (!loadText.atEnd()) {
    qs = loadText.readLine();
    addArp(qs);
    arpData->arpWidget(arpData->midiArpCount() - 1)->readArp(loadText);
  }
  tabWidget->setCurrentWidget(arpData->arpWidget(0));
}

void Gui::save() {

  int l1;
  QString qs, qs2; 
  QString selfile =  QFileDialog::getSaveFileName(this, QString::null, "", "QMidiArp files (*.qma)");
  if (selfile == "") {
    return;
  }
  qs = selfile;
  QFile f(qs);
  if (!f.open(QIODevice::WriteOnly)) {
    qs2.sprintf("Could not write to file %s.", qPrintable(qs));
    QMessageBox::information(this, "QMidiArp", qs2);
    return;
  }          
  QTextStream saveText(&f);
  saveText << (int)arpData->seqDriver->discardUnmatched;
  saveText << " " << arpData->seqDriver->portUnmatched << "\n";
  saveText << arpData->seqDriver->grooveTick;
  saveText << " " << arpData->seqDriver->grooveVelocity;
  saveText << " " << arpData->seqDriver->grooveLength << "\n";
  for (l1 = 0; l1 < arpData->arpWidgetCount(); l1++) {
    saveText << qPrintable(arpData->arpWidget(l1)->arpName) << "\n";
    arpData->arpWidget(l1)->writeArp(saveText);
  }
}
void Gui::updateTempo(int p_tempo) {

  emit(newTempo(p_tempo));
}

void Gui::updateRunQueue(bool on) {

  emit(runQueue(on));
}
void Gui::midiClockToggle (bool on) {
	arpData->seqDriver->setUseMidiClock(on);
	runQueueCheck->setDisabled(on);
	runQueueCheck->setChecked(true);
}
