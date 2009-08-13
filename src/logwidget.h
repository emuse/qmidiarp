#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
    QTextEdit *logText;
    QCheckBox *enableLog;
    QCheckBox *logMidiClock;
    bool logActive;
  	bool logMidiActive;


  public:
    LogWidget(QWidget* parent=0);
    ~LogWidget();
  signals:
	// void runQueue(bool);
	 
  public slots:
    void logMidiToggle(bool on);
	void enableLogToggle(bool on);
    void appendEvent(snd_seq_event_t *ev);
	void appendText(QString);
    void clear();
};
  
#endif
