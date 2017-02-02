/*
 *  This is main procedure for ogltras.exe
 *  Copyright (c) 2009-2010 Pertimm by Patrick Constant, Lois Rigouste
 *  Dev : December 2009, February 2010
 *  Version 1.2
*/
#include <logltras.h>
#include <logpath.h>
#include <logauta.h>
#include <loguni.h>

#define DOgLtrasDefaultCharset          DOgCharset_windows_1252
#define DOgLtrasDefaultFrequencyRatio   0.0
#define DOgLtrasDefaultScoreFactor      0.5


struct og_info {
  struct og_ltras_param *param;
  char flow_chart[DPcPathSize];
  char filename[DPcPathSize];
  int send_result,log_result;
  og_bool log_pos;
  double frequency_ratio;
  double score_factor;
  int codepage;
  int lang;
  void *hltras;
  };

STATICF(int) OgReadFile(pr(struct og_info *));
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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR sCmdParameters, int nCmdShow)
{
int pos=0;
char *argv0=0;

#endif
#endif

char cwd[DPcPathSize],cmd[DPcPathSize],cmd_param[DPcPathSize],msg[DPcPathSize];
struct og_charset_param ccharset_param,*charset_param=&ccharset_param;
struct og_ltras_param cparam, *param=&cparam;
unsigned char charset[DPcPathSize];
struct og_info cinfo,*info=&cinfo;
char codepage_string[DPcPathSize];
int must_exit=0;
void *hcharset;
time_t ltime;
int found=0;
char *nil;


memset(info,0,sizeof(struct og_info));
info->frequency_ratio=DOgLtrasDefaultFrequencyRatio;
info->score_factor=DOgLtrasDefaultScoreFactor;
strcpy(info->flow_chart,"(del-add/phon/swap)-exc-term");
info->send_result=1;
info->log_result=1;
info->param=param;

memset(param,0,sizeof(struct og_ltras_param));
param->loginfo.trace = DOgLtrasTraceMinimal+DOgLtrasTraceMemory;
param->loginfo.where = "ogltras";
IFn(param->hmsg=OgLogInit("ogltras","ogltras", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) return(0);
param->herr= OgLogGetErr(param->hmsg);
param->hmutex=OgLogGetMutex(param->hmsg);
strcpy(param->configuration_file,DOgLtrasModuleConfiguration);
strcpy(param->caller_label,"ogltras");
strcpy(param->output_file,DOgLtrasOutputFile);

memset(codepage_string,0,DPcPathSize);

strcpy(cmd,"ogltras");
OgGetExecutablePath(argv0,cwd); chdir(cwd);

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
    OgMessageBox(0,DOgLtrasBanner,param->loginfo.where,DOgMessageBoxInformation);
    must_exit=1;
    }
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(info,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (!strcmp(cmd_param,"-noresult")) {
    info->send_result=0;
    }
  else if (!strcmp(cmd_param,"-pos")) {
    info->log_pos=TRUE;
    }
  else if (!strcmp(cmd_param,"-nolog")) {
    info->log_result=0;
    }
  else if (!memcmp(cmd_param,"-cp",3)) {
    strcpy(codepage_string,cmd_param+3);
    }
  else if (!memcmp(cmd_param,"-fc",3)) {
    strcpy(info->flow_chart,cmd_param+3);
    }
  else if (!memcmp(cmd_param,"-sf",3)) {
    info->score_factor=atof(cmd_param+3);
    }
  else if (!memcmp(cmd_param,"-fr",3)) {
    info->frequency_ratio=atof(cmd_param+3);
    }
  else if (!memcmp(cmd_param,"-t",2)) {
    param->loginfo.trace = strtol(cmd_param+2,&nil,16);
    }
  else if (!memcmp(cmd_param,"-o",2)) {
    strcpy(param->output_file,cmd_param+2);
    }
  else if (!memcmp(cmd_param,"-l",2)) {
    info->lang = OgCodeToIso639(cmd_param + 2);
    }
  else if (cmd_param[0] != '-') {
    strcpy(info->filename,cmd_param);
    }
  }

if (must_exit) return(0);

IF(OgCheckOrCreateDir(DOgDirLog,0,param->loginfo.where)) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0
    ,"Can't create directory '%s'",DOgDirLog);
  return(1);
  }

if (param->loginfo.trace&DOgLtrasTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
     ,"\nProgram %s.exe starting with pid %x at %.24s"
       , param->loginfo.where, getpid(), OgGmtime(&ltime));
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0, "Command line: %s", cmd);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0, "%s",DOgLtrasBanner);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0,"Current directory is '%s'",cwd);
  }


/* dealing with charsets */
memset(charset_param,0,sizeof(struct og_charset_param));
charset_param->herr=param->herr;
charset_param->hmutex=param->hmutex;
charset_param->loginfo.trace = DOgAutaTraceMinimal+DOgAutaTraceMemory;
charset_param->loginfo.where = param->loginfo.where;
IFn(hcharset=OgCharsetInit(charset_param)) return(0);

IF(found=OgCharsetGetCode(hcharset,codepage_string,&info->codepage)) {
  OgExit(param->herr,param->loginfo.where); return(1);
  }
if (!found) {
  info->codepage=DOgLtrasDefaultCharset;
  IF(OgCharsetGetString(hcharset,info->codepage,charset)) {
    OgExit(param->herr,param->loginfo.where); return(1);
    }
  sprintf(msg,"OgLtras: Input charset '%s' not found defaults to '%s'"
                 , codepage_string, charset);
  OgMsg(param->hmsg,"",DOgMsgDestInLog, "%s", msg);
  }

IF(OgReadFile(info)) {
  OgExit(param->herr,param->loginfo.where); return(1);
  }

IFE(OgCharsetFlush(hcharset));

OgLogFlush(param->hmsg);
//IF(OgFlushCriticalSection(param->hmutex)) {
//  OgExit(param->herr,param->loginfo.where); return(1);
//  }
//OgErrFlush(param->herr);

if (param->loginfo.trace&DOgLtrasTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
    ,"\nProgram %s.exe exiting at %.24s\n",param->loginfo.where,OgGmtime(&ltime));
  }

return(0);
}




STATICF(int) OgReadFile(info)
struct og_info *info;
{
struct og_ltras_input cinput,*input=&cinput;
unsigned char line[DPcPathSize+9];
struct og_ltra_trfs *trfs;
FILE *fd;

if (info->score_factor < 0 || 1 < info->score_factor) {
  OgMsg(info->param->hmsg,"",DOgMsgDestInLog
    ,"OgReadFile: bad score factor %f should be between 0 and 1, reset to %f"
    ,info->score_factor,DOgLtrasDefaultScoreFactor);
  info->score_factor=DOgLtrasDefaultScoreFactor;
  }

/* Testing the reuse of handles (set to 1 to do the test)
 * this emulates what happens in ogm_ssrv with a multi-threaded
 * environment (listening threads) */
if (0) {
  IFn(info->hltras=OgLtrasInit(info->param)) DPcErr;
  info->param->hltras_to_inherit=info->hltras;
  }
IFn(info->hltras=OgLtrasInit(info->param)) DPcErr;

/* reading and converting input file */
IFn(fd=fopen(info->filename,"rb")) {
  OgMsg(info->param->hmsg,"",DOgMsgDestInLog,"OgReadFile: impossible to fopen '%s'",info->filename);
  DONE;
  }

while(fgets(line,DPcPathSize,fd)){
  int irequest; unsigned char *request;
  int ioperation; unsigned char *operation;
  int isuggestion; unsigned char *suggestion;
  int iuni_request; unsigned char uni_request[DPcPathSize*2];
  int iuni_suggestion; unsigned char uni_suggestion[DPcPathSize*2];
  int i,c,start,end;
  int iline=strlen(line);

  IFE(OgLtrasSuggestionInit(info->hltras));

  for (i=0,start=0,end=0; !end; i++) {
    if (i>=iline) { c=':'; end=1; }
    else c=line[i];
    if (c==':' || c=='|') {
      line[i]=0;
      if (start==0) {
        request=line; OgTrimString(request,request); irequest=strlen(request);
        IFE(OgCpToUni(irequest,request,DPcPathSize*2,&iuni_request,uni_request,OgNewCpToOld(info->codepage),0,0));
        }
      else {
        suggestion=line+start; OgTrimString(suggestion,suggestion); isuggestion=strlen(suggestion);
        IFE(OgCpToUni(isuggestion,suggestion,DPcPathSize*2,&iuni_suggestion,uni_suggestion,OgNewCpToOld(info->codepage),0,0));
        IFE(OgLtrasSuggestionAdd(info->hltras,iuni_suggestion,uni_suggestion));
        }
      start=i+1;
      if(c=='|'){
        operation=line+start; OgTrimString(operation,operation); ioperation=strlen(operation);
        IFE(OgLtrasOperationGet(info->hltras,ioperation,operation));
        end=1;
        }
      }
    }
  memset(input,0,sizeof(struct og_ltras_input));
  input->language_code = info->lang;
  input->request_length=iuni_request; input->request=uni_request;
  input->frequency_ratio=info->frequency_ratio;
  input->score_factor=info->score_factor;
  input->flow_chart=info->flow_chart;
  input->log_pos=info->log_pos;
  IFE(OgLtras(info->hltras,input,&trfs));
  if (info->log_result) {
    IFE(OgLtrasTrfsLog(info->hltras,trfs));
    }
  IFE(OgLtrasTrfsAddResult(info->hltras,trfs,info->send_result));
  IFE(OgLtrasTrfsDestroy(info->hltras,trfs));
  }

fclose(fd);

IFE(OgLtrasTrfsConsolidateResults(info->hltras,info->send_result));
IFE(OgLtrasFlush(info->hltras));

DONE;
}



STATICF(int) OgUse(info,hInstance,nCmdShow)
struct og_info *info;
void *hInstance;
int nCmdShow;
{
char buffer[8192];
#if ( DPcSystem == DPcSystemWin32)
int j,k;
#endif

sprintf(buffer,               "Usage : ogltras [-v] [-h] [options] <file>\n");
sprintf(buffer+strlen(buffer),"<file> contains one word without smiley per line\n");
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"    -cp<codepage> (default is 'windows-1252')\n");
sprintf(buffer+strlen(buffer),"    -fc<flow_chart> default is '%s'\n",info->flow_chart);
sprintf(buffer+strlen(buffer),"    -fr<frequency ratio> value from 0 to 1 default is '%f'\n",DOgLtrasDefaultFrequencyRatio);
sprintf(buffer+strlen(buffer),"    -sf<score factor> value from 0 to 1 default is '%f'\n",DOgLtrasDefaultScoreFactor);
sprintf(buffer+strlen(buffer),"    -h prints this message\n");
sprintf(buffer+strlen(buffer),"    -l<lang> lang (fr)\n");
sprintf(buffer+strlen(buffer),"    -noresult does not send results except timings\n");
sprintf(buffer+strlen(buffer),"    -nolog does not log results\n");
sprintf(buffer+strlen(buffer),"    -pos log positions in original string of each corrected word\n");
sprintf(buffer+strlen(buffer),"    -o<output_filename> default is '%s'\n",DOgLtrasOutputFile);
sprintf(buffer+strlen(buffer),"    -t<n>: trace options for logging (default 0x%x)\n",info->param->loginfo.trace);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"        0x1: minimal, 0x2: memory, 0x4: information, 0x8: conf\n");
sprintf(buffer+strlen(buffer),"        0x10: flowchart, 0x20: module calls, 0x40: selection\n");
sprintf(buffer+strlen(buffer),"        0x100: cut, 0x200: del, 0x400: paste, 0x800: phon\n");
sprintf(buffer+strlen(buffer),"        0x1000: swap, 0x2000: term, 0x4000: exc, 0x8000: lem\n");
sprintf(buffer+strlen(buffer),"        0x10000: tra, 0x20000: ref\n");
sprintf(buffer+strlen(buffer),"    -v gives version number of the program\n");

#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"ogltras help",edit_buffer,420,270);
#else
OgMessageBox(0,buffer,info->param->loginfo.where,DOgMessageBoxInformation);
#endif

DONE;
}



STATICF(void) OgExit(herr,where)
void *herr; char *where;
{
int is_error;
is_error=OgErrLog(herr,where,0,0,0,0);
}


