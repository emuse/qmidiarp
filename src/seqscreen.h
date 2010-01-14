#ifndef SEQSCREEN_H
#define SEQSCREEN_H

#include <QLabel>
#include <QMouseEvent>
#include <QSize>
#include <QSizePolicy>
#include <QString>
#include <QTimer>
#include <QWidget>
#include "midiseq.h"

#define SEQSCREEN_MINIMUM_WIDTH   180
#define SEQSCREEN_MINIMUM_HEIGHT  212
#define SEQSCREEN_VMARGIN          10
#define SEQSCREEN_HMARGIN          20


class SeqScreen : public QWidget
{
  Q_OBJECT

  private:
    //QTimer *timer;
    QVector<SeqSample> p_seqData, seqData;
    int mouseX, mouseY;
    int w, h;
    
  protected:
    virtual void paintEvent(QPaintEvent *);
   
  public:
    SeqScreen(QWidget* parent=0);
    ~SeqScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
    
  signals:
    void seqMousePressed(double, double, int);
    void seqMouseMoved(double, double, int);
    
  public slots: 
    void updateScreen(QVector<SeqSample> seqData);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
};
  
#endif