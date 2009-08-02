#ifndef PASSWIDGET_H
#define PASSWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>

class PassWidget : public QWidget

{
  Q_OBJECT

  private:
    QCheckBox *discardCheck, *runQueueCheck;
    QSpinBox *portUnmatchedSpin;
    QSpinBox *tempoSpin;
          
  public:
    PassWidget(int p_portcount, QWidget* parent=0);
    ~PassWidget();
    void setDiscard(bool on);
    void setPortUnmatched(int id);
    
  signals:
    void discardToggled(bool);  
    void newPortUnmatched(int);
    void newTempo(int);
    void runQueue(bool);
        
  public slots:
    void updateDiscard(bool on);
    void updatePortUnmatched(int);
    void updateTempo(int);
    void updateRunQueue(bool);
};
  
#endif
