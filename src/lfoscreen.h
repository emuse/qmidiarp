#ifndef LFOSCREEN_H
#define LFOSCREEN_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <QSizePolicy>
#include <QSize>
#include "midilfo.h"

#define LFOSCREEN_MINIMUM_WIDTH   250
#define LFOSCREEN_MINIMUM_HEIGHT  120
#define LFOSCREEN_VMARGIN          10
#define LFOSCREEN_HMARGIN          20


class LfoScreen : public QWidget
{
  Q_OBJECT

  private:
    //QTimer *timer;
	QVector<LfoSample> p_lfoData, lfoData;

  protected:
    virtual void paintEvent(QPaintEvent *);
   
  public:
    LfoScreen(QWidget* parent=0);
    ~LfoScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
   
  public slots: 
    void updateLfoScreen(QVector<LfoSample> lfoData);
};
  
#endif
