#ifndef PASSWIDGET_H
#define PASSWIDGET_H

#include <QCheckBox>
#include <QSpinBox>

class PassWidget : public QWidget

{
  Q_OBJECT

  private:
    QCheckBox *forwardCheck;
    QSpinBox *portUnmatchedSpin;
          
  public:
    PassWidget(int p_portcount, QWidget* parent=0);
    ~PassWidget();
    void setForward(bool on);
    void setPortUnmatched(int id);
    QCheckBox *cbuttonCheck, *compactStyleCheck;
    QSpinBox *mtpbSpin;
    bool compactStyle;
    
  signals:
    void forwardToggled(bool);  
    void newPortUnmatched(int);
    void midiControlToggle(bool);
    void compactLayoutToggle(bool);
    void newMIDItpb(int);
        
  public slots:
    void updateForward(bool on);
    void updatePortUnmatched(int);
    void updateMIDItpb_pw(int);
    void updateControlSetting(bool);   
    void updateCompactStyle(bool);   
};
  
#endif
