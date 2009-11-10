#ifndef ARPWIDGET_H
#define ARPWIDGET_H

#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QAction>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include "midiarp.h"
#include "slider.h"
#include "arpscreen.h"

class ArpWidget : public QWidget

{
  Q_OBJECT

  private:
    QSpinBox *chIn;                        // Channel of input events
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QSpinBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QComboBox *repeatPatternThroughChord;
    QComboBox *patternPresetBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;

    MidiArp *midiArp;
    QLineEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
    Slider *attackTime, *releaseTime;
    bool modified;
    
  public:
    QString arpName;
    ArpScreen *arpScreen;
    QStringList patternPresets, patternNames;
    QCheckBox *muteOut;

  public:
    ArpWidget(MidiArp *p_midiArp, int portCount, QWidget* parent=0);
    ~ArpWidget();
    MidiArp *getMidiArp();
    
    void readArp(QTextStream& arpText);
    void writeArp(QTextStream& arpText);
    void setChIn(int value);
    void setIndexIn(int index, int value);
    void setChannelOut(int value);
    void setPortOut(int value);
    void setRangeIn(int index, int value);
    void loadPatternPresets();
    bool isModified();
    void setModified(bool);
      
  signals:
    void patternChanged();
    void presetsChanged(QString, QString, int); 
                    //int 0 for pattern to append
                    //or index>0 for pattern to remove
    
  public slots:
    void updateChIn(int value);
    void updateIndexIn(int value);
    void updateRangeIn(int value);
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateText(QString newtext);
    void updateRepeatPattern(int);
    void selectPatternPreset(int);
    void updatePatternPresets(QString n, QString p, int index);
    void openTextEditWindow(bool on);
    void storeCurrentPattern();
    void removeCurrentPattern();
};
  
#endif
