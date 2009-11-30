#include <QLabel>
#include <QBoxLayout>
#include <QStringList>
#include <QGroupBox>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QDir>
#include <QMessageBox>

#include "midiarp.h"
#include "arpwidget.h"
#include "slider.h"
#include "arpscreen.h"
#include "config.h"

#include "pixmaps/arpremove.xpm"
#include "pixmaps/arprename.xpm"
#include "pixmaps/editmodeon.xpm"
#include "pixmaps/patternremove.xpm"
#include "pixmaps/patternstore.xpm"



ArpWidget::ArpWidget(MidiArp *p_midiArp, int portCount, QWidget *parent)
: QWidget(parent), midiArp(p_midiArp), modified(false)
{
    QGridLayout *arpWidgetLayout = new QGridLayout;

    // Management Buttons on the right top
    QHBoxLayout *manageBoxLayout = new QHBoxLayout;

    renameAction = new QAction(QIcon(arprename_xpm), tr("&Rename..."), this);
    renameAction->setToolTip(tr("Rename this Arp"));
    QToolButton *renameButton = new QToolButton(this);
    renameButton->setDefaultAction(renameAction);
    connect(renameAction, SIGNAL(triggered()), this, SLOT(moduleRename()));
    
    deleteAction = new QAction(QIcon(arpremove_xpm), tr("&Delete..."), this);
    deleteAction->setToolTip(tr("Delete this Arp"));
    QToolButton *deleteButton = new QToolButton(this);
    deleteButton->setDefaultAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(moduleDelete()));
    
    manageBoxLayout->addStretch();
    manageBoxLayout->addWidget(renameButton);
    manageBoxLayout->addWidget(deleteButton);
    
    // Input group box on left side
    QGroupBox *inBox = new QGroupBox(tr("Input"), this);

    QLabel *indexInLabel = new QLabel(tr("&Note"), inBox);
    indexIn[0] = new QSpinBox(inBox);
    indexIn[1] = new QSpinBox(inBox);
    indexInLabel->setBuddy(indexIn[0]);
    indexIn[0]->setRange(0, 127);
    indexIn[1]->setRange(0, 127);
    indexIn[1]->setValue(127);
    indexIn[0]->setKeyboardTracking(false);
    indexIn[1]->setKeyboardTracking(false);
    connect(indexIn[0], SIGNAL(valueChanged(int)), this,
            SLOT(updateIndexIn(int)));
    connect(indexIn[1], SIGNAL(valueChanged(int)), this,
            SLOT(updateIndexIn(int)));

    QLabel *rangeInLabel = new QLabel(tr("&Velocity"), inBox);
    rangeIn[0] = new QSpinBox(inBox);
    rangeIn[1] = new QSpinBox(inBox);
    rangeInLabel->setBuddy(rangeIn[0]);
    rangeIn[0]->setRange(0, 127);
    rangeIn[1]->setRange(0, 127);
    rangeIn[1]->setValue(127);
    rangeIn[0]->setKeyboardTracking(false);
    rangeIn[1]->setKeyboardTracking(false);
    connect(rangeIn[0], SIGNAL(valueChanged(int)), this,
            SLOT(updateRangeIn(int)));
    connect(rangeIn[1], SIGNAL(valueChanged(int)), this,
            SLOT(updateRangeIn(int)));

    QLabel *chInLabel = new QLabel(tr("&Channel"), inBox);
    chIn = new QSpinBox(inBox);
    chIn->setRange(1, 16);
    chIn->setKeyboardTracking(false);
    chInLabel->setBuddy(chIn);
    connect(chIn, SIGNAL(valueChanged(int)), this, SLOT(updateChIn(int)));

    QGridLayout *inBoxLayout = new QGridLayout;

    inBoxLayout->addWidget(indexInLabel, 0, 0);
    inBoxLayout->addWidget(indexIn[0], 0, 1);
    inBoxLayout->addWidget(indexIn[1], 0, 2);
    inBoxLayout->addWidget(rangeInLabel, 1, 0);
    inBoxLayout->addWidget(rangeIn[0], 1, 1);
    inBoxLayout->addWidget(rangeIn[1], 1, 2);
    inBoxLayout->addWidget(chInLabel, 2, 0);
    inBoxLayout->addWidget(chIn, 2, 2);

    inBox->setLayout(inBoxLayout); 


    // Output group box on left side
    QGroupBox *portBox = new QGroupBox(tr("Output"), this);

    QLabel *muteLabel = new QLabel(tr("&Mute"),portBox);
    muteOut = new QCheckBox(this);
    connect(muteOut, SIGNAL(toggled(bool)), midiArp, SLOT(muteArp(bool)));
    muteLabel->setBuddy(muteOut);

    QLabel *portLabel = new QLabel(tr("&Port"), portBox);
    portOut = new QSpinBox(portBox);
    portLabel->setBuddy(portOut);
    portOut->setRange(1, portCount);
    portOut->setKeyboardTracking(false);
    connect(portOut, SIGNAL(valueChanged(int)), this, SLOT(updatePortOut(int)));

    QLabel *channelLabel = new QLabel(tr("C&hannel"), portBox);
    channelOut = new QSpinBox(portBox);
    channelLabel->setBuddy(channelOut);
    channelOut->setRange(1, 16);
    channelOut->setKeyboardTracking(false);
    connect(channelOut, SIGNAL(valueChanged(int)), this,
            SLOT(updateChannelOut(int)));

    QGridLayout *portBoxLayout = new QGridLayout;
    portBoxLayout->addWidget(muteLabel, 0, 0);
    portBoxLayout->addWidget(muteOut, 0, 1);
    portBoxLayout->addWidget(portLabel, 1, 0);
    portBoxLayout->addWidget(portOut, 1, 1);
    portBoxLayout->addWidget(channelLabel, 2, 0);
    portBoxLayout->addWidget(channelOut, 2, 1);
    portBox->setLayout(portBoxLayout);


    // Layout for left/right placements of in/out group boxes
    QVBoxLayout *inOutBoxLayout = new QVBoxLayout();
    inOutBoxLayout->addLayout(manageBoxLayout);
    inOutBoxLayout->addWidget(inBox);
    inOutBoxLayout->addWidget(portBox);
    inOutBoxLayout->addStretch();

    // group box for pattern setup
    QGroupBox *patternBox = new QGroupBox(tr("Pattern"), this);
    QVBoxLayout *patternBoxLayout = new QVBoxLayout;

    textEditButton = new QToolButton(this); 
    textEditAction = new QAction(QIcon(editmodeon_xpm),
            tr("&Edit Pattern"), this);
    connect(textEditAction, SIGNAL(toggled(bool)), this,
            SLOT(openTextEditWindow(bool)));
    textEditAction->setCheckable(true);
    textEditButton->setDefaultAction(textEditAction);

    textRemoveButton = new QToolButton(this);   
    textRemoveAction = new QAction(QIcon(patternremove_xpm),
            tr("&Remove Pattern"), this);
    connect(textRemoveAction, SIGNAL(triggered()), this,
            SLOT(removeCurrentPattern()));
    textRemoveButton->setDefaultAction(textRemoveAction);
    textRemoveAction->setEnabled(false);

    textStoreButton = new QToolButton(this);
    textStoreAction = new QAction(QIcon(patternstore_xpm),
            tr("&Store Pattern"), this);
    connect(textStoreAction, SIGNAL(triggered()), this,
            SLOT(storeCurrentPattern()));
    textStoreAction->setEnabled(false);
    textStoreButton->setDefaultAction(textStoreAction);

    patternPresetBox = new QComboBox(patternBox);
    loadPatternPresets();
    patternPresetBox->insertItems(0, patternNames);
    patternPresetBox->setCurrentIndex(0);
    patternPresetBox->setToolTip(tr("Pattern preset"));
    patternPresetBox->setMinimumContentsLength(20);
    connect(patternPresetBox, SIGNAL(activated(int)), this,
            SLOT(selectPatternPreset(int)));

    repeatPatternThroughChord = new QComboBox(patternBox);
    QStringList repeatPatternNames; 
    repeatPatternNames << tr("Static") << tr("Up") << tr("Down");
    repeatPatternThroughChord->insertItems(0, repeatPatternNames);
    repeatPatternThroughChord->setToolTip(tr("Repeat mode"));
    connect(repeatPatternThroughChord, SIGNAL(highlighted(int)), this,
            SLOT(updateRepeatPattern(int)));
    repeatPatternThroughChord->setCurrentIndex(1);

    QHBoxLayout *patternPresetLayout = new QHBoxLayout;
    patternPresetLayout->setMargin(1);
    patternPresetLayout->setSpacing(1);
    patternPresetLayout->addWidget(textStoreButton);    
    patternPresetLayout->addWidget(textEditButton);
    patternPresetLayout->addWidget(textRemoveButton);

    patternPresetLayout->addWidget(patternPresetBox);
    patternPresetLayout->addStretch(2);
    patternPresetLayout->addWidget(repeatPatternThroughChord);  

    patternText = new QLineEdit(patternBox); 
    //patternText->setLineWrapMode(QPlainTextEdit::NoWrap);
    connect(patternText, SIGNAL(textChanged(QString)), this,
            SLOT(updateText(QString)));
    patternText->setHidden(true);
    //patternText->setMaximumHeight(50);
    patternText->setToolTip(
            tr("0..9  note played on keyboard, ascending order\n"
            "( ) chord mode on/off\n"
            "  + -  octave up/down\n"
            " < . > tempo up/reset/down\n"
            "  d h  note length up/down\n"
            "  / \\  velocity up/down\n"
            "   p   pause"));


    QWidget *arpScreenBox = new QWidget(patternBox);
    QHBoxLayout *arpScreenBoxLayout = new QHBoxLayout;
    arpScreen = new ArpScreen(patternBox); 
    arpScreenBox->setMinimumHeight(80);
    arpScreenBoxLayout->addWidget(arpScreen);
    arpScreenBoxLayout->setMargin(1);
    arpScreenBoxLayout->setSpacing(1);
    arpScreenBox->setLayout(arpScreenBoxLayout);

    patternBoxLayout->addWidget(arpScreenBox);
    patternBoxLayout->addLayout(patternPresetLayout);
    patternBoxLayout->addWidget(patternText);
    patternBoxLayout->setMargin(1);
    patternBoxLayout->setSpacing(1);
    patternBox->setLayout(patternBoxLayout); 

    // group box for random settings
    QGroupBox *randomBox = new QGroupBox(tr("Random"), this);
    QVBoxLayout *randomBoxLayout = new QVBoxLayout;

    randomTick = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("&Shift"), randomBox);
    connect(randomTick, SIGNAL(valueChanged(int)), midiArp,
            SLOT(updateRandomTickAmp(int)));

    randomVelocity = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("Vel&ocity"), randomBox);
    connect(randomVelocity, SIGNAL(valueChanged(int)), midiArp,
            SLOT(updateRandomVelocityAmp(int)));

    randomLength = new Slider(0, 100, 1, 5, 0, Qt::Horizontal,
            tr("&Length"), randomBox);
    connect(randomLength, SIGNAL(valueChanged(int)), midiArp,
            SLOT(updateRandomVelocityAmp(int))); 
             
    randomBoxLayout->addWidget(randomTick);
    randomBoxLayout->addWidget(randomVelocity);
    randomBoxLayout->addWidget(randomLength);
    randomBoxLayout->addStretch();
    randomBox->setLayout(randomBoxLayout);
      
    QGroupBox *envelopeBox = new QGroupBox(tr("Envelope"), this);
    QVBoxLayout *envelopeBoxLayout = new QVBoxLayout;
    attackTime = new Slider(0, 20, 1, 1, 0, Qt::Horizontal,
            tr("&Attack (s)"), envelopeBox);
    connect(attackTime, SIGNAL(valueChanged(int)), midiArp,
            SLOT(updateAttackTime(int)));
    releaseTime = new Slider(0, 20, 1, 1, 0, Qt::Horizontal,
            tr("&Release (s)"), envelopeBox);
    connect(releaseTime, SIGNAL(valueChanged(int)), midiArp,
            SLOT(updateReleaseTime(int)));
              
    envelopeBoxLayout->addWidget(attackTime);
    envelopeBoxLayout->addWidget(releaseTime);
    envelopeBoxLayout->addStretch();
    envelopeBox->setLayout(envelopeBoxLayout);
    
    arpWidgetLayout->addWidget(patternBox, 0, 0);
    arpWidgetLayout->addWidget(randomBox, 1, 0);
    arpWidgetLayout->addWidget(envelopeBox, 2, 0);
    arpWidgetLayout->addLayout(inOutBoxLayout, 0, 1, 3, 1);
    arpWidgetLayout->setRowStretch(3, 1);
    arpWidgetLayout->setColumnStretch(0, 5);
    arpWidgetLayout->setColumnMinimumWidth(0, 300);
    setLayout(arpWidgetLayout);
}

ArpWidget::~ArpWidget()
{
}

MidiArp *ArpWidget::getMidiArp()
{
    return (midiArp);
}

void ArpWidget::updateChIn(int value)
{
    midiArp->chIn = value - 1;
}

void ArpWidget::updateIndexIn(int value)
{
    if (indexIn[0] == sender()) {
        midiArp->indexIn[0] = value; 
    } else {
        midiArp->indexIn[1] = value;
    }  
}

void ArpWidget::updateRangeIn(int value)
{ 
    if (rangeIn[0] == sender()) {
        midiArp->rangeIn[0] = value; 
    } else {
        midiArp->rangeIn[1] = value;
    }  
}

void ArpWidget::updatePortOut(int value)
{
    midiArp->portOut = value - 1;
}

void ArpWidget::updateChannelOut(int value)
{
    midiArp->channelOut = value - 1;
}

void ArpWidget::writeArp(QTextStream& arpText)
{
    arpText << midiArp->chIn << ' '
        << midiArp->repeatPatternThroughChord << '\n';
    arpText << midiArp->indexIn[0] << ' ' << midiArp->indexIn[1] << '\n';
    arpText << midiArp->rangeIn[0] << ' ' << midiArp->rangeIn[1] << '\n';
    arpText << midiArp->channelOut << ' ' << midiArp->portOut << '\n';
    arpText << midiArp->randomTickAmp << ' '
        << midiArp->randomVelocityAmp << ' '
        << midiArp->randomLengthAmp << '\n';
    arpText << "Envelope" << '\n';
    arpText << attackTime->value() << ' ' << releaseTime->value() << '\n';
    arpText << midiArp->pattern << '\n';
    arpText << "EOP\n"; // End Of Pattern
    modified = false;
}                                      

void ArpWidget::readArp(QTextStream& arpText)
{
    QString qs, qs2;

    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    chIn->setValue(qs2.toInt() + 1);
    qs2 = qs.section(' ', 1, 1);
    repeatPatternThroughChord->setCurrentIndex(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    indexIn[0]->setValue(qs2.toInt());
    qs2 = qs.section(' ', 1, 1); 
    indexIn[1]->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    rangeIn[0]->setValue(qs2.toInt());
    qs2 = qs.section(' ', 1, 1); 
    rangeIn[1]->setValue(qs2.toInt());
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    channelOut->setValue(qs2.toInt() + 1);
    qs2 = qs.section(' ', 1, 1); 
    portOut->setValue(qs2.toInt() + 1);
    qs = arpText.readLine();
    qs2 = qs.section(' ', 0, 0); 
    randomTick->setValue(qs2.toInt());
    qs2 = qs.section(' ', 1, 1); 
    randomVelocity->setValue(qs2.toInt());
    qs2 = qs.section(' ', 2, 2); 
    randomLength->setValue(qs2.toInt());
    qs = arpText.readLine();
    if (qs == "Envelope")
    {
        qs = arpText.readLine();
        qs2 = qs.section(' ', 0, 0);
        attackTime->setValue(qs2.toInt());
        qs2 = qs.section(' ', 1, 1);
        releaseTime->setValue(qs2.toInt());
        qs = arpText.readLine();
    }
    while (!arpText.atEnd()) {
        qs2 = arpText.readLine();

        if (qs2.contains("EOP", Qt::CaseSensitivity(TRUE))) {
            break;
        }
        qs += '\n' + qs2;
    }
    patternText->setText(qs);                    
    modified = false;
}                                      

void ArpWidget::setChIn(int value)
{
    chIn->setValue(value);
    modified = true;
}

void ArpWidget::setIndexIn(int index, int value)
{
    indexIn[index]->setValue(value); 
    modified = true;
}

void ArpWidget::setRangeIn(int index, int value)
{
    rangeIn[index]->setValue(value);
    modified = true;
}

void ArpWidget::setPortOut(int value)
{
    portOut->setValue(value);
    modified = true;
}

void ArpWidget::setChannelOut(int value)
{

    channelOut->setValue(value);
    modified = true;
}

void ArpWidget::updateText(QString newtext)
{ 
    patternPresetBox->setCurrentIndex(0);
    textRemoveAction->setEnabled(false);
    textStoreAction->setEnabled(true);
    midiArp->updatePattern(newtext);
    arpScreen->updateArpScreen(newtext);
    emit(patternChanged());
    modified = true;
}

void ArpWidget::selectPatternPreset(int val)
{
    if (val) {
        patternText->setText(patternPresets.at(val));
        patternPresetBox->setCurrentIndex(val);
        midiArp->updatePattern(patternText->text());
        arpScreen->updateArpScreen(patternText->text());
        textStoreAction->setEnabled(false);
        emit(patternChanged());
        textRemoveAction->setEnabled(true);
    } else
        textRemoveAction->setEnabled(false);
    modified = true;
}

void ArpWidget::loadPatternPresets()
{
    QString qs, qs2;
    QDir qmahome = QDir(QDir::homePath());
    QString qmarcpath = qmahome.filePath(QMARCNAME);
    QFile f(qmarcpath);

    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, PACKAGE,
                tr("Could not read from resource file"));
        return;
    }   
    QTextStream loadText(&f);
    patternNames.clear();
    patternPresets.clear();

    while (!loadText.atEnd()) {
        qs = loadText.readLine();
        if (qs.startsWith('[')) break;
        qs2 = loadText.readLine();
        patternNames << qs;
        patternPresets << qs2;
    }
}

void ArpWidget::updateRepeatPattern(int val)
{
    midiArp->repeatPatternThroughChord = val;
    modified = true;
}

void ArpWidget::openTextEditWindow(bool on)
{
    patternText->setHidden(!on);
}

void ArpWidget::storeCurrentPattern()
{
    QString qs;
    bool ok;

    qs = QInputDialog::getText(this, tr("%1: Store pattern").arg(PACKAGE),
            tr("New pattern"), QLineEdit::Normal, tr("Arp pattern"), &ok);

    if (ok && !qs.isEmpty()) {
        
        emit presetsChanged(qs, patternText->text(), 0);
        patternPresetBox->setCurrentIndex(patternNames.count() - 1);
        textRemoveAction->setEnabled(true);
    }
}

void ArpWidget::updatePatternPresets(QString n, QString p, int index)
{
    if (index) {
       if (index == patternPresetBox->currentIndex()) {
            patternPresetBox->setCurrentIndex(0);
            textRemoveAction->setEnabled(false);
        }
        patternNames.removeAt(index);
        patternPresets.removeAt(index);
        patternPresetBox->removeItem(index);
    } else {
        patternNames.append(n);
        patternPresets.append(p);
        patternPresetBox->addItem(n);
    }
}

void ArpWidget::removeCurrentPattern()
{
    QString qs;

    int currentIndex = patternPresetBox->currentIndex();
    if (currentIndex < 1) {
        return;
    } 

    qs = tr("Remove \"%1\"?").arg(patternPresetBox->currentText());

    if (QMessageBox::question(0, PACKAGE, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
        
    emit presetsChanged("", "", currentIndex);
}

bool ArpWidget::isModified()
{
    return modified;
}

void ArpWidget::setModified(bool m)
{
    modified = m;
}

void ArpWidget::moduleDelete()
{
    QString qs;
    qs = tr("Delete \"%1\"?")
        .arg(name);
    if (QMessageBox::question(0, APP_NAME, qs, QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Default
                | QMessageBox::Escape, QMessageBox::NoButton)
            == QMessageBox::No) {
        return;
    }
    emit arpRemove(ID);
}

void ArpWidget::moduleRename()
{
    QString newname, oldname;
    bool ok;
    
    oldname = name;

    newname = QInputDialog::getText(this, APP_NAME,
                tr("New Name"), QLineEdit::Normal, oldname.mid(4), &ok);
                
    if (ok && !newname.isEmpty()) {
        name = "Arp:" + newname;
        emit dockRename(name, parentDockID);
    }
}
