
vmoddir = $(VMODDIR)
vmod_LTLIBRARIES = libvmod_geoip.la

libvmod_geoip_la_CFLAGS = -I$(VARNISHSRC)/include -I$(VARNISHSRC)
libvmod_geoip_la_LDFLAGS = -module -export-dynamic -avoid-version -shared \
	-lGeoIP

libvmod_geoip_la_SOURCES = \
	vmod_geoip.c
nodist_libvmod_geoip_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

VMOD_TESTS = tests/*.vtc

$(libvmod_geoip_la_OBJECTS): vcc_if.h

vcc_if.c vcc_if.h: $(VARNISHSRC)/lib/libvmod_std/vmod.py $(top_srcdir)/src/$(VCC_FILE)
	$(PYTHON) $(VARNISHSRC)/lib/libvmod_std/vmod.py $(top_srcdir)/src/$(VCC_FILE)

check:
	$(VARNISHSRC)/bin/varnishtest/varnishtest \
	    -Dvarnishd=$(VARNISHSRC)/bin/varnishd/varnishd \
	    -Dvmod_topbuild=$(abs_top_builddir) $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_geoip.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c $(builddir)/vcc_if.h
