QT_BASE_DIR=/usr/lib/qt3
QT_LIB_DIR=$(QT_BASE_DIR)/lib
QT_BIN_DIR=$(QT_BASE_DIR)/bin
QT_INCLUDE_DIR=$(QT_BASE_DIR)/include
X11_LIB_DIR=/usr/X11R6/lib
X11_INCLUDE_DIR=/usr/X11R6/include

qmidiarp: gui.o gui.moc.o \
	arpwidget.o arpwidget.moc.o \
	midiarp.o midiarp.moc.o \
	arpdata.o arpdata.moc.o \
	slider.o slider.moc.o \
	seqdriver.o seqdriver.moc.o \
	logwidget.o logwidget.moc.o \
	passwidget.o passwidget.moc.o \
	groovewidget.o groovewidget.moc.o \
	main.o
	gcc -g -o qmidiarp \
	gui.o gui.moc.o \
	arpwidget.o arpwidget.moc.o \
	midiarp.o midiarp.moc.o \
	arpdata.o arpdata.moc.o \
	slider.o slider.moc.o \
	seqdriver.o seqdriver.moc.o \
	logwidget.o logwidget.moc.o \
	passwidget.o passwidget.moc.o \
	groovewidget.o groovewidget.moc.o \
	main.o \
	-L$(QT_LIB_DIR) -L$(X11_LIB_DIR) \
	-lqt-mt -lasound

CXXFLAGS=-I$(QT_INCLUDE_DIR) -I$(X11_INCLUDE_DIR) -I. -O2 -g -Wall

gui.o: gui.cpp gui.h arpdata.h logwidget.h passwidget.h arpwidget.h groovewidget.h slider.h
gui.moc.o: gui.moc.cpp gui.h arpdata.h logwidget.h passwidget.h arpwidget.h groovewidget.h slider.h
gui.moc.cpp: gui.h gui.cpp
	$(QT_BIN_DIR)/moc gui.h -o gui.moc.cpp
arpwidget.o: arpwidget.cpp arpwidget.h midiarp.h slider.h
arpwidget.moc.o: arpwidget.moc.cpp arpwidget.h midiarp.h slider.h
arpwidget.moc.cpp: arpwidget.h arpwidget.cpp
	$(QT_BIN_DIR)/moc arpwidget.h -o arpwidget.moc.cpp
midiarp.o: midiarp.cpp midiarp.h main.h
midiarp.moc.o: midiarp.moc.cpp midiarp.h main.h
midiarp.moc.cpp: midiarp.h midiarp.cpp
	$(QT_BIN_DIR)/moc midiarp.h -o midiarp.moc.cpp
arpdata.o: arpdata.cpp arpdata.h seqdriver.h arpwidget.h
arpdata.moc.o: arpdata.moc.cpp arpdata.h seqdriver.h
arpdata.moc.cpp: arpdata.h arpdata.cpp
	$(QT_BIN_DIR)/moc arpdata.h -o arpdata.moc.cpp
seqdriver.o: seqdriver.cpp seqdriver.h midiarp.h main.h
seqdriver.moc.o: seqdriver.moc.cpp seqdriver.h midiarp.h main.h
seqdriver.moc.cpp: seqdriver.h seqdriver.cpp
	$(QT_BIN_DIR)/moc seqdriver.h -o seqdriver.moc.cpp
logwidget.o: logwidget.cpp logwidget.h 
logwidget.moc.o: logwidget.moc.cpp logwidget.h
logwidget.moc.cpp: logwidget.h logwidget.cpp
	$(QT_BIN_DIR)/moc logwidget.h -o logwidget.moc.cpp
passwidget.o: passwidget.cpp passwidget.h 
passwidget.moc.o: passwidget.moc.cpp passwidget.h
passwidget.moc.cpp: passwidget.h passwidget.cpp
	$(QT_BIN_DIR)/moc passwidget.h -o passwidget.moc.cpp
groovewidget.o: groovewidget.cpp groovewidget.h slider.h
groovewidget.moc.o: groovewidget.moc.cpp groovewidget.h slider.h
groovewidget.moc.cpp: groovewidget.h groovewidget.cpp
	$(QT_BIN_DIR)/moc groovewidget.h -o groovewidget.moc.cpp
slider.o: slider.cpp slider.h
slider.moc.o: slider.moc.cpp slider.h
slider.moc.cpp: slider.h slider.cpp
	$(QT_BIN_DIR)/moc slider.h -o slider.moc.cpp
main.o: main.cpp gui.h main.h
