/*
 *        This is include for ogm_gen.dll
 *        Copyright (c) 1998-2004 Pertimm by Patrick Constant
 *        Dev : September 1998, September, October, December 1999
 *        Dev : January, May, November 2000, June 2002, March; December 2003
 *        Dev : January,February,March,April,May,July,August,September,November,December 2004
 *        Dev : January,March,April,May,June,July,August,October 2005, January,March,May,July,August 2006
 *        Dev : March,August,December 2007
 *        Version 4.7
*/

#ifndef _OGMGENALIVE_

#define DOgGenBanner        "ogm_gen V4.66, Copyright (c) 1998-2008 Pertimm, Inc."
#define DOgGenVersion       466


#include <time.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <lpcosys.h>
#include <math.h>
#include <lpcerr.h>
#include <lpclat.h>
#include <logmess.h>
#include <logmlog.h>
#include <loglang.h>
#include <logchset.h>
#include <pthread.h>


#if 0 /** For testing memory leaks, link with include/ogmalloc.c **/
#define malloc(x)   OgMalloc(x,"gen",__FILE__,__LINE__)
#define free(x)     OgFree(x,"gen",__FILE__,__LINE__)
#endif

#include <fcntl.h>
#if (DPcSystem == DPcSystemUnix)
#include <unistd.h> /* for access, unlink */
#include <errno.h>
#include <dlfcn.h>
#include <libgen.h>   // for dirname, basename
#include <sys/time.h> // for gettimeofday

#define DOgHinstance    void *
#else
#if (DPcSystem == DPcSystemWin32)
/* including winsock2.h before windows.h prevents
 * including winsock.h, see comment at start of winsock2.h:
 * #define _WINSOCK2API_
 * #define _WINSOCKAPI_  Prevent inclusion of winsock.h in windows.h */
#include <winsock2.h>
#include <windows.h>
#include <lmerr.h>
#include <process.h> /* for getpid */
#include <direct.h> /* for chdir */
#include <io.h>    /* for access */
#define DOgHinstance    HINSTANCE
#endif
#endif

/** Needs windows.h **/
#include <logstat.h>


/** Thread safe maximum error message size, used with the OgErr* functions **/
#define DOgErrorSize        2048

/** flags for OgErrLog functions **/
#define DOgErrLogFlagNoSystemError    0x1
#define DOgErrLogFlagNotInErr         0x2
#define DOgErrLogFlagCrIsBr           0x4
#define DOgErrLogFlagNoHeader         0x8
#define DOgErrLogFlagNoDate           0x10

#define DOgNMax   0xffffffff

/** Encapsulation of message box type **/
#define DOgMessageBoxInformation  1
#define DOgMessageBoxQuestion  2
#define DOgMessageBoxWarning  3
#define DOgMessageBoxError  4


/** Used with OgHaversineDistance **/
#define DOgHaversineDimensionDecimalDegree  1
#define DOgEarthRadius                      6371

/** Geometrical precision used is several programs **/
#define DOgGeoPrecision                     1000000
#define DOgGeoLatitudeCorrection            90
#define DOgGeoLongitudeCorrection           180

#define DOgPninlen(herr,p)          OgPninlen(herr,p,__FILE__,__LINE__)
#define DOgPninLong(herr,pp,pvalue) OgPninLong(herr,pp,pvalue,__FILE__,__LINE__)
#define DOgPnin4(herr,pp,pvalue)    OgPnin4(herr,pp,pvalue,__FILE__,__LINE__)
#define DOgPnin8(herr,pp,pvalue)    OgPnin8(herr,pp,pvalue,__FILE__,__LINE__)
#define DOgPnin64(herr,pp,pvalue)   OgPnin64(herr,pp,pvalue,__FILE__,__LINE__)


/*  DOgMaxBnum*16 gives the number of bits used to represent a big number.
 *  We chose a fixed size because we want to be fast and we don't use
 *  very big numbers anyway. a value of 8 gives a number coded on 128 bits.
*/
#define DOgMaxBnum  8

struct og_bnum {
  unsigned short n[DOgMaxBnum];
  int minus;
  };

 /** Disk space statuses (return values for OgControlSpaFile function) **/
#define DOgSpaceKo    0
#define DOgSpaceNo    1
#define DOgSpaceOk    2


struct og_memory_status {
  int MemoryLoad;    /* percent of memory in use */
  struct og_bnum TotalPhys;     /* bytes of physical memory */
  struct og_bnum AvailPhys;     /* free physical memory bytes */
  struct og_bnum TotalPageFile; /* bytes of paging file */
  struct og_bnum AvailPageFile; /* free bytes of paging file */
  struct og_bnum TotalVirtual;  /* user bytes of address space */
  struct og_bnum AvailVirtual;  /* free user bytes */
  };

/*
 *  Structure that informs on the file selected:
 *    its name, and for each section, 'i', 'x' or 'n'
 *    to indicate whether it is included, excluded
 *    or neither of them.
 *  Used on OgFileSelect.
*/
struct og_selected {
  int ifile; unsigned char *file;
  int icanon; unsigned char *canon;
  char type_ds, type_fs, type_fss;
  };


struct og_alias {
  int itrue_name, ialias_name;
  int true_name, alias_name;
  };

struct og_aliases {
  int iB, nB; char *B;
  int ialias, nalias; struct og_alias *alias;
  };

#if (DPcSystem == DPcSystemUnix)
#include <dirent.h>
#include <sys/types.h>
#include <regex.h>

struct og_file {
  DIR *dp;
  regex_t comp;
  char File_Path[DPcPathSize];
  int one_file;
  };

typedef long long ogint64_t;

#else
#if (DPcSystem == DPcSystemWin32)

struct og_file {
  WIN32_FIND_DATAW FindFileData;
  HANDLE FindHandle;
  char File_Path[512];
  };

typedef __int64 ogint64_t;
#endif
#endif


/*
 *  This is a structure that represents as closely as possible the Ogmios date.
 *  The Ogmios date can be infinitely precise, so this structure is only an approximation
 *  but it is practical for usual applications. We also put a double/int field
 *  for years and second to simplify the setting of this function and avoid
 *  possible precision loss with the double field when the value is an int.
*/
struct og_date {
  double dyear; /* year since AD 0 (positive or negative) only integer part accepted */
  int iyear; /* year since AD 0 (positive or negative) as integer */
  int month; /* months since January - [1,12] */
  int mday; /* days of month - [1,31] */
  int hour; /* hours since midnight - [1,24] */
  int minute; /* minutes after the hour - [1,60] */
  double dsecond; /* seconds after the minute - [0,59] and as precise as necessary  */
  int isecond; /* seconds after the minute - [0,59] as integer  */
  int imsecond; /* micro-seconds after the second as integer [0,999999] */
  unsigned is_dyear : 1; /* if 1 dyear is set else iyear is set */
  unsigned is_dsecond : 1; /* if 1 dsecond is set else isecond is set */
  unsigned free : 14;
  };


/** Used with function OgRealToStruct **/
#define DOgMaxRealElement   1024
struct og_dispatched_real {
  int infinite,zero;
  char m_sign[9];
  char m1[9], m2[DOgMaxRealElement];
  char e_sign[9];
  int exp;
  };


/* Some words can start with d' l' m' s' t' qu' etc...
 * at the beginning of the word. We want to be able to remove it
 * in the case of dynamic concept specifications and smart queries.
 * It is a french specificity but we apply it for any language,
 * because it seems general enough. We will see later if it induces
 * side effects in other languages. Used by OgDelPrefixInit.
*/

struct og_del_prefix {
  int ich; short ch[10];
  };

/**  Used with OgGetPeriod and OgGetSpanPeriod functions. **/

#define DOgPeriodTypeSecond     's'
#define DOgPeriodTypeMinute     'n'
#define DOgPeriodTypeHour       'h'
#define DOgPeriodTypeDay        'd'
#define DOgPeriodTypeWeek       'w'
#define DOgPeriodTypeMonth      'm'
#define DOgPeriodTypeYear       'y'

struct og_period {
  int type,value;
  int nb_seconds;
  };

struct og_span_period {
  time_t starting_date;
  time_t ending_date;
  int number;
  };

/** Used with OgReadTag and OgReadTagUni **/
#define DOgReadTagDataSize      2048
#define DOgReadTagUniDataSize   4096
struct og_read_tag {
  int iname; unsigned char *name;
  int iattr; unsigned char *attr;
  int ivalue; unsigned char *value;
  int offset,closing_tag;
  };


/** Used with OgConfGetVarLines/OgConfGetVarLinesFree **/
struct og_conf_lines {
  int nb_values, *value;
  char *buffer;
  };

/** Used with OgReadDirAlias **/
struct og_dir_alias {
  struct og_dir_aliases *aliases;
  int idir, dir;
  int ialias, alias;
  };

struct og_dir_aliases {
  int iB, nB; char *B;
  int ialias, nalias; struct og_dir_alias *alias;
  };


/*
 *  Structure that informs on a bundle filename
 *  example of a double bundle:
 *    g:\test\eml\zip1.eml eml 2/PR_EPO_SDS_10.zip zip PR_EPO_SDS_10.doc
 *  example of a single bundle with a 'tail':
 *    d:\cserve\download\zlib122dll.zip zip readme.txt 2800 1061 104:10:7:12:52:52
 *  space are 0x1 ('\1') chars in those examples.
*/
#define DOgBundledFileTypeNil   0
#define DOgBundledFileTypeZip   1
#define DOgBundledFileTypeEml   2
#define DOgBundledFileTypeNsf   3
#define DOgBundledFileTypeGz    4
#define DOgBundledFileTypeTgz   5


#define DOgMaxBundleType    10

struct og_bundle_file_info_type {
  int start,length,type;
  };

struct og_bundled_file_info {
  int nb_type; struct og_bundle_file_info_type type[DOgMaxBundleType];
  int ifilename, is_tail; unsigned zipped_size;
  struct og_date date;
  struct og_stat stat;
  };


#if 0
#define DOgBundledFileTypeZip       1
#define DOgBundledFileTypeUnknown   2
#define DOgBundledFileTypeEml       3
#define DOgMaxSepBundle             10

struct bundled_file_info {
  int type, internal_start, ifilename;
  unsigned size, zipped_size, last_update;
  int year, mon, mday, hour, min, sec;
  int nb_sep, sep[DOgMaxSepBundle];
  };
#endif


/*
 * Span of any sort that can be represented by (start,length).
 * Examples:
 * - string (span of characters on Ba)
 * - expression (span of words)
 * - duration (span of time)
 */
struct og_span {
  int start;
  int length;
  };

/*
 * Range of any sort that can be represented by (from,to).
 */
typedef struct og_range og_range;
struct og_range {
  int from;
  int to;
  };


//----- Indus constants
#define DOgAdminDefFile         "admin.xml"    // Wrapper def file
#define DOgSamFile              "sam.xml"      // Stable Memory File
#define DOgClusterConf          "config.xml"   // services configuration file
#define DOgLogFile              "circlog.bin"  // Log file
#define DOgPipeName             "clogPIPE"     // pipe name for logging
#define DOgAdminLuaScript       "ogadmin.lua"  // lua script for ogadmin
#define DOgHeartProdConfFile    "prodconf.xml"
#define DOgSpreadProdConfFile   "spread.conf"            // Name of generated spread config
#define DOgSsiFeedConf          "ssi_xml_feed_conf.xml"  // xml description of indexed data
#define DOgOgmSsiTxt            "ogm_ssi.txt"


/* It is possible to put a bigger value, but for now, this would make the eBusiness log (v3)
 * unreadable because all log are prefixed with the instance name */
#define DOgInstanceNameSize     16            // max instance name size

#if (DPcSystem==DPcSystemWin32)
#define NATIVE_FILE             HANDLE
#define PROCESS_MUTEX           HANDLE
#define THREAD_MUTEX        HANDLE
#define FILE_LOCKSHARED         0
#define FILE_LOCKEXCLUSIVE      LOCKFILE_EXCLUSIVE_LOCK
#define FILE_UNLOCK            -1

typedef struct {
  int              waitersCount;     // Count of the number of waiters.
  CRITICAL_SECTION waitersCountLock; // Serialize access to <waiters_count_>.
  int              releaseCount;     // Number of threads to release via a <pthread_cond_broadcast> or a <pthread_cond_signal>.
  int              waitGenCount;     // Keeps track of the current "generation" so that we don't allow one thread to steal
                                     // all the "releases" from the broadcast.
  HANDLE           event;            // A manual-reset event that's used to block and release waiting threads.
} pthread_cond_t;

#define R_OK 4                 // Test Read permission (access function, unistd.h)
#define W_OK 2                 // Test for wrte permission  (access function, unistd.h)
#define X_OK 1                 // Test for execute (search) permission  (access function, unistd.h) [unused for windws]
#define F_OK 0                 // Test for existence of file  (access function, unistd.h)

typedef unsigned char *         caddr_t;

// convert FILE * to internal HANDLE
#define OgFileToHandle(fp)      (NATIVE_FILE) _get_osfhandle(_fileno(fp))
#define OgFdToHandle(fd)        (NATIVE_FILE) _get_osfhandle(fd)

#else

#define PROCESS_MUTEX           int
#define THREAD_MUTEX        pthread_mutex_t

#define NATIVE_FILE             int
#define FILE_LOCKSHARED         0
#define FILE_LOCKEXCLUSIVE      1
#define FILE_UNLOCK             2

#define OgFileToHandle(fp)      (NATIVE_FILE) fileno(fp)
#define OgFdToHandle(fd)        (NATIVE_FILE) fd
#define closesocket(fd)         close(fd)

#define min(x,y)                fmin(x,y)
#define max(x,y)                fmax(x,y)
#endif
#define THREAD_COND        pthread_cond_t


#ifndef  __cplusplus
typedef int bool;
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


#if (DPcSystem==DPcSystemWin32)
#ifndef _GETOPT_H_
DEFPUBLIC(int) getopt (int argc, char *const *argv, const char *shortopts);
DEFPUBLIC(int) getopt_long (int argc, char *const *argv, const char *shortopts, const struct option *longopts, int *longind);
#endif
// ISO C99 Definition of cross platform "width" int (http://www.opengroup.org/onlinepubs/009695399/basedefs/stdint.h.html
#include <pstdint.h>

// New definition for deprecated Windows functions
#define inline    __inline
#define snprintf  _snprintf
#define vsnprintf _vsnprintf
#define strdup    _strdup
#define unlink    _unlink
#define strtok_r  strtok_s
#define read      _read
#define write     _write
#define lseek     _lseek
#define access    _access
#define close     _close
#define getcwd    _getcwd
#define chmod     _chmod

#define socklen_t  int
// pipe redirection to stdout/stderr
#define OgStdInput()  GetStdHandle(STD_INPUT_HANDLE)
#define OgStdOutput() GetStdHandle(STD_OUTPUT_HANDLE)
#define OgStdError()  GetStdHandle(STD_ERROR_HANDLE)
#define OgStdNull()   INVALID_HANDLE_VALUE
#else
#define OgStdInput()  0
#define OgStdOutput() 1
#define OgStdError()  2
#define OgStdNull()  -1
#endif



/** old wfile.h **/
DEFPUBLIC(int) OgRename_Ext(char *, char *, int, char *, char *, int);
DEFPUBLIC(int) MeWriteToFile(char *, char *, const void *, unsigned long);
DEFPUBLIC(int) WriteToFile(char *, char *, const char *, ...);
DEFPUBLIC(int) OgCheckOrCreateDir(char *, int, char *);
DEFPUBLIC(int) OgCheckDir(char *, int);
DEFPUBLIC(int) OgGetFileSize(char *);

/** old vsfind.h **/
DEFPUBLIC(void) next_filename_to_create(char *, char *, char *);
DEFPUBLIC(int) next_filenumber_to_create(char *, char *, unsigned long int *);
DEFPUBLIC(int) DelFilesInDir(char *, char *, unsigned long int *);
DEFPUBLIC(int) find_lowest_filename(char *, char *, char *, unsigned long int *);
DEFPUBLIC(int) find_nb_files(char *, char *);
DEFPUBLIC(int) wait_on_rename(char *filename);
DEFPUBLIC(int) patterned_rename(char *, char *, char *, char *);
DEFPUBLIC(int) patterned_access(char *);
DEFPUBLIC(int) patterned_delete(char *, char *, char *);

DEFPUBLIC(int) OgLog(char *, char *);
DEFPUBLIC(int) OgUtoA(unsigned char *, unsigned char *, int);
DEFPUBLIC(int) OgUtoAsafe(int, unsigned char *, int, unsigned char *);

DEFPUBLIC(long) OgNtoC(unsigned char *, unsigned char**);
DEFPUBLIC(char *) OgCtoN(unsigned long, char *, unsigned long *);
DEFPUBLIC(char *) OgLanguageToStr(int);

DEFPUBLIC(int) OggNlenLong(unsigned long);
DEFPUBLIC(int) OggNlen(unsigned);
DEFPUBLIC(int) OggNlen8(unsigned long);
DEFPUBLIC(int) OggNinlen(unsigned char*);
DEFPUBLIC(unsigned long) OggNinLong(unsigned char **);
DEFPUBLIC(unsigned) OggNin4(unsigned char **);
DEFPUBLIC(unsigned long) OggNin8(unsigned char **);
DEFPUBLIC(ogint64_t) OggNin64(unsigned char **);

DEFPUBLIC(int) OgPninlen(void *,unsigned char *,char *,int);
DEFPUBLIC(int) OgPninLong(void *,unsigned char **,unsigned long *,char *,int);
DEFPUBLIC(int) OgPnin4(void *,unsigned char **,unsigned *,char *,int);
DEFPUBLIC(int) OgPnin8(void *,unsigned char **,unsigned long *,char *,int);
DEFPUBLIC(int) OgPnin64(void *,unsigned char **,ogint64_t *,char *,int);

DEFPUBLIC(int) OggNoutLong(unsigned long, unsigned char **);
DEFPUBLIC(void) OggNout(unsigned, unsigned char **);
DEFPUBLIC(void) OggNout8(unsigned long, unsigned char **);
DEFPUBLIC(void) OggNout64(ogint64_t, unsigned char **);

DEFPUBLIC(int) OggNormNout(unsigned, unsigned, unsigned char **);
DEFPUBLIC(int) OggNormNin(unsigned, unsigned char **);

DEFPUBLIC(char *) OgExceptionToStr(int);

DEFPUBLIC(int) OgSysErr(void);
DEFPUBLIC(int) OgSysErrMes(int, int, char *);

DEFPUBLIC(int) OgGetMemoryStatus(struct og_memory_status *);

DEFPUBLIC(int) OgConfGetVar(char *, char *, char *, int, int);
DEFPUBLIC(int) OgConfGetVarLine(char *, char *, char *, int);
DEFPUBLIC(int) OgDipperConfGetVar(char *, char *, char *, int);

DEFPUBLIC(size_t) OgArgSize(char *);
DEFPUBLIC(int) OgArgPeriod(char *);
DEFPUBLIC(ogint64_t) OgArgSize64(char *);
DEFPUBLIC(int) OgBigArgSize(char *, struct og_bnum *);
DEFPUBLIC(int) OgFormatThousand(unsigned, char *);
DEFPUBLIC(int) OgBigFormatThousand(struct og_bnum *, char *);
DEFPUBLIC(int) Og64FormatThousand(ogint64_t, char *, int);
DEFPUBLIC(ogint64_t) Ogstrto64(char *, int);

DEFPUBLIC(int) OgGetMaxSortBufferSize(char *);
DEFPUBLIC(int) OgGetDiskRamSortRatio(char *);

DEFPUBLIC(int) OgSleep(int);
DEFPUBLIC(int) OgNanoSleep(ogint64_t nb_nanoseconds);
DEFPUBLIC(int) OgClock(void);
DEFPUBLIC(ogint64_t) OgMicroClock(void);
DEFPUBLIC(int) OgMilliClock(void);
DEFPUBLIC(int) OgTtoA(time_t, int, char *);
DEFPUBLIC(int) OgAtoT(int, char *, unsigned *);
DEFPUBLIC(int) OgAtoOgDate(int, char *, struct og_date *);
DEFPUBLIC(int) OgAtoOgDateReal(int, char *, int, unsigned char *);
DEFPUBLIC(char *) OgGmtime(const time_t *time);

DEFPUBLIC(int) OgFileExists(char *);
DEFPUBLIC(int) OgDirExists(char *);
DEFPUBLIC(int) OgRemoveEndSlash(char *);
DEFPUBLIC(int) CleanFilename(char *, char *);
DEFPUBLIC(int) OgRenameToStalled(char *, int, char *);

DEFPUBLIC(int) OgGetBundledFileInfo(int, char *, struct og_bundled_file_info *);

DEFPUBLIC(int) OgFileToLog(char *, char *);

DEFPUBLIC(int) OgReadAlias(char *, struct og_aliases *);
DEFPUBLIC(int) OgFreeAlias(struct og_aliases *);
DEFPUBLIC(int) OgGetAlias(struct og_aliases *, char *, char *);
DEFPUBLIC(int) OgParseAliases(int, unsigned char *, int *, unsigned char *, int *);
DEFPUBLIC(int) OgInAliasName(int);

DEFPUBLIC(int) OgMkdir(char *, int);
DEFPUBLIC(int) OgIsDir(int);

DEFPUBLIC(int) OgGetModuleFileName(void *, char *, int);

DEFPUBLIC(void) OgOgMessageLog(int, char *, char *);

DEFPUBLIC(void) OgMessageBox(void *, char *, char *, unsigned);

DEFPUBLIC(int) OgFindFirstFile(struct og_file *, char *);
DEFPUBLIC(int) OgFindNextFile(struct og_file *);
DEFPUBLIC(int) OgFindClose(struct og_file *);

DEFPUBLIC(int) OgSetBnum(struct og_bnum *, unsigned, int, int);
DEFPUBLIC(int) OgCopyBnum(struct og_bnum *, struct og_bnum *);
DEFPUBLIC(int) OgAcmpBnums(struct og_bnum *, struct og_bnum *);
DEFPUBLIC(int) OgAddBnums(struct og_bnum *, struct og_bnum *, struct og_bnum *);
DEFPUBLIC(int) OgAddBnus(struct og_bnum *, struct og_bnum *);
DEFPUBLIC(int) OgAddBnum(struct og_bnum *, unsigned, int);
DEFPUBLIC(int) OgMulBnum(struct og_bnum *, unsigned, int);
DEFPUBLIC(int) OgBnumToStr(struct og_bnum *, int, char *);
DEFPUBLIC(int) OgStrToBnum(char *, struct og_bnum *);
DEFPUBLIC(int) OgSlideBnum(struct og_bnum *, int);
DEFPUBLIC(int) OgDivBnums(struct og_bnum *, struct og_bnum *, struct og_bnum *, struct og_bnum *);
DEFPUBLIC(int) OgDivBnum(struct og_bnum *, struct og_bnum *, struct og_bnum *, unsigned, int);
DEFPUBLIC(int) OgMinusBnum(struct og_bnum *);
DEFPUBLIC(int) OgBnumToUnsigned(struct og_bnum *, unsigned *, int);
DEFPUBLIC(int) OgBnumToInt(struct og_bnum *, int *, int);
DEFPUBLIC(int) OgBnumToHighLow(struct og_bnum *, unsigned *, unsigned *, int);
DEFPUBLIC(int) OgHighLowToBnum(unsigned, unsigned, struct og_bnum *);
DEFPUBLIC(int) OgBnumIsZero(struct og_bnum *);
DEFPUBLIC(int) OgBnumToDouble(struct og_bnum *, double *);
#if (DPcSystem == DPcSystemWin32)
DEFPUBLIC(int) OgBnumToLong64(struct og_bnum *, __int64 *, int);
DEFPUBLIC(int) OgLong64ToBnum(__int64, struct og_bnum *);
#endif
#if (DPcArch == DPcArchtru64)
DEFPUBLIC(int) OgBnumToLong64(struct og_bnum *, long *, int);
#endif

DEFPUBLIC(int) OgDeclareNamedPid(char *, char *);
DEFPUBLIC(int) OgDirDeclareNamedPid(char *, char *, char *);
DEFPUBLIC(int) OgCleanNamedPid(char *);
DEFPUBLIC(int) OgDirCleanNamedPid(char *, char *);
DEFPUBLIC(int) OgCleanDeadNamedPid(char *);
DEFPUBLIC(int) OgDirCleanDeadNamedPid(char *, char *);
DEFPUBLIC(int) OgRunningNamedPid(char *);
DEFPUBLIC(int) OgDirRunningNamedPid(char *, char *);
DEFPUBLIC(int) OgDirGetRunningNamedPid(char *, char *, unsigned *);
DEFPUBLIC(int) OgProcessExists(unsigned);
DEFPUBLIC(int) OgGetNamedPidPath(int, char *);

DEFPUBLIC(int) OgRunningNamedPidW(char *);
DEFPUBLIC(int) OgDirRunningNamedPidW(char *, char *);
DEFPUBLIC(int) OgDirGetRunningNamedPidW(char *, char *, unsigned *);

DEFPUBLIC(int) OgScanDir(char *, int (*)(void *, int, int, char *), void *, char *);
DEFPUBLIC(int) OgScanDirEx(char *, int (*)(void *, int, int, char *, struct og_stat *), void *, int, char *);

DEFPUBLIC(int) OgCopyFile(char *, char *, int);

DEFPUBLIC(int) OgReadCptFile(char *, unsigned *);
DEFPUBLIC(int) OgReadTotFile(char *, unsigned *);
DEFPUBLIC(int) OgDirReadCptFile(char *, char *, unsigned *);
DEFPUBLIC(int) OgDirReadExtFile(char *, char *, unsigned *, char *);
DEFPUBLIC(int) OgWriteCptFile(char *, unsigned);
DEFPUBLIC(int) OgWriteTotFile(char *, unsigned);
DEFPUBLIC(int) OgDirWriteCptFile(char *, char *, unsigned);
DEFPUBLIC(int) OgDirWriteExtFile(char *, char *, unsigned, char *);
DEFPUBLIC(int) OgRemoveCptFile(char *);
DEFPUBLIC(int) OgRemoveTotFile(char *);
DEFPUBLIC(int) OgDirRemoveCptFile(char *, char *);
DEFPUBLIC(int) OgDirRemoveExtFile(char *, char *, char *);

DEFPUBLIC(int) OgReadSpaFile(char *, unsigned *);
DEFPUBLIC(int) OgWriteSpaFile(char *, unsigned);
DEFPUBLIC(int) OgRemoveSpaFile(char *);

DEFPUBLIC(int) OgControlSpaFile(char *, char *, struct og_bnum *, struct og_bnum *);
DEFPUBLIC(int) OgGetDiskSpace(char *, struct og_bnum *, struct og_bnum *);

DEFPUBLIC(int) OgIsTag(int, int, char *, int *);
DEFPUBLIC(int) OgReadTag(int, unsigned char *, int, int (*)(void *, struct og_read_tag *), void *);
DEFPUBLIC(int) OgReadTagUni(int, unsigned char *, int, int (*)(void *, struct og_read_tag *), void *);

DEFPUBLIC(int) Ogstricmp(const char *, const char *);
DEFPUBLIC(int) Ogmemicmp(const char *, const char *, int);

DEFPUBLIC(int) Ogrename(char *, char *, int, char *, char *, int);
DEFPUBLIC(int) Ogremove(char *, int, char *, char *, int);

DEFPUBLIC(int) OgDelFiles(char *, char *, char *);

DEFPUBLIC(int) OgLittleEndian(void);
DEFPUBLIC(int) OgBigEndian(void); /** Same function as OgLittleEndian, but wrong name **/

DEFPUBLIC(int) OgSetIoBufferSize(int);
DEFPUBLIC(void *) OgFopen(char *, char *);
DEFPUBLIC(int) OgSizeOffT(void);
DEFPUBLIC(int) OgFseek(void *, struct og_bnum *, int);
DEFPUBLIC(int) OgFtell(void *, struct og_bnum *);
DEFPUBLIC(int) OgFwrite(void *, void *, unsigned);
DEFPUBLIC(int) OgFread(void *, void *, unsigned);
DEFPUBLIC(int) OgFflush(void *);
DEFPUBLIC(int) OgFclose(void *);
DEFPUBLIC(int) OgFsync(void *);

DEFPUBLIC(int) OgMimeEncode(int, unsigned char *, int *, unsigned char *);
DEFPUBLIC(int) OgMimeDecode(int, unsigned char *, int *, unsigned char *);

DEFPUBLIC(int) OgIsspace(int);
DEFPUBLIC(int) OgCharToShort(int, unsigned char *, unsigned short *);

DEFPUBLIC(int) OgGmtToOgDate(time_t, struct og_date *);
DEFPUBLIC(int) OgLotToOgDate(time_t, struct og_date *);
DEFPUBLIC(int) OgDateToTime_t(struct og_date *, time_t *);
DEFPUBLIC(int) OgDateToOgReal(struct og_date *, int, unsigned char *);
DEFPUBLIC(int) OgDateLength(unsigned char *);
DEFPUBLIC(int) OgRealToOgDate(unsigned char *, struct og_date *);
DEFPUBLIC(int) OgRealToOgDateString(unsigned char *, char *);
DEFPUBLIC(int) OgGmtToOgReal(time_t, int, unsigned char *);
DEFPUBLIC(int) OgLotToOgReal(time_t, int, unsigned char *);
DEFPUBLIC(int) OgRealToTime_t(unsigned char *, time_t *);

DEFPUBLIC(int) OgUnsignedToOgReal(unsigned, int, unsigned char *);
DEFPUBLIC(int) OgIntToOgReal(int, int, unsigned char *);
DEFPUBLIC(int) OgDoubleToOgReal(double, int, unsigned char *);
DEFPUBLIC(int) OgRealToUnsigned(unsigned char *, unsigned *, int);
DEFPUBLIC(int) OgRealToInt(unsigned char *, int *, int);
DEFPUBLIC(int) OgRealToDouble(unsigned char *, double *);
DEFPUBLIC(int) OgRealLength(unsigned char *);
DEFPUBLIC(int) OgRealToStruct(unsigned char *, struct og_dispatched_real *);
DEFPUBLIC(int) OgStringToOgReal(char *, int, unsigned char *);
DEFPUBLIC(int) OgRealToString(unsigned char *, int, char *);
DEFPUBLIC(int) OgRealToHumanString(unsigned char *, int, char *);
DEFPUBLIC(int) OgRoundReal(int, unsigned char *, int *, unsigned char *, int, int);

DEFPUBLIC(void *) OgFileSelectInit(char *ogm_conf, const char *exclude_path, char *where);
DEFPUBLIC(int) OgFileSelect(void *, char *, int (*)(void *, struct og_selected *), void *);
DEFPUBLIC(int) OgFileSelectLine(void *, int, char *, int (*)(void *, struct og_selected *), void *);
DEFPUBLIC(int) OgFileSelectFlush(void *);

DEFPUBLIC(void *) OgFileSelectZoneInit(char *, char *, char *);
DEFPUBLIC(int) OgFileSelectZoneLine(void *, int, char *, int (*)(void *, struct og_selected *), void *);
DEFPUBLIC(int) OgFileSelectZoneFlush(void *);

DEFPUBLIC(int) OgDelPrefixInit(struct og_del_prefix **);
DEFPUBLIC(int) OgDelPrefixEqual(struct og_del_prefix *, unsigned short *);

DEFPUBLIC(int) OgCrypt(int, unsigned char *, unsigned char *, int, unsigned char *);
DEFPUBLIC(int) OgCryptMimeEncode(int, unsigned char *, int *, unsigned char *, int, unsigned char *);
DEFPUBLIC(int) OgCryptMimeDecode(int, unsigned char *, int *, unsigned char *, int, unsigned char *);

DEFPUBLIC(int) OgUeToUb(int, unsigned char *, unsigned short *);
DEFPUBLIC(int) OgUbToUe(int, unsigned short *, unsigned char *);

#if (DPcSystem == DPcSystemWin32)
DEFPUBLIC(int) OgEditBox(HINSTANCE, int, char *, char *, int, int);
DEFPUBLIC(int) OgPidName(unsigned, int, char *);
#endif

DEFPUBLIC(int) OgGetLongFileName(char *,  char *);

DEFPUBLIC(void) TokSgmlToUnicode(unsigned char *, unsigned short *);

DEFPUBLIC(char *) OgUnixArch(void);

DEFPUBLIC(int) OgGetPeriod(char *, struct og_period *);
DEFPUBLIC(int) OgGetSpanPeriod(struct og_span_period *, time_t, struct og_period *, struct og_span_period *);
DEFPUBLIC(int) OgGetEndPeriod(time_t, struct og_period *, time_t *);

DEFPUBLIC(int) OgArgDate(int, char *, struct og_date *);

DEFPUBLIC(int) OgTrimString(unsigned char *, unsigned char *);
DEFPUBLIC(int) OgTrimStringChar(unsigned char *, unsigned char *, int);
DEFPUBLIC(int) OgTrimUnicode(int, unsigned char *, int *, unsigned char *);
DEFPUBLIC(int) OgTrimUnicodeChar(int, unsigned char *, int *, unsigned char *, int);
DEFPUBLIC(int) OgReverseString(unsigned char *, unsigned char *);
DEFPUBLIC(int) OgZoneOverlap(int start1, int length1, int start2, int length2);
DEFPUBLIC(int) OgStrCpySized(unsigned char *dest_buffer, int dest_buffer_size, const unsigned char *src_string, int src_string_length);
DEFPUBLIC(int) OgStrCpySizedTerm(unsigned char *dest_buffer, int dest_buffer_size, const unsigned char *src_string, int src_string_length);

DEFPUBLIC(int) OgHtmlToPagecode(unsigned char *, unsigned char *, int *, int);
DEFPUBLIC(int) OgHtmlToPagecodeTest(void);

DEFPUBLIC(int) OgConfGetVarLines(char *, char *, struct og_conf_lines *, char *);
DEFPUBLIC(int) OgConfGetVarLinesFlush(struct og_conf_lines *);

#define OgErrMsg(hErr,fmt,...) _OgErrMsg(hErr,__FILE__,__LINE__, fmt, __VA_ARGS__)
DEFPUBLIC(int) _OgErrMsg(void *handle, char *srcFile, int line, char *format, ...);
DEFPUBLIC(void *) OgErrInit(void);
DEFPUBLIC(int) OgErrFlush(void *);
DEFPUBLIC(int) OgErr(void *, char *);
DEFPUBLIC(int) OgErrLast(void *, char *, time_t *);
DEFPUBLIC(int) OgErrReset(void *);
DEFPUBLIC(int) OgErrNumber(void *);
DEFPUBLIC(int) OgErrGetIeme(void *ptr, char *last_erreur, unsigned ieme_error, time_t *ptime);
DEFPUBLIC(size_t) OgErrGetAllocatedMemory(void *handle);

DEFPUBLIC(int) OgErrLog(void *, char *, int, int, char *, int);
DEFPUBLIC(void) OgGetSystemError(int *, char *);

DEFPUBLIC(int) OgGetOriginalName(int, int, char *, int, int *, char *);
DEFPUBLIC(int) OgDownloadedFilenameToAssociatedFilename(unsigned char *downloaded_filename, unsigned char* associated_filename);

DEFPUBLIC(int) OgGetHardDriveSerialNumber(char *);

DEFPUBLIC(int) OgReadDirAlias(void *, char *, struct og_dir_aliases *);

DEFPUBLIC(int) OgGetDiskInfo(char *, unsigned *, unsigned *);

DEFPUBLIC(int) OgGetConfServerAddress(char *, char *, char *, int *);
DEFPUBLIC(int) OgParseServerAddress(const char *, char *, int *);

DEFPUBLIC(int) OgGetStringDate(char *, time_t *);

DEFPUBLIC(int) OgChangeDate(void *, char *, time_t);

DEFPUBLIC(int) OgStatForDir(struct og_file *, unsigned, struct og_stat *);

DEFPUBLIC(int) OgValidFilename(int, char *);
DEFPUBLIC(int) OgValidateFilename(int, char *, int);

DEFPUBLIC(int) OgDosToAnsi(unsigned char);
DEFPUBLIC(int) OgStringDosToAnsi(unsigned char *);
DEFPUBLIC(int) OgAnsiToDos(unsigned char);
DEFPUBLIC(int) OgStringAnsiToDos(unsigned char *);

DEFPUBLIC(int) OgLocalize(int, char *, char *, char *, char *);

DEFPUBLIC(int) OgHaversineDistance(double, double, double, double, double, double *, int);

DEFPUBLIC(int) OgSrand(unsigned);
DEFPUBLIC(int) OgRand(unsigned, unsigned *);

//-- OS tools
DEFPUBLIC(char *)           OgGetOsInfo(void);

//-- File tools
DEFPUBLIC(FILE *)           OgHandleToFile(NATIVE_FILE handle,char *mode);
DEFPUBLIC(char *)           OgDirName(char *filename);
DEFPUBLIC(char *)           OgBaseName(char *filename);
DEFPUBLIC(int)              OgFileLock(NATIVE_FILE hFile,int mode);
DEFPUBLIC(int)              OgFileSync(NATIVE_FILE hFile);
DEFPUBLIC(int)              OgFileTruncate(NATIVE_FILE hFile, int size);

//-- Time function

DEFPUBLIC(int)              OgElapseTime(struct timeval *result, struct timeval *stop, struct timeval *start);
DEFPUBLIC(void)             OgUsleep(unsigned long microseconds);
//-- Error function
DEFPUBLIC(int)              OgErrno(void);
DEFPUBLIC(char *)           OgWriteErrnoMsg(void); // DEPRECATED
DEFPUBLIC(char *)           OgStrError(char *msg);
DEFPUBLIC(int)              OgGetTimeOfDay(struct timeval *tvp, void *tzp);
DEFPUBLIC(struct tm *)      OgLocalTime(const time_t *, struct tm *);
DEFPUBLIC(char *)           OgGetTimeISO8601(char *buffer, int size);

//-- pipe
DEFPUBLIC(NATIVE_FILE)      OgOpenReaderPipe(void *herr, const char *fifoPath);
DEFPUBLIC(NATIVE_FILE)      OgOpenWriterPipe(void *herr, const char *fifoPath);
DEFPUBLIC(int)              OgClosePipe(void *herr, NATIVE_FILE fifoID);
DEFPUBLIC(int)              OgWritePipe(void *herr, NATIVE_FILE fifoID, const char *data, int size);
DEFPUBLIC(int)              OgReadPipe(void *herr, NATIVE_FILE fifoID, char *buffer, int size);
DEFPUBLIC(int)              OgDeletePipe(void *herr, const char *fifoPath);

//  Mutex (Process)
DEFPUBLIC(PROCESS_MUTEX)    OgInitProcMutex(char *mutex_name);
DEFPUBLIC(int)              OgLockProcMutex(PROCESS_MUTEX mutex_id);
DEFPUBLIC(int)              OgTryLockProcMutex(PROCESS_MUTEX mutex_id);
DEFPUBLIC(int)              OgUnlockProcMutex(PROCESS_MUTEX mutex_id);

// Mutex (Threads)
DEFPUBLIC(void)             OgInitMutex(THREAD_MUTEX *pm);
DEFPUBLIC(void)             OgDelMutex(THREAD_MUTEX *pm);
DEFPUBLIC(void)             OgLockMutex(THREAD_MUTEX *pm);
DEFPUBLIC(int)              OgTryLockMutex(THREAD_MUTEX *pm);
DEFPUBLIC(void)             OgUnlockMutex(THREAD_MUTEX *pm);

// Conditionnal Mutex (Threads)
DEFPUBLIC(void)             OgInitCondMutex(THREAD_COND *cv);
DEFPUBLIC(void)             OgDelCondMutex(THREAD_COND *cv);
DEFPUBLIC(void)             OgWaitCondMutex(THREAD_COND *cv, THREAD_MUTEX *external_mutex);
DEFPUBLIC(void)             OgSignalCondMutex(THREAD_COND *cv);
DEFPUBLIC(void)             OgBroadcastCondMutex(THREAD_COND *cv);

// instances management
DEFPUBLIC(int)              OgConfGetWorkingDirectory(char *value,int ivalue);

// Unicode to UTF-8 transformations see also OgUniToUTF8 and OgUTF8ToUni function in ogm_uni
DEFPUBLIC(int) OgUnisToUTF8(int iunicode, unsigned short *unicode, int scode, int *icode, unsigned char *code, int *position,int *truncated);
DEFPUBLIC(int) OgUTF8ToUnis(int icode, unsigned char *code, int sunicode, int *iunicode, unsigned short *unicode, int *position, int *truncated);

#define _OGMGENALIVE_

#endif

