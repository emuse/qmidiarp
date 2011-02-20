#ifndef LFOSCREEN_H
#define LFOSCREEN_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QMouseEvent>
#include <QTimer>
#include <QSizePolicy>
#include <QSize>
#include <QWheelEvent>
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
    QVector<Sample> p_data, data;
    int mouseX, mouseY, mouseW;
    int w, h;
    int currentIndex;
    int clip(int value, int min, int max, bool *outOfRange);
    bool recordMode;
    bool isMuted;
    bool onlyCursor;

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    LfoScreen(QWidget* parent=0);
    ~LfoScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  signals:
    void mousePressed(double, double, int);
    void mouseMoved(double, double, int);
    void mouseWheel(int);

  public slots:
    void updateScreen(const QVector<Sample>& data);
    void updateScreen(int p_index);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void setRecord(bool on);
    void wheelEvent(QWheelEvent* event);
    void setMuted(bool on);
};

#endif
