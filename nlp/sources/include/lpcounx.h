/*
 *  Systeme Unix : DPcSystem=DPcSystemUnix
 *  Copyright (c) 1996,97,98  Patrick Constant
 *  Dev : Janvier,Septembre 1996, Mars 1997, September 1998
 *  Dev : May 2000
 *  Version 1.4
*/

#include <sys/types.h>
#include <sys/stat.h>

#define DPcSprintf  sprintf

#define DPcDirectorySeparator  "/"

#if (DPcCompiler == DPcCompilerCc)
#define KILLPROTO
#endif

#ifdef  KILLPROTO
# define pr_(x)
# define pr(x)
# define prfs(x)
# define prfs2(x)
# define prfe
# define prfe_
# define prcast(x)  x
#else
# define pr_(x)    x,
# define pr(x)    x
# define prfs(x)  x (*)(
# define prfs2(x)  x (**)(
# define prfe    )
# define prfe_    ),
# define prcast(x)  (x)
# define CHARPROTO  int
# define UCHARPROTO  int
# define SHORTPROTO  int
# define USHORTPROTO  int
#endif

#define STATICV(x)  static x
#define STATICF(x)  static x
#define STATICFF(x)  static x
#define GLOBALDEF(x)  x
#define GLOBALREF(x)  extern x
#define PUBLIC(x)  __attribute__((visibility("default"))) x
#define EXPORTF(x)  x
#define  PRIVATE(x)  x

#ifdef DPcCpp
# define DEFPUBLIC(x)  extern "C" x
#else
# define DEFPUBLIC(x)  __attribute__((visibility("default"))) x
#endif



