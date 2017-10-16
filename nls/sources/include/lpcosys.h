/*
 *	Gestion des diff\'erentes machines/syst\`emes
 *	Copyright (c) 1996-2000	Ogmios by Patrick Constant
 *	Dev : Janvier,Septembre 1996, Mars 1997, Janvier,Septembre 1998
 *	Dev : Janvier,Mai 2000
 *	Version 1.6
*/

#ifndef _LPCOSYSALIVE_

#define DPcIncludeBanner  "Include V1.93, Copyright (c) 1997-2003 Pertimm, Inc."
#define DPcIncludeVersion 193

/*
 *	Valeurs pour la macro DPcSystem
*/
#define DPcSystemUnix	0	/** Unix **/
#define DPcSystemWin16	1	/** Windows 3.1 **/
#define DPcSystemWin32	2	/** Windows NT/95 **/

/*
 *	Valeurs de la macro DPcCompiler pour DPcSystem=DPcUnix
*/
#define DPcCompilerCc		0	/** compilateur cc natif **/
#define DPcCompilerGcc		1	/** compilateur gcc de GNU **/

/*
 *	Valeurs de la macro DPcCompiler pour DPcSystem=DPcWin32 ou DPcWin16
*/
#define DPcCompilerMicrosoft	2	/** compilateur Microsoft **/
#define DPcCompilerBorland	3	/** compilateur Borland **/

/*
 *	Valeurs de la macro DPcArch pour DPcSystem=DPcUnix
*/
#define DPcArchlinux	1
#define DPcArchsolaris	2
#define DPcArchtru64	3
#define DPcArchaix  	4

/*
 *	Macros classiques.
*/

#define NULLS		((char*)0)
#define NULL2S		((char**)0)
#define	FNULL		((FILE*)0)

/*
 *	D\'efinitions fines pour supporter les checks de type lint.
*/

/* Replace by DPcFclose to be on the safe size
#define Fclose(x)	    {if ((x) != FNULL) (void)fclose((x)); (x)=FNULL;}
*/
#define DPcFclose(x)	{if ((x) != 0) (void)fclose((x)); (x)=0;}
#define Strcpy(x,y)	    (void)strcpy((char*)(x),(char*)(y))
#define Memcpy(x,y,n)	(void)memcpy((char*)(x),(char*)(y),(unsigned int)(n))
#define Memcmp(x,y,n)	memcmp((char*)(x),(char*)(y),(int)(n))
#define Unlink(x)	    (void)unlink((char*)(x))
/** Windows plante avec la définition suivante **/
/* #define Free(x) 	    {if ((x)!=0) PcFree((void*)(x)); (x)=0;} */
#define DPcFree(x) 	    {if ((x)!=0) free(x); (x)=0;}
#define SortCast        (int (*)(pr_(char *) pr(char *)))
#define DPcSprintfSize	256


#if (DPcSystem == DPcSystemUnix)
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <lpcounx.h>
#else
#if (DPcSystem == DPcSystemWin16)
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <lpcowin.h>
#else
#if (DPcSystem == DPcSystemWin32)
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <lpcownt.h>
#else
ERREUR
#endif
#endif
#endif

#define _LPCOSYSALIVE_

#endif

