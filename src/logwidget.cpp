#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qboxlayout.h>
#include <qgroupbox.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <alsa/asoundlib.h>
#include "logwidget.h"

LogWidget::LogWidget(QWidget *parent) : QWidget(parent) {
QVBoxLayout *logWidgetLayout = new QVBoxLayout;
   logText = new QTextEdit(this);
  logActive = true;
  QWidget *buttonBox = new QWidget(this);
  QHBoxLayout *buttonBoxLayout = new QHBoxLayout;
  QLabel *enableLabel = new QLabel("Enable Log", buttonBox);
  enableLog = new QCheckBox(buttonBox);
  enableLog->setChecked(logActive);
  QObject::connect(enableLog, SIGNAL(toggled(bool)), this, SLOT(enableLogToggle(bool)));
  new QWidget(buttonBox);
  QPushButton *clearButton = new QPushButton("Clear", buttonBox);
  QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));


  buttonBoxLayout->addWidget(enableLabel);
  buttonBoxLayout->addWidget(enableLog);
  buttonBoxLayout->addWidget(clearButton);
  buttonBoxLayout->setMargin(2);
  buttonBoxLayout->setSpacing(2);
  logWidgetLayout->addWidget(logText);
  logWidgetLayout->addWidget(buttonBox);
buttonBox->setLayout(buttonBoxLayout);
setLayout(logWidgetLayout);

}

LogWidget::~LogWidget() {
  
}

void LogWidget::appendEvent(snd_seq_event_t *ev) {

  QString qs, qs2;
  
  if (!logActive) {
    return;
  }
  switch (ev->type) {
    case SND_SEQ_EVENT_NOTEON:
      qs.sprintf("Ch %2d, Note On %3d, Vel %3d", ev->data.control.channel, 
                  ev->data.note.note, ev->data.note.velocity);
      break;
    case SND_SEQ_EVENT_NOTEOFF:
      qs.sprintf("Ch %2d, Note Off %3d", ev->data.control.channel, 
                  ev->data.note.note);
      break;
    case SND_SEQ_EVENT_CONTROLLER:
      qs.sprintf("Ch %2d, Ctrl %3d, Val %3d", ev->data.control.channel, 
                  ev->data.control.param, ev->data.control.value);
      break;
    case SND_SEQ_EVENT_PITCHBEND:
      qs.sprintf("Ch %2d, Pitch %5d", ev->data.control.channel, 
                  ev->data.control.value);
      break;
    case SND_SEQ_EVENT_PGMCHANGE:
      qs.sprintf("Ch %2d, PrgChg %5d", ev->data.control.channel, 
                  ev->data.control.value);
      break;
    default:
      qs.sprintf("Unknown event type");
      break;
  }
  logText->append(QTime::currentTime().toString("hh:mm:ss.zzz") + "  " + qs);
}

void LogWidget::enableLogToggle(bool on) {

  logActive = on;
}

void LogWidget::clear() {

  logText->clear();
}

