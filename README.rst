==========
vmod_geoip
==========

---------------------------
Varnish GeoIP Lookup Module
---------------------------

:Author: Adam Schumacher
:Date: 2014-12-23
:Version: 0.3
:Manual section: 3

SYNOPSIS
========

import geoip;

DESCRIPTION
===========

This Varnish module exports functions to look up GeoIP country codes, country names, city names, and long/lat information.
Requires GeoIP City (and the base GeoIP) library, along with the geoipcity and geoipcityv6 binary .dat databases

Forked from: https://github.com/varnish/libvmod-geoip

Inspired by http://drcarter.info/2010/07/another-way-to-link-varnish-and-maxmind-geoip/


FUNCTIONS
=========

country_code
------------

Prototype
        ::

                country_code(STRING S)
Return value
	STRING
Description
	Returns two-letter country code string from IP address in string S
Example
        ::

                set req.http.X-Country-Code = geoip.country_code("127.0.0.1");

country_code_ip 
---------------

Prototype
        ::

                country_code_ip(IP I)
Return value
	STRING
Description
	Returns two-letter country code string from IP address
Example
        ::

                set req.http.X-Country-Code = geoip.country_code_ip(client.ip);


country_name_ip
---------------

Prototype
        ::

                country_name_ip(IP I)
Return value
	STRING
Description
	Returns country name string from IP address
Example
        ::

                set req.http.X-Country-Name = geoip.country_name_ip(client.ip);

country_name
------------

Prototype
        ::

                country_name(STRING S)
Return value
	STRING
Description
	Returns country name string
Example
        ::

                set req.http.X-Country-Name = geoip.country_name("127.0.0.1");


latitude
--------

Prototype
	::

	    latitude(STRING S)
Return value
	STRING
Description
	Returns latitude
Example
	::
	    
	    set req.http.X-Geo-Latitude = geoip.latitude("127.0.0.1");

latitude_ip
-----------

Prototype
	::
	    
	    latitude_ip(IP I)
Return value
	STRING
Description
	Returns latitude
Example
	::

	    set req.http.X-Geo-Latitude = geoip.latitude_ip(client.ip);

city
----

Prototype
	::

	    city(STRING S)
Return value
	STRING
Description
	Returns the name of the city
Example
	::

	    set req.http.X-Geo-City = geoip.city("127.0.0.1");

city_ip
----

Prototype
        ::

            city_ip(IP I)
Return value
        STRING
Description
        Returns the name of the city
Example
        ::

            set req.http.X-Geo-City = geoip.city_ip("127.0.0.1");


longitude
--------

Prototype
        ::

            longitude(STRING S)
Return value
        STRING
Description
        Returns longitude 
Example
        ::
            
            set req.http.X-Geo-Latitude = geoip.longitude("127.0.0.1");

longitude_ip
-----------

Prototype
        ::
            
            longitude_ip(IP I)
Return value
        STRING
Description
        Returns longitude
Example
        ::

            set req.http.X-Geo-Latitude = geoip.longitude_ip(client.ip);



region_name_ip 
---------

Prototype
        ::

                region_name_ip(IP I)
Return value
	STRING
Description
	Returns region name string from IP address
Example
        ::

                set req.http.X-Region-Name = geoip.ip_region_name(client.ip);

region_name 
-----------

Prototype
        ::

                region_name(STRING S)
Return value
	STRING
Description
	Returns region name string
Example
        ::

                set req.http.X-Region-Name = geoip.region_name("127.0.0.1");

IPv6 FUNCTIONS
==============

These functions work identically to their ipv4 counterparts, except they take an ipv6 address as an argument and search the v6 database (if configured and available at compile-time)

* country_code_v6
* country_code_v6_ip
* country_name_v6
* country_name_v6_ip
* city_v6
* city_v6_ip
* latitude_v6
* latitude_v6_ip
* longitude_v6
* longitude_v6_ip
* region_name_v6
* region_name_v6_ip


INSTALLATION
============

The source tree is based on autotools to configure the building, and
does also have the necessary bits in place to do functional unit tests
using the varnishtest tool.

Install the GeoIP and GeoIPCity library headers
Ensure that there is a .dat for the city data, and ipv6 data if you want to support that

To check out the current development source::

 git clone git://github.com/zeroschism/libvmod-geoip.git
 cd libvmod-geoip; ./autogen.sh

Usage::

 ./configure --with-dat=/usr/local/share/GeoIP/GeoIPCity.dat --with-ipv6-dat=/usr/local/share/GeoIP/GeoLiteCityv6.dat --with-varnishsrc=/usr/ports/www/varnish/work/varnish-3.0.5 --with-vmod-dir=/usr/local/lib/varnish/vmods

 ./configure VARNISHSRC=DIR [VMODDIR=DIR] [--with-ipv6[=<location/to/GeoIPCityv6.dat>]] [--with-dat=<location/to/GeoIPCity.dat]

`with-dat` is the path to the GeoIPCity.dat file containing the binary database to use for lookups.

`with-ipv6-dat` is the path to the GeoIPCityv6.dat file containing the binary database to use for ipv6 lookups.  This option also enables the ipv6 functions.

`with-varnishsrc` is the directory of the Varnish source tree against which to
compile your vmod. 

`with-vmod-dir` Optionally you can also set the vmod install directory (defaults to the pkg-config discovered directory from your
Varnish installation).

Make targets:

* make - builds the vmod
* make install - installs your vmod in `VMODDIR`
* make check - runs the unit tests in ``src/tests/*.vtc``

In your VCL you could then use this vmod along the following lines::
        
        import geoip;

        sub vcl_req {
                # This sets req.http.X-Country-Code to the country code
                # associated with the client IP address
                set req.http.X-Country-Code = geoip.country_code_ip(client.ip);
        }

HISTORY
=======

2011-09-26 -- Original version by Hauke Lampe
2014-12-23 -- Refinements to utilize the expanded data available in GeoIPCity and access ipv6 from GeoIPCityv6 by Adam Schumacher
2015-01-16 -- Updates to the install files and documentation by Adam Schumacher

COPYRIGHT
=========

The code is licensed to you under following MIT-style License:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.TODO
