/*
 * Definitions of country codes according to ISO 3166
 *  Copyright (c) 2014 Pertimm by Patrick Constant
 *  Dev : September 2014
 * Version 1.0
*/

#ifndef _LOGIS639_3166ALIVE_

#include <lpcosys.h>


DEFPUBLIC(unsigned char *) OgIso639_3166ToCode(int lang_country, unsigned char *slang_country);
DEFPUBLIC(int) OgCodeToIso639_3166(void *herr,unsigned char *slang_country);
DEFPUBLIC(int) OgIso639_3166ToIso639_3166(int lang, int country);
DEFPUBLIC(int) OgIso639_3166ToLang(int lang_country);
DEFPUBLIC(int) OgIso639_3166ToCountry(int lang_country);


#define _LOGIS639_3166ALIVE_

#endif


