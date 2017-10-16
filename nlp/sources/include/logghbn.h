/*
 *  Public header for cached version of gethostbyname
 *  Copyright (c) 2004-2006 Pertimm
 *  Dev : June 2004, August 2005, March 2006
 *  Version 1.2
*/
#ifndef _LOGGHBNALIVE_
#include <logsock.h>

#define DOgGhbnBanner  "ogm_ghbn V1.10, Copyright (c) 2004-2007 Pertimm, Inc."
#define DOgGhbnVersion 110


#define DOgGhbnTraceMinimal     0x1
#define DOgGhbnTraceMemory      0x2
#define DOgGhbnTraceTimeout     0x4
#define DOgGhbnTraceCache       0x8

DEFPUBLIC(int) OgGetHostByName(pr_(void *) pr_(char *) pr_(struct og_hostent *) pr(int));
DEFPUBLIC(void *) OgGetHostByNameInit(pr_(int) pr(char *));
DEFPUBLIC(int) OgGetHostByNameFlush(pr(void *));

#define _LOGGHBNALIVE_
#endif

