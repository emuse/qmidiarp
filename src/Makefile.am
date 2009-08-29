# Makefile.am for qmidiarp
SUBDIRS = pixmaps

bin_PROGRAMS = qmidiarp

nodist_qmidiarp_SOURCES = \
	arpdata_moc.cpp \
	arpscreen_moc.cpp \
	arpwidget_moc.cpp \
	groovewidget_moc.cpp \
	mainwindow_moc.cpp \
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
	mainwindow.cpp mainwindow.h \
	logwidget.cpp logwidget.h \
	main.cpp main.h \
	midiarp.cpp midiarp.h \
	passwidget.cpp passwidget.h \
	seqdriver.cpp seqdriver.h \
	slider.cpp slider.h

translationsdir = $(pkgdatadir)/translations
translations = \
	translations/qmidiarp_de.ts \
	translations/qmidiarp_fr.ts

translations_DATA = \
	translations/qmidiarp_de.qm \
	translations/qmidiarp_fr.qm


AM_LDFLAGS = @QT_LIBS@ @DEFS@
AM_CXXFLAGS = @QT_CXXFLAGS@
DEFS = -Wall -Wextra -D_REENTRANT -DTRANSLATIONSDIR=\"$(translationsdir)\" @DEFS@

# misc files which are distributed but not installed
EXTRA_DIST = qmidiarp.pro $(translations)

# all generated files to be removed by "make clean"
CLEANFILES = $(nodist_qmidiarp_SOURCES)  $(translations_DATA)

# rule for moc-file generation
SUFFIXES = _moc.cpp

.h_moc.cpp: $(qmidiarp_SOURCES)
	$(MOC) $< -o $@

# rule for translation file updates
$(translations): $(qmidiarp_SOURCES)
	$(LUPDATE) -verbose $(qmidiarp_SOURCES) -ts $@

# rule to generate binary translation files
.ts.qm: $(translations)
	$(LRELEASE) -verbose $< -qm $@

