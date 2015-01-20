/**
 * libvmod-geoip - varnish interface to MaxMind's GeoIP library
 * GeoIP API: http://www.maxmind.com/app/c
 *
 * See file README.rst for usage instructions
 * 
 * This code is licensed under a MIT-style License, see file LICENSE
*/

#include <stdlib.h>
#include <stdio.h>
#include <GeoIPCity.h>

#include "vrt.h"
#include "vrt_obj.h"
#include "bin/varnishd/cache.h"

#include "vcc_if.h"

// The default string in case the GeoIP lookup fails
#define GI_UNKNOWN_STRING ""
#define HEADER_MAXLEN 255

int
init_priv(struct vmod_priv *pp)
{
	// The README says:
	// If GEOIP_MMAP_CACHE doesn't work on a 64bit machine, try adding
	// the flag "MAP_32BIT" to the mmap call. MMAP is not avail for WIN32.
	//pp->priv = GeoIP_new(GEOIP_MMAP_CACHE);
	//pp->priv = GeoIP_open_type(GEOIP_CITY_EDITION_REV1,GEOIP_MMAP_CACHE);
	//pp->priv = GeoIP_open(GEOIP_DAT_FILE,GEOIP_MMAP_CACHE);

	pp->priv = GeoIP_open_type(GEOIP_CITY_EDITION_REV1,GEOIP_MMAP_CACHE);
	if (pp->priv) {
	    pp->free = (vmod_priv_free_f *)GeoIP_delete;
	    GeoIP_set_charset((GeoIP *)pp->priv, GEOIP_CHARSET_UTF8);
	    return 1;
	}
	else {
	    return 0;
	}
}

GeoIPRecord*
get_geoip_record(struct sess *sp, struct vmod_priv *pp, const char * ip) {
    if (!pp->priv) {
	if (! init_priv(pp)) {
	    WSP(sp,SLT_Debug, "%s", "Unable to load geoip dat file");
	    return NULL;
	}
    }
    
    return GeoIP_record_by_addr(pp->priv, ip);
}

const char *
vmod_city(struct sess *sp, struct vmod_priv *pp, const char * ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record)
	    return record->city;
    }

    //no ip passed, or no record retrieved   
    return GI_UNKNOWN_STRING;
}

const char *
vmod_city_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage * ip) {
    return (vmod_city(sp, pp, VRT_IP_string(sp, ip)));
}


const char *  
vmod_latitude(struct sess *sp, struct vmod_priv *pp, const char * ip) {
    GeoIPRecord *record;
    char *lat;
    unsigned v, u;

    u = WS_Reserve(sp->wrk->ws,HEADER_MAXLEN);
    lat = sp->wrk->ws->f;
	    
    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record) {
	    v = snprintf(lat, HEADER_MAXLEN, "%f", record->latitude);
	    v++;
	}
	else {
	    v = snprintf(lat, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	    v++;
	}
    }
    else {
	v = snprintf(lat, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	v++;
    }

    if (v > u) {
	WS_Release(sp->wrk->ws, 0);
	return NULL;
    }

    WS_Release(sp->wrk->ws, v);

    return (lat);    
}

const char *
vmod_latitude_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_latitude(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_longitude(struct sess *sp, struct vmod_priv *pp, const char * ip) {
    GeoIPRecord *record;
    char *longitude;
    unsigned v, u;

    u = WS_Reserve(sp->wrk->ws,HEADER_MAXLEN);
    longitude = sp->wrk->ws->f;


    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record) {
	    v = snprintf(longitude, HEADER_MAXLEN, "%f", record->longitude);
	    v++;
	}
	else {
	    snprintf(longitude, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	    v++;
	}
    }
    else {
	v = snprintf(longitude, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	v++;
    }

    if (v > u) {
	WS_Release(sp->wrk->ws,0);
	return NULL;
    }

    WS_Release(sp->wrk->ws,v);
    return (longitude);
}

const char *
vmod_longitude_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_longitude(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_country_code(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record)
	    return record->country_code;
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_country_code_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_code(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_country_name(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record)
	    return record->country_name;
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_country_name_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_name(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_region_name(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record) 
	    return record->region;
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_region_name_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_region_name(sp, pp, VRT_IP_string(sp, ip)));
}

//#ifdef WITH_GEOIP_V6

int
init_priv_v6(struct vmod_priv *pp)
{  

    pp->priv = GeoIP_open_type(GEOIP_CITY_EDITION_REV1_V6,GEOIP_MMAP_CACHE);
    if (pp->priv) {
        pp->free = (vmod_priv_free_f *)GeoIP_delete;
        GeoIP_set_charset((GeoIP *)pp->priv, GEOIP_CHARSET_UTF8);
        return 1;
    }
    else {
        return 0;
    }
}

GeoIPRecord*
get_geoip_record_v6(struct sess *sp,struct vmod_priv *pp, const char *ip) {
    if (!pp->priv) {
	if (!init_priv_v6(pp)) {
	    WSP(sp,SLT_Debug, "%s", "Unable to load geoipv6 dat file");
	    return NULL;
	}
    }

    return GeoIP_record_by_addr_v6(pp->priv, ip);
}

const char *
vmod_city_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record_v6(sp, pp, ip);
	if (record)
	    return record->city;
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_city_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_city_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_latitude_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    char *lat;
    unsigned v, u;

    u = WS_Reserve(sp->wrk->ws,HEADER_MAXLEN);
    lat = sp->wrk->ws->f;

    if (ip) {
	record = get_geoip_record_v6(sp, pp, ip);
	if (record) {
	    v = snprintf(lat, HEADER_MAXLEN, "%f", record->latitude);
	    v++;
	}
	else {
	    v = snprintf(lat, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	    v++;
	}
    }
    else {
	v = snprintf(lat, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	v++;
    }

    if (v > u) {
	WS_Release(sp->wrk->ws, 0);
	return NULL;
    }

    WS_Release(sp->wrk->ws, v);

    return (lat);
}

const char *
vmod_latitude_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_latitude_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_longitude_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    char *longitude;
    unsigned v, u;

    u = WS_Reserve(sp->wrk->ws,HEADER_MAXLEN);
    longitude = sp->wrk->ws->f;

    if (ip) {
        record = get_geoip_record_v6(sp, pp, ip);
        if (record) {
            v = snprintf(longitude, HEADER_MAXLEN, "%f", record->longitude);
            v++;
        }
        else {
            v = snprintf(longitude, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
            v++;
        }
    }
    else {
        v = snprintf(longitude, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
        v++;
    }
    
    if (v > u) {
        WS_Release(sp->wrk->ws, 0);
        return NULL;
    }
    
    WS_Release(sp->wrk->ws, v);

    return (longitude);
}

const char *
vmod_longitude_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_longitude_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_country_code_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record_v6(sp, pp, ip);
	if (record) {
	    return record->country_code;
	}
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_country_code_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_code_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_country_name_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
        record = get_geoip_record_v6(sp, pp, ip);
        if (record)
            return record->country_name;
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_country_name_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_name_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_region_name_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record_v6(sp, pp, ip);
	if (record)
	    return record->region;
    }

    return GI_UNKNOWN_STRING;
}

const char *
vmod_region_name_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_region_name(sp, pp, VRT_IP_string(sp,ip)));
}


//#endif
