/*
 *  This is include for small virtual disk handling
 *  Copyright (c) 1997-2002 Pertimm by Patrick Constant
 *  Dev : October 1997, January 1998, December 2002, February 2006, August 2007
 *  From ogm_gen.dll
 *  Version 1.4
*/

#ifndef _LOGMLOGALIVE_

#include <lpcosys.h>

/* Max size of the .log file before it is change into a .lof file
 * changed from 500k (1997) to 2m (2006) to 4m (dec 2006)
 * Can be dynamically changed by function OgMlogMaxFileSizeSet */
#define DOgMlogMaxFileSize      4000000

/** Do not create a string bigger than that size using OgMessageLog **/
#define DOgMlogMaxMessageSize   0x3000 /* 12k */

/*
 *    Macros for OgMessageLog function.
 *    DOgMlogInLog  : writes in the file 'LOG/title.log' the message 'file_ptr'
 *    DOgMlogMBox   : the message 'file_ptr' is sent to a message box
 *    DOgMlogInErr  : writes in the file 'LOG/title.err' the message 'file_ptr'
 *    DOgMlogSysErr : gets the system error and writes it in the above files
 *      DOgMlogDate   : writes the date in the above files on a separate line
 *      DOgMlogDateIn : writes the date in the above files on the same line
 *      DOgMlogNoCr   : doesn't write a CR at the end of the line
*/
#define DOgMlogInLog  0x1
#define DOgMlogMBox    0x2
#define DOgMlogInErr  0x4
#define DOgMlogSysErr  0x8
#define DOgMlogDate    0x10
#define DOgMlogDateIn  0x20
#define DOgMlogNoCr    0x40
#define DOgMlogUnicode  0x80

#define DOgSrcPos    __FILE__,__LINE__
#define DOgLogErr(w)  OgLogErr(DOgMlogInLog+DOgMlogInErr,w,DOgSrcPos)


struct og_loginfo {
  char *where;
  int config;
  int trace;
};


DEFPUBLIC(int) GetExePath(pr(char *));
DEFPUBLIC(int) OgGetExecutablePath(pr_(char *) pr(char *));
DEFPUBLIC(int) OgGetTempPath(pr_(int) pr_(char *) pr(void *));
DEFPUBLIC(int) OgAbsolutizePath(pr_(char *) pr(char *));
DEFPUBLIC(int) OgIsAbsolutePath(pr(char *));
DEFPUBLIC(int) OgNormalizePath(pr_(char *) pr(char *));
DEFPUBLIC(int) MessageInfoLog(pr_(int) pr_(char *) pr_(char *)
                pr_(const char *) pr(...));
DEFPUBLIC(int) MessageInfoDLog(pr_(int) pr_(char *) pr_(char *)
                pr_(const char *) pr(...));
DEFPUBLIC(int) MessageErrorLog(pr_(int) pr_(char *) pr_(char *)
                 pr_(const char *) pr(...));
DEFPUBLIC(int) OgGetCmdParameter(pr_(char *) pr_(char *) pr(int *));
DEFPUBLIC(int) OgSetWorkingDirLog(const char *);
DEFPUBLIC(const char *) OgGetWorkingDirLog();
DEFPUBLIC(int) OgMlogMaxFileSizeSet(size_t max_file_size);
DEFPUBLIC(int) OgMessageLog(pr_(int) pr_(char *) pr_(char *)
                 pr_(const char *) pr(...));
DEFPUBLIC(int) OgLogErr(pr_(int) pr_(char *) pr_(char *) pr(int));
DEFPUBLIC(int) OgMessageShortUnicodeLog(pr_(char *) pr_(int) pr(short *));
DEFPUBLIC(int) OgMessageUnicodeLog(pr_(char *) pr_(int) pr(unsigned char *));


#define _LOGMLOGALIVE_

#endif
