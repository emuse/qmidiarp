# Makefile.am for qmidiarp
#
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = man examples src

svgdatadir=@datadir@/icons/hicolor/scalable/apps
dist_svgdata_DATA = qmidiarp.svg

applicationsdir = @datadir@/applications
dist_applications_DATA = qmidiarp.desktop

appdatadir = @datadir@/metainfo
dist_appdata_DATA = qmidiarp.appdata.xml

include $(top_srcdir)/aminclude.am

EXTRA_DIST = Doxyfile html/qmidiarp_logo_med2.png \
	qmidiarp.pro configure-qmake.sh install-qmake.sh \
	CMakeLists.txt \
	qmidiarp_arp.lv2/CMakeLists.txt qmidiarp_lfo.lv2/CMakeLists.txt \
	qmidiarp_seq.lv2/CMakeLists.txt \
	robtk/* robtk/*/*

if BUILD_LV2
arp_lv2datadir=@libdir@/lv2/qmidiarp_arp.lv2
lfo_lv2datadir=@libdir@/lv2/qmidiarp_lfo.lv2
seq_lv2datadir=@libdir@/lv2/qmidiarp_seq.lv2

dist_arp_lv2data_DATA = qmidiarp_arp.lv2/manifest.ttl \
	qmidiarp_arp.lv2/qmidiarp_arp.ttl qmidiarp_arp.lv2/qmidiarp_arp_ui.ttl

dist_lfo_lv2data_DATA = qmidiarp_lfo.lv2/manifest.ttl \
	qmidiarp_lfo.lv2/qmidiarp_lfo.ttl qmidiarp_lfo.lv2/qmidiarp_lfo_ui.ttl

dist_seq_lv2data_DATA = qmidiarp_seq.lv2/manifest.ttl \
	qmidiarp_seq.lv2/qmidiarp_seq.ttl qmidiarp_seq.lv2/qmidiarp_seq_ui.ttl


# install LV2 .so only, not .la

install-exec-hook:
	rm -f $(DESTDIR)$(arp_lv2datadir)/*.la
	rm -f $(DESTDIR)$(lfo_lv2datadir)/*.la
	rm -f $(DESTDIR)$(seq_lv2datadir)/*.la
	
uninstall-hook:
	rm -f $(DESTDIR)$(arp_lv2datadir)/*.so
	rm -f $(DESTDIR)$(lfo_lv2datadir)/*.so
	rm -f $(DESTDIR)$(seq_lv2datadir)/*.so
	rmdir $(DESTDIR)$(arp_lv2datadir)
	rmdir $(DESTDIR)$(lfo_lv2datadir)
	rmdir $(DESTDIR)$(seq_lv2datadir)

if BUILD_LV2_UI_RTK

nodist_seq_lv2data_DATA = qmidiarp_seq.lv2/qmidiarp_seq_ui_gl.so
nodist_lfo_lv2data_DATA = qmidiarp_lfo.lv2/qmidiarp_lfo_ui_gl.so
nodist_arp_lv2data_DATA = qmidiarp_arp.lv2/qmidiarp_arp_ui_gl.so

CLEANFILES = \
	qmidiarp_seq.lv2/qmidiarp_seq_ui_gl.so \
	qmidiarp_lfo.lv2/qmidiarp_lfo_ui_gl.so \
	qmidiarp_arp.lv2/qmidiarp_arp_ui_gl.so

qmidiarp_seq.lv2/qmidiarp_seq_ui_gl.so: robtk/ui_gl.c robtk/pugl/pugl_x11.c
	$(CC)  $(LV2_UI_RTK_CFLAGS) -DPLUGIN_SOURCE="\"src/seqwidget_lv2_rtk.c\"" -o $@ $^ \
	$(LV2_UI_RTK_LDFLAGS) $(LV2_UI_RTK_LIBS)
	
qmidiarp_lfo.lv2/qmidiarp_lfo_ui_gl.so: robtk/ui_gl.c robtk/pugl/pugl_x11.c
	$(CC)  $(LV2_UI_RTK_CFLAGS) -DPLUGIN_SOURCE="\"src/lfowidget_lv2_rtk.c\"" -o $@ $^ \
	$(LV2_UI_RTK_LDFLAGS) $(LV2_UI_RTK_LIBS)

qmidiarp_arp.lv2/qmidiarp_arp_ui_gl.so: robtk/ui_gl.c robtk/pugl/pugl_x11.c
	$(CC)  $(LV2_UI_RTK_CFLAGS) -DPLUGIN_SOURCE="\"src/arpwidget_lv2_rtk.c\"" -o $@ $^ \
	$(LV2_UI_RTK_LDFLAGS) $(LV2_UI_RTK_LIBS)

endif
endif
