/*
 *  Public header for wrapper function for exceptions
 *  Copyright (c) 2004 Pertimm by Patrick Constant
 *  Dev : June 2004
 *  Version 1.0
*/
#ifndef _LOGXPTALIVE_
#include <loggen.h>

#define DOgXptBanner  "ogm_xpt V1.03, Copyright (c) 2004 Pertimm, Inc."
#define DOgXptVersion 103


DEFPUBLIC(int) OgExceptionSafe(pr_(int (*)(pr(void *))) pr_(int (*)(pr_(void *) pr(int))) pr(void *));
DEFPUBLIC(int) OgRunWithTimeout(pr_(double) pr_(int (*) (pr(void *))) pr_(void *) pr(char *));


#define _LOGXPTALIVE_
#endif

