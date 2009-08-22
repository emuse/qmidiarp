#ifndef ARPSCREEN_H
#define ARPSCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <QSizePolicy>
#include <QSize>
#include <alsa/asoundlib.h>

#define ARPSCREEN_MINIMUM_WIDTH   250
#define ARPSCREEN_MINIMUM_HEIGHT   80
#define ARPSCREEN_VMARGIN          10
#define ARPSCREEN_HMARGIN          16




class ArpScreen : public QWidget
{
  Q_OBJECT

  private:
    int maxRef;
    int globalMax, globalMaxResetCount;
    //QTimer *timer;
    QString pattern;
    QString a_pattern;

  protected:
    virtual void paintEvent(QPaintEvent *);
    double follower_tick;
   
  public:
    ArpScreen(int p_maxRef, QWidget* parent=0);
    ~ArpScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
   
  public slots: 
    void updateArpScreen(const QString&);
    void updateArpScreen(snd_seq_tick_time_t tick);
};
  
#endif
