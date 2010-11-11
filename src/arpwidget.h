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
    QSpinBox *channelOut, *portOut;        // Output channel / port (ALSA Sequencer)
    QComboBox *repeatPatternThroughChord;
    QComboBox *patternPresetBox;
    QToolButton *textEditButton, *textStoreButton, *textRemoveButton;
    QAction *textEditAction, *textStoreAction, *textRemoveAction;
    QAction *deleteAction, *renameAction;
    QAction *cancelMidiLearnAction;
    MidiArp *midiArp;
    QLineEdit *patternText;
    Slider *randomVelocity, *randomTick, *randomLength;
    Slider *attackTime, *releaseTime;
    bool modified;
    
  public:
    QString name;
    int ID, parentDockID;
    ArpScreen *arpScreen;
    QStringList patternPresets, patternNames;
    QCheckBox *muteOut;

  public:
    ArpWidget(MidiArp *p_midiArp, int portCount, bool compactStyle, QWidget* parent=0);
    ~ArpWidget();
    MidiArp *getMidiArp();
    
    void readArp(QXmlStreamReader& xml);
    void readArpText(QTextStream& arpText);
    void writeArp(QXmlStreamWriter& xml);
    void writeArpText(QTextStream& arpText);
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
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateText(const QString& newtext);
    void updateRepeatPattern(int);
    void selectPatternPreset(int);
    void updatePatternPresets(const QString& n, const QString& p, int index);
    void openTextEditWindow(bool on);
    void storeCurrentPattern();
    void removeCurrentPattern();
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
