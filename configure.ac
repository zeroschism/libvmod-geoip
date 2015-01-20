AC_PREREQ(2.59)
AC_INIT([libvmod-geoip], [0.3], [adam.schumacher@flightaware.com], [libvmod-geoip])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR(src/vmod_geoip.vcc)
AM_CONFIG_HEADER(config.h)

AC_CANONICAL_SYSTEM
AC_LANG(C)

AM_INIT_AUTOMAKE([foreign])

AC_GNU_SOURCE
AC_PROG_CC
AC_PROG_CC_STDC
if test "x$ac_cv_prog_cc_c99" = xno; then
	AC_MSG_ERROR([Could not find a C99 compatible compiler])
fi
AC_PROG_CPP

AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET

# Check for pkg-config
PKG_PROG_PKG_CONFIG
#PKG_CHECK_MODULES([libGeoIP], [geoip])

# Checks for header files.
CFLAGS="$CFLAGS -I/usr/local/include"
LDFLAGS="$LDFLAGS -L/usr/local/lib"

AC_HEADER_STDC
AC_CHECK_HEADER([GeoIPCity.h], [ ], [AC_MSG_FAILURE([GeoIPCity.h not found. This is required.])])

# Check if ipv6 enabled
AC_ARG_WITH([ipv6], AS_HELP_STRING([--with-ipv6], [Enable ipv6 lookups (requires ipv6 dat file)]))

AS_IF([test "x$with_ipv6" != "xno"],
	[AC_DEFINE([HAVE_GEOIP_V6], [1], [Enable ipv6 functionality])]
	[AC_SUBST([VCC_FILE], ['vmod_geoip_v6.vcc'])],
    [AC_SUBST([VCC_FILE], ['vmod_geoip.vcc'])])

# Check for python
AC_CHECK_PROGS(PYTHON, [python3 python3.1 python3.2 python2.7 python2.6 python2.5 python2 python], "no")
if test "x$PYTHON" = "xno"; then
	AC_MSG_ERROR([Python is needed to build this vmod, please install python.])
fi

# Varnish source tree
AC_ARG_WITH([varnishsrc], AS_HELP_STRING([--with-varnishsrc=/path/to/varnish], [path to Varnish source tree (mandatory)]))

AS_IF([test "x$with_varnishsrc" != "xno"],
    [AC_CHECK_FILE(["$with_varnishsrc"/include/varnishapi.h], [AC_SUBST([VARNISHSRC], [$with_varnishsrc])], [AC_MSG_FAILURE(["$with_varnishsrc" is not a Varnish source directory])])],
    [AC_MSG_ERROR([You must specifiy a path to the Varnish source directory])])

# Check that varnishtest is built in the varnish source directory
AC_CHECK_FILE(["$with_varnishsrc"/bin/varnishtest/varnishtest],
	[],
	[AC_MSG_FAILURE([Cannot find "$with_varnishsrc"/bin/varnishtest/varnishtest. Please build your varnish source directory])]
)

# vmod installation dir
AC_ARG_WITH([vmod-dir], AS_HELP_STRING([--with-vmod-dir\[=/path/to/installdir\]], [vmod installation directory @<:@LIBDIR/varnish/vmods@:>@]))

AS_IF([test "x$with_vmod_dir" = "x"],
    [AC_SUBST([VMODDIR], [`pkg-config --variable=vmoddir varnishapi`])],
    [AC_SUBST([VMODDIR], [$with_vmod_dir])])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
