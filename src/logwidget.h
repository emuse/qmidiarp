#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QBoxLayout>
#include <QString>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDateTime>

#include "arpdata.h"


class LogWidget : public QWidget

{
  Q_OBJECT

  private:
    QVBoxLayout vBox;
    QTextEdit *logText;
    bool logActive;
    bool logMidiActive;


  public:
    LogWidget(QWidget* parent=0);
    ~LogWidget();
    QCheckBox *enableLog;
    QCheckBox *logMidiClock;

  public slots:
    void logMidiToggle(bool on);
    void enableLogToggle(bool on);
    void appendEvent(int type, int data, int channel, int value);
    void appendText(const QString&);
    void clear();
};

#endif
