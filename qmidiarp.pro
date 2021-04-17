DEFINES += PACKAGE='\\"qmidiarp\\"'
DEFINES += PACKAGE_VERSION='\\"0.6.6\\"'
DEFINES += APP_NAME='\\"QMidiArp\\"'

DEFINES += APPBUILD JACK_SESSION
QT += core gui widgets
CONFIG += qt

Release:DESTDIR = release/bin
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:RCC_DIR = release/.rcc
Release:UI_DIR = release/.ui

SOURCES += \
    src/cursor.cpp\
    src/engine.cpp\
    src/arpscreen.cpp\
    src/lfoscreen.cpp\
    src/seqscreen.cpp\
    src/arpwidget.cpp\
    src/lfowidget.cpp\
    src/seqwidget.cpp\
    src/groovewidget.cpp\
    src/mainwindow.cpp\
    src/globstore.cpp\
    src/indicator.cpp\
    src/modulewidget.cpp\
    src/logwidget.cpp\
    src/main.cpp\
    src/midiworker.cpp\
    src/midiarp.cpp\
    src/midilfo.cpp \
    src/midiseq.cpp \
    src/midicctable.cpp\
    src/midicontrol.cpp\
    src/parstore.cpp\
    src/prefs.cpp\
    src/prefswidget.cpp\
    src/jackdriver.cpp\
    src/screen.cpp\
    src/seqdriver.cpp\
    src/slider.cpp\
    src/storagebutton.cpp

HEADERS += \
    src/cursor.h\
    src/engine.h\
    src/arpscreen.h\
    src/lfoscreen.h\
    src/seqscreen.h\
    src/arpwidget.h\
    src/lfowidget.h\
    src/seqwidget.h\
    src/groovewidget.h\
    src/mainwindow.h\
    src/globstore.h\
    src/indicator.h\
    src/modulewidget.h\
    src/logwidget.h\
    src/main.h\
    src/midiworker.h\
    src/midiarp.h\
    src/midilfo.h \
    src/midiseq.h \
    src/midicctable.h\
    src/midicontrol.h\
    src/parstore.h\
    src/prefs.h\
    src/prefswidget.h\
    src/jackdriver.h\
    src/screen.h\
    src/seqdriver.h\
    src/slider.h\
    src/storagebutton.h\
    src/midievent.h \
    src/nsm.h \
    src/driverbase.h

TRANSLATIONS += \
        src/translations/qmidiarp_cs.ts \
        src/translations/qmidiarp_de.ts \
        src/translations/qmidiarp_es.ts \
        src/translations/qmidiarp_fr.ts
            

LIBS += c:/Qt/Tools/mingw492_32/lib/libjack.lib
INCLUDEPATH = c:/Qt/Tools/mingw492_32/include c:/Qt/5.6/mingw49_32/include/QtWidgets
