/*!
 * @file seqscreen.h
 * @brief Header for the SeqScreen class
 */
#ifndef SEQSCREEN_H
#define SEQSCREEN_H

#include <QLabel>
#include <QMouseEvent>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <QSizePolicy>
#include <QSize>

#include "midiseq.h"

#define SEQSCR_MIN_W   180
#define SEQSCR_MIN_H   212
#define SEQSCR_VMARG    10
#define SEQSCR_HMARG    20

/*! @brief Drawing widget for visualization of sequences using QPainter
 *
 * SeqScreen is created and embedded by SeqWidget. The painter callback
 * produces a streak map of a waveform as a piano roll display. The
 * display is updated by calling SeqScreen::updateScreen() with the
 * Sample vector as argument. A cursor is placed at the corresponding
 * vector index by calling SeqScreen::updateScreen() with the integer
 * current index as an overloaded member.
 * SeqScreen emits mouse events corresponding to the Qt mousePressed()
 * and mouseMoved() events. The mouse position is transferred as a
 * double from 0 ... 1.0 representing the relative mouse position on the
 * entire SeqScreen display area.
 */
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
    int currentIndex;
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
    void updateScreen(int currentIndex);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void setRecordMode(bool on);
    void setCurrentRecStep(int currentRecStep);
    void setMuted(bool on);
};

#endif
