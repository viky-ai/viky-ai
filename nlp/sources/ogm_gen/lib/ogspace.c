/*
 *  General function handling disk space.
 *  Copyright (c) 2000-2006 Pertimm by Patrick Constant
 *  Dev : April 2000, March 2006
 *  Version 2.1
*/
#include <loggen.h>



/*
 *  Controls space file for 'letter' which is 'f' for filter
 *  and 'a' for ocea. Returns 1 if no space asked, 2 if space asked
 *  is available, and 0 if space asked is not available.
 *  Space is asked through space files.
 *  Returns -1 on error.
*/

PUBLIC(int) OgControlSpaFile(char *letter, char *path, struct og_bnum *bfree0, struct og_bnum *bspa0)
{
int retour;
unsigned spa;
struct og_bnum bfree,bspa;

IFE(OgGetDiskSpace(path,0,&bfree));
if (bfree0) { IFE(OgSetBnum(&bspa,0,0,0)); *bfree0 = bfree; }
if (bspa0) { IFE(OgSetBnum(&bspa,0,0,0)); *bspa0 = bspa; }

/* OgReadSpa does not use a big number because it is used
 * to handle needed space for Pertimm and thus usually 
 * a space that is not very important (a few mega-bytes).
*/
IFE(retour=OgReadSpaFile(letter,&spa)); if (!retour) return(DOgSpaceNo);
IFE(OgSetBnum(&bspa,spa,0,0));

if (OgAcmpBnums(&bfree,&bspa) > 0) {
  IFE(OgRemoveSpaFile(letter));
  return(DOgSpaceOk);
  }

return(DOgSpaceKo);
}




/*
 *  Gets total disk space and free disk space from the volume
 *  which contains directory 'path'. Uses big number to get good values
 *  the GetDiskFreeSpace function does not work properly on volumes
 *  bigger than 2 Gb on Windows 95 there exist a GetDiskFreeSpaceEx
 *  that is used on Windows 95 OSR2. See the documentation for more details.
*/

#if (DPcSystem == DPcSystemUnix)

#if (DPcArch == DPcArchlinux)
#include <sys/vfs.h>

PUBLIC(int) OgGetDiskSpace(char *path, struct og_bnum *total, struct og_bnum *free_space)
{
char erreur[DPcSzErr];
char sys_erreur[DPcSzSysErr];
struct statfs stat_file_system;

IF(statfs(path,&stat_file_system)) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgGetDiskSpace: error statfs on '%s': %s",path,sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
if (free_space) {
  unsigned f_bavail=(unsigned)stat_file_system.f_bavail;
  IFE(OgSetBnum(free_space,f_bavail,0,0));
  IFE(OgMulBnum(free_space,stat_file_system.f_bsize,0));
  }
if (total) {
  IFE(OgSetBnum(total,stat_file_system.f_blocks,0,0));
  IFE(OgMulBnum(total,stat_file_system.f_bsize,0));
  }

#ifdef DEVERMINE
  {
  char t1[100], f1[100];
  if (total) OgBigFormatThousand(total,t1); else strcpy(t1,"non calculated");
  OgBigFormatThousand(free_space,f1);
  MessageInfoLog( 0, "ogm_fltr", 0 ,"total=%s free=%s",t1,f1);
  }
#endif

DONE;
}

#else
#if (DPcArch == DPcArchsolaris)
#include <sys/types.h>
#include <sys/statvfs.h>


PUBLIC(int) OgGetDiskSpace(path,total,free_space)
char *path; struct og_bnum *total, *free_space;
{
char erreur[DPcSzErr];
char sys_erreur[DPcSzSysErr];
statvfs_t stat_file_system;

IF(statvfs(path,&stat_file_system)) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgGetDiskSpace: error statvfs: %s",sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
if (free_space) {
  if (sizeof(fsblkcnt_t)==4) {
    unsigned f_bavail=(unsigned)stat_file_system.f_bavail;
    IFE(OgSetBnum(free_space,f_bavail,0,0));
    }
  else if (sizeof(fsblkcnt_t)==8) {
    unsigned low,high;
    high = (unsigned)(stat_file_system.f_bavail >> 32);
    low = (unsigned)(stat_file_system.f_bavail & 0xffffffff);
    IFE(OgHighLowToBnum(high,low,free_space));
    }
  else {
    DPcSprintf(erreur,"OgGetDiskSpace: sizeof(fsblkcnt_t)=%d not implemented"
      , sizeof(fsblkcnt_t));
    PcErr(-1,erreur); DPcErr;
    }
  IFE(OgMulBnum(free_space,stat_file_system.f_frsize,0));
  }
if (total) {
  if (sizeof(fsblkcnt_t)==32) {
    IFE(OgSetBnum(total,stat_file_system.f_blocks,0,0));
    }
  else if (sizeof(fsblkcnt_t)==64) {
    unsigned low,high;
    high = (unsigned)(stat_file_system.f_blocks >> 32);
    low = (unsigned)(stat_file_system.f_blocks & 0xffffffff);
    IFE(OgHighLowToBnum(high,low,total));
    }
  else {
    DPcSprintf(erreur,"OgGetDiskSpace: sizeof(fsblkcnt_t)=%d not implemented"
      , sizeof(fsblkcnt_t));
    PcErr(-1,erreur); DPcErr;
    }
  IFE(OgMulBnum(total,stat_file_system.f_frsize,0));
  }

#ifdef DEVERMINE
  {
  char t1[100], f1[100];
  if (total) OgBigFormatThousand(total,t1); else strcpy(t1,"non calculated");
  OgBigFormatThousand(free_space,f1);
  MessageInfoLog( 0, "ogm_fltr", 0 ,"total=%s free=%s",t1,f1);
  }
#endif

DONE;
}

#else
#if (DPcArch == DPcArchtru64)
#include <sys/mount.h>

PUBLIC(int) OgGetDiskSpace(path,total,free_space)
char *path; struct og_bnum *total, *free_space;
{
char erreur[DPcSzErr];
char sys_erreur[DPcSzSysErr];
struct statfs stat_file_system;

IF(statfs(path,&stat_file_system)) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgGetDiskSpace: error statvfs: %s",sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
if (free_space) {
  unsigned f_bavail=(unsigned)stat_file_system.f_bavail;
  IFE(OgSetBnum(free_space,f_bavail,0,0));
  IFE(OgMulBnum(free_space,stat_file_system.f_fsize,0));
  }
if (total) {
  IFE(OgSetBnum(total,stat_file_system.f_blocks,0,0));
  IFE(OgMulBnum(total,stat_file_system.f_fsize,0));
  }

#ifdef DEVERMINE
  {
  char t1[100], f1[100];
  if (total) OgBigFormatThousand(total,t1); else strcpy(t1,"non calculated");
  OgBigFormatThousand(free_space,f1);
  MessageInfoLog( 0, "ogm_fltr", 0 ,"total=%s free=%s",t1,f1);
  }
#endif

DONE;
}

#else
#if (DPcArch == DPcArchaix)
#include <sys/statfs.h>

PUBLIC(int) OgGetDiskSpace(path,total,free_space)
char *path; struct og_bnum *total, *free_space;
{
char erreur[DPcSzErr];
char sys_erreur[DPcSzSysErr];
struct statfs stat_file_system;

IF(statfs(path,&stat_file_system)) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgGetDiskSpace: error statvfs: %s",sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
if (free_space) {
  unsigned f_bavail=(unsigned)stat_file_system.f_bavail;
  IFE(OgSetBnum(free_space,f_bavail,0,0));
  IFE(OgMulBnum(free_space,stat_file_system.f_bsize,0));
  }
if (total) {
  IFE(OgSetBnum(total,stat_file_system.f_blocks,0,0));
  IFE(OgMulBnum(total,stat_file_system.f_bsize,0));
  }

#ifdef DEVERMINE
  {
  char t1[100], f1[100];
  if (total) OgBigFormatThousand(total,t1); else strcpy(t1,"non calculated");
  OgBigFormatThousand(free_space,f1);
  MessageInfoLog( 0, "ogm_fltr", 0 ,"total=%s free=%s",t1,f1);
  }
#endif

DONE;
}

#else
Error DPcArch value unknown
#endif
#endif
#endif
#endif


#else
#if (DPcSystem == DPcSystemWin32)

#include <windows.h>


PUBLIC(int) OgGetDiskSpace(name,total,free)
char *name; struct og_bnum *total, *free;
{
char erreur[DPcSzErr];
char dirname[DPcPathSize];
char sys_erreur[DPcSzSysErr];
unsigned dwBytesPerSector, dwSectorsPerCluster, dwFreeClusters, dwTotalClusters;
int i,idirname;

if (OgIsAbsolutePath(name)) {
  strcpy(dirname,name);
  }
else {
  getcwd(dirname,DPcPathSize);
  }
idirname=strlen(dirname);

/** handling "\\MyServer\MyShare\" **/
if (dirname[0]=='/' || dirname[0]=='\\') {
  int slash=0;
  for (i=2; i<idirname; i++) {
    if (dirname[i]=='/' || dirname[i]=='\\') {
      slash++; if (slash==2) break;
      }
    }
  if (slash==2) dirname[i+1]=0;
  } 
/** handling "c:\" **/
else if (dirname[1]==':') {
  dirname[2]='\\'; dirname[3]=0;
  }
idirname=strlen(dirname);

if (!GetDiskFreeSpace(dirname,
		  &dwSectorsPerCluster,
		  &dwBytesPerSector,
		  &dwFreeClusters,
		  &dwTotalClusters)) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgGetDiskSpace: error GetDiskFreeSpace: %s",sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }
if (total) {
  IFE(OgSetBnum(total,dwTotalClusters,0,0));
  IFE(OgMulBnum(total,dwSectorsPerCluster,0));
  IFE(OgMulBnum(total,dwBytesPerSector,0));
  }
if (free) {
  IFE(OgSetBnum(free,dwFreeClusters,0,0));
  IFE(OgMulBnum(free,dwSectorsPerCluster,0));
  IFE(OgMulBnum(free,dwBytesPerSector,0));
  }
DONE;
}
  

#endif
#endif
