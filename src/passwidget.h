#ifndef PASSWIDGET_H
#define PASSWIDGET_H

#include <cstdio>
#include <cstdlib>
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
    QCheckBox *forwardCheck;
    QSpinBox *portUnmatchedSpin, *mtpbSpin;
	QLabel *portLabel, *mtpbLabel, *cnumberLabel;
          
  public:
    PassWidget(int p_portcount, QWidget* parent=0);
    ~PassWidget();
    void setForward(bool on);
    void setPortUnmatched(int id);
	QCheckBox *cbuttonCheck;
	QSpinBox *cnumberSpin;
    
  signals:
    void forwardToggled(bool);  
    void newPortUnmatched(int);
    void midiMuteToggle(bool);
	void newMIDItpb(int);
	void newCnumber(int);
	    
  public slots:
	void updateForward(bool on);
    void updatePortUnmatched(int);
    void updateMIDItpb_pw(int);
    void updateCnumber(int);
    void updateControlSetting(bool);
   
};
  
#endif
