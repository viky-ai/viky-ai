/*
 *  All generic functions using automatons
 *  Copyright (c) 2006-2007 Pertimm par Patrick Constant
 *  Dev: March 2006, November 2007, November 2008
 *  Version 2.8
*/
#ifndef _LOGAUTAALIVE_
#include <logaut.h>

#define DOgAutaBanner	"ogm_auta V1.13, Copyright (c) 2006-2008 Pertimm, Inc."
#define DOgAutaVersion	113


/** Trace levels **/
#define DOgAutaTraceMinimal          0x1
#define DOgAutaTraceMemory           0x2

/** Used for quarantine functions **/
#define DOgMaxQuarantineLevel   2

struct og_charset_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  }; 

struct og_entity_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  }; 


DEFPUBLIC(void *) OgDeleteWordInit(pr(void));
DEFPUBLIC(int) OgDeleteWord(pr_(void *) pr_(int) pr_(int) pr_(unsigned char *) pr(int));
DEFPUBLIC(int) OgDeleteWordFlush(pr(void *));

DEFPUBLIC(void *) OgQuarantineInit(pr(char *));
DEFPUBLIC(int) OgQuarantined(pr_(void *) pr(char *));
DEFPUBLIC(int) OgQuarantineSet(pr_(void *) pr_(char *) pr(int));
DEFPUBLIC(int) OgQuarantineFlush(pr(void *));

DEFPUBLIC(void *) OgCharsetInit(pr(struct og_charset_param *));
DEFPUBLIC(int) OgCharsetGetCode(pr_(void *) pr_(char *) pr(int *));
DEFPUBLIC(int) OgCharsetGetString(pr_(void *) pr_(int) pr(char *));
DEFPUBLIC(int) OgCharsetFlush(pr(void *));

DEFPUBLIC(void *) OgEntityInit(pr(struct og_entity_param *));
DEFPUBLIC(int) OgEntityGetCode(pr_(void *) pr_(char *) pr(int *));
DEFPUBLIC(int) OgEntityGetString(pr_(void *) pr_(int) pr(char *));
DEFPUBLIC(int) OgEntityFlush(pr(void *));


#define _LOGAUTAALIVE_
#endif

