/*
 *  This is main procedure for ogaut.exe
 *  Copyright (c) 1996-2007 Pertimm by Patrick Constant
 *  Dev: April 2005, October 2007, August 2008
 *  Version 1.2
*/
#include <logaut.h>
#include <loguni.h>
#include <logpath.h>


#define DOgCommandTypeNil                   0
#define DOgCommandTypeCompile               1
#define DOgCommandTypeScan                  2
#define DOgCommandTypeRead                  3
#define DOgCommandTypeDump                  4
#define DOgCommandTypeAdd                   5
#define DOgCommandTypeEndian                6
#define DOgCommandTypeString                7
#define DOgCommandTypeMinimize              8
#define DOgCommandTypePretty                9
#define DOgCommandTypeUfu                   10
#define DOgCommandTypeTest                  11


struct og_info {
  struct og_aut_param *param;
  char string1[DPcPathSize];
  char string2[DPcPathSize];
  char ext[DPcPathSize];
  int automaton_type;
  int ling_automaton;
  int ling_flags;
  int command_type;
  int state_number;
  int nocharcase;
  int minimize;
  int unicode;
  int is_aug;
  void *haut;
  };


STATICF(int) Aut(pr(struct og_info *));
STATICF(int) AutCompile(pr(struct og_info *));
STATICF(int) AutScan(pr(struct og_info *));
STATICF(int) AutString(pr(struct og_info *));
STATICF(int) AutRead(pr(struct og_info *));
STATICF(int) AutDump(pr(struct og_info *));
STATICF(int) AutAdd(pr(struct og_info *));
STATICF(int) AutEndian(pr(struct og_info *));
STATICF(int) AutMinimize(pr(struct og_info *));
STATICF(int) AutPretty(pr(struct og_info *));
STATICF(int) AutUfu(pr(struct og_info *));
STATICF(int) AutTest(pr(struct og_info *));
STATICF(char *) AutExtensionString(pr(int));
STATICF(int) OgUse(pr_(struct og_info *) pr_(void *) pr(int));
STATICF(void) DoExit(pr_(void *) pr_(char *) pr(int));




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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdParameters, int nCmdShow)
{
int pos=0;
char *argv0=0;

#endif
#endif

char cmd_param[8192];
char cwd[1024],cmd[8192];
struct og_info cinfo,*info=&cinfo;
struct og_aut_param cparam, *param=&cparam;
ogmutex_t mutex;
int must_exit=0;
time_t ltime;
char *nil;

memset(info,0,sizeof(struct og_info));
info->automaton_type=DOgAutNormal;
info->ling_flags=DPcAulDirect;
info->param=param;

memset(param,0,sizeof(struct og_aut_param));
param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
param->loginfo.where = "ogaut";
/** main thread error handle **/
IFn(param->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0,"ogaut: OgErrInit error");
  return(1);
  }
/** general mutex handle **/
param->hmutex=&mutex;
IF(OgInitCriticalSection(param->hmutex,"ogaut")) {
  DoExit(param->herr,param->loginfo.where,1);
  }

strcpy(cmd,"ogaut");

/** We want to use this program anywhere, so, we do not chdir **/
/* OgGetExecutablePath(argv0,cwd); chdir(cwd); */

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
    OgMessageBox(0,OgAutBanner(),param->loginfo.where,DOgMessageBoxInformation);
    must_exit = 1;
    }
  else if (!strcmp(cmd_param,"-a")) {
    info->command_type = DOgCommandTypeAdd;
    }
  else if (!strcmp(cmd_param,"-aut")) {
    info->automaton_type = DOgAutNormal;
    }
  else if (!strcmp(cmd_param,"-auf")) {
    info->automaton_type = DOgAutFast;
    }
  else if (!memcmp(cmd_param,"-c",2)) {
    info->command_type = DOgCommandTypeCompile;
    }
  else if (!memcmp(cmd_param,"-d",2)) {
    info->command_type = DOgCommandTypeDump;
    }
  else if (!memcmp(cmd_param,"-e",2)) {
    info->command_type = DOgCommandTypeEndian;
    }
  else if (!memcmp(cmd_param,"-g",2)) {
    info->is_aug = 1;
    }
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(info,hInstance,nCmdShow);
    must_exit = 1;
    }
  else if (!memcmp(cmd_param,"-l",2)) {
    info->ling_automaton = 1;
    info->ling_flags = strtol(cmd_param+2,&nil,16);
    IFn(info->ling_flags) info->ling_flags=DPcAulDirect;
    }
  else if (!memcmp(cmd_param,"-m",2)) {
    info->minimize = 1;
    }
  else if (!strcmp(cmd_param,"-ncc")) {
    info->nocharcase = 1;
    }
  else if (!memcmp(cmd_param,"-n",2)) {
    info->state_number = OgArgSize(cmd_param+2);
    }
  else if (!memcmp(cmd_param,"-p",2)) {
    info->command_type = DOgCommandTypePretty;
    }
  else if (!memcmp(cmd_param,"-q",2)) {
    info->command_type = DOgCommandTypeString;
    }
  else if (!memcmp(cmd_param,"-r",2)) {
    info->command_type = DOgCommandTypeRead;
    }
  else if (!memcmp(cmd_param,"-s",2)) {
    info->command_type = DOgCommandTypeScan;
    }
  else if (!memcmp(cmd_param,"-t",2)) {
    param->loginfo.trace = strtol(cmd_param+2,&nil,16);
    }
  else if (!strcmp(cmd_param,"-ufu")) {
    info->command_type = DOgCommandTypeUfu;
    }
  else if (!memcmp(cmd_param,"-u",2)) {
    info->unicode = 1;
    }
  else if (!memcmp(cmd_param,"-x",2)) {
    info->command_type = DOgCommandTypeTest;
    }
  else if (cmd_param[0] != '-') {
    IFn(info->string1[0]) strcpy(info->string1,cmd_param);
    else IFn(info->string2[0]) strcpy(info->string2,cmd_param);
    }
  }

if (must_exit) return(0);

IF(OgCheckOrCreateDir(DOgDirLog,0,param->loginfo.where)) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0
    ,"Can't create directory '%s'",DOgDirLog);
  return(1);
  }

if (param->loginfo.trace&DOgAutTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
     ,"\nProgram %s.exe starting with pid %x at %.24s"
       , param->loginfo.where, getpid(), OgGmtime(&ltime));
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0, "Command line: %s", cmd);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0, "%s",DOgAutBanner);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0,"Current directory is '%s'",cwd);
  }

IF(Aut(info)) {
  DoExit(param->herr,param->loginfo.where,1);
  }

IF(OgFlushCriticalSection(param->hmutex)) {
  DoExit(param->herr,param->loginfo.where,1);
  }
OgErrFlush(param->herr);

if (param->loginfo.trace&DOgAutTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
    ,"Program %s.exe exiting at %.24s\n",param->loginfo.where,OgGmtime(&ltime));
  }

DoExit(0,param->loginfo.where,0);
return(0);
}





STATICF(int) Aut(info)
struct og_info *info;
{
IFn(info->haut=OgAutInit(info->param)) DPcErr;

if (info->command_type==DOgCommandTypeNil && info->minimize) {
  info->command_type=DOgCommandTypeMinimize;
  }

switch (info->command_type) {
  case DOgCommandTypeCompile:
    IFE(AutCompile(info));
    break;
  case DOgCommandTypeScan:
    IFE(AutScan(info));
    break;
  case DOgCommandTypeRead:
    IFE(AutRead(info));
    break;
  case DOgCommandTypeDump:
    IFE(AutDump(info));
    break;
  case DOgCommandTypeAdd:
    IFE(AutAdd(info));
    break;
  case DOgCommandTypeEndian:
    IFE(AutEndian(info));
    break;
  case DOgCommandTypeString:
    IFE(AutString(info));
    break;
  case DOgCommandTypeMinimize:
    IFE(AutMinimize(info));
    break;
  case DOgCommandTypePretty:
    IFE(AutPretty(info));
    break;
  case DOgCommandTypeUfu:
    IFE(AutUfu(info));
    break;
  case DOgCommandTypeTest:
    IFE(AutTest(info));
    break;

  }

IFE(OgAutFlush(info->haut));
DONE;
}





STATICF(int) AutCompile(info)
struct og_info *info;
{
int i,istring1,found;
char erreur[DOgErrorSize];

IFn(info->string1[0]) {
  sprintf(erreur,"Usage: ogaut -c input_file [automaton_file]");
  OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(info->string2[0]) {
  istring1=strlen(info->string1); found=0;
  for (i=istring1-1; i>=istring1-4; i--) {
    if (info->string1[i]=='.') { found=1; break; }
    }
  if (found) {
    sprintf(info->string2,"%.*s",i,info->string1);
    }
  else {
    strcpy(info->string2,info->string1);
    }
  sprintf(info->string2+strlen(info->string2),".%s",AutExtensionString(info->automaton_type));
  }

OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
  ,"Compiling '%s' into '%s'",info->string1,info->string2);

if (info->ling_automaton) {
  IFE(OgAul(info->haut,info->string1,info->nocharcase,info->ling_flags));
  }
else if (info->unicode) {
  IFE(OgAutu(info->haut,info->string1));
  }
else {
  IFE(OgAut(info->haut,info->string1));
  }
if (info->minimize) {
   IFE(OgAum(info->haut));
  }

if (info->automaton_type==DOgAutNormal) {
  IFE(OgAutWrite(info->haut,info->string2));
  }
else if (info->automaton_type==DOgAutFast) {
  IFE(OgAuf(info->haut,info->is_aug));
  IFE(OgAufWrite(info->haut,info->string2));
  }

DONE;
}





STATICF(int) AutRead(info)
struct og_info *info;
{
FILE *fd;
int retour;
char erreur[DOgErrorSize];
int iline; char line[DPcPathSize];
char *result;

if (info->string1[0]==0 || info->string2[0]==0) {
  sprintf(erreur,"Usage: ogaut -r input_file automaton_file");
  OgErr(info->param->herr,erreur); DPcErr;
  }


IFn(fd=fopen(info->string1,"r")) {
  sprintf(erreur,"AutRead: impossible to open '%s'",info->string1);
  OgErr(info->param->herr,erreur); DPcErr;
  }

while(fgets(line,DPcPathSize,fd)) {
  iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
  if (info->automaton_type == DOgAutNormal) {
    IFE(OgAutRead(info->haut,info->string2));
    IFE(retour=OgAutTry(info->haut,-1,line));
    }
  else if (info->automaton_type == DOgAutFast) {
    IFE(OgAufRead(info->haut,info->string2));
    IFE(retour=OgAufTry(info->haut,-1,line));
    }
  if (retour==2) result="final";
  else if (retour==1) result="substring";
  else result="inexistant";
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"%s -> %s",line,result);
  }

fclose(fd);

DONE;
}





STATICF(int) AutScan(info)
struct og_info *info;
{
FILE *fd;
char erreur[DOgErrorSize];
int i,iline; char line[DPcPathSize];
int retour,nstate0,nstate1; oindex states[DPcAutMaxBufferSize];
unsigned char out[DPcAutMaxBufferSize];

if (info->string1[0]==0 || info->string2[0]==0) {
  sprintf(erreur,"Usage: ogaut -s input_file automaton_file");
  OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(fd=fopen(info->string1,"r")) {
  sprintf(erreur,"AutScan: impossible to open '%s'",info->string1);
  OgErr(info->param->herr,erreur); DPcErr;
  }
if (info->ling_automaton) {
  struct att_value out[120];
  int attributs=DPcAttribut_all;
  if (info->automaton_type == DOgAutNormal) {
    IFE(OgAutRead(info->haut,info->string2));
    }
  else if (info->automaton_type == DOgAutFast) {
    IFE(OgAufRead(info->haut,info->string2));
    }
  while(fgets(line,DPcPathSize,fd)) {
    iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
    IFE(OgAulScan(info->haut,attributs,line,out,120));
    for (i=0; out[i].type_attribut!=0; i++) {
      OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
        ,"  %s:%s:%s",out[i].entry, out[i].attribut, out[i].value);
      }
    }
  }
else if (info->automaton_type == DOgAutNormal) {
  IFE(OgAutRead(info->haut,info->string2));
  while(fgets(line,DPcPathSize,fd)) {
    iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
    if ((retour=OgAutScanf(info->haut,-1,line,0,out,&nstate0,&nstate1,states))) {
      OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s -> ",line);
      do { IFE(retour); OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s ",out); }
      while((retour=OgAutScann(info->haut,0,out,nstate0,&nstate1,states)));
      OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"");
      }
    else OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"%s not found",line);
    }
  }
else if (info->automaton_type == DOgAutFast) {
  IFE(OgAufRead(info->haut,info->string2));
  while(fgets(line,DPcPathSize,fd)) {
    iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
    if ((retour=OgAufScanf(info->haut,-1,line,0,out,&nstate0,&nstate1,states))) {
      OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s -> ",line);
      do { IFE(retour); OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s ",out); }
      while((retour=OgAufScann(info->haut,0,out,nstate0,&nstate1,states)));
      OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"");
      }
    else OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"%s not found",line);
    }
  }

fclose(fd);
DONE;
}




STATICF(int) AutString(info)
struct og_info *info;
{
char erreur[DOgErrorSize];
int i,retour,nstate0,nstate1; oindex states[DPcAutMaxBufferSize];
unsigned char out[DPcAutMaxBufferSize];

if (info->string1[0]==0 || info->string2[0]==0) {
  sprintf(erreur,"Usage: ogaut -q automaton_file string");
  OgErr(info->param->herr,erreur); DPcErr;
  }


if (info->ling_automaton) {
  struct att_value out[120];
  int attributs=DPcAttribut_all;
  if (info->automaton_type == DOgAutNormal) {
    IFE(OgAutRead(info->haut,info->string1));
    }
  else if (info->automaton_type == DOgAutFast) {
    IFE(OgAufRead(info->haut,info->string1));
    }

  IFE(OgAulScan(info->haut,attributs,info->string2,out,120));
  for (i=0; out[i].type_attribut!=0; i++) {
    OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
     ,"  %s:%s:%s",out[i].entry, out[i].attribut, out[i].value);
    printf("  %s:%s:%s",out[i].entry, out[i].attribut, out[i].value);
    }
  }
else if (info->automaton_type == DOgAutNormal) {
  IFE(OgAutRead(info->haut,info->string1));
  if ((retour=OgAutScanf(info->haut,-1,info->string2,0,out,&nstate0,&nstate1,states))) {
    OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s -> ",info->string2);
    do {
      IFE(retour);
      OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s ",out);
      printf("%s\n",out); fflush(stdout);
    }
    while((retour=OgAutScann(info->haut,0,out,nstate0,&nstate1,states)));
      OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"");
      }
    else OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"%s not found",info->string2);
  }
else if (info->automaton_type == DOgAutFast) {
  IFE(OgAufRead(info->haut,info->string1));
  if ((retour=OgAufScanf(info->haut,-1,info->string2,0,out,&nstate0,&nstate1,states))) {
    OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s -> ",info->string2);
    do {
      IFE(retour);
      OgMessageLog(DOgMlogInLog+DOgMlogNoCr,info->param->loginfo.where,0,"%s ",out);
      printf("%s\n",out); fflush(stdout);
      }
    while((retour=OgAufScann(info->haut,0,out,nstate0,&nstate1,states)));
    OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"");
    }
  else OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"%s not found",info->string2);
  }
 DONE;
}





STATICF(int) AutDump(info)
struct og_info *info;
{
int i,istring1,automaton_type,found,iout;
int retour,nstate0,nstate1; oindex states[DPcAutMaxBufferSize];
unsigned char out[DPcAutMaxBufferSize];
char erreur[DOgErrorSize];
FILE *fd;

IFn(info->string1[0]) {
  sprintf(erreur,"Usage: ogaut -d automaton_file [dump_file]");
  OgErr(info->param->herr,erreur); DPcErr;
  }

istring1=strlen(info->string1);
if (!strcmp(info->string1+istring1-4,".aut")) automaton_type=DOgAutNormal;
else if (!strcmp(info->string1+istring1-4,".aum")) automaton_type=DOgAutNormal;
else if (!strcmp(info->string1+istring1-4,".auf")) automaton_type=DOgAutFast;
else automaton_type=DOgAutNormal;

IFn(info->string2[0]) {
  for (found=0,i=istring1-1; i>=istring1-4; i--) {
    if (info->string1[i]=='.') { found=1; break; }
    }
  if (found) {
    sprintf(info->string2,"%.*s",i,info->string1);
    }
  else {
    strcpy(info->string2,info->string1);
    }
  sprintf(info->string2+strlen(info->string2),".dmp");
  }

IFn(fd=fopen(info->string2,"wb")) {
  sprintf(erreur,"AutDump: impossible to open '%s'",info->string2);
  OgErr(info->param->herr,erreur); DPcErr;
  }

OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
  ,"Dumping '%s' into '%s'",info->string1,info->string2);

if (automaton_type == DOgAutNormal) {
  IFE(OgAutRead(info->haut,info->string1));
  if ((retour=OgAutScanf(info->haut,-1,"",&iout,out,&nstate0,&nstate1,states))) {
    do {
      IFE(retour);
      #if ( DPcSystem == DPcSystemWin32)
      if (info->unicode) { out[iout++]=0; out[iout++]='\n'; }
      else { out[iout++]='\r'; out[iout++]='\n'; }
      #else
      #if(DPcSystem==DPcSystemUnix)
      if (info->unicode) { out[iout++]=0; out[iout++]='\n'; }
      else { out[iout++]='\n'; }
      #endif
      #endif
      fwrite(out,1,iout,fd);
      }
    while((retour=OgAutScann(info->haut,&iout,out,nstate0,&nstate1,states)));
    }
  }
else if (automaton_type == DOgAutFast) {
  IFE(OgAufRead(info->haut,info->string1));
  if ((retour=OgAufScanf(info->haut,-1,"",&iout,out,&nstate0,&nstate1,states))) {
    do {
      IFE(retour);
      #if ( DPcSystem == DPcSystemWin32)
      if (info->unicode) { out[iout++]=0; out[iout++]='\n'; }
      else { out[iout++]='\r'; out[iout++]='\n'; }
      #else
      #if(DPcSystem==DPcSystemUnix)
      if (info->unicode) { out[iout++]=0; out[iout++]='\n'; }
      else { out[iout++]='\n'; }
      #endif
      #endif
      fwrite(out,1,iout,fd);
      }
    while((retour=OgAufScann(info->haut,&iout,out,nstate0,&nstate1,states)));
    }
  }

fclose(fd);

DONE;
}




STATICF(int) AutAdd(info)
struct og_info *info;
{
FILE *fd;
char erreur[DOgErrorSize];
int iline; char line[DPcPathSize];
int i, istring1, found;

if (info->string1[0]==0) {
  sprintf(erreur,"Usage: ogaut -a input_file [automaton_file]");
  OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(info->string2[0]) {
  istring1=strlen(info->string1); found=0;
  for (i=istring1-1; i>=istring1-4; i--) {
    if (info->string1[i]=='.') { found=1; break; }
    }
  if (found) {
    sprintf(info->string2,"%.*s",i,info->string1);
    }
  else {
    strcpy(info->string2,info->string1);
    }
  sprintf(info->string2+strlen(info->string2),".%s",AutExtensionString(info->automaton_type));
  }
else {
  IFE(OgAutRead(info->haut,info->string2));
  }

IFn(fd=fopen(info->string1,"r")) {
  sprintf(erreur,"AutRead: impossible to open '%s'",info->string1);
  OgErr(info->param->herr,erreur); DPcErr;
  }

/* We can add or delete elements using:
 *   a word (to add a word)
 *   d word (to delete a word) */
while(fgets(line,DPcPathSize,fd)) {
  iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
  if (iline<=0) continue; if (line[0]!='a' && line[0]!='d') continue;
  for (i=1; i<iline; i++) {
    if (!isspace(line[i])) break;
    }
  if (i>=iline) continue;
  if (line[0]=='a') {
    IFE(OgAutAdd(info->haut,iline-i,line+i));
    }
  else if (line[0]=='d') {
    IFE(OgAutDel(info->haut,iline-i,line+i));
    }
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0,"%c %s",line[0],line+i);
  }

IFE(OgAutWrite(info->haut,info->string2));

fclose(fd);
DONE;
}





STATICF(int) AutEndian(info)
struct og_info *info;
{
IFE(OgAutEndian(info->haut));
DONE;
}





STATICF(int) AutMinimize(info)
struct og_info *info;
{
int i,istring1,found;
char erreur[DOgErrorSize];
int input_automaton_type;

IFn(info->string1[0]) {
  sprintf(erreur,"Usage: ogaut -m automaton_file [automaton_file]");
  OgErr(info->param->herr,erreur); DPcErr;
  }
istring1=strlen(info->string1);
input_automaton_type=DOgAutNormal;
if (!Ogstricmp(info->string1+istring1-4,".auf")) {
  input_automaton_type=DOgAutFast;
  }

IFn(info->string2[0]) {
  found=0;
  for (i=istring1-1; i>=istring1-4; i--) {
    if (info->string1[i]=='.') { found=1; break; }
    }
  if (found) {
    sprintf(info->string2,"%.*s",i,info->string1);
    }
  else {
    strcpy(info->string2,info->string1);
    }
  sprintf(info->string2+strlen(info->string2),".min.%s",AutExtensionString(info->automaton_type));
  }

OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
  ,"Minimizing '%s' into '%s'",info->string1,info->string2);

if (input_automaton_type==DOgAutNormal) {
  IFE(OgAutRead(info->haut,info->string1));
  }
else {
  IFE(OgAufRead(info->haut,info->string1));
  IFE(OgAufToAut(info->haut));
  }

IFE(OgAum(info->haut));

if (info->automaton_type==DOgAutNormal) {
  IFE(OgAutWrite(info->haut,info->string2));
  }
else if (info->automaton_type==DOgAutFast) {
  IFE(OgAuf(info->haut,info->is_aug));
  IFE(OgAufWrite(info->haut,info->string2));
  }

DONE;
}




STATICF(int) AutPretty(info)
struct og_info *info;
{
unsigned char pretty[]="0123456789ABCDEFGHIJKLMNOPQR#TUVWXYZ";
char erreur[DOgErrorSize];
int iB; unsigned char *B;
int i,istring1,found;
struct stat filestat;
char *filename;
FILE *fd;

IFn(info->string1[0]) {
  sprintf(erreur,"Usage: ogaut -p dump_file [pretty_dump_file]");
  OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(info->string2[0]) {
  istring1=strlen(info->string1); found=0;
  for (i=istring1-1; i>=istring1-4; i--) {
    if (info->string1[i]=='.') { found=1; break; }
    }
  if (found) {
    sprintf(info->string2,"%.*s",i,info->string1);
    }
  else {
    strcpy(info->string2,info->string1);
    }
  sprintf(info->string2+strlen(info->string2),".pre");
  }

OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
  ,"Pretty dump file '%s' into '%s'",info->string1,info->string2);

filename=info->string1;
IFn(fd=fopen(filename,"rb")) {
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"AutPretty: impossible to fopen '%s' for reading",filename);
  DONE;
  }

IF(fstat(fileno(fd),&filestat)) {
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"AutPretty: impossible to fstat '%s'",filename);
  DONE;
  }

IFn(B=(char *)malloc(filestat.st_size+9)) {
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"AutPretty: impossible to allocate %d bytes for '%s'",filestat.st_size,filename);
  fclose(fd); DONE;
  }

IFn(iB=fread(B,1,filestat.st_size,fd)) {
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"AutPretty: impossible to fread '%s'",filename);
  DPcFree(B); fclose(fd); DONE;
  }

fclose(fd);

for (i=0; i<iB; i++) {
  if (B[i]=='\n' || B[i]=='\r') continue;
  else if (B[i]<32) B[i]=pretty[B[i]];
  }

filename=info->string2;
IFn(fd=fopen(filename,"wb")) {
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"AutPretty: impossible to fopen '%s' for writing",filename);
  DONE;
  }
IFn(iB=fwrite(B,1,iB,fd)) {
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"AutPretty: impossible to fwrite '%s'",filename);
  DPcFree(B); fclose(fd); DONE;
  }

fclose(fd);
DPcFree(B);

DONE;
}




/*
 * Ufu stands for Utf8:freq to Unicode \1 freq
 * input string is utf8_string:frequency
 * output string  unicode_string\1<ognum frequency>
*/


STATICF(int) AutUfu(info)
struct og_info *info;
{
int ibuffer; unsigned char *p,buffer[DPcPathSize];
int iline; unsigned char line[DPcPathSize];
int iuni; unsigned char uni[DPcPathSize];
int i,istring1,found,frequency;
char erreur[DOgErrorSize];
int nb_lines,max_length;
FILE *fd;

IFn(info->string1[0]) {
  sprintf(erreur,"Usage: ogaut -ufu [-m] input_file [automaton_file]");
  OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(info->string2[0]) {
  istring1=strlen(info->string1); found=0;
  for (i=istring1-1; i>=istring1-4; i--) {
    if (info->string1[i]=='.') { found=1; break; }
    }
  if (found) {
    sprintf(info->string2,"%.*s",i,info->string1);
    }
  else {
    strcpy(info->string2,info->string1);
    }
  sprintf(info->string2+strlen(info->string2),".%s",AutExtensionString(info->automaton_type));
  }

OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
  ,"Compiling (utf8:freq) '%s' into (unicode\1freq) '%s'",info->string1,info->string2);

IFn(info->haut=OgAutInit(info->param)) DPcErr;

IFn(fd=fopen(info->string1,"r")) {
  sprintf(erreur,"AutRead: impossible to open '%s'",info->string1);
  OgErr(info->param->herr,erreur); DPcErr;
  }

nb_lines=0; max_length=0;
while(fgets(line,DPcPathSize,fd)) {
  int colon=(-1);
  iline=strlen(line); if (line[iline-1]=='\n') line[--iline]=0;
  for (i=0; i<iline; i++) {
    if (line[i]==':') { colon=i; break; }
    }
  if (colon < 0) continue; line[colon]=0;

  IFE(OgCpToUni(colon,line,DPcPathSize,&iuni,uni,DOgCodePageUTF8,0,0));
  memcpy(buffer,uni,iuni); ibuffer=iuni;
  buffer[ibuffer++]=0; buffer[ibuffer++]=1;

  frequency=atoi(line+colon+1);
  p = buffer+ibuffer; OggNout(frequency,&p);

  ibuffer = p - buffer;
  IFE(OgAutAdd(info->haut,ibuffer,buffer));
  if (max_length < iuni) max_length = iuni;
  nb_lines++;
  }

fclose(fd);

if (info->minimize) {
   IFE(OgAum(info->haut));
  }

if (info->automaton_type==DOgAutNormal) {
  IFE(OgAutWrite(info->haut,info->string2));
  }
else if (info->automaton_type==DOgAutFast) {
  IFE(OgAuf(info->haut,info->is_aug));
  IFE(OgAufWrite(info->haut,info->string2));
  }

OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
  ,"Compiling '%s' '%s' finished with %d strings and max string lengh %d"
  ,info->string1, info->string2, nb_lines, max_length);

DONE;
}



struct og_aut_test {
  int toto;
  int titi;
  };

static int og_aut_test_cmp(const void *ptr1, const void *ptr2)
{
  struct og_aut_test *table1 = (int *) ptr1;
  struct og_aut_test *table2 = (int *) ptr2;
  return (table1->toto - table2->toto);
}


STATICF(int) AutTest(info)
struct og_info *info;
{
int i,istring1,automaton_type,iout;
int retour,nstate0,nstate1; oindex states[DPcAutMaxBufferSize];
unsigned char out[DPcAutMaxBufferSize];
ogint64_t micro_clock_start;
struct og_aut_test *table;
char erreur[DOgErrorSize];
int elapsed,nb_entries=0;
double elapsed_per_entry;

IFn(info->string1[0]) {
  sprintf(erreur,"Usage: ogaut -x automaton_file");
  OgErr(info->param->herr,erreur); DPcErr;
  }

istring1=strlen(info->string1);
if (!strcmp(info->string1+istring1-4,".aut")) automaton_type=DOgAutNormal;
else if (!strcmp(info->string1+istring1-4,".aum")) automaton_type=DOgAutNormal;
else if (!strcmp(info->string1+istring1-4,".auf")) automaton_type=DOgAutFast;
else automaton_type=DOgAutNormal;

OgMessageLog(DOgMlogInLog+DOgMlogMBox,info->param->loginfo.where,0,"Testing '%s'",info->string1);

if (automaton_type == DOgAutNormal) {
  IFE(OgAutRead(info->haut,info->string1));
  micro_clock_start = OgMicroClock();
  if ((retour=OgAutScanf(info->haut,-1,"",&iout,out,&nstate0,&nstate1,states))) {
    do {
      IFE(retour);
      nb_entries++;
      }
    while((retour=OgAutScann(info->haut,&iout,out,nstate0,&nstate1,states)));
    }
  }
else if (automaton_type == DOgAutFast) {
  IFE(OgAufRead(info->haut,info->string1));
  micro_clock_start = OgMicroClock();
  if ((retour=OgAufScanf(info->haut,-1,"",&iout,out,&nstate0,&nstate1,states))) {
    do {
      IFE(retour);
      nb_entries++;
      }
    while((retour=OgAufScann(info->haut,&iout,out,nstate0,&nstate1,states)));
    }
  }

elapsed = (int) (OgMicroClock() - micro_clock_start);
elapsed_per_entry = elapsed; elapsed_per_entry /= nb_entries;
OgMessageLog(DOgMlogInLog+DOgMlogMBox,info->param->loginfo.where,0
  ,"automaton elapsed=%d micro-seconds for %d entries, thus %f micro-seconds per entry",elapsed,nb_entries,elapsed_per_entry);

nb_entries=0x100000;
table=(struct og_aut_test *)malloc(sizeof(struct og_aut_test)*nb_entries);
micro_clock_start = OgMicroClock();
for (i=0; i<nb_entries; i++) {
  int toto=table[i].toto;
  toto++;
  }
elapsed = (int) (OgMicroClock() - micro_clock_start);
elapsed_per_entry = elapsed; elapsed_per_entry /= nb_entries;
OgMessageLog(DOgMlogInLog+DOgMlogMBox,info->param->loginfo.where,0
  ,"table elapsed=%d micro-seconds for %d entries, thus %f micro-seconds per entry",elapsed,nb_entries,elapsed_per_entry);

srand(123456);
for (i=0; i<nb_entries; i++) {
  table[i].toto=rand();
  }

micro_clock_start = OgMicroClock();
qsort(table,nb_entries,sizeof(struct og_aut_test),og_aut_test_cmp);
elapsed = (int) (OgMicroClock() - micro_clock_start);
elapsed_per_entry = elapsed; elapsed_per_entry /= nb_entries;
OgMessageLog(DOgMlogInLog+DOgMlogMBox,info->param->loginfo.where,0
  ,"table sort elapsed=%d micro-seconds for %d entries, thus %f micro-seconds per entry",elapsed,nb_entries,elapsed_per_entry);


DONE;
}







STATICF(char *) AutExtensionString(automaton_type)
int automaton_type;
{
switch(automaton_type) {
  case DOgAutNormal: return("aut");
  case DOgAutFast: return("auf");
  }
return("aut");
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

sprintf(buffer,               "Usage: ogaut [-v] [-h] [options] [<string1> [<string2>]]\n");
sprintf(buffer+strlen(buffer),"all results are printed in log/%s.log\n",info->param->loginfo.where);
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"    -aut or -auf gives the type of the automaton\n");
sprintf(buffer+strlen(buffer),"        default is -aut\n");
sprintf(buffer+strlen(buffer),"    -c compiles an automaton\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -c input_file [automaton_file]\n");
sprintf(buffer+strlen(buffer),"    -d dumps an automaton\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -d automaton_file [dump_file]\n");
sprintf(buffer+strlen(buffer),"    -g fast automaton is forced into a big automaton\n");
sprintf(buffer+strlen(buffer),"    -e testing little/big endian conversion\n");
sprintf(buffer+strlen(buffer),"    -h prints this message\n");
sprintf(buffer+strlen(buffer),"    -l<n> automaton has linguistic format (default 0x%x)\n",info->ling_flags);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"        0x1: direct compilation\n");
sprintf(buffer+strlen(buffer),"        0x2: building of internal source file\n");
sprintf(buffer+strlen(buffer),"        0x4: idem with termination\n");
sprintf(buffer+strlen(buffer),"        0x8: indirect compilation\n");
sprintf(buffer+strlen(buffer),"    -m minimize automaton, used with -c option, or alone\n");
sprintf(buffer+strlen(buffer),"       example when used alone: ogaut -m automaton_file\n");
sprintf(buffer+strlen(buffer),"    -n<n> sets initial number of states (default is zero)\n");
sprintf(buffer+strlen(buffer),"    -ncc nocharcase (default is false, used for arabic for example)\n");
sprintf(buffer+strlen(buffer),"    -p pretty a dump automaton (for better debugging)\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -p dump_file [pretty_dump_file]\n");
sprintf(buffer+strlen(buffer),"    -r reads and tests an automaton\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -r input_file automaton_file\n");
sprintf(buffer+strlen(buffer),"    -s scans an automaton\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -s input_file automaton_file\n");
sprintf(buffer+strlen(buffer),"    -q query a string\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -q automaton_file string\n");
sprintf(buffer+strlen(buffer),"    -t<n>: trace options for logging (default 0x%x)\n",info->param->loginfo.trace);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"        0x1: minimal, 0x2: memory\n");
sprintf(buffer+strlen(buffer),"    -ufu compiles utf8:freq source file into unicode\\1freq automaton\n");
sprintf(buffer+strlen(buffer),"    -u unicode automaton\n");
sprintf(buffer+strlen(buffer),"    -v gives version number of the program\n");
sprintf(buffer+strlen(buffer),"    -x tests an automaton and prints some performance statistics\n");
sprintf(buffer+strlen(buffer),"        example: ogaut -x automaton_file\n");

#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"ogaut help",edit_buffer,410,430);
#else
OgMessageBox(0,buffer,info->param->loginfo.where,DOgMessageBoxInformation);
#endif

DONE;
}



/*
 *  Exits from program with 'retval' value.
 *  Prints errors. Returns 1 on error and zero otherwise.
*/

STATICF(void) DoExit(herr,where,retval)
void *herr; char *where; int retval;
{
int is_error;
is_error=OgErrLog(herr,where,1,0,0,0);
if (retval) OgMessageLog( DOgMlogInLog+DOgMlogDateIn,where,0,"cdco finished on erreur");
exit(retval);
}


