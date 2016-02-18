/*
 *  IO function for files. This list of function is suppose to be plateform dependent. 
 *  The ANSI version (fopen/fseek/write/read/fclose) is quite plateform
 *  independant, but is limited (Linux, Windows 95/98/NT) to 2 giga-bytes files.
 *  The Win32 API (CreateFile/SetFilePointer/WriteFile/ReadFile/CloseHandle)
 *  does not have this limitation.
 *  The Solaris and Linux Transitory version (fopen/fseeko/write/read/fclose)
 *  is working provided option -D_LARGEFILE_SOURCE `getconf LFS_CFLAGS`
 *  are given and (for linux) kernel is 2.4 or higher to handle 
 *  files that are larger than 2 giga-bytes.
 *  Copyright (c) 2000-2002	Pertimm, Inc. by Patrick Constant
 *  Dev : September 2000, May 2001, March 2002
 *  Version 1.2
*/
#include <loggen.h>
#include <logstat.h>

/* CreateFile/SetFilePointer/WriteFile/ReadFile/CloseHandle
 * OgFsync is missing, but we can avoid using the Windows cache */
#define DOgIoWin1	1
/** 64 bit open/seek/tell/write/read/close **/
#define DOgIoWin2	2
/* buffered 64 bit open/seek/tell/write/read/close
 * used for 3 years without any problem */
#define DOgIoWin3	3


#define DOgIoWin	DOgIoWin1


#if (DPcSystem == DPcSystemUnix)

#include "ogiounx.c"

#else
#if (DPcSystem == DPcSystemWin32)

#if (DOgIoWin == DOgIoWin1) 
#include "ogiowin1.c"
#else
#if (DOgIoWin == DOgIoWin2) 
#include "ogiowin2.c"
#else
#if (DOgIoWin == DOgIoWin3) 
#include "ogiowin3.c"
#else
bad value for macro DOgIoWin 
#endif
#endif
#endif
#endif

#endif


