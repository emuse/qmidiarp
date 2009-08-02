#include <stdio.h>
#include <stdlib.h>
#include <qstring.h>
#include <qlabel.h>
#include <qslider.h> 
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qgroupbox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregexp.h>
#include "groovewidget.h"
#include "slider.h"

GrooveWidget::GrooveWidget(QWidget *parent) : QWidget(parent) {
QVBoxLayout *GrooveWidgetLayout = new QVBoxLayout;
  
  QWidget *tickBox = new QWidget(this);
  QHBoxLayout *tickBoxLayout = new QHBoxLayout;
  QLabel *tickLabel = new QLabel("Groove Shift", tickBox);
  tickLabel->setFixedWidth(120);
  grooveTick = new Slider(-100,100,0,0, Qt::Horizontal, this);
  QObject::connect(grooveTick, SIGNAL(valueChanged(int)), this, SLOT(updateGrooveTick(int)));

  tickBoxLayout->addWidget(tickLabel);
  tickBoxLayout->addWidget(grooveTick);
  tickBox->setLayout(tickBoxLayout);
  
  QWidget *velocityBox = new QWidget(this);
  
  QHBoxLayout *velocityBoxLayout = new QHBoxLayout;
  QLabel *velocityLabel = new QLabel("Groove Velocity", velocityBox);
  velocityLabel->setFixedWidth(120);
  grooveVelocity = new Slider(-100,100,0,0, Qt::Horizontal, this);
  QObject::connect(grooveVelocity, SIGNAL(valueChanged(int)), this, SLOT(updateGrooveVelocity(int)));


  velocityBoxLayout->addWidget(velocityLabel);
  velocityBoxLayout->addWidget(grooveVelocity);
  velocityBox->setLayout(velocityBoxLayout);
 
  QWidget *lengthBox = new QWidget(this);
  QHBoxLayout *lengthBoxLayout = new QHBoxLayout;
  QLabel *lengthLabel = new QLabel("Groove Length", lengthBox);
  lengthLabel->setFixedWidth(120);
  grooveLength = new Slider(-100,100,0,0, Qt::Horizontal, this);
  QObject::connect(grooveLength, SIGNAL(valueChanged(int)), this, SLOT(updateGrooveLength(int)));
 

  lengthBoxLayout->addWidget(lengthLabel);
  lengthBoxLayout->addWidget(grooveLength);
  lengthBox->setLayout(lengthBoxLayout);
  
  GrooveWidgetLayout->setMargin(1);
  GrooveWidgetLayout->setSpacing(1);
  GrooveWidgetLayout->addWidget(tickBox);
  GrooveWidgetLayout->addWidget(velocityBox);
  GrooveWidgetLayout->addWidget(lengthBox);
  setLayout(GrooveWidgetLayout);
}

GrooveWidget::~GrooveWidget() {
  
}

void GrooveWidget::updateGrooveVelocity(int val) {

  emit(newGrooveVelocity(val));
}

void GrooveWidget::updateGrooveTick(int val) {

  emit(newGrooveTick(val));
}

void GrooveWidget::updateGrooveLength(int val) {

  emit(newGrooveLength(val));
}

        
