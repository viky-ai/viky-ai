/*
 *	Quelques macros standard pour se d\'efinir le style de la programmation
 *	Copyright (c) 1996,97,98	Patrick Constant
 *	Dev : Janvier 1996, Mars 1997, October 1998, August 1999, May 2003
 *	From ogm_gen.dll
 *	Version 1.4
*/

#ifndef _LPCMCRALIVE_


#define ERREUR          (-1)

#define CORRECT         (0)
#define CONTINUE        (1)
#define BREAK           (2)

/** DPcErr must replace ERR, and ERR should disappear **/
/** because ERR is defined in some system headers **/
//#define DPcErr	        { FILE *fd; fd=fopen("erreur.log","a"); fprintf(fd,"file '%s', line %d\n",__FILE__,__LINE__); fclose(fd); return(ERREUR); }
// Show stack trace when error occurred
//#include <execinfo.h>
//#define DPcErr          { FILE *DPT_fd = fopen("trace.log","a"); void * DPT_trace[255]; int DPT_i, DPT_trace_size = 0; char **DPT_messages = NULL; DPT_trace_size = backtrace(DPT_trace, sizeof(DPT_trace)); DPT_messages = backtrace_symbols(DPT_trace, DPT_trace_size); fprintf(DPT_fd, "ERROR : in file '%s', line %d\n", __FILE__, __LINE__ ); for (DPT_i = 0; DPT_i < DPT_trace_size; DPT_i++) { fprintf(DPT_fd, "\t%s\n", DPT_messages[DPT_i]); }; free(DPT_messages); fclose(DPT_fd); return(ERREUR); }
#define DPcErr	        return(ERREUR)
/** Removed May 3rd 2003 **/
//#define ERR             DPcErr
#define DONE            return(CORRECT)
#define CONT            return(CONTINUE)
#define STOP            return(BREAK)

#define IF(x)           if ((int)(x) == ERREUR)
#define NIF(x)          if ((int)(x) != ERREUR)
#define IFE(x)          if ((int)(x) == ERREUR) DPcErr

#define IFn(x)          if ((x) == 0)
#define IFx(x)          if ((x) != 0)
#define IFN(x)          if ((x) == NULL)
#define IFX(x)          if ((x) != NULL)

/* Conflicts too often with stdlib.h (23/10/98)
#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#define abs(a)          (((a) < 0) ? -(a) : (a))
*/

#define DPcPathSize	1024

typedef unsigned long lindex;
typedef unsigned oindex;


#define _LPCMCRALIVE_

#endif
