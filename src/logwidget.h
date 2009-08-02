#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qdatetime.h>
#include <alsa/asoundlib.h>

class LogWidget : public QVBox

{
  Q_OBJECT

  private:
    QTextEdit *logText;
    QCheckBox *enableLog;
    bool logActive;

  public:
    LogWidget(QWidget* parent=0, const char *name=0);
    ~LogWidget();
    
  public slots:
    void enableLogToggle(bool on);
    void appendEvent(snd_seq_event_t *ev);
    void clear();
};
  
#endif
