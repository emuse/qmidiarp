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

    QHBoxLayout *portBoxLayout = new QHBoxLayout;
    portLabel = new QLabel(tr("&Send unmatched events to port"), this);
    portUnmatchedSpin = new QSpinBox(this);
    portLabel->setBuddy(portUnmatchedSpin);
    portLabel->setDisabled(true);
    portUnmatchedSpin->setDisabled(true);
    portUnmatchedSpin->setRange(1, p_portcount);
 	portUnmatchedSpin->setKeyboardTracking(false);
    QObject::connect(portUnmatchedSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updatePortUnmatched(int)));
    portBoxLayout->addWidget(portLabel);
    portBoxLayout->addStretch(1);
    portBoxLayout->addWidget(portUnmatchedSpin);

    QHBoxLayout *mtpbBoxLayout = new QHBoxLayout;
    mtpbLabel = new QLabel(tr("MIDI &Clock rate (tpb)"), this);
    mtpbSpin = new QSpinBox(this);
    mtpbLabel->setBuddy(mtpbSpin);
    QObject::connect(mtpbSpin, SIGNAL(valueChanged(int)), this,
            SLOT(updateMIDItpb_pw(int)));
    mtpbSpin->setRange(24,384);
    mtpbSpin->setValue(96);
    mtpbSpin->setSingleStep(24);
 	mtpbSpin->setKeyboardTracking(false);
    mtpbSpin->setDisabled(true);
    mtpbLabel->setDisabled(true);
    mtpbBoxLayout->addWidget(mtpbLabel);
    mtpbBoxLayout->addStretch(1);
    mtpbBoxLayout->addWidget(mtpbSpin);

    mbuttonCheck = new QCheckBox(this);
    mbuttonCheck->setText(tr("&Use incoming MIDI Clock"));
    QObject::connect(mbuttonCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateClockSetting(bool)));
    mbuttonCheck->setChecked(false);
    mbuttonCheck->setDisabled(true);

    QVBoxLayout *passWidgetLayout = new QVBoxLayout;
    passWidgetLayout->addWidget(discardCheck);
    passWidgetLayout->addLayout(portBoxLayout);
    passWidgetLayout->addWidget(mbuttonCheck);
    passWidgetLayout->addLayout(mtpbBoxLayout);
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

void PassWidget::updateClockSetting(bool on)
{
    mtpbSpin->setEnabled(on);
    mtpbLabel->setEnabled(on);
    emit midiClockToggle(on);
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

