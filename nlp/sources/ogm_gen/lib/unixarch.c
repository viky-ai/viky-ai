/*
 *	Gives the unix architecture
 *	Copyright (c) 2003 Pertimm by Patrick Constant
 *	Dev : July 2003
 *	Version 1.0
*/
#define DPcInDll
#include <loggen.h>




PUBLIC(char *) OgUnixArch(void)
{
unsigned char *arch;
#if (DPcArch == DPcArchlinux)
arch="linux";
#else
#if (DPcArch == DPcArchsolaris) 
arch="solaris";
#else
#if (DPcArch == DPcArchtru64)
arch="true64";
#else
arch="aix";
#if (DPcArch == DPcArchaix) 
#else
DPcArch must be defined 
#endif
#endif
#endif
#endif
return(arch);
}

