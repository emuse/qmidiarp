#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>

#include "groovewidget.h"
#include "slider.h"


GrooveWidget::GrooveWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *GrooveWidgetLayout = new QVBoxLayout;
    GrooveWidgetLayout->setMargin(6);
    GrooveWidgetLayout->setSpacing(8);

    QHBoxLayout *tickBoxLayout = new QHBoxLayout;
    QLabel *tickLabel = new QLabel(tr("Groove &shift"), this);
    grooveTick = new Slider(-100, 100, 0, 0, Qt::Horizontal, this);
    tickLabel->setBuddy(grooveTick);
    connect(grooveTick, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveTick(int)));

    tickBoxLayout->addWidget(tickLabel);
    tickBoxLayout->addStretch(1);
    tickBoxLayout->addWidget(grooveTick);

    QHBoxLayout *velocityBoxLayout = new QHBoxLayout;
    QLabel *velocityLabel = new QLabel(tr("Groove &velocity"), this);
    grooveVelocity = new Slider(-100, 100, 0, 0, Qt::Horizontal, this);
    velocityLabel->setBuddy(grooveVelocity);
    connect(grooveVelocity, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveVelocity(int)));

    velocityBoxLayout->addWidget(velocityLabel);
    velocityBoxLayout->addStretch(1);
    velocityBoxLayout->addWidget(grooveVelocity);

    QHBoxLayout *lengthBoxLayout = new QHBoxLayout;
    QLabel *lengthLabel = new QLabel(tr("Groove &length"), this);
    grooveLength = new Slider(-100, 100, 0, 0, Qt::Horizontal, this);
    lengthLabel->setBuddy(grooveLength);
    connect(grooveLength, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveLength(int)));

    lengthBoxLayout->addWidget(lengthLabel);
    lengthBoxLayout->addStretch(1);
    lengthBoxLayout->addWidget(grooveLength);

    GrooveWidgetLayout->addLayout(tickBoxLayout);
    GrooveWidgetLayout->addLayout(velocityBoxLayout);
    GrooveWidgetLayout->addLayout(lengthBoxLayout);
    GrooveWidgetLayout->addStretch();

    setLayout(GrooveWidgetLayout);
}

GrooveWidget::~GrooveWidget()
{
}

void GrooveWidget::updateGrooveVelocity(int val)
{
    emit(newGrooveVelocity(val));
}

void GrooveWidget::updateGrooveTick(int val)
{
    emit(newGrooveTick(val));
}

void GrooveWidget::updateGrooveLength(int val)
{
    emit(newGrooveLength(val));
}

