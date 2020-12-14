/*!
 * @file inoutbox.cpp
 * @brief Implements the InOutBox GUI class
 *
 *
 *      Copyright 2009 - 2021 <qmidiarp-devel@lists.sourceforge.net>
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

#include "inoutbox.h"
#include "config.h"

#ifdef APPBUILD

#include "pixmaps/lfowavcp.xpm"
#include "pixmaps/seqwavcp.xpm"
#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"


InOutBox::InOutBox(MidiWorker *p_midiWorker, GlobStore *p_globStore, 
    Prefs *p_prefs, bool inOutVisible, const QString& p_name):
    midiWorker(p_midiWorker),
    name(p_name),
    globStore(p_globStore),
    prefs(p_prefs),
    modified(false)
{
    bool compactStyle = p_prefs->compactStyle;
    int portCount = p_prefs->portCount;
    midiControl = new MidiControl(this);

    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

    QToolButton *cloneButton = new QToolButton;
    if (name.startsWith('S') || name.startsWith('L')) {
        if (name.startsWith('S')) {
            cloneAction = new QAction(QPixmap(seqwavcp_xpm), tr("&Clone..."), this);
        }
        else {
            cloneAction = new QAction(QPixmap(lfowavcp_xpm), tr("&Clone..."), this);
        }
        cloneAction->setToolTip(tr("Duplicate this Module in muted state"));
        cloneButton->setDefaultAction(cloneAction);
        connect(cloneAction, SIGNAL(triggered()), this, SLOT(moduleClone()));
    }
    else cloneButton->hide();
    renameAction = new QAction(QPixmap(arprename_xpm), tr("&Rename..."), this);
    renameAction->setToolTip(tr("Rename this Module"));
    QToolButton *renameButton = new QToolButton;
    renameButton->setDefaultAction(renameAction);
    connect(renameAction, SIGNAL(triggered()), this, SLOT(moduleRename()));

    deleteAction = new QAction(QPixmap(arpremove_xpm), tr("&Delete..."), this);
    deleteAction->setToolTip(tr("Delete this Module"));
    QToolButton *deleteButton = new QToolButton;
    deleteButton->setDefaultAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));

    manageBoxLayout->addStretch();
    manageBoxLayout->addWidget(cloneButton);
    manageBoxLayout->addWidget(renameButton);
    manageBoxLayout->addWidget(deleteButton);

#else
InOutBox::InOutBox(const QString& name):
    midiWorker(NULL),
    modified(false)
{
    bool compactStyle = true;
    bool inOutVisible = true;
#endif

    // Input group box on left side
    QGroupBox *inBox = new QGroupBox(tr("Input"));
    
    QLabel *enableNoteInLabel = new QLabel(tr("&Note"));
    enableNoteIn = new QCheckBox;
    enableNoteInLabel->setBuddy(enableNoteIn);
    enableNoteIn->setToolTip(tr("Transpose the sequence following incoming notes"));
    if (!name.startsWith('S')) {
        enableNoteIn->hide();
        enableNoteInLabel->hide();
    }
    else enableNoteIn->setChecked(true);

    QLabel *enableVelInLabel = new QLabel(tr("&Velocity"));
    enableVelIn = new QCheckBox;
    enableVelInLabel->setBuddy(enableVelIn);
    enableVelIn->setToolTip(tr("Set sequence velocity to that of incoming notes"));
    if (!name.startsWith('S')) {
        enableVelIn->hide();
        enableVelInLabel->hide();
    }
    else enableVelIn->setChecked(true);

    QLabel *enableNoteOffLabel = new QLabel(tr("&Note Off"));
    enableNoteOff = new QCheckBox;
    enableNoteOffLabel->setBuddy(enableNoteOff);
    enableNoteOff->setToolTip(tr("Stop output when Note is released"));
    if (name.startsWith('A')) {
        enableNoteOff->hide();
        enableNoteOffLabel->hide();
    }

    QLabel *ccnumberInLabel = new QLabel(tr("MIDI &CC#"));
    ccnumberInBox = new QSpinBox;
    ccnumberInLabel->setBuddy(ccnumberInBox);
    ccnumberInBox->setRange(0, 127);
    ccnumberInBox->setKeyboardTracking(false);
    ccnumberInBox->setValue(74);
    ccnumberInBox->setToolTip(tr("MIDI Controller number to record"));
    if (!name.startsWith('L')) {
        ccnumberInBox->hide();
        ccnumberInLabel->hide();
    }

    QLabel *enableRestartByKbdLabel = new QLabel(tr("&Restart"));
    enableRestartByKbd = new QCheckBox;
    enableRestartByKbdLabel->setBuddy(enableRestartByKbd);
    enableRestartByKbd->setToolTip(tr("Restart when a new note is received"));

    QLabel *enableTrigByKbdLabel = new QLabel(tr("&Trigger"));
    enableTrigByKbd = new QCheckBox;
    enableTrigByKbdLabel->setBuddy(enableTrigByKbd);
    enableTrigByKbd->setToolTip(tr("Retrigger when a new note is received"));

    QLabel *enableTrigLegatoLabel = new QLabel(tr("&Legato"));
    enableTrigLegato = new QCheckBox;
    enableTrigLegatoLabel->setBuddy(enableTrigLegato);
    enableTrigLegato->setToolTip(tr("Retrigger / restart upon new legato note as well"));

    QLabel *chInLabel = new QLabel(tr("&Channel"));
    chIn = new QComboBox;
    int l1;
    for (l1 = 0; l1 < 16; l1++) chIn->addItem(QString::number(l1 + 1));
    chIn->addItem("Omni");
    chIn->setCurrentIndex(OMNI);
    chInLabel->setBuddy(chIn);

    inputFilterBox = new QGroupBox(tr("Note Filter"));
    indexInLabel = new QLabel(tr("&Note"));
    indexIn[0] = new QSpinBox;
    indexIn[1] = new QSpinBox;
    indexInLabel->setBuddy(indexIn[0]);
    indexIn[0]->setRange(0, 127);
    indexIn[1]->setRange(0, 127);
    indexIn[1]->setValue(127);
    indexIn[0]->setKeyboardTracking(false);
    indexIn[1]->setKeyboardTracking(false);

    rangeInLabel = new QLabel(tr("&Velocity"));
    rangeIn[0] = new QSpinBox;
    rangeIn[1] = new QSpinBox;
    rangeInLabel->setBuddy(rangeIn[0]);
    rangeIn[0]->setRange(0, 127);
    rangeIn[1]->setRange(0, 127);
    rangeIn[1]->setValue(127);
    rangeIn[0]->setKeyboardTracking(false);
    rangeIn[1]->setKeyboardTracking(false);


    QGridLayout *inputFilterBoxLayout = new QGridLayout;
    inputFilterBoxLayout->addWidget(indexInLabel, 0, 0);
    inputFilterBoxLayout->addWidget(indexIn[0], 0, 1);
    inputFilterBoxLayout->addWidget(indexIn[1], 0, 2);
    inputFilterBoxLayout->addWidget(rangeInLabel, 1, 0);
    inputFilterBoxLayout->addWidget(rangeIn[0], 1, 1);
    inputFilterBoxLayout->addWidget(rangeIn[1], 1, 2);
    inputFilterBoxLayout->setMargin(2);
    inputFilterBoxLayout->setSpacing(2);
    connect(inputFilterBox, SIGNAL(toggled(bool)), this,
            SLOT(setInputFilterVisible(bool)));
    inputFilterBox->setCheckable(true);
    inputFilterBox->setChecked(false);
    inputFilterBox->setFlat(true);
    inputFilterBox->setLayout(inputFilterBoxLayout);

    QGridLayout *inBoxLayout = new QGridLayout;
    inBoxLayout->addWidget(ccnumberInLabel, 0, 0);
    inBoxLayout->addWidget(ccnumberInBox, 0, 1);
    inBoxLayout->addWidget(enableNoteInLabel, 1, 0);
    inBoxLayout->addWidget(enableNoteIn, 1, 1);
    inBoxLayout->addWidget(enableVelInLabel, 2, 0);
    inBoxLayout->addWidget(enableVelIn, 2, 1);
    inBoxLayout->addWidget(enableNoteOffLabel, 3, 0);
    inBoxLayout->addWidget(enableNoteOff, 3, 1);
    inBoxLayout->addWidget(enableRestartByKbdLabel, 4, 0);
    inBoxLayout->addWidget(enableRestartByKbd, 4, 1);
    inBoxLayout->addWidget(enableTrigByKbdLabel, 5, 0);
    inBoxLayout->addWidget(enableTrigByKbd, 5, 1);
    inBoxLayout->addWidget(enableTrigLegatoLabel, 6, 0);
    inBoxLayout->addWidget(enableTrigLegato, 6, 1);
    inBoxLayout->addWidget(chInLabel, 7, 0);
    inBoxLayout->addWidget(chIn, 7, 1);
    inBoxLayout->addWidget(inputFilterBox, 8, 0, 1, 2);
    if (compactStyle) {
        inBoxLayout->setMargin(2);
        inBoxLayout->setSpacing(1);
    }
    inBox->setLayout(inBoxLayout);


    // Output group box on right side
    QGroupBox *portBox = new QGroupBox(tr("Output"));

    QLabel *ccnumberLabel = new QLabel(tr("MIDI &CC#"));
    ccnumberBox = new QSpinBox;
    ccnumberLabel->setBuddy(ccnumberBox);
    ccnumberBox->setRange(0, 127);
    ccnumberBox->setKeyboardTracking(false);
    ccnumberBox->setValue(74);
    ccnumberBox->setToolTip(tr("MIDI Controller number sent to output"));
    if (!name.startsWith('L')) {
        ccnumberBox->hide();
        ccnumberLabel->hide();
    }
    

    QLabel *channelLabel = new QLabel(tr("C&hannel"));
    channelOut = new QComboBox;
    channelLabel->setBuddy(channelOut);
    for (l1 = 0; l1 < 16; l1++) channelOut->addItem(QString::number(l1 + 1));
    
    QGridLayout *portBoxLayout = new QGridLayout;
    portBoxLayout->addWidget(ccnumberLabel, 0, 0);
    portBoxLayout->addWidget(ccnumberBox, 0, 1);
    portBoxLayout->addWidget(channelLabel, 1, 0);
    portBoxLayout->addWidget(channelOut, 1, 1);
#ifdef APPBUILD
    QLabel *portLabel = new QLabel(tr("&Port"));
    portOut = new QComboBox;
    portLabel->setBuddy(portOut);
    for (l1 = 0; l1 < portCount; l1++) portOut->addItem(QString::number(l1 + 1));
    portBoxLayout->addWidget(portLabel, 2, 0);
    portBoxLayout->addWidget(portOut, 2, 1);
#endif
    if (compactStyle) {
        portBoxLayout->setMargin(2);
        portBoxLayout->setSpacing(1);
    }
    portBox->setLayout(portBoxLayout);

    // Mute button that has to be added to each module widget outside the box
    muteOutAction = new QAction(tr("&Mute"),this);
    muteOutAction->setCheckable(true);
    muteOut = new QToolButton;
    muteOut->setDefaultAction(muteOutAction);
    muteOut->setMinimumSize(QSize(35,20));
    connect(muteOutAction, SIGNAL(toggled(bool)), this, 
            SLOT(setMuted(bool)));
#ifdef APPBUILD
    muteOutAction->setChecked(p_prefs->mutedAdd);
#endif
    
    // Defer button that has to be added to each module widget outside the box
    deferChangesAction = new QAction("D", this);
    deferChangesAction->setToolTip(tr("Defer mute to pattern end"));
    deferChangesAction->setCheckable(true);
    deferChangesButton = new QToolButton;
    deferChangesButton->setDefaultAction(deferChangesAction);
    deferChangesButton->setFixedSize(20, 20);

    // Hiding button that has to be added to each module widget outside the box
    hideInOutBoxAction = new QAction(tr("&Show/hide in-out settings"), this);
    hideInOutBoxButton = new QToolButton;
    hideInOutBoxAction->setCheckable(true);
    hideInOutBoxAction->setChecked(inOutVisible);
    hideInOutBoxButton->setDefaultAction(hideInOutBoxAction);
    hideInOutBoxButton->setFixedSize(10, 80);
    hideInOutBoxButton->setArrowType (Qt::ArrowType(0));

#ifdef APPBUILD
        parStore = new ParStore(globStore, name, muteOutAction
                    , deferChangesAction, this);
        connect(parStore, SIGNAL(store(int, bool)),
                 this, SLOT(storeParams(int, bool)));
        connect(parStore, SIGNAL(restore(int)),
                 this, SLOT(restoreParams(int)));
    if (compactStyle) parStore->setStyleSheet( COMPACT_STYLE );
    midiControl->addMidiLearnMenu("Note Low", indexIn[0], NOTE_LOW);
    midiControl->addMidiLearnMenu("Note Hi", indexIn[1], NOTE_HIGH);
    midiControl->addMidiLearnMenu("MuteToggle", muteOut, MUTE_BUTTON);
    midiControl->addMidiLearnMenu("Restore_"+name, parStore->topButton, PARAM_RESTORE);
#endif
    // Layout for left/right placements of in/out group boxes
    inOutBoxWidget = new QWidget;
    QVBoxLayout *inOutBoxLayout = new QVBoxLayout;
#ifdef APPBUILD
    inOutBoxLayout->addLayout(manageBoxLayout);
#endif
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();
    inOutBoxWidget->setLayout(inOutBoxLayout);
    inOutBoxWidget->setVisible(inOutVisible);
    
    connect(ccnumberBox, SIGNAL(valueChanged(int)), this, 
            SLOT(updateCcnumber(int)));
    connect(ccnumberInBox, SIGNAL(valueChanged(int)), this, 
            SLOT(updateCcnumberIn(int)));
    connect(enableVelIn, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableVelIn(bool)));
    connect(enableNoteIn, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableNoteIn(bool)));
    connect(enableNoteOff, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableNoteOff(bool)));
    connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableRestartByKbd(bool)));
    connect(enableTrigByKbd, SIGNAL(toggled(bool)), this, 
            SLOT(updateEnableTrigByKbd(bool)));
    connect(enableTrigLegato, SIGNAL(toggled(bool)), this, 
            SLOT(updateTrigLegato(bool)));
    connect(chIn, SIGNAL(activated(int)), this, 
            SLOT(updateChIn(int)));
    connect(indexIn[0], SIGNAL(valueChanged(int)), this,
            SLOT(updateIndexIn(int)));
    connect(indexIn[1], SIGNAL(valueChanged(int)), this,
            SLOT(updateIndexIn(int)));
    connect(rangeIn[0], SIGNAL(valueChanged(int)), this,
            SLOT(updateRangeIn(int)));
    connect(rangeIn[1], SIGNAL(valueChanged(int)), this,
            SLOT(updateRangeIn(int)));
    connect(channelOut, SIGNAL(activated(int)), this,
            SLOT(updateChannelOut(int)));
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, 
            SLOT(updateDeferChanges(bool)));
#ifdef APPBUILD
    connect(portOut, SIGNAL(activated(int)), this, 
            SLOT(updatePortOut(int)));
#endif
    connect(hideInOutBoxAction, SIGNAL(toggled(bool)), inOutBoxWidget, 
                SLOT(setVisible(bool)));
    needsGUIUpdate=false;
    dataChanged = false;
}

InOutBox::~InOutBox()
{
#ifdef APPBUILD
    delete parStore;
#endif
}

bool InOutBox::isModified()
{
    bool mcmod = false;
#ifdef APPBUILD
    mcmod = midiControl->isModified();
#endif
    return (modified || mcmod);
}

void InOutBox::setModified(bool m)
{
    modified = m;
#ifdef APPBUILD
    midiControl->setModified(m);
#endif
}

void InOutBox::setInputFilterVisible(bool on)
{
    rangeIn[0]->setVisible(on);
    rangeIn[1]->setVisible(on);
    rangeInLabel->setVisible(on);
    indexIn[0]->setVisible(on);
    indexIn[1]->setVisible(on);
    indexInLabel->setVisible(on);
}

void InOutBox::checkIfInputFilterSet()
{
    if (((indexIn[1]->value() - indexIn[0]->value()) < 127)
            || ((rangeIn[1]->value() - rangeIn[0]->value()) < 127)) {
        inputFilterBox->setFlat(false);
        inputFilterBox->setTitle(tr("Note Filter - ACTIVE"));
    }
    else {
        inputFilterBox->setFlat(true);
        inputFilterBox->setTitle(tr("Note Filter"));
    }
}

void InOutBox::updateChIn(int value)
{
    if (midiWorker) midiWorker->chIn = value;
    modified = true;
}

void InOutBox::updateIndexIn(int value)
{
    if (indexIn[0] == sender()) {
        if (midiWorker) midiWorker->indexIn[0] = value;
    } else {
        if (midiWorker) midiWorker->indexIn[1] = value;
    }
    checkIfInputFilterSet();
    modified = true;
}

void InOutBox::updateRangeIn(int value)
{
    if (rangeIn[0] == sender()) {
        if (midiWorker) midiWorker->rangeIn[0] = value;
    } else {
        if (midiWorker) midiWorker->rangeIn[1] = value;
    }
    checkIfInputFilterSet();
    modified = true;
}

void InOutBox::updateChannelOut(int value)
{
    if (midiWorker) midiWorker->channelOut = value;
    modified = true;
}

void InOutBox::updateCcnumber(int val)
{
    if (midiWorker)
        midiWorker->ccnumber = val;
    modified = true;
}

void InOutBox::updateCcnumberIn(int val)
{
    if (midiWorker) midiWorker->ccnumberIn = val;
    modified = true;
}

void InOutBox::updatePortOut(int value)
{
    if (midiWorker) midiWorker->portOut = value;
    modified = true;
}

void InOutBox::updateEnableNoteIn(bool on)
{
    if (midiWorker) midiWorker->enableNoteIn = on;
    modified = true;
}

void InOutBox::updateEnableVelIn(bool on)
{
    if (midiWorker) midiWorker->enableVelIn = on;
    modified = true;
}

void InOutBox::updateEnableNoteOff(bool on)
{
    if (midiWorker) midiWorker->enableNoteOff = on;
    modified = true;
}

void InOutBox::updateEnableRestartByKbd(bool on)
{
    if (midiWorker) midiWorker->restartByKbd = on;
    modified = true;
}

void InOutBox::updateEnableTrigByKbd(bool on)
{
    if (midiWorker) midiWorker->trigByKbd = on;
    modified = true;
}

void InOutBox::updateTrigLegato(bool on)
{
    if (midiWorker) midiWorker->trigLegato = on;
    modified = true;
}

void InOutBox::setMuted(bool on)
{
    if (!midiWorker) return;
    midiWorker->setMuted(on);
    needsGUIUpdate = true;
    modified = true;
}

void InOutBox::updateDeferChanges(bool on)
{
    if (midiWorker) midiWorker->updateDeferChanges(on);
    modified = true;
}

void InOutBox::storeParams(int ix, bool empty)
{
#ifdef APPBUILD
    parStore->temp.empty = empty;
    parStore->temp.muteOut = muteOut->isChecked();
    parStore->temp.chIn = chIn->currentIndex();
    parStore->temp.channelOut = channelOut->currentIndex();
    parStore->temp.portOut = portOut->currentIndex();
    parStore->temp.indexIn0 = indexIn[0]->value();
    parStore->temp.indexIn1 = indexIn[1]->value();
    parStore->temp.rangeIn0 = rangeIn[0]->value();
    parStore->temp.rangeIn1 = rangeIn[1]->value();
    doStoreParams(ix);
    
#else
    (void)ix;
    (void)empty;
#endif
}

void InOutBox::restoreParams(int ix)
{
#ifdef APPBUILD
    doRestoreParams(ix);
    if (!parStore->onlyPatternList.at(ix)) {
        if (prefs->storeMuteState) muteOutAction->setChecked(parStore->list.at(ix).muteOut);
        indexIn[0]->setValue(parStore->list.at(ix).indexIn0);
        indexIn[1]->setValue(parStore->list.at(ix).indexIn1);
        rangeIn[0]->setValue(parStore->list.at(ix).rangeIn0);
        rangeIn[1]->setValue(parStore->list.at(ix).rangeIn1);
        chIn->setCurrentIndex(parStore->list.at(ix).chIn);
        updateChIn(parStore->list.at(ix).chIn);
        channelOut->setCurrentIndex(parStore->list.at(ix).channelOut);
        updateChannelOut(parStore->list.at(ix).channelOut);
        setPortOut(parStore->list.at(ix).portOut);
        updatePortOut(parStore->list.at(ix).portOut);
    }
#else
    (void)ix;
#endif
}

#ifdef APPBUILD
void InOutBox::setPortOut(int value)
{
    portOut->setCurrentIndex(value);
    modified = true;
}
#endif

void InOutBox::moduleDelete()
{
#ifdef APPBUILD
    QString qs;
    qs = tr("Delete \"%1\"?")
        .arg(name);
    if (QMessageBox::question(0, APP_NAME, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
    emit moduleRemove(ID);
#endif
}

void InOutBox::moduleRename()
{
#ifdef APPBUILD
    QString newname, oldname;
    bool ok;
    qWarning("name %s", qPrintable(name));
    oldname = name;

    newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);

    if (ok && !newname.isEmpty()) {
        name = oldname.left(4) + newname;
        emit dockRename(name, parentDockID);
    }
#endif
}

void InOutBox::moduleClone()
{
#ifdef APPBUILD
        emit moduleClone(ID);
#endif
}

#ifdef APPBUILD
void InOutBox::writeCommonData(QXmlStreamWriter& xml)
{
    xml.writeStartElement(name.left(3));
    xml.writeAttribute("name", name.mid(name.indexOf(':') + 1));
    xml.writeAttribute("inOutVisible", QString::number(inOutBoxWidget->isVisible()));

        xml.writeStartElement("input");
            if (!name.startsWith('A')) {
            xml.writeTextElement("enableNoteOff", QString::number(
                enableNoteOff->isChecked()));
            }
            if (name.startsWith('S')) {
            xml.writeTextElement("enableNote", QString::number(
                enableNoteIn->isChecked()));
            xml.writeTextElement("enableVelocity", QString::number(
                enableVelIn->isChecked()));
            }
            xml.writeTextElement("restartByKbd", QString::number(
                enableRestartByKbd->isChecked()));
            xml.writeTextElement("trigByKbd", QString::number(
                enableTrigByKbd->isChecked()));
            xml.writeTextElement("trigLegato", QString::number(
                enableTrigLegato->isChecked()));
            xml.writeTextElement("channel", QString::number(
                chIn->currentIndex()));
            xml.writeTextElement("indexMin", QString::number(
                indexIn[0]->value()));
            xml.writeTextElement("indexMax", QString::number(
                indexIn[1]->value()));
            xml.writeTextElement("rangeMin", QString::number(
                rangeIn[0]->value()));
            xml.writeTextElement("rangeMax", QString::number(
                rangeIn[1]->value()));
            if (name.startsWith('L')) {
            xml.writeTextElement("ccnumber", QString::number(
                ccnumberInBox->value()));
            }
        xml.writeEndElement();

        xml.writeStartElement("output");
            xml.writeTextElement("muted", QString::number(
                muteOut->isChecked()));
            xml.writeTextElement("defer", QString::number(
                deferChangesButton->isChecked()));
            xml.writeTextElement("port", QString::number(
                portOut->currentIndex()));
            xml.writeTextElement("channel", QString::number(
                channelOut->currentIndex()));
            if (name.startsWith('L')) {
            xml.writeTextElement("ccnumber", QString::number(
                ccnumberBox->value()));
            }
        xml.writeEndElement();
        
        midiControl->writeData(xml);

        parStore->writeData(xml);
}

void InOutBox::readCommonData(QXmlStreamReader& xml)
{
    int tmp = 0;
    
    if (xml.isStartElement() && (xml.name() == "midiControllers")) {
        midiControl->readData(xml);
    }
    else if (xml.isStartElement() && (xml.name() == "globalStores")) {
        parStore->readData(xml);
    }

    else if (xml.isStartElement() && (xml.name() == "input")) {
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isEndElement())
                break;
                
            if (xml.name() == "enableNote")
                enableNoteIn->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "enableNoteOff")
                enableNoteOff->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "enableVelocity")
                enableVelIn->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "restartByKbd")
                enableRestartByKbd->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "trigByKbd")
                enableTrigByKbd->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "trigLegato")
                enableTrigLegato->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "channel") {
                tmp = xml.readElementText().toInt();
                chIn->setCurrentIndex(tmp);
            }
            else if (xml.name() == "indexMin")
                indexIn[0]->setValue(xml.readElementText().toInt());
            else if (xml.name() == "indexMax")
                indexIn[1]->setValue(xml.readElementText().toInt());
            else if (xml.name() == "rangeMin")
                rangeIn[0]->setValue(xml.readElementText().toInt());
            else if (xml.name() == "rangeMax")
                rangeIn[1]->setValue(xml.readElementText().toInt());
            else if (xml.name() == "ccnumber")
                ccnumberInBox->setValue(xml.readElementText().toInt());
            else skipXmlElement(xml);
        }
    }
    else if (xml.isStartElement() && (xml.name() == "output")) {
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isEndElement())
                break;
            if (xml.name() == "muted")
                muteOutAction->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "defer")
                deferChangesAction->setChecked(xml.readElementText().toInt());
            else if (xml.name() == "channel") {
                tmp = xml.readElementText().toInt();
                channelOut->setCurrentIndex(tmp);
            }
            else if (xml.name() == "port") {
                tmp = xml.readElementText().toInt();
                portOut->setCurrentIndex(tmp);
            }
            else if (xml.name() == "ccnumber")
                ccnumberBox->setValue(xml.readElementText().toInt());
            else skipXmlElement(xml);
        }
    }
}

void InOutBox::skipXmlElement(QXmlStreamReader& xml)
{
    if (xml.isStartElement()) {
        qWarning("Unknown Element in XML File: %s",qPrintable(xml.name().toString()));
        while (!xml.atEnd()) {
            xml.readNext();

            if (xml.isEndElement())
                break;

            if (xml.isStartElement()) {
                skipXmlElement(xml);
            }
        }
    }
}

void InOutBox::newGrooveValues(int p_grooveTick, int p_grooveVelocity,
        int p_grooveLength)
{
    // grooveTick is only updated on pair steps to keep quantization
    // newGrooveTick stores the GUI value temporarily
    midiWorker->newGrooveTick = p_grooveTick;
    midiWorker->grooveVelocity = p_grooveVelocity;
    midiWorker->grooveLength = p_grooveLength;
    midiWorker->needsGUIUpdate = true;
}

void InOutBox::updateIndicators()
{
    int ci = midiWorker->getFramePtr();

    int percent;
    if (midiWorker->nPoints)
        percent = ci * 100 / (midiWorker->nPoints);
    else
        percent = 0;
    
    parStore->ndc->updatePercent(percent);
    
    if (parStore->isRestoreMaster
            && (!globStore->timeModeBox->currentIndex())) {
        globStore->indicator->updatePercent(percent);
    }
}

void InOutBox::checkIfRestore(int64_t *restoreTick, bool *restoreFlag)
{
    if (!midiWorker->getFramePtr() && restoreFlag
        && parStore->isRestoreMaster
        && !globStore->timeModeBox->currentIndex()) {
        *restoreTick = midiWorker->nextTick;
        *restoreFlag = false;
    }
}

#endif
