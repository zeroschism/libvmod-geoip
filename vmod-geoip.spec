Summary: geoip VMOD for Varnish
Name: vmod-geoip
Version: 0.5
Release: 1%{?dist}
License: MIT
Group: System Environment/Daemons
Source0: libvmod-geoip.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Requires: varnish > 3.0, GeoIP
BuildRequires: make
BuildRequires: python-docutils
BuildRequires: GeoIP-devel

%description
This Varnish module exports functions to look up GeoIP country codes, country names, city names, and long/lat information. Requires GeoIP City (and the base GeoIP) library, along with the geoipcity and geoipcityv6 binary .dat databases

%prep
%setup -n libvmod-geoip

%build
%configure
make
make check

%install
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root,-)
%{_libdir}/varnis*/vmods/
%{_mandir}/man?/*

%changelog
* Tue Nov 14 2012 Lasse Karstensen <lasse@varnish-software.com> - 0.1-0.20121114
- Initial version.
* Tue Dec 23 2014 Adam Schumacher <adam.schumacher@flightaware.com> - 0.20121114-0.3
- Changed to use GeoIPCity.h functions and data
- Updated to include ipv6 database as well
* Fri Jan 16 2015 Adam Schumacher <adam.schumacher@flightaware.com> - 0.3-0.4
- Updates to the install files and documentation
* Tue Jan 20 2015 Adam Schumacher <adam.schumacher@flightaware.com> - 0.4-0.5
- Refine how the dat files are loaded and simplify install options
