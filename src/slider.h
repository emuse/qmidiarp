#ifndef SLIDER_H
#define SLIDER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>

class Slider : public QVBox

{
  Q_OBJECT

  private:
    QSlider *slider;
    QLabel *valueLabel, *minLabel, *maxLabel;
    
  public:
    Slider(int minValue, int maxValue, int pageStep, int value, 
           Orientation orientation, QWidget * parent, const char * name = 0 );
    ~Slider();
    int value();
    
  signals:
    void valueChanged(int);
    
  public slots:
    void setValue(int);
    
  private slots:
    void updateLabel(int);  
};
  
#endif
