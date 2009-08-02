#ifndef GUI_H
#define GUI_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qptrlist.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
#include "arpwidget.h"
#include "logwidget.h"
#include "arpdata.h"
#include "passwidget.h"
#include "groovewidget.h"

const QString aboutText = "QMidiArp 0.0.2\nby Matthias Nagorni\n(c)2004 Novell\n\n"
                          "QMidiArp is licensed under the GPL.\n";

class Gui : public QVBox
{
  Q_OBJECT

  private:
    QMessageBox *aboutWidget;
    QTabWidget *tabWidget;
    ArpData *arpData;
    LogWidget *logWidget;
    PassWidget *passWidget;
    GrooveWidget *grooveWidget;
    QPushButton *removeMapButton;
    QPushButton *removeArpButton;
    
  public:
    Gui(int p_portCount, QWidget* parent=0, const char *name=0);
    ~Gui();
    
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
};
  
#endif
