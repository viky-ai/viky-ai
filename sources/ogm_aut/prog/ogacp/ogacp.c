/*
 *	This example is used to test the OgAcp function
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : October 2009
 *	Version 1.0
*/
#include <logaut.h>
#include <logxml.h>
#include <logmsg.h>
#include <loguni.h>
#include <logpath.h>
#include <logstopword.h>

#define DOgAcpTraceMinimal    0x1

#define DOgOutputFileName       "ogacp.xml"
#define DOgAdtaxFileName        "adtax.txt"
#define DOgExpressionsFileName  "expressions.txt"
#define DOgWordsFileName        "words.txt"
#define DOgDefaultFreq          10
#define DOgWhereName            "ogacp"

#define DOgMaxCriteriaLength    128
#define DOgMaxNbSubitems        1000

struct subitem {
  unsigned char criteria[DOgMaxCriteriaLength];
  int weight;
  };


struct ogacp_info {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char output_file[DPcPathSize];
  char adtax_file[DPcPathSize];
  char expressions_file[DPcPathSize];
  char words_file[DPcPathSize];
  int freq;
  void *ha_expressions,*ha_find_expressions;
  void *ha_adtax;
  int nb_expressions_in_requests;
  int nb_adtax_in_requests,nb_adtax_not_in_requests;
  int nb_expressions, nb_adtax_expressions, nb_pure_expressions;
  int multiple;
  void *hstopword;
  };  


static int Acp(pr(struct ogacp_info *));
static int AcpReadExpressions(pr(struct ogacp_info *));
static int AcpReadAdtax(pr(struct ogacp_info *));
static int AcpIsExpressions(struct ogacp_info *info,int iexpr,unsigned char *expr);
static int AcpIsAdtax(struct ogacp_info *info,int iexpr,unsigned char *expr);
static int AcpCleanAdtax(struct ogacp_info *info,int *piline,unsigned char *line);
static int AcpCalculateFrequency(struct ogacp_info *info,int iexpr,unsigned char *expr,int *pfreq);
static int AcpGenerateOutput(struct ogacp_info *info);
static int AcpGenerateMultipleOutput(struct ogacp_info *info);
static int AcpGenerateMultipleOutput1(struct ogacp_info *info, FILE *fd, int iname, unsigned char *name, int nb_subitems, struct subitem *subitem);
static int AcpGenerateOutputAtMiddle(struct ogacp_info *info, FILE *fd, int is, unsigned char *s);
static int AcpGenerateOutputRemoveMultipleLetters(struct ogacp_info *info, int *piequivalent, unsigned char *equivalent);
static int AcpGenerateInformation(struct ogacp_info *info,FILE *fd,int iexpr,unsigned char *expr);

static int OgUse(pr_(struct ogacp_info *) pr_(void *) pr(int));
static void OgExit(pr_(void *) pr(char *));



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
char cmd_info[8192];
char cwd[1024],cmd[8192],buffer[4096];
struct ogacp_info cinfo,*info=&cinfo;
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
ogmutex_t mutex;
int must_exit=0;
time_t ltime;
char *nil;

memset(info,0,sizeof(struct ogacp_info));
info->loginfo.trace = DOgAcpTraceMinimal; 
info->loginfo.where = "ogacp";
/** main thread error handle **/
IFn(info->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,info->loginfo.where,0,"oggl: OgErrInit error");
  return(1);
  }
/** general mutex handle **/
info->hmutex=&mutex;
IF(OgInitCriticalSection(info->hmutex,"ogacp")) { 
  OgExit(info->herr,info->loginfo.where); return(1); 
  }

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=info->herr; 
msg_param->hmutex=info->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory; 
msg_param->loginfo.where = info->loginfo.where;
msg_param->module_name = "ogacp";
IFn(info->hmsg=OgMsgInit(msg_param)) return(0);

buffer[0]=0; strcpy(cmd,"ogacp");
strcpy(info->output_file,DOgOutputFileName);
strcpy(info->adtax_file,DOgAdtaxFileName);
strcpy(info->expressions_file,DOgExpressionsFileName);
strcpy(info->words_file,DOgWordsFileName);
info->freq = DOgDefaultFreq;

OgGetExecutablePath(argv0,cwd); chdir(cwd);

#if ( DPcSystem == DPcSystemUnix)

for (i=1; i<argc; i++) {
  strcpy(cmd_info,argv[i]);

#else
#if ( DPcSystem == DPcSystemWin32)

while(OgGetCmdParameter(sCmdParameters,cmd_info,&pos)) {
#endif
#endif
  sprintf(cmd+strlen(cmd)," %s",cmd_info);
  if (cmd_info[0]!='-') {
    strcpy(info->output_file,cmd_info);
    }
  else if (!Ogmemicmp(cmd_info,"-a",2)) {
    strcpy(info->adtax_file,cmd_info+2);
    }
  else if (!Ogmemicmp(cmd_info,"-e",2)) {
    strcpy(info->expressions_file,cmd_info+2);
    }
  else if (!Ogmemicmp(cmd_info,"-f",2)) {
    info->freq=OgArgSize(cmd_info+2);
    }
  else if (!Ogmemicmp(cmd_info,"-w",2)) {
    strcpy(info->words_file,cmd_info+2);
    }
  else if (!Ogmemicmp(cmd_info,"-m",2)) {
    info->multiple=1;
    }
  else if (!strcmp(cmd_info,"-h")) {
    OgUse(info,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (cmd_info[1]=='t') {
    info->loginfo.trace = strtol(cmd_info+2,&nil,16);
    } 
  }

if (must_exit) return(0);

IF(OgCheckOrCreateDir(DOgDirLog,0,info->loginfo.where)) { 
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,info->loginfo.where,0
    ,"Can't create directory '%s'",DOgDirLog);    
  return(1);
  }

if (info->loginfo.trace&DOgAcpTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
	   ,"\nProgram %s.exe starting with pid %x at %.24s"
       , info->loginfo.where, getpid(), ctime(&ltime));
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0, "Command line: %s", cmd);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0,"Current directory is '%s'",cwd);
  }

IF(Acp(info))  { 
  OgExit(info->herr,info->loginfo.where); return(1); 
  }

IF(OgFlushCriticalSection(info->hmutex)) { 
  OgExit(info->herr,info->loginfo.where); return(1); 
  }
OgMsgFlush(info->hmsg); 
OgErrFlush(info->herr);

if (info->loginfo.trace&DOgAcpTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,info->loginfo.where,0
    ,"\nProgram %s.exe exiting at %.24s\n",info->loginfo.where,ctime(&ltime));
  }

return(0);
}



static int Acp(info)
struct ogacp_info *info;
{
struct og_stopword_param param[1];
memset(param,0,sizeof(struct og_stopword_param));
param->hmsg=info->hmsg; 
param->herr=info->herr; 
param->hmutex=info->hmutex;
param->loginfo.trace = DOgStopwordTraceMinimal+DOgStopwordTraceMemory; 
param->loginfo.where = info->loginfo.where;
IFn(info->hstopword=OgStopwordInit(param)) DPcErr;


IFE(AcpReadExpressions(info));
IFE(AcpReadAdtax(info));
if (info->multiple) {
  IFE(AcpGenerateMultipleOutput(info));
  }
else {
  IFE(AcpGenerateOutput(info));
  }

OgMessageLog(DOgMlogInLog,info->loginfo.where,0
  ,"nb_expressions_in_requests = %d",info->nb_expressions_in_requests);
OgMessageLog(DOgMlogInLog,info->loginfo.where,0
  ,"nb_adtax_in_requests = %d",info->nb_adtax_in_requests);
OgMessageLog(DOgMlogInLog,info->loginfo.where,0
  ,"nb_adtax_not_in_requests = %d",info->nb_adtax_not_in_requests);
OgMessageLog(DOgMlogInLog,info->loginfo.where,0
  ,"nb_expressions = %d",info->nb_expressions);
OgMessageLog(DOgMlogInLog,info->loginfo.where,0
  ,"nb_adtax_expressions = %d",info->nb_adtax_expressions);
info->nb_pure_expressions = info->nb_expressions - info->nb_adtax_expressions;
OgMessageLog(DOgMlogInLog,info->loginfo.where,0
  ,"nb_pure_expressions = %d",info->nb_pure_expressions);

DONE;
}




static int AcpReadExpressions(info)
struct ogacp_info *info;
{
struct og_aut_param caut_param,*aut_param=&caut_param;
int ibuffer; unsigned char buffer[DPcPathSize];
int i,iline; unsigned char line[DPcPathSize];
char erreur[DOgErrorSize];
int nb_lines;
FILE *fd;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=info->herr; 
aut_param->hmutex=info->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = info->loginfo.where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"expressions");
IFn(info->ha_expressions=OgAutInit(aut_param)) return(0);

sprintf(aut_param->name,"find_expressions");
IFn(info->ha_find_expressions=OgAutInit(aut_param)) return(0);

/* read the expressions.txt list formated as freq|word 
 * into an automaton containing only 'word' */
IFn(fd=fopen(info->expressions_file,"r")) {
  sprintf(erreur,"Impossible to fopen '%s'",info->expressions_file);
  OgErr(info->herr,erreur);
  DPcErr;
  }

nb_lines=0;
while(fgets(line,DPcPathSize,fd)) {
  int pipe=(-1),freq;
  /** removing all '\r' and '\n' at end of line **/
  iline=strlen(line);
  for (i=iline-1; i>=0; i--) {
    if (line[i]!='\r' && line[i]!='\n') { iline=i+1; break; }
    }
  line[iline]=0;
  for (i=0; i<iline; i++) {
    if (line[i]=='|') { pipe=i; break; }
    }
  if (pipe < 0) continue;
  freq = atoi(line);
  if (freq < info->freq) break;
  freq = 100000000 - freq;
  sprintf(buffer,"%.8d|%s",freq,line+pipe+1); ibuffer=strlen(buffer);
  IFE(OgAutAdd(info->ha_expressions,ibuffer,buffer));
  sprintf(buffer,"%s|%.8d",line+pipe+1,freq); ibuffer=strlen(buffer);
  IFE(OgAutAdd(info->ha_find_expressions,ibuffer,buffer));
  info->nb_expressions_in_requests++;
  nb_lines++; 
  }

fclose(fd);

DONE;
}






static int AcpIsExpressions(info,iexpr,expr)
struct ogacp_info *info;
int iexpr; unsigned char *expr;
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
char buffer[DPcPathSize];

sprintf(buffer,"%.*s|",iexpr,expr);
if ((retour=OgAutScanf(info->ha_find_expressions,strlen(buffer),buffer,&iout,out,&nstate0,&nstate1,states))) {
  do { 
    IFE(retour);
    return(1);
    }
  while((retour=OgAutScann(info->ha_find_expressions,&iout,out,nstate0,&nstate1,states)));
  }
return(0);
}





static int AcpIsAdtax(info,iexpr,expr)
struct ogacp_info *info;
int iexpr; unsigned char *expr;
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
char buffer[DPcPathSize];

sprintf(buffer,"%.*s|",iexpr,expr);
if ((retour=OgAutScanf(info->ha_adtax,strlen(buffer),buffer,&iout,out,&nstate0,&nstate1,states))) {
  do { 
    IFE(retour);
    return(1);
    }
  while((retour=OgAutScann(info->ha_adtax,&iout,out,nstate0,&nstate1,states)));
  }
return(0);
}






/*
 *  Here we read the adtax.txt file, clean entries 
 *  check if the adtax is in the expressions
 *  if not, calculate an equivalent frequency 
 *  and add it into the expressions automaton
 *  at the same time we add the line into the addtax automaton
*/

static int AcpReadAdtax(info)
struct ogacp_info *info;
{
struct og_aut_param caut_param,*aut_param=&caut_param;
int i,iline; unsigned char line[DPcPathSize];
int ibuffer; unsigned char buffer[DPcPathSize];
char erreur[DOgErrorSize];
int nb_lines;
FILE *fd;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=info->herr; 
aut_param->hmutex=info->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = info->loginfo.where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"adtax");
IFn(info->ha_adtax=OgAutInit(aut_param)) return(0);

/* read the expressions.txt list formated as freq|word 
 * into an automaton containing only 'word' */
IFn(fd=fopen(info->adtax_file,"r")) {
  sprintf(erreur,"Impossible to fopen '%s'",info->adtax_file);
  OgErr(info->herr,erreur);
  DPcErr;
  }

nb_lines=0;
while(fgets(line,DPcPathSize,fd)) {
  int pipe=(-1),iexpr,must_keep,freq;
  /** removing all '\r' and '\n' at end of line **/
  iline=strlen(line);
  for (i=iline-1; i>=0; i--) {
    if (line[i]!='\r' && line[i]!='\n') { iline=i+1; break; }
    }
  line[iline]=0;
  for (i=0; i<iline; i++) {
    if (line[i]=='|') { pipe=i; break; }
    }
  if (pipe < 0) continue;
  line[pipe]=0;
  #if 1
  if (!strcmp(line,"restaurants")) {
    int toto=1;
    }
  #endif
  iexpr=pipe;  
  IFE(must_keep=AcpCleanAdtax(info,&iexpr,line));
  if (!must_keep) continue;
  /** Recording the cleaned line for later use (building of output) **/
  sprintf(buffer,"%s|%s",line,line+pipe+1); ibuffer=strlen(buffer);
  IFE(OgAutAdd(info->ha_adtax,ibuffer,buffer));
  if (0) {
    if (AcpIsExpressions(info,iexpr,line)) {
      info->nb_adtax_in_requests++;
      continue;
      }
    /** if adtax is not in expressions, we add it with a frequency **/
    IFE(AcpCalculateFrequency(info,iexpr,line,&freq));
    freq = 100000000 - freq;
    sprintf(buffer,"%.8d|%s",freq,line); ibuffer=strlen(buffer);
    IFE(OgAutAdd(info->ha_expressions,ibuffer,buffer));
    sprintf(buffer,"%s|%.8d",line,freq); ibuffer=strlen(buffer);
    IFE(OgAutAdd(info->ha_find_expressions,ibuffer,buffer));
    info->nb_adtax_not_in_requests++;
    }
  nb_lines++; 
  }

fclose(fd);

DONE;
}



/*
 * returns 1 if it has been cleaned and 0 if it must be discarded 
*/

static int AcpCleanAdtax(struct ogacp_info *info,int *piline,unsigned char *line)
{
int i,j,iline = *piline;
if (iline < 4) return(0);
for (i=0; i<iline; i++) {
  if (line[i]<32) return(0);
  if (line[i]>126) return(0);
  }
for (i=0; i<iline; i++) {
  line[i]=PcTolower(line[i]);
  }  
if (line[0] < 'a' || 'z' < line[0]) return(0);  
OgTrimString(line,line); iline=strlen(line);
/** keeping only one space between words **/
for (i=0,j=0; i<iline; i++) {
  if (!PcIsspace(line[i])) { line[j++]=line[i]; continue; }
  if (i+1<iline && PcIsspace(line[i+1])) continue;
  line[j++]=line[i];
  }
iline=j;
line[iline]=0;
/** TODO: remove lines that have only French words **/
*piline=iline;
return(1);
}


/* 
 * Frequency is Calculated as follows : formula is y = ax + b 
 * where x is length of string and y is frequency
 * and we set two points : F = a 1 + b
 * and 1 = a M + b, thus formula becomes :
 * a = (F-1)/(M-1) and b = F - a 
*/

static int AcpCalculateFrequency(struct ogacp_info *info,int iexpr,unsigned char *expr,int *pfreq)
{
double F = info->freq*1, M = 30;
double a,b,y,x;

x = iexpr;
if (x > M) { *pfreq=1; DONE; }
a = (1-F); a/= (M-1);
b = F - a;
y = a*x + b;
if (y < 1) *pfreq = 1;
else *pfreq = (int)y;
DONE;
}




static int AcpGenerateOutput(struct ogacp_info *info)
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
int iname; unsigned char name[DPcPathSize];
int iequivalent; unsigned char equivalent[DPcPathSize];
int i,j,c,freq,has_space,modified;
char erreur[DOgErrorSize];
FILE *fd;

IFn(fd=fopen(info->output_file,"w")) {
  sprintf(erreur,"Impossible to fopen '%s'",info->output_file);
  OgErr(info->herr,erreur);
  DPcErr;
  }
fprintf(fd,
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<expressions>\n");

if ((retour=OgAutScanf(info->ha_expressions,0,"",&iout,out,&nstate0,&nstate1,states))) {
  do { 
    unsigned char *s;
    int pipe=(-1),is;
    
    for (i=0; i<iout; i++) {
      if (out[i]=='|') { pipe=i; break; }
      }
    if (pipe < 0) continue;
    
    s=out+pipe+1; is=strlen(s);
    //if (AcpIsAdtax(info,is,s)) {
    if (1) {
      int must_remove=0;
      for (i=0; i<is; i++) {
        c=s[i];
        if (c=='&' || c=='-' || c==',') must_remove=1;
        }
      if (must_remove) 
        continue;
      }
    
    freq = atoi(out);
    freq = 100000000 - freq;
    info->nb_expressions++;
        
    fprintf(fd,
    "  <expression>\n");
    
    IFE(OgXmlEncode(is,s,DPcPathSize,&iname,name,0,0));
    
    fprintf(fd,
      "    <name>%s</name>\n"
      ,name
      );
    for (i=0,has_space=0; i<iname; i++) {
      if (PcIsspace(name[i]) || name[i]=='-') { has_space=1; break; }
      }
    for (i=0; i<iname; i++) {
      if (PcIsspace(name[i]) || name[i]=='-') equivalent[i]='~';
      else equivalent[i]=name[i];
      }
    iequivalent=iname; equivalent[iequivalent]=0; 
    fprintf(fd,
      "    <equivalent>%s</equivalent>\n"
      ,equivalent
      );
    IFE(modified=AcpGenerateOutputRemoveMultipleLetters(info,&iequivalent,equivalent));
    if (modified) {
      fprintf(fd,
        "    <equivalent>%s</equivalent>\n"
        ,equivalent
        );
      }
    if (has_space) {
      for (i=0,j=0; i<iname; i++) {
        if (PcIsspace(name[i]) || name[i]=='-') continue;
        equivalent[j++]=name[i];
        }
      iequivalent=j; equivalent[iequivalent]=0; 
      fprintf(fd,
        "    <equivalent>%s</equivalent>\n"
        ,equivalent
        );
      IFE(modified=AcpGenerateOutputRemoveMultipleLetters(info,&iequivalent,equivalent));
      if (modified) {
        fprintf(fd,
          "    <equivalent>%s</equivalent>\n"
          ,equivalent
          );
        }
      IFE(AcpGenerateOutputAtMiddle(info,fd,is,s));
      }
    //IFE(AcpGenerateInformation(info,fd,strlen(out+pipe+1),out+pipe+1));
    fprintf(fd,
      "    <weight>%d</weight>\n"
      ,freq
      );
    fprintf(fd,
    "  </expression>\n");
    IFE(retour);
    }
  while((retour=OgAutScann(info->ha_expressions,&iout,out,nstate0,&nstate1,states)));
  }

fprintf(fd,
"</expressions>\n");

fclose(fd);
DONE;
}





static int AcpGenerateMultipleOutput(struct ogacp_info *info)
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
int iname; unsigned char name[DPcPathSize];
int ioname; unsigned char oname[DPcPathSize];
int icriteria; unsigned char criteria[DPcPathSize];
int nb_subitems; struct subitem subitem[DOgMaxNbSubitems];
int i,c,freq;
char erreur[DOgErrorSize];
FILE *fd;

ioname=0; oname[0]=0;
nb_subitems=0;

IFn(fd=fopen(info->output_file,"w")) {
  sprintf(erreur,"Impossible to fopen '%s'",info->output_file);
  OgErr(info->herr,erreur);
  DPcErr;
  }
fprintf(fd,
"<?xml version='1.0' encoding='UTF-8'?>\n"
"<expressions>\n");

if ((retour=OgAutScanf(info->ha_find_expressions,0,"",&iout,out,&nstate0,&nstate1,states))) {
  do { 
    int is; unsigned char *s;
    int pipe1=(-1),pipe2=(-1);
    
    for (i=0; i<iout; i++) {
      if (out[i]=='|') { pipe1=i; break; }
      }
    if (pipe1 < 0) continue;

    for (i=pipe1+1; i<iout; i++) {
      if (out[i]=='|') { pipe2=i; break; }
      }
    if (pipe2 < 0) continue;
    
    s=out; is=pipe1;
    if (1) {
      int must_remove=0;
      for (i=0; i<is; i++) {
        c=s[i];
        if (c=='&' || c=='-' || c==',') must_remove=1;
        }
      if (must_remove) 
        continue;
      }
    
    freq = atoi(out+pipe2+1);
    freq = 100000000 - freq;
    
    IFE(OgXmlEncode(is,s,DPcPathSize,&iname,name,0,0));
    IFE(OgXmlEncode(pipe2-pipe1-1,out+pipe1+1,DPcPathSize,&icriteria,criteria,0,0));
    strcpy(subitem[nb_subitems].criteria,criteria);
    subitem[nb_subitems].weight=freq;
    nb_subitems++;
    
    if (iname==ioname && !strcmp(name,oname)) continue;
    IFE(AcpGenerateMultipleOutput1(info,fd,ioname,oname,nb_subitems-1,subitem));
    strcpy(oname,name); ioname=iname;
    memcpy(subitem,subitem+nb_subitems-1,sizeof(struct subitem));
    nb_subitems=1; 
    IFE(retour);
    }
  while((retour=OgAutScann(info->ha_find_expressions,&iout,out,nstate0,&nstate1,states)));
  }

IFE(AcpGenerateMultipleOutput1(info,fd,ioname,oname,nb_subitems-1,subitem));

fprintf(fd,
"</expressions>\n");

fclose(fd);
DONE;
}




static int AcpGenerateMultipleOutput1(struct ogacp_info *info, FILE *fd, int iname, unsigned char *name, int nb_subitems, struct subitem *subitem)
{
int iequivalent; unsigned char equivalent[DPcPathSize];
int i,j,has_space,modified;
int generic_weight=0;

if (iname==0) DONE;
if (nb_subitems<=0) DONE;

info->nb_expressions++;

for (i=0; i<nb_subitems; i++) {
  generic_weight += subitem[i].weight;
  }

fprintf(fd,
"  <expression>\n");

fprintf(fd,
  "    <name>%s</name>\n"
  ,name
  );
for (i=0,has_space=0; i<iname; i++) {
  if (PcIsspace(name[i]) || name[i]=='-') { has_space=1; break; }
  }
for (i=0; i<iname; i++) {
  if (PcIsspace(name[i]) || name[i]=='-') equivalent[i]='~';
  else equivalent[i]=name[i];
  }
iequivalent=iname; equivalent[iequivalent]=0; 
fprintf(fd,
  "    <equivalent>%s</equivalent>\n"
  ,equivalent
  );
IFE(modified=AcpGenerateOutputRemoveMultipleLetters(info,&iequivalent,equivalent));
if (modified) {
  fprintf(fd,
    "    <equivalent>%s</equivalent>\n"
    ,equivalent
    );
  }
if (has_space) {
  for (i=0,j=0; i<iname; i++) {
    if (PcIsspace(name[i]) || name[i]=='-') continue;
    equivalent[j++]=name[i];
    }
  iequivalent=j; equivalent[iequivalent]=0; 
  fprintf(fd,
    "    <equivalent>%s</equivalent>\n"
    ,equivalent
    );
  IFE(modified=AcpGenerateOutputRemoveMultipleLetters(info,&iequivalent,equivalent));
  if (modified) {
    fprintf(fd,
      "    <equivalent>%s</equivalent>\n"
      ,equivalent
      );
    }
  }
fprintf(fd,
  "    <generic_weight>%d</generic_weight>\n"
  ,generic_weight
  );

for (i=0; i<nb_subitems; i++) {
  fprintf(fd,
    "    <subitem>\n");
  fprintf(fd,
    "      <criteria>%s</criteria>\n"
    ,subitem[i].criteria
    );
  fprintf(fd,
    "      <weight>%d</weight>\n"
    ,subitem[i].weight
    );
  fprintf(fd,
    "    </subitem>\n");
  }


fprintf(fd,
"  </expression>\n");
DONE;
}


#define DOgMaxAcpWord 100
struct acp_word {
  int start,length;
  int is_stop;
  };


static int AcpGenerateOutputAtMiddle(struct ogacp_info *info, FILE *fd, int is, unsigned char *s)
{
int new_word_length; unsigned char new_word[DPcPathSize];
int iequivalent; unsigned char equivalent[DPcPathSize];
int uni_length; unsigned char uni[DPcPathSize];
struct acp_word word[DOgMaxAcpWord];
int i,j,c,state=1,end=0,start=0;
og_stopword_output output[1];
og_stopword_input input[1];
int retour,modified;
int nb_words=0;


for (i=0; !end; i++) {
  if (i>=is) { end=1; c=' '; }
  else c = s[i];
  switch (state) {
    case 1:
      if (PcIsspace(c)) {
        word[nb_words].start=start;
        word[nb_words++].length=i-start;
        state=2;
        }
      else state=1;      
      break;
    case 2:
      if (PcIsspace(c)) state=2;
      else { start=i; state=1; }
      break;
    }
  }

for (i=0; i<nb_words; i++) {
  word[i].is_stop=0;
  IFE(OgCpToUni(word[i].length,s+word[i].start,DPcPathSize,&uni_length,uni,DOgCodePageUTF8,0,0));
  memset(input,0,sizeof(og_stopword_input));
  input->word_length=uni_length;
  input->word=uni;  
  IFE(OgStopwordScanInit(info->hstopword,input));
  IFE(retour=OgStopwordScanNext(info->hstopword,output));
  if (retour) word[i].is_stop=1;
  }

for (i=1; i<nb_words; i++) {
  if (word[i].is_stop) continue;
  new_word_length=0;
  for (j=i; j<nb_words; j++) {
    if (j>i) {
      memcpy(new_word+new_word_length," ",1);
      new_word_length++;
      }
    memcpy(new_word+new_word_length,s+word[j].start,word[j].length);
    new_word_length+=word[j].length;
    }

  for (j=0; j<new_word_length; j++) {
    if (PcIsspace(new_word[j]) || new_word[j]=='-') equivalent[j]='~';
    else equivalent[j]=new_word[j];
    }
  iequivalent=new_word_length; equivalent[iequivalent]=0; 
  fprintf(fd,
    "    <equivalent>%s</equivalent>\n"
    ,equivalent
    );
  IFE(modified=AcpGenerateOutputRemoveMultipleLetters(info,&iequivalent,equivalent));
  if (modified) {
    fprintf(fd,
      "    <equivalent>%s</equivalent>\n"
      ,equivalent
      );
    }
  
  }

DONE;
}




static int AcpGenerateOutputRemoveMultipleLetters(struct ogacp_info *info, int *piequivalent, unsigned char *equivalent)
{
int i,j,iequivalent;
iequivalent = *piequivalent;
for (i=0,j=0; i+1<iequivalent; i++) {
  if (equivalent[i]==equivalent[i+1]) continue;
  equivalent[j++]=equivalent[i];
  }
equivalent[j++]=equivalent[i];
equivalent[j]=0;
*piequivalent=j;
if (iequivalent!=j) return(1);
return(0);
}





static int AcpGenerateInformation(struct ogacp_info *info,FILE *fd,int iexpr,unsigned char *expr)
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
char buffer[DPcPathSize];

sprintf(buffer,"%.*s|",iexpr,expr);
if ((retour=OgAutScanf(info->ha_adtax,strlen(buffer),buffer,&iout,out,&nstate0,&nstate1,states))) {
  do { 
    IFE(retour);
    fprintf(fd,
      "    <information>%.*s</information>\n"
      ,iout,out
      );
    }
  while((retour=OgAutScann(info->ha_adtax,&iout,out,nstate0,&nstate1,states)));
  }

DONE;
}





static int OgUse(info,hInstance,nCmdShow)
struct ogacp_info *info;
void *hInstance;
int nCmdShow;
{
char buffer[8192],edit_buffer[8192];
#if ( DPcSystem == DPcSystemWin32)
int j,k;
#endif

sprintf(buffer               ,"Usage : ogacp [options] [<output file>]\n");
sprintf(buffer+strlen(buffer),"  default output file is '%s'\n",DOgOutputFileName);
sprintf(buffer+strlen(buffer),"  -h: this message\n");
sprintf(buffer+strlen(buffer),"  -a<adtax file>: default adtax file is'%s'\n",DOgAdtaxFileName);
sprintf(buffer+strlen(buffer),"  -e<expressions file>: default expressions file is'%s'\n",DOgExpressionsFileName);
sprintf(buffer+strlen(buffer),"  -f<min frequency>: default min frequency is %d\n",DOgDefaultFreq);
sprintf(buffer+strlen(buffer),"  -e<words file>: default words file is'%s'\n",DOgWordsFileName);
sprintf(buffer+strlen(buffer),"  -t<n>: trace options for logging\n");
sprintf(buffer+strlen(buffer),"    <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"    0x1: minimal\n");

#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"ogacp help",edit_buffer,450,350);
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


