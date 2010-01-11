#ifndef SLIDER_H
#define SLIDER_H

#include <QString>
#include <QSlider>
#include <QSpinBox> 
#include <QWidget>


class Slider : public QWidget

{
  Q_OBJECT

  private:
    QSlider *slider;
    QSpinBox *sliderSpin;
    
  public:
    Slider(int minValue, int maxValue, int pageStep, int tickStep, int value, 
           Qt::Orientation orientation, const QString& label, QWidget * parent);
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
