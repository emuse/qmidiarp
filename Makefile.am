# Makefile.am for qmidiarp
#
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = man examples src

svgdatadir=@datadir@/icons/hicolor/scalable/apps
dist_svgdata_DATA = qmidiarp.svg

applicationsdir = @datadir@/applications
dist_applications_DATA = qmidiarp.desktop

include $(top_srcdir)/aminclude.am
EXTRA_DIST = Doxyfile html/qmidiarp_logo_med2.png

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
endif
