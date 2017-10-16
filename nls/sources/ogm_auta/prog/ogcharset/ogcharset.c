/*
 *	This example is used to get a charset value from a charset or reverse. 
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : November 2007
 *	Version 1.0
*/
#include <logauta.h>



STATICF(void) use(pr(void));
STATICF(void) DoExit(pr_(struct og_charset_param *) pr(int));




int main(argc,argv)
int argc; char *argv[];
{
struct og_charset_param cparam, *param=&cparam;
char *nil,charset[DPcPathSize];
int i,code,trace;
ogmutex_t mutex;
void *hcharset;

memset(param,0,sizeof(struct og_charset_param));
param->loginfo.trace = DOgAutaTraceMinimal+DOgAutaTraceMemory; 
param->loginfo.where = "ogcharset";
/** main thread error handle **/
IFn(param->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0,"ogcharset: OgErrInit error");
  return(1);
  }
/** general mutex handle **/
param->hmutex=&mutex;
IF(OgInitCriticalSection(param->hmutex,"ogcharset")) DoExit(param,1); 

IFn(hcharset=OgCharsetInit(param)) DoExit(param,1);

for (i=1; i<argc; i++) {
  if (argv[i][0]!='-') {
    IF(OgCharsetGetCode(hcharset,argv[i],&code)) DoExit(param,1);
    printf("%d\n",code);
    }
  else if (argv[i][1]=='h') { use(); exit(0); }
  else if (argv[i][1]=='n') { 
    code=atoi(argv[i]+2);
    IF(OgCharsetGetString(hcharset,code,charset)) DoExit(param,1);
    printf("%s\n",charset);
    }
  else if (argv[i][1]=='t') {
    trace = strtol(argv[i]+2,&nil,16);
    } 
  }

IF(OgCharsetFlush(hcharset)) DoExit(param,1);

IF(OgFlushCriticalSection(param->hmutex)) DoExit(param,1);
OgErrFlush(param->herr);

DONE;
}





STATICF(void) use()
{
printf("Usage : ogcharset [options] [-n<number> | <charset>]\n");
printf("  -h: this message\n");
printf("  -t<n>: trace options for logging\n");
printf("    <n> has a combined hexadecimal value of:\n");
printf("    0x1: minimal\n");
}





STATICF(void) DoExit(param,retour)
struct og_charset_param *param;
int retour;
{
int is_error;
is_error=OgErrLog(param->herr,param->loginfo.where,0,0,0,0);
exit(retour);
}


