#ifndef PASSWIDGET_H
#define PASSWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QString>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>

class PassWidget : public QWidget

{
  Q_OBJECT

  private:
    QCheckBox *discardCheck;
    QSpinBox *portUnmatchedSpin, *mtpbSpin;
	QLabel *portLabel, *mtpbLabel;
          
  public:
    PassWidget(int p_portcount, QWidget* parent=0);
    ~PassWidget();
    void setDiscard(bool on);
    void setPortUnmatched(int id);
	QCheckBox *mbuttonCheck;
    
  signals:
    void discardToggled(bool);  
    void newPortUnmatched(int);
    void midiClockToggle(bool);
	void newMIDItpb(int);
	    
  public slots:
    void updateClockSetting(bool);
	void updateDiscard(bool on);
    void updatePortUnmatched(int);
    void updateMIDItpb_pw(int);
   
};
  
#endif
