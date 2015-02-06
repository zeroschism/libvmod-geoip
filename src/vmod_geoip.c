/**
 * libvmod-geoip - varnish interface to MaxMind's GeoIP library
 * GeoIP API: http://www.maxmind.com/app/c
 *
 * See file README.rst for usage instructions
 * 
 * This code is licensed under a MIT-style License, see file LICENSE
*/

#include "config.h"

#include <stdlib.h>
//#include <stdio.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

#include "vrt.h"
#include "vrt_obj.h"
#include "bin/varnishd/cache.h"

#include "vcc_if.h"

// The default string in case the GeoIP lookup fails
#define GI_UNKNOWN_STRING ""
#define HEADER_MAXLEN 255

typedef struct vmod_geoip_db_type { 
    GeoIP    *ipv4;
    GeoIP    *ipv6;
} GeoipDB;

const char * Geoip_header_name[8] =
{
    "\013X-Geo-City:",
    "\016X-Geo-Country:",
    "\015X-Geo-Region:",
    "\017X-Geo-Latitude:",
    "\020X-Geo-Longitude:",
    "\020X-Geo-Latitude1:",
    "\021X-Geo-Longitude1:",
    "\011X-Geo-IP:"
};

static void
cleanup_db(GeoipDB *db)
{
    if (db->ipv4)
	GeoIP_delete(db->ipv4);

    if (db->ipv6)
	GeoIP_delete(db->ipv6);

    free(db);
}

int
init_priv(struct vmod_priv *pp)
{
	pp->priv = malloc(sizeof(GeoipDB));
	if (!pp->priv)
	    return 1;

	GeoipDB *db = (GeoipDB *)pp->priv;
	db->ipv4 = GeoIP_open_type(GEOIP_CITY_EDITION_REV1,GEOIP_MMAP_CACHE);
	//db->ipv4 = GeoIP_open(GeoIPDBFileName[GEOIP_CITY_EDITION_REV1],GEOIP_MMAP_CACHE);
	if (!db->ipv4)
	    return 1;
	GeoIP_set_charset(db->ipv4, GEOIP_CHARSET_UTF8);
	#ifdef HAVE_GEOIP_V6
	    db->ipv6 = GeoIP_open_type(GEOIP_CITY_EDITION_REV1_V6,GEOIP_MMAP_CACHE);
	    //db->ipv6 = GeoIP_open(GeoIPDBFileName[GEOIP_CITY_EDITION_REV1_V6],GEOIP_MMAP_CACHE);
	    if (!db->ipv6)
		return 1;
	    GeoIP_set_charset(db->ipv6, GEOIP_CHARSET_UTF8);
	#endif
	pp->free = (vmod_priv_free_f *)cleanup_db;
	
	return 0;
}

GeoIPRecord*
get_geoip_record(struct sess *sp, struct vmod_priv *pp, const char * ip) {
    if (!pp->priv) {
	if (init_priv(pp)) {
	    WSP(sp,SLT_VCL_Log, "%s", "Unable to load geoip dat file");
	    return NULL;
	}
    }
    GeoipDB *db = (GeoipDB *)pp->priv;    
    return GeoIP_record_by_addr(db->ipv4, ip);
}

void
vmod_set_headers(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	const char *resolved = NULL;

	if (!record) {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[0], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[2], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[1], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[3], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[5], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[4], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[6], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[7], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    WSP(sp,SLT_VCL_Log, "%s %s", "No record for ip", ip);
	    return;
	}

	if (record->city) { 
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[0], record->city, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, "city ", record->city,vrt_magic_string_end);
	}
	else {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[0], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, "city ",vrt_magic_string_end);
	}
	if (record->region) {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[2], record->region, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " region ", record->region,vrt_magic_string_end);
	}
	else {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[2], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " region ",vrt_magic_string_end);
	}
	if (record->country_code) {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[1], record->country_code, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " country ", record->country_code, vrt_magic_string_end);
	}
	else {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[1], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " country ",vrt_magic_string_end);
	}
	if (record->latitude) {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[3], VRT_double_string(sp,record->latitude), vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[5], VRT_int_string(sp,(int)record->latitude), vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " lat ", VRT_double_string(sp,record->latitude), vrt_magic_string_end);
	}
	else {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[3], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[5], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " lat ",vrt_magic_string_end);
	}
	if (record->longitude) {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[4], VRT_double_string(sp,record->longitude), vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[6], VRT_int_string(sp,(int)record->longitude), vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " lon ", VRT_double_string(sp,record->longitude), vrt_magic_string_end);
	}
	else {
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[4], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[6], GI_UNKNOWN_STRING, vrt_magic_string_end);
	    resolved = VRT_WrkString(sp, resolved, " lon ", vrt_magic_string_end);
	}

	//resolved = VRT_WrkString(sp, resolved, " ip ", ip, vrt_magic_string_end);
	VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[7], VRT_WrkString(sp, resolved, " ip ", ip, vrt_magic_string_end), vrt_magic_string_end);

	if (record)
	    GeoIPRecord_delete(record);
    }
    else {
	WSP(sp,SLT_VCL_Log, "%s", "Headers not set, no IP to lookup");
	return;
    }
}

void
vmod_set_headers_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    vmod_set_headers(sp,pp,VRT_IP_string(sp,ip));
}

const char *
vmod_city(struct sess *sp, struct vmod_priv *pp, const char * ip) {
    GeoIPRecord *record;
    const char * city = GI_UNKNOWN_STRING;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record->city) {
	    city = WS_Dup(sp->wrk->ws, record->city);
	}
	if (record)
	    GeoIPRecord_delete(record);
	
    }

    //no ip passed, or no record retrieved  
    return city;
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
	if (record->latitude) {
	    v = snprintf(lat, HEADER_MAXLEN, "%f", record->latitude);
	    v++;
	}
	else {
	    v = snprintf(lat, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	    v++;
	}
	if (record)
	    GeoIPRecord_delete(record);
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
	if (record->longitude) {
	    v = snprintf(longitude, HEADER_MAXLEN, "%f", record->longitude);
	    v++;
	}
	else {
	    snprintf(longitude, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	    v++;
	}
	if (record)
	    GeoIPRecord_delete(record);
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
    const char *country_code = GI_UNKNOWN_STRING;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record->country_code) {
	    country_code = WS_Dup(sp->wrk->ws,record->country_code);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return country_code;
}

const char *
vmod_country_code_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_code(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_country_name(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    const char * country_name = GI_UNKNOWN_STRING;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record->country_name) {
	    country_name = WS_Dup(sp->wrk->ws, record->country_name);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return country_name;
}

const char *
vmod_country_name_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_name(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_region_name(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    const char *region = GI_UNKNOWN_STRING;

    if (ip) {
	record = get_geoip_record(sp, pp, ip);
	if (record->region) {
	    region = WS_Dup(sp->wrk->ws, record->region);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return region;
}

const char *
vmod_region_name_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_region_name(sp, pp, VRT_IP_string(sp, ip)));
}

#ifdef HAVE_GEOIP_V6
 
GeoIPRecord*
get_geoip_record_v6(struct sess *sp,struct vmod_priv *pp, const char *ip) {
    if (!pp->priv) {
	if (init_priv(pp)) {
	    WSP(sp,SLT_VCL_Log, "%s", "Unable to load geoipv6 dat file");
	    return NULL;
	}
    }
    GeoipDB *db = (GeoipDB *)pp->priv;
    return GeoIP_record_by_addr_v6(db->ipv6, ip);
}

void
vmod_set_headers_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;

    if (ip) {
        record = get_geoip_record_v6(sp, pp, ip);
        const char *resolved = NULL;

	if (!record) {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[0], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[2], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[1], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[3], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[5], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[4], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[6], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[7], GI_UNKNOWN_STRING, vrt_magic_string_end);
            WSP(sp,SLT_VCL_Log, "%s %s", "No record for ip", ip);
            return;
        }

        if (record->city) {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[0], record->city, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, "city ", record->city,vrt_magic_string_end);
        }
        else {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[0], GI_UNKNOWN_STRING, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, "city ",vrt_magic_string_end);
        }
        if (record->region) {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[2], record->region, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " region ", record->region,vrt_magic_string_end);
        }
        else {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[2], GI_UNKNOWN_STRING, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " region ",vrt_magic_string_end);
        }
        if (record->country_code) {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[1], record->country_code, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " country ", record->country_code, vrt_magic_string_end);
        }
        else {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[1], GI_UNKNOWN_STRING, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " country ",vrt_magic_string_end);
        }
        if (record->latitude) {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[3], VRT_double_string(sp,record->latitude), vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[5], VRT_int_string(sp,(int)record->latitude), vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " lat ", VRT_double_string(sp,record->latitude), vrt_magic_string_end);
        }
        else {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[3], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[5], GI_UNKNOWN_STRING, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " lat ",vrt_magic_string_end);
        }
        if (record->longitude) {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[4], VRT_double_string(sp,record->longitude), vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[6], VRT_int_string(sp,(int)record->longitude), vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " lon ", VRT_double_string(sp,record->longitude), vrt_magic_string_end);
        }
        else {
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[4], GI_UNKNOWN_STRING, vrt_magic_string_end);
            VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[6], GI_UNKNOWN_STRING, vrt_magic_string_end);
            resolved = VRT_WrkString(sp, resolved, " lon ", vrt_magic_string_end);
        }

        VRT_SetHdr(sp, HDR_REQ, Geoip_header_name[7], VRT_WrkString(sp, resolved, " ip ", ip, vrt_magic_string_end), vrt_magic_string_end);

        if (record)
            GeoIPRecord_delete(record);
    }
    else {
        WSP(sp,SLT_VCL_Log, "%s", "Headers not set, no IP to lookup");
	return;
    }
}

void
vmod_set_headers_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    vmod_set_headers_v6(sp,pp,VRT_IP_string(sp,ip));
}

const char *
vmod_city_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    const char *city = GI_UNKNOWN_STRING;

    if (ip) {
	record = get_geoip_record_v6(sp, pp, ip);
	if (record->city) {
	    city = WS_Dup(sp->wrk->ws,record->city);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return city;
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
	if (record->latitude) {
	    v = snprintf(lat, HEADER_MAXLEN, "%f", record->latitude);
	    v++;
	}
	else {
	    v = snprintf(lat, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
	    v++;
	}
	if (record)
	    GeoIPRecord_delete(record);
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
        if (record->longitude) {
            v = snprintf(longitude, HEADER_MAXLEN, "%f", record->longitude);
            v++;
        }
        else {
            v = snprintf(longitude, HEADER_MAXLEN, "%s", GI_UNKNOWN_STRING);
            v++;
        }
	if (record)
	    GeoIPRecord_delete(record);
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
    const char *country_code = GI_UNKNOWN_STRING;

    if (ip) {
	record = get_geoip_record_v6(sp, pp, ip);
	if (record->country_code) {
	    country_code = WS_Dup(sp->wrk->ws,record->country_code);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return country_code;
}

const char *
vmod_country_code_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_code_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_country_name_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    const char *country_name = GI_UNKNOWN_STRING;

    if (ip) {
        record = get_geoip_record_v6(sp, pp, ip);
        if (record->country_name) {
	    const char *country_name = WS_Dup(sp->wrk->ws,record->country_name);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return country_name;
}

const char *
vmod_country_name_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_country_name_v6(sp, pp, VRT_IP_string(sp, ip)));
}

const char *
vmod_region_name_v6(struct sess *sp, struct vmod_priv *pp, const char *ip) {
    GeoIPRecord *record;
    const char *region = GI_UNKNOWN_STRING;

    if (ip) {
        record = get_geoip_record_v6(sp, pp, ip);
	if (record->region) {
	    region = WS_Dup(sp->wrk->ws,record->region);
	}
	if (record)
	    GeoIPRecord_delete(record);
    }

    return region;
}

const char *
vmod_region_name_v6_ip(struct sess *sp, struct vmod_priv *pp, struct sockaddr_storage *ip) {
    return (vmod_region_name_v6(sp, pp, VRT_IP_string(sp,ip)));
}


#endif
