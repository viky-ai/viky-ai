/*
 *  This program removes all links from masculine to feminine
 *  by setting feminine roots, this is when we do not want such rules.
 *  Copyright (c) 2007 Pertimm by Patrick Constant
 *  Dev : June 2007
 *  Version 1.0
*/
#include <loggen.h>
#include <logthr.h>
#include <logaut.h>
#include <logpath.h>

#define DOgFeminTraceMinimal  0x1
#define DOgFeminTraceMemory   0x2
#define DOgFeminTraceChange   0x4

#define DOgFeminBanner "ogfemin V1.00, Copyright (c) 2007 Pertimm, Inc."

struct og_info {
  void *herr; ogmutex_t *hmutex; struct og_loginfo loginfo;
  char string1[DPcPathSize],string2[DPcPathSize];
  void *ha; FILE *fdout;
  };


STATICF(int) OgFemin(pr(struct og_info *));
STATICF(int) OgFemin1(pr_(struct og_info *) pr_(int) pr_(unsigned char *) pr(int));
STATICF(int) OgUse(pr_(struct og_info *) pr_(void *) pr(int));
STATICF(void) OgExit(pr_(void *) pr(char *));



#if ( DPcSystem == DPcSystemUnix)
int main (int argc, char * argv[])
{
char *argv0 = argv[0];
void *hInstance=0;
int nCmdShow=0;
int i;

#else
#if ( DPcSystem == DPcSystemWin32)

LONG APIENTRY MainWndProc(pr_(HWND) pr_(UINT) pr_(UINT) pr(LONG));


int WINAPI WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR sCmdParameters, int nCmdShow)
{
int pos=0;
char *argv0=0;

#endif
#endif

char cmd_param[DPcPathSize];
struct og_info cinfo,*info=&cinfo;
char cwd[DPcPathSize],cmd[DPcPathSize];
ogmutex_t mutex;
int must_exit=0;
time_t ltime;
char *nil;

memset(info,0,sizeof(struct og_info));

info->loginfo.trace = DOgFeminTraceMinimal+DOgFeminTraceMemory;
info->loginfo.where = "ogfemin";
/** main thread error handle **/
IFn(info->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,info->loginfo.where,0,"ogfemin: OgErrInit error");
  return(1);
  }
/** general mutex handle **/
info->hmutex=&mutex;
IF(OgInitCriticalSection(info->hmutex,"ogfemin")) {
  OgExit(info->herr,info->loginfo.where); return(1);
  }

strcpy(cmd,"ogfemin");

//OgGetExecutablePath(argv0,cwd); chdir(cwd);

#if ( DPcSystem == DPcSystemUnix)

for (i=1; i<argc; i++) {
  strcpy(cmd_param,argv[i]);

#else
#if ( DPcSystem == DPcSystemWin32)

while(OgGetCmdParameter(sCmdParameters,cmd_param,&pos)) {
#endif
#endif

  sprintf(cmd+strlen(cmd)," %s",cmd_param);
  if (!strcmp(cmd_param,"-v")) {
    OgMessageBox(0,DOgFeminBanner,info->loginfo.where,DOgMessageBoxInformation);
    must_exit=1;
    }
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(info,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (!memcmp(cmd_param,"-t",2)) {
    info->loginfo.trace = strtol(cmd_param+2,&nil,16);
    }
  else if (cmd_param[0] != '-') {
    if (info->string1[0]==0) strcpy(info->string1,cmd_param);
    else strcpy(info->string2,cmd_param);
    }
  }

if (must_exit) return(0);

IF(OgCheckOrCreateDir(DOgDirLog,0,info->loginfo.where)) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,info->loginfo.where,0
    ,"Can't create directory '%s'",DOgDirLog);
  return(1);
  }

if (info->loginfo.trace&DOgFeminTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
     ,"\nProgram %s.exe starting with pid %x at %.24s"
       , info->loginfo.where, getpid(), ctime(&ltime));
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0, "Command line: %s", cmd);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0, "%s",DOgFeminBanner);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0,"Current directory is '%s'",cwd);
  }

IF(OgFemin(info)) {
  OgExit(info->herr,info->loginfo.where); return(1);
  }

IF(OgFlushCriticalSection(info->hmutex)) {
  OgExit(info->herr,info->loginfo.where); return(1);
  }
OgErrFlush(info->herr);

if (info->loginfo.trace&DOgFeminTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
    ,"\nProgram %s.exe exiting at %.24s\n",info->loginfo.where,ctime(&ltime));
  }

return(0);
}





STATICF(int) OgFemin(info)
struct og_info *info;
{
struct og_aut_param caut_param,*aut_param=&caut_param;
int iline; char line[DPcPathSize];
FILE *fd;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=info->herr;
aut_param->hmutex=info->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = info->loginfo.where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"femin");
IFn(info->ha=OgAutInit(aut_param)) DPcErr;

IFn(info->fdout=fopen(info->string2,"w")) {
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
    , "OgFemin: impossible to fopen '%s' for writing", info->string2);
  DONE;
  }

IFn(fd=fopen(info->string1,"r")) {
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
    , "OgFemin: impossible to fopen '%s'", info->string1);
  DONE;
  }

/** First we build the masc -> femin automaton **/
while(fgets(line,DPcPathSize,fd)) {
  iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
  IFE(OgFemin1(info,iline,line,1));
  }

rewind(fd);

/** then we build the transformed file **/
while(fgets(line,DPcPathSize,fd)) {
  iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
  IFE(OgFemin1(info,iline,line,2));
  }

fclose(fd);
fclose(info->fdout);

IFE(OgAutFlush(info->ha));

DONE;
}



/* Format is:
 *  abaissante::abaissant+Afpfs-
 *  abaissantes::abaissant+Afpfp-
 *  abaisses::abaisse+Ncfp--
 *  abaisseuse::abaisseur+Afpfs-
 *  abaisseuses::abaisseur+Afpfp-
 *  abandonn�e::abandonn�+Afpfs-
 *  abandonn�e::abandonn�+Ncfs--
 *  abandonn�es::abandonn�+Afpfp-
 *  abandonn�es::abandonn�+Ncfp--
 *  abasies::abasie+Ncfp--
*/

STATICF(int) OgFemin1(info,is,s,pass)
struct og_info *info;
int is; unsigned char *s;
int pass;
{
int i,c,state=1,end=0,start=0;
int ibuffer=0; char buffer[DPcPathSize];
int iform=0; char form[DPcPathSize];
int irac=0; char rac[DPcPathSize];
int isyn=0; char syn[DPcPathSize];


for (i=0; !end; i++) {
  if (i>=is) { c=0; end=1; }
  else c=s[i];
  switch (state) {
    /** before abaissante **/
    case 1:
      if (c==':') { form[iform]=0; state=2; }
      else form[iform++]=c;
      break;
    case 2:
      if (c==':') state=2;
      else { rac[irac++]=c; state=3; }
      break;
    case 3:
      if (c=='+') { rac[irac]=0; state=4; }
      else rac[irac++]=c;
      break;
    case 4:
      syn[isyn++]=c;
      break;
    }
  }

if (info->loginfo.trace&DOgFeminTraceChange) {
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
    , "OgFemin1: pass=%d form='%s' rac='%s' syn='%s'", pass, form, rac, syn);
  }

if (pass==1) {
  if (!memcmp(syn,"Afpfs",5) || !memcmp(syn,"Ncfs",4)) {
    sprintf(buffer,"%s:%s:%s",rac,form,syn); ibuffer=strlen(buffer);
    IFE(OgAutAdd(info->ha,ibuffer,buffer));
    }
  }
else if (pass==2) {
  int retour,nstate0,nstate1,iout,colon=(-1),found=0;
  unsigned char out[DPcAutMaxBufferSize+10];
  oindex states[DPcAutMaxBufferSize+10];
  sprintf(buffer,"%s:",rac);
  if ((retour=OgAutScanf(info->ha,strlen(buffer),buffer,&iout,out,&nstate0,&nstate1,states))) {
    do {
      IFE(retour);
      for (i=0; i<iout; i++) {
        if (out[i]==':') colon=i;
        }
      if (colon<0) continue;
      if (out[colon+1]!=syn[0]) continue;
      fprintf(info->fdout,"%s::%.*s+%s\n",form,colon,out,syn);
      found=1; break;
      }
    while((retour=OgAutScann(info->ha,&iout,out,nstate0,&nstate1,states)));
    }
  if (!found) {
    fprintf(info->fdout,"%s\n",s);
    }
  }

DONE;
}





STATICF(int) OgUse(info,hInstance,nCmdShow)
struct og_info *info;
void *hInstance;
int nCmdShow;
{
char buffer[8192],edit_buffer[8192];
#if ( DPcSystem == DPcSystemWin32)
int j,k;
#endif

sprintf(buffer,               "Usage : ogfemin [-v] [-h] [options] <file1> <file2>\n");
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"    -h prints this message\n");
sprintf(buffer+strlen(buffer),"    -t<n>: trace options for logging (default 0x%x)\n",info->loginfo.trace);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"        0x1: minimal, 0x2: memory, 0x4: extract\n");
sprintf(buffer+strlen(buffer),"    -v gives version number of the program\n");

#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"oggl help",edit_buffer,420,200);
#else
OgMessageBox(0,buffer,info->loginfo.where,DOgMessageBoxInformation);
#endif

DONE;
}



STATICF(void) OgExit(herr,where)
void *herr; char *where;
{
int is_error;
is_error=OgErrLog(herr,where,0,0,0,0);
}


