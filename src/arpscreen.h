#ifndef ARPSCREEN_H
#define ARPSCREEN_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <QSizePolicy>
#include <QSize>
#include <alsa/asoundlib.h>

#define ARPSCREEN_MINIMUM_WIDTH   250
#define ARPSCREEN_MINIMUM_HEIGHT  120
#define ARPSCREEN_VMARGIN          10
#define ARPSCREEN_HMARGIN          16




class ArpScreen : public QWidget
{
  Q_OBJECT

  private:
    int grooveTick, grooveVelocity, grooveLength;
    //QTimer *timer;
    QString pattern;
    QString a_pattern;

  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent(QResizeEvent *);
    double follower_tick;
   
  public:
    ArpScreen(QWidget* parent=0);
    ~ArpScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
   
  public slots: 
    void updateArpScreen(const QString&);
    void updateArpScreen(snd_seq_tick_time_t tick);
    void setGrooveTick(int tick);
    void setGrooveVelocity(int vel);
    void setGrooveLength(int length);
};
  
#endif
