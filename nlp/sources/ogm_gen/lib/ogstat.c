/*
 *	Encapsulation of Sleep function
 *	Copyright (c) 2000	Ogmios by Patrick Constant
 *	Dev : September 2000
 *	Version 1.2
*/
#include <loggen.h>
#include <logstat.h>
#include <time.h>


static int OgIsDst(void);



#if (DPcSystem == DPcSystemUnix)
#include <sys/stat.h>

PUBLIC(int) OgStat(char *path, unsigned int mask, struct og_stat *filestat)
{
int retour;
char erreur[DPcSzErr];
struct stat orig_filestat;
retour=stat(path,&orig_filestat);
if (retour==0) {
  filestat->atime = orig_filestat.st_atime;
  filestat->mtime = orig_filestat.st_mtime;
  filestat->ctime = orig_filestat.st_ctime;
  if (sizeof(orig_filestat.st_size)==4) {
    filestat->size_high = 0;
    filestat->size_low = orig_filestat.st_size;
    }
  else if (sizeof(orig_filestat.st_size)==8) {
    filestat->size_high = orig_filestat.st_size>>32;
    filestat->size_low = (orig_filestat.st_size & 0xffffffff);
    }
  else {
    sprintf(erreur, "OgFstat: unhandled sizeof(orig_filestat.st_size) = %d"
      , sizeof(orig_filestat.st_size));
    PcErr(-1,erreur); DPcErr;
    }
  if (OgIsDir(orig_filestat.st_mode)) filestat->is_dir  = 1;
  else filestat->is_dir  = 0;
  }
return(retour);
}



PUBLIC(int) OgStatForDir(struct og_file *str_file, unsigned int mask, struct og_stat *filestat)
{
return(OgStat(str_file->File_Path,mask,filestat));
}



#else
#if (DPcSystem == DPcSystemWin32)
#include <windows.h>


static int OgStatOriginal(char *, unsigned, struct og_stat *);
static int OgStatWindows(char *, unsigned, struct og_stat *);


/*
 *  Returns 0 if 'filestat' has been filled, returns -1 otherwise
 *  Since OgFindFirstFile does not work on x: and \\serveur\dir
 *  we use the stat function in those cases.
*/
PUBLIC(int) OgStat(path,mask,filestat)
char *path; unsigned mask;
struct og_stat *filestat;
{
char pathslash[DPcPathSize];
char *dirsep=0;
int retour;

/** We need to add a backslash in two cases for stat function **/
if (path[1]==':' && path[2]==0) dirsep=DPcDirectorySeparator;
else if (!memcmp(path,"\\\\",2)) dirsep=DPcDirectorySeparator;

if (dirsep) {
  sprintf(pathslash,"%s%s",path,dirsep);
  retour=OgStatOriginal(pathslash,mask,filestat);
  if (retour==0) return(retour);
  /* In the case of files or dir starting with two backslash and 
   * if we cannot stat, we simply try the OgStatWindows function. 
   * This is because directories will work with OgStatOriginal
   * but not with OgStatWindows. And because files will not
   * work with OgStatOriginal (because of the slash) and we
   * prefer to use OgStatWindows on files. */
  }
else {
  /** removing '/' at end because OgStatWindows doesn't like it **/
  int i,ipath=strlen(path);
  strcpy(pathslash,path); path=pathslash;
  for (i=ipath-1; i>=0; i++) {
    if (path[i]=='/' || path[i]=='\\') path[i]=0;
    else break;
    }
  }
return(OgStatWindows(path,mask,filestat));  
}




static int OgStatOriginal(path,mask,filestat)
char *path; unsigned mask;
struct og_stat *filestat;
{
int retour;
struct stat orig_filestat;
retour=stat(path,&orig_filestat);
if (retour==0) {
  filestat->atime = orig_filestat.st_atime;
  filestat->mtime = orig_filestat.st_mtime;
  filestat->ctime = orig_filestat.st_ctime;
  filestat->size_high = 0;
  filestat->size_low = orig_filestat.st_size;
  if (OgIsDir(orig_filestat.st_mode)) filestat->is_dir  = 1;
  else filestat->is_dir  = 0;
  }
return(retour);
}




static int OgStatWindows(path,mask,filestat)
char *path; unsigned mask;
struct og_stat *filestat;
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
IFE(retour=OgFindFirstFile(str_file,path)); 
if(retour) { 
  OgFindClose(str_file);
  IFE(OgStatForDir(str_file,mask,filestat));
  return(0);
  }
DPcErr;
}



/*
 *  The FILETIME structure is a 64-bit value representing 
 *  the number of 100-nanosecond intervals since January 1, 1601. 
*/

PUBLIC(int) OgStatForDir(str_file,mask,filestat)
struct og_file *str_file; unsigned mask;
struct og_stat *filestat;
{
memset(filestat,0,sizeof(struct og_stat));
if (mask & DOgStatMask_mtime) {
  IFE(OgFileTimeToTime(&str_file->FindFileData.ftLastWriteTime,&filestat->mtime));
  }

if (mask & DOgStatMask_ctime) {
  IFE(OgFileTimeToTime(&str_file->FindFileData.ftCreationTime,&filestat->ctime));
  }

if (mask & DOgStatMask_atime) {
  IFE(OgFileTimeToTime(&str_file->FindFileData.ftLastAccessTime,&filestat->atime));
  }

filestat->size_high = str_file->FindFileData.nFileSizeHigh;
filestat->size_low = str_file->FindFileData.nFileSizeLow;
if (str_file->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) filestat->is_dir=1; 
else filestat->is_dir=0;
DONE;
}



/*
 * FILETIME is expressed un UTC/GMT time, which is 3 hours less in summer 
 * and 2 hours less in winter in France.
 * wYear   The current year.
 * wMonth   The current month; January is 1.
 * wDayOfWeek   The current day of the week; Sunday is 0, Monday is 1, and so on.
 * wDay   The current day of the month.
 * wHour   The current hour.
 * wMinute   The current minute.
 * wSecond   The current second.
 * wMilliseconds   The current millisecond.
 * 
 * 
 * tm_hour	Hours since midnight (0-23)
 * tm_isdst	Positive if daylight saving time is in effect; 0 if daylight saving time is not in effect; negative if status of daylight saving time is unknown. The C run-time library assumes the United States’s rules for implementing the calculation of Daylight Saving Time (DST). 
 * tm_mday	Day of month (1-31)
 * tm_min	Minutes after hour (0-59)
 * tm_mon	Month (0-11; January = 0)
 * tm_sec	Seconds after minute (0-59)
 * tm_wday	Day of week (0-6; Sunday = 0)
 * tm_yday	Day of year (0-365; January 1 = 0)
 * tm_year	Year (current year minus 1900)
*/

PUBLIC(int) OgFileTimeToTime(ft,time)
FILETIME *ft; time_t *time;
{
SYSTEMTIME st;
struct tm ctm;
struct tm *tm = &ctm;

FileTimeToSystemTime(ft,&st);
memset(tm,0,sizeof(struct tm));
tm->tm_year = st.wYear - 1900;
tm->tm_mon  = st.wMonth - 1;
tm->tm_mday = st.wDay;
tm->tm_hour = st.wHour;// + 2;
tm->tm_min  = st.wMinute;
tm->tm_sec  = st.wSecond;
tm->tm_isdst = -1;

tzset(); /** Instanciate 'timezone' **/
/** Add timezone from GMT, timezone is -3600 for France **/
*time = mktime(tm) - timezone + OgIsDst()*3600; 
/*
 * OgMessageLog( DOgMlogInLog, "ogm_ocea", 0, "month=%d day=%d hour=%d timezone=%d daylight=%d",
 *   st.wMonth,st.wDay,st.wHour,timezone,OgIsDst());
*/
if (*time == (time_t)-1) DPcErr;
DONE;
}

#endif
#endif



static int OgIsDst(void)
{
time_t ltime;
struct tm *today;
static int result=2;

if (result==2) {
  time(&ltime); 
  today = localtime( &ltime );
  if (today->tm_isdst) result=1; else result=0;
  }
return(result);
}


