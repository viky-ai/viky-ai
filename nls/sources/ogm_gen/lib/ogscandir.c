/*
 *  Recursive scan of a directory.
 *  Copyright (c) 2000-2007 Pertimm by Patrick Constant
 *  Dev : April 2000, January 2001, November 2007
 *  Version 1.2
*/
#include <loggen.h>
#include <string.h>
#include <logstat.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#define DOgMaxScanPath  100000
#define DOgMaxBufferSize 0x200000 /* 2Mb */
 
 
struct og_scandir {
  int start; short length;
  short expanded; int father;
  };
 
static int GetFullPath(char *, int, struct og_scandir *, char *);
 
 
 
/*
 *  Scans recursively the directory given by path0 and calls
 *  function 'func' on each directory or file found.
 *  func takes 4 arguments:
 *  'ptr': passed by OgScanDir to give a context
 *  'is_dir': 'path' is a directory
 *  'ipath': length of 'path'
 *  'path' null terminated string that represents the full path
 *  of the found file or directory.
 *  Some messages can be logged in logfile if logfile != 0
*/
 
PUBLIC(int) OgScanDir(char *path0, int (*func) (void *, int, int, char *), void *ptr, char *logfile)
{
int retour;
struct stat filestat;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
int isd; struct og_scandir *sd;
char pattern_path[DPcPathSize];
struct og_stat og_filestat;
char erreur[DPcSzErr];
char *bdirs, *path;
 
#if (DPcSystem == DPcSystemUnix)
#include <sys/stat.h>
#include <unistd.h>
#endif
 
#ifdef DEVERMINE
int max_ibdirs, nmax_ibdirs;
#endif

if (stat(path0,&filestat)==0 && !OgIsDir(filestat.st_mode)) {
  IFE((*func)(ptr,0,strlen(path0),path0));
  DONE;
  }

IFn(sd=(struct og_scandir *)malloc(sizeof(struct og_scandir)*DOgMaxScanPath)) {
  sprintf(erreur,"OgScanDir: impossible to malloc %d bytes (sd)"
    ,sizeof(struct og_scandir)*DOgMaxScanPath);
  PcErr(-1,erreur); DPcErr;
  }
IFn(bdirs=(char *)malloc(DOgMaxBufferSize)) {
  sprintf(erreur,"OgScanDir: impossible to malloc %d bytes (bdirs)",DOgMaxBufferSize);
  PcErr(-1,erreur); DPcErr;
  }
IFn(path=(char *)malloc(DOgMaxBufferSize)) {
  sprintf(erreur,"OgScanDir: impossible to malloc %d bytes (path)",DOgMaxBufferSize);
  PcErr(-1,erreur); DPcErr;
  }

isd=0; sd[isd].start = 0; sd[isd].length = strlen(path0);
sd[isd].expanded = 0; sd[isd].father = -1;
sprintf(bdirs+sd[isd].start,"%s",path0);
 
#ifdef DEVERMINE
/** This is just for statistics on bdirs reached size **/
max_ibdirs = sd[isd].length+1;
#endif
 
isd++; 
 
while(1) {
  int isd_father, some_dir_added=0;
  for (isd--; isd>=0; isd--) {
    if (!sd[isd].expanded) break;
    }
  if (isd < 0) break;
  sd[isd].expanded = 1;
  isd_father = isd;
  GetFullPath(pattern_path,isd,sd,bdirs);
  sprintf(pattern_path+strlen(pattern_path),"*");
 
#ifdef DEVERMINE
  printf("pattern path is '%s' for isd=%d\n",pattern_path,isd);
#endif
 
  IF(retour=OgFindFirstFile(str_file,pattern_path)) {
    PcErrLast(-1,erreur);
    OgMessageLog(DOgMlogInLog,logfile,0,"Warning: %s",erreur);
    retour=0;
    } 
  if(!retour) continue; 
  do {
    int is_dir,ipath,idir,is_lnk=0;
    if (!strcmp(str_file->File_Path,".")) continue;
    if (!strcmp(str_file->File_Path,"..")) continue;
    strtok(pattern_path,"*");
    sprintf(path,"%s%s",pattern_path,str_file->File_Path); ipath=strlen(path);
    if (OgStat(path,DOgStatMask_is_dir,&og_filestat)==0 && 
        og_filestat.is_dir) is_dir=1; else is_dir=0;
    /* This the original encoding that is very slow see logstat.h for more details
     * if (stat(path,&filestat)==0 && OgIsDir(filestat.st_mode)) is_dir=1; else is_dir=0;
    */
#if (DPcSystem == DPcSystemUnix)
/******************************************************************/
/*Do not forget the Symbolic Link Files                           */
/*To find them use the function lstat and the macro S_ISLNK       */
/******************************************************************/
   { 
   struct stat linkstat;
   lstat(path,&linkstat);
   if (S_ISLNK(linkstat.st_mode)) is_lnk = 1; else is_lnk = 0;
   }
#endif
 
    IFE((*func)(ptr,is_dir,ipath,path));
    if (is_dir && !is_lnk) {
      idir = strlen(str_file->File_Path); isd++; 
      if (isd >= DOgMaxScanPath) {
        sprintf(erreur,"OgScanDir: too many directories, max is %d",DOgMaxScanPath);
        PcErr(-1,erreur); DPcErr;
        }
      sd[isd].start = sd[isd-1].start + sd[isd-1].length + 1; sd[isd].length=idir;
      sd[isd].expanded = 0; sd[isd].father = isd_father;
      some_dir_added++;
      if (sd[isd].start + sd[isd].length +1 >= DOgMaxBufferSize) {
        sprintf(erreur,"OgScanDir: directories buffer max size reached (%d)",DOgMaxBufferSize);
        PcErr(-1,erreur); DPcErr;
        }
      
      sprintf(bdirs+sd[isd].start,"%s",str_file->File_Path);
      
#ifdef DEVERMINE
      nmax_ibdirs = sd[isd].start + sd[isd].length +1;
      if (nmax_ibdirs > max_ibdirs) max_ibdirs = nmax_ibdirs;
      printf("writing '%s' at %d len=%d ilp=%d max=%d\n"
            , bdirs+sd[isd].start, sd[isd].start, sd[isd].length, isd, max_ibdirs);
#endif
      }
    }
  while (OgFindNextFile(str_file));
  if (some_dir_added) isd++;
  OgFindClose(str_file);
  }
/*OgFindClose(str_file);*/
 
#ifdef DEVERMINE
printf("Max buffer length reached = %d\n",max_ibdirs);
#endif

DPcFree(bdirs);
DPcFree(path);
DPcFree(sd);
 
DONE;
}
 
 
 
/*
 *  Basically, we scan one directory and store all new directories
 *  in this directory. in 'sd'. We only store the name of those directories
 *  to avoid taking too much space in the 'bdirs' buffer. But then
 *  we need to rebuild the full path. Since the links are reversed
 *  we then reverse the string to get the full path in the right order.
*/
 
 
static int GetFullPath(char *path, int isd, struct og_scandir *sd, char *bdirs)
{
int i, oldi, ir=0, ipath=0;
char r[DPcPathSize];
r[ir++]='\0';
while (1) {
  memcpy(r+ir, bdirs+sd[isd].start, sd[isd].length);
  ir += sd[isd].length; r[ir++] = '\0';
  if (sd[isd].father == -1) break;
  isd = sd[isd].father;
  }
for (oldi=i=ir-2; i>=0; i--) {
  if (r[i]=='\0') {
    memcpy(path+ipath,r+i+1,oldi-i);
    ipath += oldi-i; oldi = i-1;
    sprintf(path+ipath,"%s",DPcDirectorySeparator);
    ipath++;
    }
  }
DONE;
}
 
 
