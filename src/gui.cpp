#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>
#include <qstrlist.h>
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

Gui::Gui(int p_portCount, QWidget *parent, const char *name) : QVBox(parent, name) {

  setSpacing(10);
  setMargin(10);
  
  arpData = new ArpData(this);
  arpData->registerPorts(p_portCount);
  aboutWidget = new QMessageBox(this); 
  tabWidget = new QTabWidget(this);
  logWidget = new LogWidget(tabWidget);
  tabWidget->insertTab(logWidget, "Event Log");
  QObject::connect(arpData->seqDriver, SIGNAL(midiEvent(snd_seq_event_t *)), 
                   logWidget, SLOT(appendEvent(snd_seq_event_t *)));
  passWidget = new PassWidget(p_portCount, tabWidget);
  tabWidget->insertTab(passWidget, "Settings");
  grooveWidget = new GrooveWidget(tabWidget);
  tabWidget->insertTab(grooveWidget, "Groove");
  QObject::connect(passWidget, SIGNAL(discardToggled(bool)), 
                   arpData->seqDriver, SLOT(setDiscardUnmatched(bool)));
  QObject::connect(passWidget, SIGNAL(newPortUnmatched(int)), 
                   arpData->seqDriver, SLOT(setPortUnmatched(int)));
  QObject::connect(passWidget, SIGNAL(newTempo(int)), 
                   arpData->seqDriver, SLOT(setQueueTempo(int)));
  QObject::connect(passWidget, SIGNAL(runQueue(bool)), 
                   arpData, SLOT(runQueue(bool)));
  QObject::connect(grooveWidget, SIGNAL(newGrooveTick(int)), 
                   arpData->seqDriver, SLOT(setGrooveTick(int)));
  QObject::connect(grooveWidget, SIGNAL(newGrooveVelocity(int)), 
                   arpData->seqDriver, SLOT(setGrooveVelocity(int)));
  QObject::connect(grooveWidget, SIGNAL(newGrooveLength(int)), 
                   arpData->seqDriver, SLOT(setGrooveLength(int)));
  QHBox *arpButtonBox = new QHBox(this);
  QPushButton *addArpButton = new QPushButton("Add Arp", arpButtonBox);
  QObject::connect(addArpButton, SIGNAL(clicked()), this, SLOT(addArp()));
  QPushButton *renameArpButton = new QPushButton("Rename Arp", arpButtonBox);
  QObject::connect(renameArpButton, SIGNAL(clicked()), this, SLOT(renameArp()));
  removeArpButton = new QPushButton("Remove Arp", arpButtonBox);
  removeArpButton->setDisabled(true);
  QObject::connect(removeArpButton, SIGNAL(clicked()), this, SLOT(removeArp()));
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
  qs = QInputDialog::getText("QMidiArp: Add MIDI Arp", "Add MIDI Arp:", 
                              QLineEdit::Normal, qs2, &ok, this);
  addArp(qs);
}

void Gui::addArp(QString qs) {

  removeArpButton->setEnabled(true);    
  MidiArp *midiArp = new MidiArp();
  arpData->addMidiArp(midiArp);   
  ArpWidget *arpWidget = new ArpWidget(midiArp, arpData->getPortCount(), tabWidget);
  arpData->addArpWidget(arpWidget);
  arpData->seqDriver->sendGroove();
  tabWidget->insertTab(arpWidget, qs);
  tabWidget->showPage(arpWidget);
  arpWidget->arpName = qs;
}

void Gui::renameArp() {

  QString qs, qs2;
  bool ok;
  
  qs2 = tabWidget->label(tabWidget->currentPageIndex());
  qs = QInputDialog::getText("QMidiArp: Rename Arp", "New Name:", 
                              QLineEdit::Normal, qs2, &ok, this);
  tabWidget->setTabLabel(tabWidget->currentPage(), qs);                                
  ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentPage();
  arpWidget->arpName = qs;
}

void Gui::removeArp() {

  QString qs;

  if (tabWidget->currentPageIndex() < 3) {
    return;
  } 
  ArpWidget *arpWidget = (ArpWidget *)tabWidget->currentPage();
  qs.sprintf("Remove %s ?", tabWidget->label(tabWidget->currentPageIndex()).latin1());
  if (QMessageBox::question(0, "QMidiArp", qs, QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton)
      == QMessageBox::No) {
    return;
  }
  arpData->removeMidiArp(arpWidget->getMidiArp());
  arpData->removeArpWidget(arpWidget);
  tabWidget->removePage(arpWidget);
  if (arpData->midiArpCount() < 1) {  
    removeArpButton->setDisabled(true);
  }
}

void Gui::removeArp(int index) {

  QString qs;
  
  ArpWidget *arpWidget = arpData->arpWidget(index);
  arpData->removeMidiArp(arpWidget->getMidiArp());
  arpData->removeArpWidget(arpWidget);
  tabWidget->removePage(arpWidget);
  if (arpData->midiArpCount() < 1) {
    removeArpButton->setDisabled(true);
  }                      
}

void Gui::clear() {

  while (arpData->midiArpCount()) {
    removeArp(arpData->midiArpCount() - 1);
  }
}

void Gui::load() {

  QString qs, qs2; 
  
  if (!(qs = QString(QFileDialog::getOpenFileName(QString::null, "QMidiArp files (*.qma)")))) {
    return;
  }
  load(qs);
}

void Gui::load(QString name) {

  QString qs, qs2; 
  
  clear();
  QFile f(name);
  if (!f.open(IO_ReadOnly)) {
    qs2.sprintf("Could not read from file %s.", qs.latin1());
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
    arpData->arpWidget(arpData->midiArpCount() - 1)->readArp(&f);
  }
  tabWidget->showPage(arpData->arpWidget(0));
}

void Gui::save() {

  int l1;
  QString qs, qs2; 
  
  if (!(qs = QString(QFileDialog::getSaveFileName(QString::null, "QMidiArp files (*.qma)")))) {
    return;
  }

  QFile f(qs);
  if (!f.open(IO_WriteOnly)) {
    qs2.sprintf("Could not write to file %s.", qs.latin1());
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
    saveText << arpData->arpWidget(l1)->arpName.latin1() << "\n";
    arpData->arpWidget(l1)->writeArp(&f);
  }
}
