#ifndef SLIDER_H
#define SLIDER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QString>
#include <QLabel>
#include <QWidget>
#include <QSlider>
#include <QSpinBox> 
#include <QBoxLayout>

class Slider : public QWidget

{
  Q_OBJECT

  private:
    QSlider *slider;
    QSpinBox *sliderSpin;
    
  public:
    Slider(int minValue, int maxValue, int pageStep, int tickStep, int value, 
           Qt::Orientation orientation, QString label, QWidget * parent);
    ~Slider();
    int value();
    
  signals:
    void valueChanged(int);
    
  public slots:
    void setValue(int);
    
  private slots:
    void updateSpinBox(int);  
};
  
#endif
