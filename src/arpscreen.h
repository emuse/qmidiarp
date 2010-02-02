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
    QString pattern;
    QString a_pattern;
    double follower_tick;
    int offset_tick, last_tick, pattern_updated;
    
  protected:
    virtual void paintEvent(QPaintEvent *);
    
  public:
    ArpScreen(QWidget* parent=0);
    ~ArpScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;
   
  public slots: 
    void updateArpScreen(const QString&);
    void updateArpScreen(int tick);
    void setGrooveTick(int tick);
    void setGrooveVelocity(int vel);
    void setGrooveLength(int length);
};
  
#endif
