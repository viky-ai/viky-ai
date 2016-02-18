/*
 *  Encapsulation of mkdir function
 *  Copyright (c) 2000  Ogmios
 *  Dev : March 2000
 *  Version 1.0
*/
#include <loggen.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>



#if (DPcSystem == DPcSystemUnix)
#include <fcntl.h>
#include <unistd.h>



PUBLIC(int) OgMkdir(char *pathname, int mode)
{
return(mkdir(pathname,mode));
}


PUBLIC(int) OgIsDir(int st_mode)
{
return(st_mode&S_IFDIR);
}


#else
#if (DPcSystem == DPcSystemWin32)
#include <fcntl.h>
#include <direct.h>


PUBLIC(int) OgMkdir(pathname,mode)
char *pathname;
int mode;
{
return(mkdir(pathname));
}


PUBLIC(int) OgIsDir(st_mode)
int st_mode;
{
return(st_mode&S_IFDIR);
}


#endif
#endif

