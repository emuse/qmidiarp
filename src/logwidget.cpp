#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QBoxLayout>
#include <QGroupBox>
#include <QFile>
#include <QRegExp>
#include <QDateTime>
#include <alsa/asoundlib.h>

#include "arpdata.h"
#include "logwidget.h"


LogWidget::LogWidget(QWidget *parent) : QWidget(parent)
{
    logActive = true;
    logMidiActive = false;

    QVBoxLayout *logWidgetLayout = new QVBoxLayout;
    logText = new QTextEdit(this);
    logText->setTextColor(QColor(0,0,255));
    logText->setCurrentFont(QFont("Courier", 7));
    
    QHBoxLayout *buttonBoxLayout = new QHBoxLayout;
    buttonBoxLayout->setMargin(2);
    buttonBoxLayout->setSpacing(4);

    QLabel *enableLabel = new QLabel("Enable Log", this);
    QCheckBox *enableLog = new QCheckBox(this);
    QObject::connect(enableLog, SIGNAL(toggled(bool)), this,
            SLOT(enableLogToggle(bool)));
    enableLog->setChecked(logActive);
    
    QLabel *MidiClockLabel = new QLabel("Log MIDI Clock", this);
    QCheckBox *logMidiClock = new QCheckBox(this);
    QObject::connect(logMidiClock, SIGNAL(toggled(bool)), this,
            SLOT(logMidiToggle(bool)));
    logMidiClock->setChecked(logMidiActive);
    
    QPushButton *clearButton = new QPushButton("Clear", this);
    QObject::connect(clearButton, SIGNAL(clicked()), this, SLOT(clear()));


    buttonBoxLayout->addWidget(enableLabel);
    buttonBoxLayout->addWidget(enableLog);
    buttonBoxLayout->addWidget(MidiClockLabel);
    buttonBoxLayout->addWidget(logMidiClock);
    buttonBoxLayout->addWidget(clearButton);

    logWidgetLayout->addWidget(logText);
    logWidgetLayout->addLayout(buttonBoxLayout);
    
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
            logText->setTextColor(QColor(100,160,0));
            qs.sprintf("Ch %2d, Ctrl %3d, Val %3d", ev->data.control.channel, 
                    ev->data.control.param, ev->data.control.value);
            break;
        case SND_SEQ_EVENT_PITCHBEND:
            logText->setTextColor(QColor(100,0,255));
            qs.sprintf("Ch %2d, Pitch %5d", ev->data.control.channel, 
                    ev->data.control.value);
            break;
        case SND_SEQ_EVENT_PGMCHANGE:
            logText->setTextColor(QColor(0,100,100));
            qs.sprintf("Ch %2d, PrgChg %5d", ev->data.control.channel, 
                    ev->data.control.value);
            break;
        case SND_SEQ_EVENT_CLOCK:
            if (logMidiActive) {
                logText->setTextColor(QColor(150,150,150));
                qs.sprintf("MIDI Clock");
            }
            break;
        case SND_SEQ_EVENT_START:
            logText->setTextColor(QColor(0,192,0));
            qs.sprintf("MIDI Start (Transport)");
            break;
        case SND_SEQ_EVENT_CONTINUE:
            logText->setTextColor(QColor(0,128,0));
            qs.sprintf("MIDI Continue (Transport)");
            break;
        case SND_SEQ_EVENT_STOP:
            logText->setTextColor(QColor(128,96,0));
            qs.sprintf("MIDI Stop (Transport)");
            break;
        default:
            logText->setTextColor(QColor(0,0,0));
            qs.sprintf("Unknown event type");
            break;
    }
    if ((ev->type != SND_SEQ_EVENT_CLOCK) || logMidiActive)
        logText->append(QTime::currentTime().toString(
                    "hh:mm:ss.zzz") + "  " + qs);
    logText->setTextColor(QColor(0,0,255));
}

void LogWidget::enableLogToggle(bool on)
{
    logActive = on;
}

void LogWidget::logMidiToggle(bool on)
{
    logMidiActive = on;
}

void LogWidget::clear()
{
    logText->clear();
}

void LogWidget::appendText(QString qs)
{
    logText->append(qs);
}

