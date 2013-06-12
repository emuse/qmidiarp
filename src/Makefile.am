# Makefile.am for qmidiarp
SUBDIRS = pixmaps
lfoplugindir = /usr/lib64/lv2/qmidiarp_lfo.lv2
seqplugindir = /usr/lib64/lv2/qmidiarp_seq.lv2

bin_PROGRAMS = qmidiarp

lfoplugin_LTLIBRARIES = qmidiarp_lfo.la
seqplugin_LTLIBRARIES = qmidiarp_seq.la

nodist_qmidiarp_SOURCES = \
	cursor_moc.cpp \
	engine_moc.cpp \
	arpscreen_moc.cpp \
	lfoscreen_moc.cpp \
	seqscreen_moc.cpp \
	arpwidget_moc.cpp \
	lfowidget_moc.cpp \
	seqwidget_moc.cpp \
	managebox_moc.cpp \
	groovewidget_moc.cpp \
	mainwindow_moc.cpp \
	globstore_moc.cpp \
	indicator_moc.cpp \
	logwidget_moc.cpp \
	midiarp_moc.cpp \
	midilfo_moc.cpp \
	midiseq_moc.cpp \
	midicctable_moc.cpp \
	midicontrol_moc.cpp \
	passwidget_moc.cpp \
	parstore_moc.cpp \
	jackdriver_moc.cpp \
	seqdriver_moc.cpp \
	slider_moc.cpp \
	storagebutton_moc.cpp

qmidiarp_SOURCES = \
	cursor.cpp cursor.h \
	engine.cpp engine.h \
	arpscreen.cpp arpscreen.h \
	lfoscreen.cpp lfoscreen.h \
	seqscreen.cpp seqscreen.h \
	arpwidget.cpp arpwidget.h \
	lfowidget.cpp lfowidget.h \
	seqwidget.cpp seqwidget.h \
	managebox.cpp managebox.h \
	groovewidget.cpp groovewidget.h \
	mainwindow.cpp mainwindow.h \
	globstore.cpp globstore.h \
	indicator.cpp indicator.h \
	logwidget.cpp logwidget.h \
	main.cpp main.h \
	midiarp.cpp midiarp.h \
	midilfo.cpp midilfo.h \
	midiseq.cpp midiseq.h \
	midicctable.cpp midicctable.h \
	midicontrol.cpp midicontrol.h \
	midievent.h \
	nsm.h \
	driverbase.h \
	parstore.cpp parstore.h \
	passwidget.cpp passwidget.h \
	jackdriver.cpp jackdriver.h \
	seqdriver.cpp seqdriver.h \
	slider.cpp slider.h \
	storagebutton.cpp storagebutton.h

qmidiarp_CXXFLAGS = $(AM_CXXFLAGS)
qmidiarp_LDADD = $(QT_LIBS)

nodist_qmidiarp_lfo_la_SOURCES = \
	cursor_moc.cpp \
	globstore_moc.cpp \
	indicator_moc.cpp \
	lfowidget_moc.cpp \
	lfoscreen_moc.cpp \
	managebox_moc.cpp \
	midicontrol_moc.cpp \
	midilfo_moc.cpp \
	parstore_moc.cpp \
	slider_moc.cpp \
	storagebutton_moc.cpp \
	qmidiarp_lfowidget_lv2_moc.cpp \
	qmidiarp_lfo_lv2_moc.cpp

qmidiarp_lfo_la_SOURCES = \
	cursor.cpp cursor.h \
	globstore.cpp globstore.h \
	indicator.cpp indicator.h \
	lfowidget.cpp lfowidget.h \
	lfoscreen.cpp lfoscreen.h \
	managebox.cpp managebox.h \
	midicontrol.cpp midicontrol.h \
	midilfo.cpp midilfo.h \
	parstore.cpp parstore.h \
	slider.cpp slider.h \
	storagebutton.cpp storagebutton.h \
	qmidiarp_lfowidget_lv2.cpp qmidiarp_lfowidget_lv2.h \
	qmidiarp_lfo_lv2.cpp qmidiarp_lfo_lv2.h

qmidiarp_lfo_la_LDFLAGS = -module -avoid-version -E
qmidiarp_lfo_la_LIBADD = $(QT_LIBS) -llv2-plugin

nodist_qmidiarp_seq_la_SOURCES = \
	cursor_moc.cpp \
	globstore_moc.cpp \
	indicator_moc.cpp \
	seqwidget_moc.cpp \
	seqscreen_moc.cpp \
	managebox_moc.cpp \
	midicontrol_moc.cpp \
	midiseq_moc.cpp \
	parstore_moc.cpp \
	slider_moc.cpp \
	storagebutton_moc.cpp \
	qmidiarp_seqwidget_lv2_moc.cpp \
	qmidiarp_seq_lv2_moc.cpp

qmidiarp_seq_la_SOURCES = \
	cursor.cpp cursor.h \
	globstore.cpp globstore.h \
	indicator.cpp indicator.h \
	seqwidget.cpp seqwidget.h \
	seqscreen.cpp seqscreen.h \
	managebox.cpp managebox.h \
	midicontrol.cpp midicontrol.h \
	midiseq.cpp midiseq.h \
	parstore.cpp parstore.h \
	slider.cpp slider.h \
	storagebutton.cpp storagebutton.h \
	qmidiarp_seqwidget_lv2.cpp qmidiarp_seqwidget_lv2.h \
	qmidiarp_seq_lv2.cpp qmidiarp_seq_lv2.h

qmidiarp_seq_la_LDFLAGS = -module -avoid-version -E
qmidiarp_seq_la_LIBADD = $(QT_LIBS) -llv2-plugin

if ENABLE_TRANSLATIONS
translationsdir = $(pkgdatadir)/translations
translations = \
	translations/qmidiarp_cs.ts \
	translations/qmidiarp_de.ts \
	translations/qmidiarp_es.ts \
	translations/qmidiarp_fr.ts

translations_DATA = \
	translations/qmidiarp_cs.qm \
	translations/qmidiarp_de.qm \
	translations/qmidiarp_es.qm \
	translations/qmidiarp_fr.qm

TRANSLATION_DEFS = -DTRANSLATIONSDIR=\"$(translationsdir)\"
endif

AM_CXXFLAGS = @QT_CXXFLAGS@
DEFS = -Wall -Wextra -D_REENTRANT $(TRANSLATION_DEFS) @DEFS@

# misc files which are distributed but not installed
EXTRA_DIST = qmidiarp.pro $(translations)

# all generated files to be removed by "make clean"
CLEANFILES = $(nodist_qmidiarp_SOURCES)  $(translations_DATA)

# rule for moc-file generation
SUFFIXES = _moc.cpp

.h_moc.cpp: $(qmidiarp_SOURCES)
	$(MOC) $< -o $@

if ENABLE_TRANSLATIONS
# rule for translation file updates
$(translations): $(qmidiarp_SOURCES)
	$(LUPDATE) -verbose $(qmidiarp_SOURCES) -ts $@

# rule to generate binary translation files
.ts.qm: $(translations)
	$(LRELEASE) -verbose $< -qm $@
endif

# install LV2 .so only, not .la

install-exec-hook:

	rm -f $(DESTDIR)$(plugindir)/qmidiarp_lfo.la
	rm -f $(DESTDIR)$(plugindir)/qmidiarp_seq.la
