
vmoddir = $(VMOD_DIR)
vmod_LTLIBRARIES = libvmod_geoip.la

libvmod_geoip_la_CFLAGS = $(VMOD_INCLUDES)
libvmod_geoip_la_LDFLAGS = -module -export-dynamic -avoid-version -shared \
	-lGeoIP

libvmod_geoip_la_SOURCES = \
	vmod_geoip.c
nodist_libvmod_geoip_la_SOURCES = \
	vcc_if.c \
	vcc_if.h

VMOD_TESTS = tests/*.vtc

$(libvmod_geoip_la_OBJECTS): vcc_if.h

vcc_if.c vcc_if.h: $(VMODTOOL) $(top_srcdir)/src/vmod_geoip.vcc
	$(PYTHON) $(VMODTOOL) $(top_srcdir)/src/vmod_geoip.vcc

check:
	$(VARNISHTEST) -Dvarnishd=$(VARNISHD) \
	    -Dvmod_topbuild=$(abs_top_builddir) $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_geoip.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c \
	$(builddir)/vcc_if.h \
	$(builddir}/vmod_geoip.man.rst \
	$(builddir)/vmod_geoip.rst
