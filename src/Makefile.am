# Makefile.am for qmidiarp
if BUILD_APP
SUBDIRS = pixmaps
bin_PROGRAMS = qmidiarp

nodist_qmidiarp_SOURCES = \
	cursor_moc.cpp \
	engine_moc.cpp \
	arpscreen_moc.cpp \
	lfoscreen_moc.cpp \
	seqscreen_moc.cpp \
	arpwidget_moc.cpp \
	lfowidget_moc.cpp \
	seqwidget_moc.cpp \
	groovewidget_moc.cpp \
	mainwindow_moc.cpp \
	globstore_moc.cpp \
	indicator_moc.cpp \
	modulewidget_moc.cpp \
	logwidget_moc.cpp \
	midiworker_moc.cpp \
	midiarp_moc.cpp \
	midilfo_moc.cpp \
	midiseq_moc.cpp \
	midicctable_moc.cpp \
	midicontrol_moc.cpp \
	prefswidget_moc.cpp \
	parstore_moc.cpp \
	prefs_moc.cpp \
	jackdriver_moc.cpp \
	screen_moc.cpp \
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
	groovewidget.cpp groovewidget.h \
	mainwindow.cpp mainwindow.h \
	globstore.cpp globstore.h \
	indicator.cpp indicator.h \
	modulewidget.cpp modulewidget.h \
	logwidget.cpp logwidget.h \
	main.cpp main.h \
	midiworker.cpp midiworker.h \
	midiarp.cpp midiarp.h \
	midilfo.cpp midilfo.h \
	midiseq.cpp midiseq.h \
	midicctable.cpp midicctable.h \
	midicontrol.cpp midicontrol.h \
	midievent.h \
	nsm.h \
	driverbase.h \
	parstore.cpp parstore.h \
	prefswidget.cpp prefswidget.h \
	prefs.cpp prefs.h \
	jackdriver.cpp jackdriver.h \
	screen.cpp screen.h \
	seqdriver.cpp seqdriver.h \
	slider.cpp slider.h \
	storagebutton.cpp storagebutton.h

qmidiarp_CXXFLAGS = $(AM_CXXFLAGS) -DAPPBUILD -Wno-deprecated-copy -std=c++17 -Wall -Wextra -D_REENTRANT $(TRANSLATION_DEFS) 
qmidiarp_LDFLAGS =  -Wl,--as-needed
qmidiarp_LDADD = $(LIBS_APP) $(QT_LIBS)

endif

if BUILD_LV2

if LIBDIR_IS_HOME
plugindir = @libdir@/.lv2
else
plugindir = @libdir@/lv2
endif

arpplugindir = $(plugindir)/qmidiarp_arp.lv2
lfoplugindir = $(plugindir)/qmidiarp_lfo.lv2
seqplugindir = $(plugindir)/qmidiarp_seq.lv2

arpplugin_LTLIBRARIES = qmidiarp_arp.la
lfoplugin_LTLIBRARIES = qmidiarp_lfo.la
seqplugin_LTLIBRARIES = qmidiarp_seq.la

qmidiarp_lfo_la_SOURCES = \
	lv2_common.h \
	main.h \
	midiworker.cpp midiworker.h \
	midilfo.cpp midilfo.h \
	midilfo_lv2.cpp midilfo_lv2.h

qmidiarp_lfo_la_LDFLAGS = -module -avoid-version -Wl,--as-needed 
qmidiarp_lfo_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(AM_CXXFLAGS)

qmidiarp_seq_la_SOURCES = \
	lv2_common.h \
	main.h \
	midiworker.cpp midiworker.h \
	midiseq.cpp midiseq.h \
	midiseq_lv2.cpp midiseq_lv2.h

qmidiarp_seq_la_LDFLAGS = -module -avoid-version -Wl,--as-needed
qmidiarp_seq_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(AM_CXXFLAGS)

qmidiarp_arp_la_SOURCES = \
	lv2_common.h \
	main.h \
	midiworker.cpp midiworker.h \
	midiarp.cpp midiarp.h \
	midiarp_lv2.cpp midiarp_lv2.h

qmidiarp_arp_la_LDFLAGS = -module -avoid-version -Wl,--as-needed
qmidiarp_arp_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(AM_CXXFLAGS)


if BUILD_LV2_UI
SUBDIRS = pixmaps

arppluginuidir = $(arpplugindir)
lfopluginuidir = $(lfoplugindir)
seqpluginuidir = $(seqplugindir)

arppluginui_LTLIBRARIES = qmidiarp_arp_ui.la
lfopluginui_LTLIBRARIES = qmidiarp_lfo_ui.la
seqpluginui_LTLIBRARIES = qmidiarp_seq_ui.la

nodist_qmidiarp_lfo_ui_la_SOURCES = \
	cursor_moc.cpp \
	modulewidget_moc.cpp \
	lfowidget_moc.cpp \
	lfoscreen_moc.cpp \
	screen_moc.cpp \
	slider_moc.cpp \
	lfowidget_lv2_moc.cpp

qmidiarp_lfo_ui_la_SOURCES = \
	cursor.cpp cursor.h \
	modulewidget.cpp modulewidget.h \
	lfowidget.cpp lfowidget.h \
	lfoscreen.cpp lfoscreen.h \
	lv2_common.h \
	main.h \
	screen.cpp screen.h \
	slider.cpp slider.h \
	lfowidget_lv2.cpp lfowidget_lv2.h

qmidiarp_lfo_ui_la_LDFLAGS = -module -avoid-version -Wl,--as-needed
qmidiarp_lfo_ui_la_LIBADD = $(QT_LIBS)
qmidiarp_lfo_ui_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(TRANSLATION_DEFS)  $(AM_CXXFLAGS)

nodist_qmidiarp_seq_ui_la_SOURCES = \
	cursor_moc.cpp \
	modulewidget_moc.cpp \
	screen_moc.cpp \
	seqwidget_moc.cpp \
	seqscreen_moc.cpp \
	slider_moc.cpp \
	seqwidget_lv2_moc.cpp

qmidiarp_seq_ui_la_SOURCES = \
	cursor.cpp cursor.h \
	lv2_common.h \
	main.h \
	modulewidget.cpp modulewidget.h \
	screen.cpp screen.h \
	seqwidget.cpp seqwidget.h \
	seqscreen.cpp seqscreen.h \
	slider.cpp slider.h \
	seqwidget_lv2.cpp seqwidget_lv2.h

qmidiarp_seq_ui_la_LDFLAGS = -module -avoid-version -Wl,--as-needed
qmidiarp_seq_ui_la_LIBADD = $(QT_LIBS)
qmidiarp_seq_ui_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(TRANSLATION_DEFS)  $(AM_CXXFLAGS)

nodist_qmidiarp_arp_ui_la_SOURCES = \
	cursor_moc.cpp \
	modulewidget_moc.cpp \
	arpwidget_moc.cpp \
	arpscreen_moc.cpp \
	screen_moc.cpp \
	slider_moc.cpp \
	arpwidget_lv2_moc.cpp

qmidiarp_arp_ui_la_SOURCES = \
	cursor.cpp cursor.h \
	lv2_common.h \
	main.h \
	modulewidget.cpp modulewidget.h \
	arpwidget.cpp arpwidget.h \
	arpscreen.cpp arpscreen.h \
	screen.cpp screen.h \
	slider.cpp slider.h \
	arpwidget_lv2.cpp arpwidget_lv2.h

qmidiarp_arp_ui_la_LDFLAGS = -module -avoid-version -Wl,--as-needed
qmidiarp_arp_ui_la_LIBADD = $(QT_LIBS)
qmidiarp_arp_ui_la_CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-copy -D_REENTRANT -fvisibility=hidden $(TRANSLATION_DEFS)  $(AM_CXXFLAGS)


if BUILD_LV2_UI_RTK

arpplugin_ui_gldir = $(arpplugindir)
lfoplugin_ui_gldir = $(lfoplugindir)
seqplugin_ui_gldir = $(seqplugindir)

arpplugin_ui_gl_LTLIBRARIES = qmidiarp_arp_ui_gl.la
lfoplugin_ui_gl_LTLIBRARIES = qmidiarp_lfo_ui_gl.la
seqplugin_ui_gl_LTLIBRARIES = qmidiarp_seq_ui_gl.la

qmidiarp_seq_ui_gl_la_SOURCES = ../robtk/ui_gl.c ../robtk/pugl/pugl_x11.c
qmidiarp_lfo_ui_gl_la_SOURCES = ../robtk/ui_gl.c ../robtk/pugl/pugl_x11.c
qmidiarp_arp_ui_gl_la_SOURCES = ../robtk/ui_gl.c ../robtk/pugl/pugl_x11.c

qmidiarp_seq_ui_gl_la_DEPENDENCIES = seqwidget_lv2_rtk.c seqwidget_lv2_rtk.h
qmidiarp_lfo_ui_gl_la_DEPENDENCIES = lfowidget_lv2_rtk.c lfowidget_lv2_rtk.h
qmidiarp_arp_ui_gl_la_DEPENDENCIES = arpwidget_lv2_rtk.c arpwidget_lv2_rtk.h text_input_X11.h

qmidiarp_seq_ui_gl_la_LDFLAGS = $(LV2_UI_RTK_LDFLAGS) -module -avoid-version -Wl,--as-needed 
qmidiarp_lfo_ui_gl_la_LDFLAGS = $(LV2_UI_RTK_LDFLAGS) -module -avoid-version -Wl,--as-needed
qmidiarp_arp_ui_gl_la_LDFLAGS = $(LV2_UI_RTK_LDFLAGS) -module -avoid-version -Wl,--as-needed

qmidiarp_seq_ui_gl_la_LIBADD = $(LV2_UI_RTK_LIBS)
qmidiarp_lfo_ui_gl_la_LIBADD = $(LV2_UI_RTK_LIBS)
qmidiarp_arp_ui_gl_la_LIBADD = $(LV2_UI_RTK_LIBS)

qmidiarp_seq_ui_gl_la_DEFS = -Wall -Wextra -D_REENTRANT -fvisibility=hidden
qmidiarp_lfo_ui_gl_la_DEFS = -Wall -Wextra -D_REENTRANT -fvisibility=hidden
qmidiarp_arp_ui_gl_la_DEFS = -Wall -Wextra -D_REENTRANT -fvisibility=hidden

qmidiarp_seq_ui_gl_la_CFLAGS = $(LV2_UI_RTK_CFLAGS) -DPLUGIN_SOURCE="\"seqwidget_lv2_rtk.c\""
qmidiarp_lfo_ui_gl_la_CFLAGS = $(LV2_UI_RTK_CFLAGS) -DPLUGIN_SOURCE="\"lfowidget_lv2_rtk.c\""
qmidiarp_arp_ui_gl_la_CFLAGS = $(LV2_UI_RTK_CFLAGS) -DPLUGIN_SOURCE="\"arpwidget_lv2_rtk.c\""

endif
endif
endif

translations = \
	translations/qmidiarp_cs.ts \
	translations/qmidiarp_de.ts \
	translations/qmidiarp_es.ts \
	translations/qmidiarp_fr.ts

if ENABLE_TRANSLATIONS
translationsdir = $(pkgdatadir)/translations
translations_DATA = \
	translations/qmidiarp_cs.qm \
	translations/qmidiarp_de.qm \
	translations/qmidiarp_es.qm \
	translations/qmidiarp_fr.qm

TRANSLATION_DEFS = -DTRANSLATIONSDIR=\"$(translationsdir)\"
endif

AM_CXXFLAGS = @QT_CXXFLAGS@

# misc files which are distributed but not installed
EXTRA_DIST = qmidiarp.pro $(translations) \
	$(qmidiarp_seq_ui_gl_la_DEPENDENCIES) \
	$(qmidiarp_lfo_ui_gl_la_DEPENDENCIES) \
	$(qmidiarp_arp_ui_gl_la_DEPENDENCIES) \
	CMakeLists.txt translations/CMakeLists.txt \
	cmake_config.h.in


# all generated files to be removed by "make clean"
CLEANFILES = \
	$(nodist_qmidiarp_SOURCES) \
	$(nodist_qmidiarp_arp_ui_la_SOURCES) \
	$(nodist_qmidiarp_lfo_ui_la_SOURCES) \
	$(nodist_qmidiarp_seq_ui_la_SOURCES) \
	$(translations_DATA)

if NEED_MOC
# rule for moc-file generation
SUFFIXES = _moc.cpp

.h_moc.cpp: $(qmidiarp_SOURCES)
	$(MOC) $(DEFINES) $< -o $@

if ENABLE_TRANSLATIONS
# rule for translation file updates
$(translations): $(qmidiarp_SOURCES)
	$(LUPDATE) -verbose $(qmidiarp_SOURCES) \
		arpwidget_lv2.cpp \
		lfowidget_lv2.cpp \
		seqwidget_lv2.cpp \
		-ts $@

# rule to generate binary translation files
.ts.qm: $(translations)
	$(LRELEASE) -verbose $< -qm $@
endif
endif
