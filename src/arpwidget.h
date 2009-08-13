#ifndef ARPWIDGET_H
#define ARPWIDGET_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QString>
#include <QLabel>
#include <QSlider>
#include <QBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QFile>
#include <QPlainTextEdit>
#include <QTextStream>
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
	QCheckBox *muteOut;
	QGroupBox *randomBox;
	QToolButton *textEditButton, *randomButton, *textStoreButton, *textRemoveButton;
    QAction *textEditAction, *randomAction, *textStoreAction, *textRemoveAction;

	MidiArp *midiArp;
	QPlainTextEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
	
    
  public:
    QString arpName;
    ArpScreen *arpScreen;
	QStringList patternPresets, patternNames;

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
	void writePatternPresets();
  
  signals:
    void newPattern(QString);  
    void patternChanged();
    
  public slots:
    void updateChIn(int value);
    void updateIndexIn(int value);
    void updateRangeIn(int value);
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateText();
    void updateRepeatPattern(int);
	void updatePatternPreset(int);
	void openTextEditWindow(bool on);
	void storePatternText();
	void toggleRandomBox(bool on);
	void removeCurrentPattern();
};
  
#endif
