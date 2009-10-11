#ifndef GROOVEWIDGET_H
#define GROOVEWIDGET_H

#include <QString>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include "slider.h"

class GrooveWidget : public QWidget

{
  Q_OBJECT

  public:
    Slider *grooveVelocity, *grooveTick, *grooveLength;
          
  public:
    GrooveWidget(QWidget* parent=0);
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
