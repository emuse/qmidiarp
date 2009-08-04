#ifndef GUI_H
#define GUI_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qboxlayout.h>
#include <qlist.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include "arpwidget.h"
#include "logwidget.h"
#include "arpdata.h"
#include "passwidget.h"
#include "groovewidget.h"
#include "arpscreen.h"

const QString aboutText = "QMidiArp 0.0.2\nby Matthias Nagorni\n(c)2004 Novell\n\n"
						  "Qt4 port by Frank Kober 2009\n\n"
                          "QMidiArp is licensed under the GPL.\n";

class Gui : public QWidget
{
  Q_OBJECT

  private:
    QMessageBox *aboutWidget;
    
    PassWidget *passWidget;
    GrooveWidget *grooveWidget;
    QPushButton *removeArpButton;
    QPushButton *renameArpButton;
    QPushButton *addArpButton;
	QTabWidget *tabWidget;
    LogWidget *logWidget;
    ArpData *arpData;
    QSpinBox *tempoSpin;
	QCheckBox  *runQueueCheck;
	
  public:
    Gui(int p_portCount, QWidget* parent=0);
    ~Gui();
  
  signals:  
	void newTempo(int);
    void runQueue(bool);

	
  public slots: 
    void displayAbout();
    void addArp();
    void addArp(QString qs);
    void renameArp();
    void removeArp();
    void removeArp(int index);
    void save();
    void load();
    void load(QString name);
    void clear();
	void updateTempo(int tempo);
	void updateRunQueue(bool on);
	void midiClockToggle (bool on);
};
  
#endif
