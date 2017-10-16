/*
 *  Utilities for getting disk physical information
 *  Copyright (c) 2005	Pertimm, Inc. by Patrick Constant
 *  Dev : March 2005
 *  Version 1.0
*/
#include <loggen.h>
#include <logstat.h>



#if (DPcSystem == DPcSystemUnix)


/*
 *  This function is not yet necessary under Unix.
*/

PUBLIC(int) OgGetDiskInfo(char *name, unsigned int *pdwBytesPerSector, unsigned int *pdwSectorsPerCluster)
{
if (pdwBytesPerSector) *pdwBytesPerSector = 0;
if (pdwSectorsPerCluster) *pdwSectorsPerCluster = 0;

DONE;
}



#else
#if (DPcSystem == DPcSystemWin32)


/*
 *  Gets disk clustors and sectors information.
 *  We must get format such as "f:\" or "\\MyServer\MyShare\"
 *  both format must include a trailing backslash.
*/

PUBLIC(int) OgGetDiskInfo(name,pdwBytesPerSector,pdwSectorsPerCluster)
char *name; unsigned *pdwBytesPerSector, *pdwSectorsPerCluster;
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

OgMessageLog(DOgMlogInLog,"ogm_ocea", 0, "OgGetDiskInfo on '%s'",dirname);

if (!GetDiskFreeSpace(dirname,
		  &dwSectorsPerCluster,
		  &dwBytesPerSector,
		  &dwFreeClusters,
		  &dwTotalClusters)) {
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgGetDiskInfo: error GetDiskFreeSpace: %s",sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }

if (pdwBytesPerSector) *pdwBytesPerSector = dwBytesPerSector;
if (pdwSectorsPerCluster) *pdwSectorsPerCluster = dwSectorsPerCluster;

DONE;
}


#endif
#endif


