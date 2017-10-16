/*
 *	Getting memory status.
 *	Encapsulation of OgGetMemoryStatus function
 *	Copyright (c) 1999	Ogmios by Patrick Constant
 *	Dev : November 1999
 *	Version 1.0
*/
#include <loggen.h>

#if (DPcSystem == DPcSystemUnix)
#include <unistd.h>

#if (DPcArch == DPcArchlinux)
#include <sys/sysinfo.h>

PUBLIC(int) OgGetMemoryStatus(struct og_memory_status *ogms)
{
int retour;
struct sysinfo info;

memset(ogms,0,sizeof(struct og_memory_status));
retour=sysinfo(&info); 
if (retour == -1) DONE;

OgSetBnum(&ogms->TotalPhys,    info.totalram,0,0);
OgSetBnum(&ogms->AvailPhys,    info.freeram,0,0);
OgSetBnum(&ogms->TotalVirtual, info.totalram+info.totalswap,0,0);
OgSetBnum(&ogms->AvailVirtual, info.freeram+info.freeswap,0,0);

DONE;
}

#else
#if (DPcArch == DPcArchsolaris)

PUBLIC(int) OgGetMemoryStatus(ogms)
struct og_memory_status *ogms;
{
memset(ogms,0,sizeof(struct og_memory_status));

OgSetBnum(&ogms->TotalPhys,sysconf(_SC_PHYS_PAGES),0,0);
OgSetBnum(&ogms->AvailPhys,sysconf(_SC_AVPHYS_PAGES),0,0);

IFE(OgMulBnum(&ogms->TotalPhys,sysconf(_SC_PAGESIZE),0));
IFE(OgMulBnum(&ogms->AvailPhys,sysconf(_SC_PAGESIZE),0));

DONE;
}

#else
#if (DPcArch == DPcArchtru64)
#include <sys/sysinfo.h>

PUBLIC(int) OgGetMemoryStatus(ogms)
struct og_memory_status *ogms;
{
int start=0;
long physmem;
unsigned high,low;

/** gives the physical memory in Kbytes **/
getsysinfo(GSI_PHYSMEM,(caddr_t)(&physmem),sizeof(long),&start,0,0);

high = (unsigned)(physmem>>32);
low = (unsigned)(physmem&0xffffffff);

memset(ogms,0,sizeof(struct og_memory_status));

OgHighLowToBnum(high,low,&ogms->TotalPhys);

IFE(OgMulBnum(&ogms->TotalPhys,0x400,0));

DONE;
}

#else
#if (DPcArch == DPcArchaix)
#include <sys/systemcfg.h>

PUBLIC(int) OgGetMemoryStatus(ogms)
struct og_memory_status *ogms;
{
int retour;
unsigned high,low;

memset(ogms,0,sizeof(struct og_memory_status));

high = (_system_configuration.physmem>>32);
low = (_system_configuration.physmem&0xffffffff);

IFE(OgHighLowToBnum(high,low,&ogms->TotalPhys));

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
 
PUBLIC(int) OgGetMemoryStatus(ogms)
struct og_memory_status *ogms;
{
MEMORYSTATUS ms;
memset( &ms, sizeof(MEMORYSTATUS), 0 );
ms.dwLength = sizeof(MEMORYSTATUS);
GlobalMemoryStatus( &ms );

ogms->MemoryLoad = ms.dwMemoryLoad;
OgSetBnum(&ogms->TotalPhys     , ms.dwTotalPhys     ,0,0);
OgSetBnum(&ogms->AvailPhys     , ms.dwAvailPhys     ,0,0);
OgSetBnum(&ogms->TotalPageFile , ms.dwTotalPageFile ,0,0);
OgSetBnum(&ogms->AvailPageFile , ms.dwAvailPageFile ,0,0);
OgSetBnum(&ogms->TotalVirtual  , ms.dwTotalVirtual  ,0,0);
OgSetBnum(&ogms->AvailVirtual  , ms.dwAvailVirtual  ,0,0);

DONE;
}
 
#endif
#endif

