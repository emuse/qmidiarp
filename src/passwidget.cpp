#include <QLabel>
#include <QSlider> 
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>

#include "passwidget.h"


PassWidget::PassWidget(int p_portcount, QWidget *parent) : QWidget(parent)
{
    forwardCheck = new QCheckBox(this);
    forwardCheck->setText(tr("&Forward unmatched events to port"));
    forwardCheck->setChecked(false);
    QObject::connect(forwardCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateForward(bool)));

    portUnmatchedSpin = new QSpinBox(this);
    portUnmatchedSpin->setDisabled(true);
    portUnmatchedSpin->setRange(1, p_portcount);
    portUnmatchedSpin->setKeyboardTracking(false);
    QObject::connect(portUnmatchedSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updatePortUnmatched(int)));
            
    compactStyleCheck = new QCheckBox(this);
    compactStyleCheck->setText(tr("&Compact module layout style"));
    QObject::connect(compactStyleCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateCompactStyle(bool)));
    compactStyle = false;
    
    QHBoxLayout *portBoxLayout = new QHBoxLayout;
    portBoxLayout->addWidget(forwardCheck);
    portBoxLayout->addStretch(1);
    portBoxLayout->addWidget(portUnmatchedSpin);

    cbuttonCheck = new QCheckBox(this);
    cbuttonCheck->setText(tr("&Modules mutable by MIDI controller starting at CC#"));
    cbuttonCheck->setChecked(true);
    QObject::connect(cbuttonCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateControlSetting(bool)));
    
    cnumberSpin = new QSpinBox(this);
    QObject::connect(cnumberSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateCnumber(int)));
    cnumberSpin->setRange(24,127);
    cnumberSpin->setValue(37);
    cnumberSpin->setKeyboardTracking(false);
        
    QHBoxLayout *cnumberLayout = new QHBoxLayout;
    cnumberLayout->addWidget(cbuttonCheck);
    cnumberLayout->addStretch(1);
    cnumberLayout->addWidget(cnumberSpin);
 
    
    mtpbLabel = new QLabel(tr("Incoming MIDI &Clock rate (tpb)"), this);
    mtpbSpin = new QSpinBox(this);
    mtpbLabel->setBuddy(mtpbSpin);
    QObject::connect(mtpbSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateMIDItpb_pw(int)));
    mtpbSpin->setRange(24,384);
    mtpbSpin->setValue(96);
    mtpbSpin->setSingleStep(24);
    mtpbSpin->setKeyboardTracking(false);
    
    QHBoxLayout *mtpbBoxLayout = new QHBoxLayout;
    mtpbBoxLayout->addWidget(mtpbLabel);
    mtpbBoxLayout->addStretch(1);
    mtpbBoxLayout->addWidget(mtpbSpin);

    QVBoxLayout *passWidgetLayout = new QVBoxLayout;
    passWidgetLayout->addLayout(portBoxLayout);
    passWidgetLayout->addLayout(cnumberLayout);
    passWidgetLayout->addLayout(mtpbBoxLayout);
    passWidgetLayout->addWidget(compactStyleCheck);
    passWidgetLayout->addStretch();

    setLayout(passWidgetLayout);
}

PassWidget::~PassWidget()
{
}

void PassWidget::updateForward(bool on)
{
    emit forwardToggled(on);
    portUnmatchedSpin->setDisabled(!on);
}

void PassWidget::updatePortUnmatched(int id)
{
    emit newPortUnmatched(id - 1);
}

void PassWidget::setForward(bool on)
{
    forwardCheck->setChecked(on);
}

void PassWidget::setPortUnmatched(int id)
{
    portUnmatchedSpin->setValue(id);
}

void PassWidget::updateMIDItpb_pw(int MIDItpb)
{
    emit newMIDItpb(MIDItpb);
}

void PassWidget::updateControlSetting(bool on)
{
    cnumberSpin->setEnabled(on);
    emit midiMuteToggle(on);
}

void PassWidget::updateCnumber(int cnumber)
{
    emit newCnumber(cnumber);
}

void PassWidget::updateCompactStyle(bool on)
{
    compactStyle = on;
}



