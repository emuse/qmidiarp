/*!
 * @file arpscreen.h
 * @brief Header for the ArpScreen class
 */
#ifndef ARPSCREEN_H
#define ARPSCREEN_H

#include <QWidget>
#include <QString>
#include <QLabel>
#include <QTimer>
#include <QSizePolicy>
#include <QSize>

#include "midilfo.h"

#define ARPSCR_MIN_W   250
#define ARPSCR_MIN_H  120
#define ARPSCR_VMARG          10
#define ARPSCR_HMARG          16




/*! @brief Drawing widget for visualization of arp patterns using QPainter
 *
 * ArpScreen is created and embedded by ArpWidget. The painter callback
 * analyses the pattern string and produces a streak map of its content
 * similar to a piano roll display. The display is updated
 * by calling ArpScreen::updateScreen() with the pattern text string as
 * and argument. A cursor is placed at the corresponding pattern index
 * by calling ArpScreen::updateScreen() with the integer current pattern
 * index as an overloaded member.
 */
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
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

  public slots:
    void updateScreen(const QString&);
    void updateScreen(int p_index);
    void setGrooveTick(int tick);
    void setGrooveVelocity(int vel);
    void setGrooveLength(int length);
    void setMuted(bool on);
};

#endif
