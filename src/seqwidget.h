/*
 *      seqwidget.h
 *      
 *      Copyright 2009 <alsamodular-devel@lists.sourceforge.net>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef SEQWIDGET_H
#define SEQWIDGET_H

#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QTextStream>
#include <QCheckBox>
#include <QAction>
#include <QToolButton>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "midiseq.h"
#include "slider.h"
#include "seqscreen.h"

const int seqResValues[5] = {1, 2, 4, 8, 16};
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

class SeqWidget : public QWidget
{
    Q_OBJECT

    QSpinBox *chIn;
    QSpinBox *channelOut, *portOut;
    QComboBox *waveFormBox, *resBox, *sizeBox, *freqBox;
    QAction *copyToCustomAction;
    QAction *deleteAction, *renameAction;
    QAction *cancelMidiLearnAction;
    QToolButton *copyToCustomButton;
 
    MidiSeq *midiSeq;
    QVector<SeqSample> seqData;
    bool modified, lastMute;
    bool recordMode;

  public:
    QString name;
    SeqScreen *seqScreen;
    QStringList waveForms;
    QCheckBox *muteOut;
    QCheckBox *enableNoteIn;               
    QCheckBox *enableVelIn; 
    Slider *velocity, *transpose, *notelength;
    int ID, parentDockID;
    QVector<MidiCC> ccList;
        
    SeqWidget(MidiSeq *p_midiSeq, int portCount, bool compactStyle, QWidget* parent=0);
    ~SeqWidget();
    MidiSeq *getMidiSeq();
    
    void readSeq(QXmlStreamReader& xml);
    void skipXmlElement(QXmlStreamReader& xml);
    void readSeqText(QTextStream& arpText);
    void writeSeq(QXmlStreamWriter& xml);
    void writeSeqText(QTextStream& arpText);
    void setChIn(int value);
    void setEnableNoteIn(bool on);
    void setEnableVelIn(bool on);
    void setChannelOut(int value);
    void setPortOut(int value);
    void loadWaveForms();
    bool isModified();
    void setModified(bool);
  
  signals:
    void patternChanged();
    void seqRemove(int ID);
    void dockRename(const QString& name, int parentDockID);  
    void setMidiLearn(int parentDockID, int ID, int controlID);
    void setMidiForget(int parentDockID, int ID);
    
  public slots:
    void updateChIn(int value);
    void updateEnableNoteIn(bool on);
    void updateEnableVelIn(bool on);
    void updateChannelOut(int value);
    void updatePortOut(int value);
    void updateWaveForm(int);
    void setRecord(bool on);
    void processNote(int note, int velocity);
    void updateRes(int);
    void updateSize(int);
    void updateNoteLength(int val);
    void updateVelocity(int val);
    void updateTranspose(int val);
    void mouseMoved(double, double, int);
    void mousePressed(double, double, int);
    void copyToCustom();
    void moduleDelete();
    void moduleRename();
    void appendMidiCC(int ctrlID, int ccnumber, int channel, int min, int max);
    void removeMidiCC(int ctrlID, int ccnumber, int channel);
    void midiLearnMute();
    void midiForgetMute();
    void midiLearnVel();
    void midiForgetVel();
    void midiLearnNoteLen();
    void midiForgetNoteLen();
    void midiLearnCancel();
};
  
#endif
