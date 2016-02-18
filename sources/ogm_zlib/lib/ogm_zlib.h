/*
 *  Internal header for handling mails under Zlib format (zip, gz, tgz)
 *  Copyright (c) 2005,2006 Pertimm, Inc. by Patrick Constant
 *  Dev : April 2005, March 2006
 *  Version 1.1
*/

#include <logzlib.h>


#define DOgZfileNumber  0x1000   /* 4096 structures */
#define DOgBaSize       0x100000 /* 1Mb */



struct og_ctrl_zlib {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;

  struct og_zlib_scan_param cscan_param;
  struct og_zlib_scan_param *scan_param;

  char zlib_name[DPcPathSize];
  char zlib_part[DPcPathSize];

  int ZfileNumber,ZfileUsed;
  struct og_zlib_file *Zfile;

  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize,BaUsed;
  int BaStartContent;
  };

/** zlibba.c **/
int OgZlibAppendBa(pr_(struct og_ctrl_zlib *) pr_(int) pr(unsigned char *));
int OgZlibTestReallocBa(pr_(struct og_ctrl_zlib *) pr(int));
int OgZlibReallocBa(pr_(struct og_ctrl_zlib *) pr(int));

/** zliblist.c **/
int OgZfileListing(pr(struct og_ctrl_zlib *));
int GetZfile(pr_(struct og_ctrl_zlib *) pr(struct og_zlib_file **));

