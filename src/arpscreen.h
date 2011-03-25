#ifndef ARPSCREEN_H
#define ARPSCREEN_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <QSizePolicy>
#include <QSize>

#include "midilfo.h"

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
    int pattern_updated, currentIndex;
    bool isMuted;

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    ArpScreen(QWidget* parent=0);
    ~ArpScreen();

  public slots:
    void updateScreen(const QString&);
    void updateScreen(int p_index);
    void setGrooveTick(int tick);
    void setGrooveVelocity(int vel);
    void setGrooveLength(int length);
    void setMuted(bool on);
};

#endif
