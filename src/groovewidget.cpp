#include <QString>
#include <QLabel>
#include <QSlider> 
#include <QBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QStringList>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QRegExp>

#include "groovewidget.h"
#include "slider.h"


GrooveWidget::GrooveWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *GrooveWidgetLayout = new QVBoxLayout;

    QHBoxLayout *tickBoxLayout = new QHBoxLayout;
    QLabel *tickLabel = new QLabel("Groove Shift", this);
    grooveTick = new Slider(-100, 100, 0, 0, Qt::Horizontal, this);
    connect(grooveTick, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveTick(int)));

    tickBoxLayout->addWidget(tickLabel);
    tickBoxLayout->addStretch(1);
    tickBoxLayout->addWidget(grooveTick);

    QHBoxLayout *velocityBoxLayout = new QHBoxLayout;
    QLabel *velocityLabel = new QLabel("Groove Velocity", this);
    grooveVelocity = new Slider(-100, 100, 0, 0, Qt::Horizontal, this);
    connect(grooveVelocity, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveVelocity(int)));

    velocityBoxLayout->addWidget(velocityLabel);
    velocityBoxLayout->addStretch(1);
    velocityBoxLayout->addWidget(grooveVelocity);

    QHBoxLayout *lengthBoxLayout = new QHBoxLayout;
    QLabel *lengthLabel = new QLabel("Groove Length", this);
    grooveLength = new Slider(-100, 100, 0, 0, Qt::Horizontal, this);
    connect(grooveLength, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveLength(int)));

    lengthBoxLayout->addWidget(lengthLabel);
    lengthBoxLayout->addStretch(1);
    lengthBoxLayout->addWidget(grooveLength);

    GrooveWidgetLayout->setMargin(1);
    GrooveWidgetLayout->setSpacing(1);
    GrooveWidgetLayout->addLayout(tickBoxLayout);
    GrooveWidgetLayout->addLayout(velocityBoxLayout);
    GrooveWidgetLayout->addLayout(lengthBoxLayout);
    setMaximumHeight(200);
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

