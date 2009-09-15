#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QString>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDateTime>

#include <alsa/asoundlib.h>

#include "arpdata.h"


class LogWidget : public QWidget

{
  Q_OBJECT

  private:
	QVBoxLayout vBox;
    QTextEdit *logText;
    QCheckBox *enableLog;
    QCheckBox *logMidiClock;
    bool logActive;
    bool logMidiActive;


  public:
    LogWidget(QWidget* parent=0);
    ~LogWidget();
	 
  public slots:
    void logMidiToggle(bool on);
    void enableLogToggle(bool on);
    void appendEvent(snd_seq_event_t *ev);
    void appendText(const QString&);
    void clear();
};
  
#endif
