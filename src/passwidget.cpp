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
    discardCheck = new QCheckBox(this);
    discardCheck->setText(tr("&Discard unmatched events"));
    discardCheck->setChecked(true);
    QObject::connect(discardCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateDiscard(bool)));

    portLabel = new QLabel(tr("&Send unmatched events to port"), this);
    portUnmatchedSpin = new QSpinBox(this);
    portLabel->setBuddy(portUnmatchedSpin);
    portLabel->setDisabled(true);
    portUnmatchedSpin->setDisabled(true);
    portUnmatchedSpin->setRange(1, p_portcount);
 	portUnmatchedSpin->setKeyboardTracking(false);
    QObject::connect(portUnmatchedSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updatePortUnmatched(int)));
			
    QHBoxLayout *portBoxLayout = new QHBoxLayout;
    portBoxLayout->addWidget(portLabel);
    portBoxLayout->addStretch(1);
    portBoxLayout->addWidget(portUnmatchedSpin);


	
    mtpbLabel = new QLabel(tr("MIDI &Clock rate (tpb)"), this);
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

    cbuttonCheck = new QCheckBox(this);
    cbuttonCheck->setText(tr("Use MIDI &Controller to mute arps"));
    cbuttonCheck->setChecked(true);
    QObject::connect(cbuttonCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateControlSetting(bool)));
	
    cnumberSpin = new QSpinBox(this);
    QObject::connect(cnumberSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateCnumber(int)));
    cnumberSpin->setRange(24,127);
    cnumberSpin->setValue(37);
 	cnumberSpin->setKeyboardTracking(false);
	
    cnumberLabel = new QLabel(tr("First arp is muted by CC#"), this);
    cnumberLabel->setBuddy(cnumberSpin);
	
    QHBoxLayout *cnumberLayout = new QHBoxLayout;
    cnumberLayout->addWidget(cnumberLabel);
    cnumberLayout->addStretch(1);
    cnumberLayout->addWidget(cnumberSpin);

    QVBoxLayout *passWidgetLayout = new QVBoxLayout;
    passWidgetLayout->addWidget(discardCheck);
    passWidgetLayout->addLayout(portBoxLayout);
    passWidgetLayout->addLayout(mtpbBoxLayout);
    passWidgetLayout->addWidget(cbuttonCheck);
    passWidgetLayout->addLayout(cnumberLayout);
    passWidgetLayout->addStretch();

    setLayout(passWidgetLayout);
}

PassWidget::~PassWidget()
{
}

void PassWidget::updateDiscard(bool on)
{
    emit discardToggled(on);
    portUnmatchedSpin->setDisabled(on);
    portLabel->setDisabled(on);
}

void PassWidget::updatePortUnmatched(int id)
{
    emit newPortUnmatched(id - 1);
}

void PassWidget::setDiscard(bool on)
{
    discardCheck->setChecked(on);
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
    cnumberLabel->setEnabled(on);
    emit midiMuteToggle(on);

}

void PassWidget::updateCnumber(int cnumber)
{
	emit newCnumber(cnumber);
}



