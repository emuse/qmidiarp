#ifndef PASSWIDGET_H
#define PASSWIDGET_H

#include <QComboBox>
#include <QDialog>
#include <QSpinBox>

#include "arpdata.h"

class PassWidget : public QDialog

{
  Q_OBJECT

  private:
    ArpData *arpData;
    QCheckBox *forwardCheck;
    QComboBox *portUnmatchedSpin;

  public:
    PassWidget(ArpData* arpData, int p_portcount, QWidget* parent=0);
    ~PassWidget();
    void setForward(bool on);
    void setPortUnmatched(int id);
    QCheckBox *cbuttonCheck, *compactStyleCheck;
    bool compactStyle;

  signals:
    void compactLayoutToggle(bool);

  public slots:
    void updateForward(bool on);
    void updatePortUnmatched(int);
    void updateControlSetting(bool);
    void updateCompactStyle(bool);
};

#endif
