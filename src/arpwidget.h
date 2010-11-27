#ifndef ARPWIDGET_H
#define ARPWIDGET_H

#include <QString>
#include <QTextStream>
#include <QToolButton>
#include <QAction>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "midiarp.h"
#include "slider.h"
#include "arpscreen.h"

#ifndef MIDICC_H
struct MidiCC {
        QString name;
        int min;
        int max;
        int ccnumber;
        int channel;
        int ID;
    };    
#define MIDICC_H
#endif

class ArpWidget : public QWidget

{
  Q_OBJECT

  private:
    QSpinBox *chIn;                        // Channel of input events
    QSpinBox *indexIn[2];                  // Index input
    QSpinBox *rangeIn[2];                  // Parameter that is mapped, [0] low, [1] high boundary
    QLabel *rangeInLabel, *indexInLabel;
    QSpinBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QComboBox *repeatPatternThroughChord;
    QComboBox *patternPresetBox;
    QGroupBox *inputFilterBox, *randomBox, *envelopeBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QToolButton *latchModeButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;
    QAction *latchModeAction;
    QAction *deleteAction, *renameAction;
    QAction *cancelMidiLearnAction;
    MidiArp *midiWorker;
    QLineEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
    Slider *attackTime, *releaseTime;
    bool modified;
    
  public:
    QString name;
    int ID, parentDockID;
    ArpScreen *screen;
    QStringList patternPresets, patternNames;
    QCheckBox *muteOut;

  public:
    ArpWidget(MidiArp *p_midiWorker, int portCount, bool compactStyle, QWidget* parent=0);
    ~ArpWidget();
    MidiArp *getMidiWorker();
    
    void readData(QXmlStreamReader& xml);
    void readDataText(QTextStream& arpText);
    void writeData(QXmlStreamWriter& xml);
    void writeDataText(QTextStream& arpText);
    void skipXmlElement(QXmlStreamReader& xml);
    void setChIn(int value);
    void setIndexIn(int index, int value);
    void setChannelOut(int value);
    void setPortOut(int value);
    void setRangeIn(int index, int value);
    void loadPatternPresets();
    bool isModified();
    void setModified(bool);
    QVector<MidiCC> ccList;
      
  signals:
    void presetsChanged(const QString&, const QString&, int); 
                    //int 0 for pattern to append
                    //or index>0 for pattern to remove
    void arpRemove(int ID);
    void dockRename(const QString& name, int parentDockID);
    void setMidiLearn(int parentDockID, int ID, int controlID);
    
  public slots:
    void updateChIn(int value);
    void updateIndexIn(int value);
    void updateRangeIn(int value);
    void checkIfInputFilterSet();
    void updateChannelOut(int value);
    void updateRandomVelocityAmp(int value);
    void updateRandomTickAmp(int value);
    void updateRandomLengthAmp(int value);
    void updateAttackTime(int value);
    void updateReleaseTime(int value);
    void checkIfRandomSet();
    void checkIfEnvelopeSet();
    void setMuted(bool on);
    void updatePortOut(int value);
    void updateText(const QString& newtext);
    void updateRepeatPattern(int);
    void selectPatternPreset(int);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void openTextEditWindow(bool on);
    void storeCurrentPattern();
    void removeCurrentPattern();
    void setRandomVisible(bool on);
    void setEnvelopeVisible(bool on);
    void setInputFilterVisible(bool on);
    void moduleDelete();
    void moduleRename();
    void appendMidiCC(int ctrlID, int ccnumber, int channel, int min, int max);
    void removeMidiCC(int ctrlID, int ccnumber, int channel);
    void midiLearnMute();
    void midiForgetMute();
    void midiLearnPresetSwitch();
    void midiForgetPresetSwitch();
    void midiLearnCancel();
};
  
#endif
