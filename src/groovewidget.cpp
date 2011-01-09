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

    grooveTick = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Shift"), this);
    connect(grooveTick, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveTick(int)));

    grooveVelocity = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Velocity"), this);
    connect(grooveVelocity, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveVelocity(int)));

    grooveLength = new Slider(-100, 100, 1, 10, 0, Qt::Horizontal,
            tr("&Length"), this);
    connect(grooveLength, SIGNAL(valueChanged(int)),
            this, SLOT(updateGrooveLength(int)));

    GrooveWidgetLayout->addWidget(grooveTick);
    GrooveWidgetLayout->addWidget(grooveVelocity);
    GrooveWidgetLayout->addWidget(grooveLength);
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

