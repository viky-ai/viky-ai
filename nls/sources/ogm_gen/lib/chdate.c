/*
 *	Changes the date of a file.
 *	Copyright (c) 2005 Pertimm by Patrick Constant
 *	Dev : April 2005
 *	Version 1.0
*/
#include <loggen.h>



#if (DPcSystem == DPcSystemWin32)
#include <sys/utime.h> 
#else
#if (DPcSystem == DPcSystemUnix)
#include <sys/types.h>
#include <utime.h>
#endif
#endif


/*
 * Returns 0 if date is changed and 1 otherwise
*/

PUBLIC(int) OgChangeDate(void *herr, char *filename, time_t date)
{
char erreur[DOgErrorSize];
struct utimbuf modif_time;

modif_time.actime=date; modif_time.modtime=date;

IF(utime(filename, &modif_time)) {
  sprintf(erreur,"OgChangeDate: impossible to change date for '%s'",filename);
  OgErr(herr,erreur); DPcErr;
  }

DONE;
}

