#ifndef GROOVEWIDGET_H
#define GROOVEWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include "slider.h"

class GrooveWidget : public QVBox

{
  Q_OBJECT

  public:
    Slider *grooveVelocity, *grooveTick, *grooveLength;
          
  public:
    GrooveWidget(QWidget* parent=0, const char *name=0);
    ~GrooveWidget();
    
  signals:
    void newGrooveVelocity(int);
    void newGrooveTick(int);
    void newGrooveLength(int);
            
  public slots:
    void updateGrooveVelocity(int);
    void updateGrooveTick(int);
    void updateGrooveLength(int);
};
  
#endif
