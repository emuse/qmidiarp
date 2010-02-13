#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

#include "mainwindow.h"
#include "passwidget.h"

PassWidget::PassWidget(ArpData *p_arpData, int p_portcount, QWidget *parent)
            : QDialog(parent)
{
    arpData = p_arpData;
    
    QWidget *passWidgetBox = new QWidget(this);
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
            
    QHBoxLayout *portBoxLayout = new QHBoxLayout;
    portBoxLayout->addWidget(forwardCheck);
    portBoxLayout->addStretch(1);
    portBoxLayout->addWidget(portUnmatchedSpin);

    cbuttonCheck = new QCheckBox(this);
    cbuttonCheck->setText(tr("&Modules controllable by MIDI controller"));
    cbuttonCheck->setChecked(true);
    QObject::connect(cbuttonCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateControlSetting(bool)));
    
    QLabel *mtpbLabel = new QLabel(tr("Incoming MIDI &Clock rate (tpb)"), this);
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

    compactStyleCheck = new QCheckBox(this);
    compactStyleCheck->setText(tr("&Compact module layout style"));
    QObject::connect(compactStyleCheck, SIGNAL(toggled(bool)), this,
            SLOT(updateCompactStyle(bool)));
    compactStyle = false;
        
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));    

    QVBoxLayout *passWidgetLayout = new QVBoxLayout;
    passWidgetLayout->addLayout(portBoxLayout);
    passWidgetLayout->addWidget(cbuttonCheck);
    passWidgetLayout->addLayout(mtpbBoxLayout);
    passWidgetLayout->addWidget(compactStyleCheck);
    passWidgetLayout->addWidget(buttonBox);
    passWidgetLayout->addStretch();

    passWidgetBox->setLayout(passWidgetLayout);
    
    setModal(true);
    setMinimumWidth(400);
    setMinimumHeight(150);
    setWindowTitle(tr("Settings - ") + APP_NAME);
}

PassWidget::~PassWidget()
{
}

void PassWidget::updateForward(bool on)
{
    arpData->seqDriver->setForwardUnmatched(on);
    portUnmatchedSpin->setDisabled(!on);
}

void PassWidget::updatePortUnmatched(int id)
{
    arpData->seqDriver->setPortUnmatched(id - 1);
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
    arpData->seqDriver->updateMIDItpb(MIDItpb);
}

void PassWidget::updateControlSetting(bool on)
{
    arpData->seqDriver->setMidiControllable(on);
}

void PassWidget::updateCompactStyle(bool on)
{
    compactStyle = on;
    arpData->setCompactStyle(on);
}



