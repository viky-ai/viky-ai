/*
 *	This is include for message files handling
 *	Copyright (c) 1997-2001	Ogmios
 *	Dev : October 1997, January 1998, June 2001
 *	From ogm_gen.dll
 *	Version 1.2
*/

#ifndef _LOGMESSALIVE_

#include <lpcosys.h>


DEFPUBLIC(int) OgSendMessage(pr_(char *) pr_(char *) pr_(char *) pr(int));
DEFPUBLIC(int) OgDirSendMessage(pr_(char *) pr_(char *) pr_(char *) pr_(char *) pr(int));
DEFPUBLIC(int) OgDeleteMessages(pr_(char *) pr(char *));
DEFPUBLIC(int) OgDirDeleteMessages(pr_(char *) pr_(char *) pr(char *));
DEFPUBLIC(int) OgLookMessage(pr_(char *) pr(int *));
DEFPUBLIC(int) OgDirLookMessage(pr_(char *) pr_(char *) pr(int *));
DEFPUBLIC(int) OgDirLookMessageEx(pr_(char *) pr_(char *) pr_(int *) pr(int));
DEFPUBLIC(int) OgLookSystemMessage(pr_(char *) pr(int *));
DEFPUBLIC(int) OgLookSystemMessageEx(pr_(char *) pr_(int *) pr(int));
DEFPUBLIC(int) OgLookMessageFile(pr_(char *) pr(char *));
DEFPUBLIC(int) OgDirLookMessageFile(pr_(char *) pr_(char *) pr(char *));
DEFPUBLIC(int) OgRemoveMessageFile(pr_(char *) pr(char *));
DEFPUBLIC(int) OgGetVariableFile(pr_(char *) pr_(char *) pr(char *));


#define _LOGMESSALIVE_

#endif
