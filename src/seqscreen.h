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
    QVector<Sample> p_data, data;
    int mouseX, mouseY;
    int w, h;
    bool recordMode;
    int currentRecStep;
    bool isMuted;

  protected:
    virtual void paintEvent(QPaintEvent *);

  public:
    SeqScreen(QWidget* parent=0);
    ~SeqScreen();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  signals:
    void mousePressed(double, double, int);
    void mouseMoved(double, double, int);

  public slots:
    void updateScreen(const QVector<Sample>& data);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void setRecord(bool on);
    void setCurrentRecStep(int currentRecStep);
    void setMuted(bool on);
};

#endif
