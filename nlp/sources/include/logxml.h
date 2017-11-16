/*
 *  Header for library ogm_xml.dll
 *  Copyright (c) 2004-2006 Pertimm, Inc. by Patrick Constant
 *  Dev: March,October,November,December 2004
 *  Dev: May,July 2005, May,July,September 2006
 *  Dev: May 2007, April 2008
 *  Version 1.7
*/
#ifndef _LOGXMLALIVE_
#include <lpcgentype.h>
#include <loggen.h>
#include <logthr.h>


#define DOgXmlBanner  "ogm_xml V1.25, Copyright (c) 2004-2008 Pertimm, Inc."
#define DOgXmlVersion 125


/** Trace levels **/
#define DOgXmlTraceMinimal     0x1
#define DOgXmlTraceMemory      0x2
#define DOgXmlTraceReplace     0x4

/** Used with OgParseXml **/
#define DOgParseXmlTag      1
#define DOgParseXmlContent  2
#define DOgParseXmlComment  3
#define DOgParseXmlCdata    4


/* Encoding's value  is DOgCodePageXxx from loglang.h
 * We also store the encoding string value for cases
 * When we do know the encoding value. */
#define DOgMaxEncoding  256

struct og_xml_information {
  int unicode_same_order;
  char sencoding[DOgMaxEncoding];
  int encoding;
  };

struct og_xml_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  };

DEFPUBLIC(int) OgParseXml(pr_(void *) pr_(int) pr_(int) pr_(unsigned char *)
                         pr_(int (*)(pr_(void *) pr_(int) pr_(int) pr(unsigned char *)))
                         pr(void *));
DEFPUBLIC(int) OgParseXmlUni(pr_(void *) pr_(int) pr_(int) pr_(unsigned char *)
                         pr_(int (*)(pr_(void *) pr_(int) pr_(int) pr(unsigned char *)))
                         pr(void *));

DEFPUBLIC(int) OgXmlInformation(pr_(void *) pr_(int) pr_(unsigned char *) pr(struct og_xml_information *));
DEFPUBLIC(int) OgIsXml(pr_(void *) pr_(int) pr_(int) pr_(int) pr(unsigned char *));

DEFPUBLIC(void *) OgXmlInit(pr(struct og_xml_param *));
DEFPUBLIC(int) OgXmlReplaceAdd(pr_(void *) pr_(unsigned char *) pr(unsigned char *));
DEFPUBLIC(int) OgXmlReplace(pr_(void *) pr(char *));
DEFPUBLIC(int) OgXmlFlush(pr(void *));

DEFPUBLIC(int) OgXmlAttributeEncode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout,
    int codepage, int *truncated);
DEFPUBLIC(int) OgXmlEncode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout, int codepage, int *truncated);
DEFPUBLIC(int) OgXmlDecode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout, int codepage, int *position, int *truncated);

DEFPUBLIC(int) OgXmlTopTag(pr_(void *) pr_(int) pr_(unsigned char *) pr_(int) pr(unsigned char *));

DEFPUBLIC(int) OgXmlSqueeze(int ibin, unsigned char *bin, int *ibout, unsigned char *bout);

DEFPUBLIC(int) OgXmlCheckXsd(void *hmsg, og_string xsd, og_string xml);
DEFPUBLIC(og_status) OgXmlXsdValidateFile(void *hmsg, void *herr, char *working_directory, char *xml_file,
    char *xsd_path);

#define _LOGXMLALIVE_
#endif

