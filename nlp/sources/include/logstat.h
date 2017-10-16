/*
 *	This is include for the OgStat function
 *	Copyright (c) 2000-2005 Pertimm
 *	Dev : September 2000, October 2005
 *	Version 1.2
*/

#ifndef _LOGSTATALIVE_

/*
 *  Each field is calculed if the corresponding mask is set.
*/
#define DOgStatMask_is_dir      0x1
#define DOgStatMask_size_low    0x2
#define DOgStatMask_size_high   0x4
#define DOgStatMask_size        0x6
#define DOgStatMask_mtime       0x8
#define DOgStatMask_ctime       0x10
#define DOgStatMask_atime       0x20


/*
 *  This structure is close to the ANSI 'stat' structure
 *  and is used with the OgStat and OgStatForDir functions.
 *  We do not use directly the stat function for compatibility
 *  and also because the stat function is sometimes much slower
 *  than using information coming from a directory listing function
 *  (FindFirstFile and FindNextFile gives the same information). 
 *  An example was with an access through local network to a Solaris
 *  file system of 20 Gb: directory listing: 1 hour with stat and 
 *  10 seconds without (using directory listing information).
*/

struct og_stat {
  time_t atime;       /* last access time */
  time_t mtime;       /* last modification time */
  time_t ctime;       /* creation time */
  unsigned size_high;   /* high byte order size */
  unsigned size_low;    /* lower byte order size */
  unsigned is_dir:1;
  unsigned libre:15;
  };


DEFPUBLIC(int) OgFstat(pr_(void *stream) pr(struct og_stat *));
DEFPUBLIC(int) OgStat(pr_(char *) pr_(unsigned) pr(struct og_stat *));
#if (DPcSystem == DPcSystemWin32)
DEFPUBLIC(int) OgFileTimeToTime(pr_(FILETIME *) pr(time_t *));
#endif



#define _LOGSTATALIVE_

#endif

