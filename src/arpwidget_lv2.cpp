/*!
 * @file arpwidget_lv2.cpp
 * @brief Implements the the LV2 GUI for the QMidiArp Arp plugin.
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
 *
 */

#include "arpwidget_lv2.h"

#include <unistd.h>

ArpWidgetLV2::ArpWidgetLV2 (
        LV2UI_Controller ct,
        LV2UI_Write_Function write_function,
        const LV2_Feature *const *host_features)
        : ArpWidget()
{
    m_controller = ct;
    writeFunction = write_function;

    /* Scan host features for URID map */

    LV2_URID_Map *urid_map;
    for (int i = 0; host_features[i]; ++i) {
        if (::strcmp(host_features[i]->URI, LV2_URID_URI "#map") == 0) {
            urid_map = (LV2_URID_Map *) host_features[i]->data;
        }
    }
    if (!urid_map) {
        qWarning("Host does not support urid:map.");
        return;
    }

    lv2_atom_forge_init(&forge, urid_map);

    /* Map URIS */
    QMidiArpURIs* const uris = &m_uris;
    map_uris(urid_map, uris);

    receivePatternFlag = false;

    transportBox = new QCheckBox(this);
    QLabel *transportBoxLabel = new QLabel(tr("&Sync with Host"),this);
    transportBoxLabel->setBuddy(transportBox);
    transportBox->setToolTip(tr("Sync to Transport from Host"));
    tempoSpin = new QSpinBox(this);
    tempoSpin->setRange(10, 400);
    tempoSpin->setValue(120);
    tempoSpin->setKeyboardTracking(false);
    tempoSpin->setToolTip(tr("Tempo of internal clock"));
    connect(transportBox, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(transportBox, SIGNAL(toggled(bool)), tempoSpin, SLOT(setDisabled(bool)));
    transportBox->setChecked(true);

    inOutBoxWidget->layout()->addWidget(transportBoxLabel);
    inOutBoxWidget->layout()->addWidget(transportBox);
    inOutBoxWidget->layout()->addWidget(tempoSpin);

    connect(attackTime,         SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(releaseTime,        SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(randomTick,         SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(randomLength,       SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(randomVelocity,     SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(indexIn[0],         SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(indexIn[1],         SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(rangeIn[0],         SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(rangeIn[1],         SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(channelOut,         SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(chIn,               SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(tempoSpin,          SIGNAL(valueChanged(int)), this, SLOT(mapParam(int)));
    connect(patternPresetBox,   SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(repeatPatternThroughChord, SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(octaveModeBox,      SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(octaveLowBox,       SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(octaveHighBox,      SIGNAL(activated(int)), this, SLOT(mapParam(int)));
    connect(patternText,        SIGNAL(textChanged(const QString&)), this,
            SLOT(updatePattern(const QString&)));

    connect(muteOutAction,      SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(latchModeAction,    SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(deferChangesAction, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableRestartByKbd, SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableTrigByKbd,    SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));
    connect(enableTrigLegato,   SIGNAL(toggled(bool)), this, SLOT(mapBool(bool)));

    setStyleSheet("QLabel { font: 7pt; } \
    QComboBox { font: 7pt; max-height: 15px;} \
    QToolButton { max-height: 20px;} \
    QSpinBox { font: 7pt; max-height: 20px;} \
    QCheckBox { font: 7pt; max-height: 20px;} \
    QGroupBox { font: 7pt; }");

    mouseXCur = 0.0;
    mouseYCur = 0.0;
    receivedPatternOnce = false;
    sendUIisUp(true);
}

ArpWidgetLV2::~ArpWidgetLV2()
{
}

void ArpWidgetLV2::port_event ( uint32_t port_index,
        uint32_t buffer_size, uint32_t format, const void *buffer )
{

    const QMidiArpURIs* uris = &m_uris;
    LV2_Atom* atom = (LV2_Atom*)buffer;

    if (!receivedPatternOnce) sendUIisUp(true);

    if (format == uris->atom_eventTransfer
      && atom->type == uris->atom_Object) {
        receivePattern(atom);
    }
    else if (format == 0 && buffer_size == sizeof(float)) {


    float fValue = *(float *) buffer;

        switch (port_index) {
            case ATTACK:
                    attackTime->setValue(fValue);
            break;
            case RELEASE:
                    releaseTime->setValue(fValue);
            break;
            case RANDOM_TICK:
                    randomTick->setValue(fValue);
            break;
            case RANDOM_LEN:
                    randomLength->setValue(fValue);
            break;
            case RANDOM_VEL:
                    randomVelocity->setValue(fValue);
            break;
            case CH_OUT:
                    channelOut->setCurrentIndex(fValue);
            break;
            case CH_IN:
                    chIn->setCurrentIndex(fValue);
            break;
            case CURSOR_POS:
                    if (screen->currentIndex != (int)fValue) {
                        screen->updateCursor((int)fValue);
                        screen->update();
                    }
            break;
            case PATTERN_PRESET:
                    //patternPresetBox->setCurrentIndex(fValue);
                    //updatePattern(patternPresets.at(fValue));
            break;
            case MUTE:
                    muteOutAction->setChecked((bool)fValue);
                    screen->setMuted(fValue);
                    screen->update();
            break;
            case LATCH_MODE:
                    latchModeAction->setChecked((bool)fValue);
            break;
            case OCTAVE_MODE:
                    octaveModeBox->setCurrentIndex(fValue);
            break;
            case OCTAVE_LOW:
                    octaveLowBox->setCurrentIndex(-(int)fValue);
            break;
            case OCTAVE_HIGH:
                    octaveHighBox->setCurrentIndex((int)fValue);
            break;
            case INDEX_IN1:
                    indexIn[0]->setValue(fValue);
            break;
            case INDEX_IN2:
                    indexIn[1]->setValue(fValue);
            break;
            case RANGE_IN1:
                    rangeIn[0]->setValue(fValue);
            break;
            case RANGE_IN2:
                    rangeIn[1]->setValue(fValue);
            break;
            case ENABLE_TRIGLEGATO:
                    enableTrigLegato->setChecked((bool)fValue);
            break;
            case ENABLE_RESTARTBYKBD:
                    enableRestartByKbd->setChecked((bool)fValue);
            break;
            case ENABLE_TRIGBYKBD:
                    enableTrigByKbd->setChecked((bool)fValue);
            break;
            case REPEAT_MODE:
                    repeatPatternThroughChord->setCurrentIndex(fValue);
            break;
            case RPATTERNFLAG:
                    //~ if ((int)fValue != receivePatternFlag) {
                        //~ receivePatternFlag = (int)fValue;
                    //~ }
            break;
            case DEFER:
                    deferChangesAction->setChecked((bool)fValue);
            break;
            case TRANSPORT_MODE:
                    transportBox->setChecked((bool)fValue);
            break;
            case TEMPO:
                    tempoSpin->setValue((int)fValue);
            break;
            default:
            break;
        }
    }
}

void ArpWidgetLV2::updatePattern(const QString& p_pattern)
{
    QChar c;
    QString pattern = p_pattern;
    int patternLen = p_pattern.length();
    int patternMaxIndex = 0;
    double minStepWidth = 1.0;
    int minOctave = 0;
    int maxOctave = 0;

    double stepwd = 1.0;
    double nsteps = 0.;
    int chordindex = 0;
    bool chordmd = false;
    int oct = 0;
    int npoints = 0;

    // strip off trailing control tokens
    if (patternLen) {
        c = (pattern.at(patternLen - 1));
        while (!c.isDigit() && (c != 'p') && (c != ')')) {
            pattern = pattern.left(patternLen - 1);
            patternLen--;
            if (patternLen < 1) break;
            c = (pattern.at(patternLen - 1));
        }
    }

    if (!receivePatternFlag) sendPattern(pattern);

    // determine some useful properties of the arp pattern,
    // number of octaves, step width and number of steps in beats and
    // number of points

    for (int l1 = 0; l1 < patternLen; l1++) {
        c = pattern.at(l1);

        if (c.isDigit()) {
            if (!chordindex) {
                nsteps += stepwd;
                npoints++;
                if (chordmd) chordindex++;
            }
            if (c.digitValue() > patternMaxIndex)
                patternMaxIndex = c.digitValue();
        }
        switch(c.toLatin1()) {
            case '(':
                chordmd = true;
                chordindex = 0;
                break;

            case ')':
                chordmd = false;
                chordindex = 0;
                break;

            case '>':
                stepwd *= .5;
                if (stepwd < minStepWidth)
                    minStepWidth *= .5;
                break;

            case '<':
                stepwd *= 2.0;
                break;

            case '.':
                stepwd = 1.0;
                break;

            case 'p':
                if (!chordmd) {
                    nsteps += stepwd;
                    npoints++;
                }
                break;

            case '+':
                oct++;
                if (oct > maxOctave)
                    maxOctave++;
                break;

            case '-':
                oct--;
                if (oct < minOctave)
                    minOctave--;
                break;

            case '=':
                oct=0;
                break;

            default:
                ;
        }
    }
    screen->updateData(pattern, minOctave, maxOctave, minStepWidth,
                    nsteps, patternMaxIndex);
    screen->update();
}

void ArpWidgetLV2::sendPattern(const QString & p)
{

    const QMidiArpURIs* uris = &m_uris;
    uint8_t obj_buf[1024];
    QByteArray byteArray = p.toUtf8();
    const char* c = byteArray.constData();

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&forge, 0);

    /* prepare forge buffer and initialize atom-sequence */
    lv2_atom_forge_set_buffer(&forge, obj_buf, 256);
    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 1, uris->pattern_string);

    /* forge container object of type 'pattern_string' */
    lv2_atom_forge_property_head(&forge, uris->pattern_string, 0);
    lv2_atom_forge_string(&forge, c, strlen(c));

    /* close-off frame */
    lv2_atom_forge_pop(&forge, &frame);
    writeFunction(m_controller, MidiIn, lv2_atom_total_size(msg), uris->atom_eventTransfer, msg);
}

void ArpWidgetLV2::sendUIisUp(bool on)
{
    const QMidiArpURIs* uris = &m_uris;
    uint8_t obj_buf[64];
    int state;

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_frame_time(&forge, 0);

    /* prepare forge buffer and initialize atom-sequence */
    lv2_atom_forge_set_buffer(&forge, obj_buf, 16);

    if (on) state = uris->ui_up; else state=uris->ui_down;

    LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object(&forge, &frame, 1, state);

    /* close-off frame */
    lv2_atom_forge_pop(&forge, &frame);
    writeFunction(m_controller, MidiIn, lv2_atom_total_size(msg), uris->atom_eventTransfer, msg);
}

void ArpWidgetLV2::receivePattern(LV2_Atom* atom)
{
    QMidiArpURIs* const uris = &m_uris;
    if ( (atom->type != uris->atom_Blank) 
            && (atom->type != uris->atom_Object)) return;

    receivedPatternOnce = true;

    /* cast the buffer to Atom Object */
    LV2_Atom_Object* obj = (LV2_Atom_Object*)atom;
    LV2_Atom *a0 = NULL;
    lv2_atom_object_get(obj, uris->pattern_string, &a0, NULL);
    if (obj->body.otype != uris->pattern_string) return;

    /* handle pattern string as atom body */
    const char* p = (const char*)LV2_ATOM_BODY(a0);
    if (!strlen(p)) return;

    QString newPattern = QString::fromUtf8(p);
    QString txPattern = newPattern.remove(QChar(0));
    receivePatternFlag = true;
    updatePattern(txPattern);
    patternText->setText(txPattern);
    screen->update();
    receivePatternFlag = false;
}

void ArpWidgetLV2::mapBool(bool on)
{
    float value = (float)on;
    if (muteOutAction == sender()) {
        updateParam(MUTE, value);
        screen->setMuted(value);
    }
    else if (deferChangesAction == sender())    updateParam(DEFER, value);
    else if (latchModeAction == sender())       updateParam(LATCH_MODE, value);
    else if (transportBox == sender())          updateParam(TRANSPORT_MODE, value);
    else if (enableRestartByKbd == sender())    updateParam(ENABLE_RESTARTBYKBD, value);
    else if (enableTrigByKbd == sender())       updateParam(ENABLE_TRIGBYKBD, value);
    else if (enableTrigLegato == sender())      updateParam(ENABLE_TRIGLEGATO, value);
}

void ArpWidgetLV2::mapParam(int value)
{
    if      (attackTime == sender())        updateParam(ATTACK, value);
    else if (releaseTime == sender())       updateParam(RELEASE, value);
    else if (randomTick == sender())        updateParam(RANDOM_TICK, value);
    else if (randomLength == sender())      updateParam(RANDOM_LEN, value);
    else if (randomVelocity == sender())    updateParam(RANDOM_VEL, value);
    else if (channelOut == sender())        updateParam(CH_OUT, value);
    else if (chIn == sender())              updateParam(CH_IN, value);
    //else if (patternPresetBox == sender())  updateParam(PATTERN_PRESET, value);
    else if (indexIn[0] == sender())        updateParam(INDEX_IN1, value);
    else if (indexIn[1] == sender())        updateParam(INDEX_IN2, value);
    else if (rangeIn[0] == sender())        updateParam(RANGE_IN1, value);
    else if (rangeIn[1] == sender())        updateParam(RANGE_IN2, value);
    else if (repeatPatternThroughChord == sender()) updateParam(REPEAT_MODE, value);
    else if (octaveModeBox == sender())     updateParam(OCTAVE_MODE, value);
    else if (octaveLowBox == sender())      updateParam(OCTAVE_LOW, -value);
    else if (octaveHighBox == sender())     updateParam(OCTAVE_HIGH, value);
    else if (tempoSpin == sender())         updateParam(TEMPO, value);
}

void ArpWidgetLV2::updateParam(int index, float fValue) const
{
        writeFunction(m_controller, index, sizeof(float), 0, &fValue);
}

//=== The following comes from the synthv1 plugin by rncbc ====
QApplication *ArpWidgetLV2::g_qAppInstance = nullptr;
unsigned int  ArpWidgetLV2::qAppCount = 0;


void ArpWidgetLV2::qAppInstantiate(void)
{
    if (qApp == nullptr && g_qAppInstance == nullptr) {
        static int s_argc = 1;
        static const char *s_argv[] = { __func__, nullptr };
        g_qAppInstance = new QApplication(s_argc, (char **) s_argv);
    }

    if (g_qAppInstance)
        qAppCount++;
}

void ArpWidgetLV2::qAppCleanup (void)
{
    if (g_qAppInstance && --qAppCount == 0) {
        delete g_qAppInstance;
        g_qAppInstance = nullptr;
    }
}

QApplication *ArpWidgetLV2::qAppInstance(void)
{
    return g_qAppInstance;
}

int MidiArpLV2ui_resize ( LV2UI_Handle ui, int width, int height )
{
    ArpWidgetLV2 *pWidget = static_cast<ArpWidgetLV2 *> (ui);
    if (pWidget) {
        pWidget->resize(width, height);
        return 0;
    } else {
        return 1;
    }
}

static const LV2UI_Resize MidiArpLV2ui_resize_interface =
{
    nullptr, // handle: host should use its own when calling ui_resize().
    MidiArpLV2ui_resize
};

static const void *MidiArpLV2ui_extension_data ( const char *uri )
{
    if (::strcmp(uri, LV2_UI__resize) == 0)
        return (void *) &MidiArpLV2ui_resize_interface;
    else
        return nullptr;
}

//===

static LV2UI_Handle MidiArpLV2ui_instantiate (
    const LV2UI_Descriptor *, const char *, const char *,
    LV2UI_Write_Function write_function,
    LV2UI_Controller controller, LV2UI_Widget *widget,
    const LV2_Feature *const *host_features )
{
    ArpWidgetLV2::qAppInstantiate();
    ArpWidgetLV2 *pWidget = new ArpWidgetLV2(
                    controller, write_function, host_features);
    *widget = pWidget;
    return pWidget;
}

static LV2UI_Handle MidiArpLV2ui_x11_instantiate (
    const LV2UI_Descriptor *, const char *, const char *,
    LV2UI_Write_Function write_function,
    LV2UI_Controller controller, LV2UI_Widget *widget,
    const LV2_Feature *const *ui_features )
{
    WId winid, parent = 0;
    LV2UI_Resize *resize = nullptr;

    for (int i = 0; ui_features[i]; ++i) {
        if (::strcmp(ui_features[i]->URI, LV2_UI__parent) == 0)
            parent = (WId) ui_features[i]->data;
        else
        if (::strcmp(ui_features[i]->URI, LV2_UI__resize) == 0)
            resize = (LV2UI_Resize *) ui_features[i]->data;
    }

    if (!parent)
        return nullptr;

    ArpWidgetLV2::qAppInstantiate();
    ArpWidgetLV2 *pWidget
        = new ArpWidgetLV2(controller, write_function, ui_features);
    if (resize && resize->handle) {
        const QSize& hint = pWidget->sizeHint();
        resize->ui_resize(resize->handle, hint.width(), hint.height());
    }
    winid = pWidget->winId();
    pWidget->windowHandle()->setParent(QWindow::fromWinId(parent));
    pWidget->show();
    *widget = (LV2UI_Widget) winid;
    return pWidget;
}

static void MidiArpLV2ui_cleanup ( LV2UI_Handle ui )
{
    ArpWidgetLV2 *pWidget = static_cast<ArpWidgetLV2 *> (ui);
    if (pWidget) {
        pWidget->sendUIisUp(false);
        delete pWidget;
    }
    ArpWidgetLV2::qAppCleanup();
}

static void MidiArpLV2ui_port_event (
    LV2UI_Handle ui, uint32_t port_index,
    uint32_t buffer_size, uint32_t format, const void *buffer )
{
    ArpWidgetLV2 *pWidget = static_cast<ArpWidgetLV2 *> (ui);
    if (pWidget)
        pWidget->port_event(port_index, buffer_size, format, buffer);
}

static const LV2UI_Descriptor MidiArpLV2ui_descriptor =
{
    QMIDIARP_ARP_LV2UI_URI,
    MidiArpLV2ui_instantiate,
    MidiArpLV2ui_cleanup,
    MidiArpLV2ui_port_event,
    MidiArpLV2ui_extension_data
};

static const LV2UI_Descriptor MidiArpLV2ui_x11_descriptor =
{
    QMIDIARP_ARP_LV2UI_X11_URI,
    MidiArpLV2ui_x11_instantiate,
    MidiArpLV2ui_cleanup,
    MidiArpLV2ui_port_event,
    MidiArpLV2ui_extension_data
};

LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor ( uint32_t index )
{
    if (index == 0)
        return &MidiArpLV2ui_descriptor;
    else
    if (index == 1)
        return &MidiArpLV2ui_x11_descriptor;
    else
        return NULL;
}
