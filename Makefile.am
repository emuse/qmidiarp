# Makefile.am for qmidiarp
#
SUBDIRS = man examples src

svgdatadir=@datadir@/icons/hicolor/scalable/apps
dist_svgdata_DATA = qmidiarp.svg

applicationsdir = @datadir@/applications
dist_applications_DATA = qmidiarp.desktop

include $(top_srcdir)/aminclude.am
EXTRA_DIST = Doxyfile html/qmidiarp_logo_med2.png

arp_lv2datadir=@libdir@/lv2/qmidiarp_arp.lv2
lfo_lv2datadir=@libdir@/lv2/qmidiarp_lfo.lv2
seq_lv2datadir=@libdir@/lv2/qmidiarp_seq.lv2

dist_arp_lv2data_DATA = qmidiarp_arp.lv2/manifest.ttl \
	qmidiarp_arp.lv2/qmidiarp_arp.ttl
dist_lfo_lv2data_DATA = qmidiarp_lfo.lv2/manifest.ttl \
	qmidiarp_lfo.lv2/qmidiarp_lfo.ttl
dist_seq_lv2data_DATA = qmidiarp_seq.lv2/manifest.ttl \
	qmidiarp_seq.lv2/qmidiarp_seq.ttl
