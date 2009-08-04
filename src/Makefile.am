# Makefile.am for qmidiarp

bin_PROGRAMS = qmidiarp

nodist_qmidiarp_SOURCES = \
	arpdata_moc.cpp \
	arpscreen_moc.cpp \
	arpwidget_moc.cpp \
	groovewidget_moc.cpp \
	gui_moc.cpp \
	logwidget_moc.cpp \
	midiarp_moc.cpp \
	passwidget_moc.cpp \
	seqdriver_moc.cpp \
	slider_moc.cpp
	
qmidiarp_SOURCES = \
	arpdata.cpp arpdata.h \
	arpscreen.cpp arpscreen.h \
	arpwidget.cpp arpwidget.h \
	groovewidget.cpp groovewidget.h \
	gui.cpp gui.h \
	logwidget.cpp logwidget.h \
 	main.cpp main.h \
	midiarp.cpp midiarp.h \
	passwidget.cpp passwidget.h \
	seqdriver.cpp seqdriver.h \
	slider.cpp slider.h

AM_LDFLAGS = @QT_LIBS@ @DEFS@
AM_CXXFLAGS = @QT_CXXFLAGS@
DEFS = -Wall -W -D_REENTRANT -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED @DEFS@

# misc files which are distributed but not installed
EXTRA_DIST = qmidiarp.pro

# all generated files to be removed by "make clean"
CLEANFILES = $(nodist_qmidiarp_SOURCES)

# rule for moc-file generation
SUFFIXES = _moc.cpp

.h_moc.cpp: $(qmidiarp_SOURCES)
	$(MOC) $< -o $@

