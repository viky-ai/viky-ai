/*
 *  This is main procedure for ogltrac.exe
 *  Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *  Dev: November 2009, October 2010
 *  Version 1.0
*/
#include <logltrac.h>
#include <logpath.h>
#include <logauta.h>
#include <loguni.h>

#define DOgLtracDefaultCharset               DOgCharset_windows_1252

#define DOgMaxAttributeNumber         1000
#define DOgMaxAttributeStringLength   128

struct arg_attribute {
  char attribute_string[DOgMaxAttributeStringLength];
  };


struct og_info {
  struct og_ltrac_param *param;
  struct og_ltrac_input *input;
  struct arg_attribute attribute[DOgMaxAttributeNumber];
  int attribute_number,negative_attributes;
  char filename[DPcPathSize];
  int dictionaries_to_scan;
  char output[DPcPathSize];
  void *hltrac;
  };

STATICF(int) Ltrac(pr(struct og_info *));
STATICF(int) OgUse(pr_(struct og_info *) pr_(void *) pr(int));
static void OgExit(struct og_info *info);



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
struct og_ltrac_param cparam, *param=&cparam;
struct og_ltrac_input cinput, *input=&cinput;
char working_directory[DPcPathSize],*DOgPIPE;
unsigned char charset[DPcPathSize];
struct og_info cinfo,*info=&cinfo;
char codepage_string[DPcPathSize];
char prior_messages[DPcPathSize];
int must_exit=0;
void *hcharset;
time_t ltime;
int found=0;
char *nil;

prior_messages[0]=0;
memset(input,0,sizeof(struct og_ltrac_input));

memset(info,0,sizeof(struct og_info));
info->param=param;
info->input=input;

memset(param,0,sizeof(struct og_ltrac_param));
param->loginfo.trace = DOgLtracTraceMinimal+DOgLtracTraceMemory;
param->loginfo.where = "ogltrac";
if((DOgPIPE=getenv("DOgPIPE"))) {
  IFn(param->hmsg=OgLogInit("ogltrac",DOgPIPE, DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogPipe)) return(0);
  }
else {
  IFn(param->hmsg=OgLogInit("ogltrac","ogltrac", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) return(0);
  }
param->herr= OgLogGetErr(param->hmsg);
param->hmutex=OgLogGetMutex(param->hmsg);

memset(codepage_string,0,DPcPathSize);

found = OgConfGetWorkingDirectory(working_directory,DPcPathSize);
IF(found) {
  PcErrLast(-1,cmd_param);
  sprintf(prior_messages+strlen(prior_messages),"OgConfGetWorkingDirectory error: %s\n",cmd_param);
  found=0;
  }
if (found) {
  if (!Ogstricmp(working_directory,DOgTempDirectory)) {
    IF(OgGetTempPath(DPcPathSize,working_directory,param->herr)) {
      OgExit(info); return(1);
      }
    sprintf(working_directory+strlen(working_directory),"pertimm");
    }
  IF(OgSetWorkingDirLog(working_directory)) {
    OgExit(info); return(1);
    }
  }
else {
  working_directory[0] = 0;
  }

strcpy(param->WorkingDirectory,working_directory);

strcpy(cmd,"ogltrac");
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
    OgMsg(param->hmsg,"",DOgMsgDestMBox,DOgLtracBanner);
    must_exit=1;
    }
  else if (!memcmp(cmd_param,"-fswap",6)) {
    input->min_frequency_swap=OgArgSize(cmd_param+6);
    }
  else if (!memcmp(cmd_param,"-f",2)) {
    input->min_frequency=OgArgSize(cmd_param+2);
    }
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(info,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (!memcmp(cmd_param,"-cp",3)) {
    strcpy(codepage_string,cmd_param+3);
    }
  else if (!memcmp(cmd_param,"-output=",8)) {
    strcpy(info->param->dictionaries_directory,cmd_param+8);
    }
  else if (!memcmp(cmd_param,"-t",2)) {
    param->loginfo.trace = strtol(cmd_param+2,&nil,16);
    }
  else if (!memcmp(cmd_param,"-mall",5)) {
    info->input->dictionaries_minimization |= DOgLtracDictionaryTypeBase
                                            + DOgLtracDictionaryTypeSwap
                                            + DOgLtracDictionaryTypePhon;
    }
  else if (!memcmp(cmd_param,"-mbase",6)) {
    info->input->dictionaries_minimization |= DOgLtracDictionaryTypeBase;
    }
  else if (!memcmp(cmd_param,"-mswap",6)) {
    info->input->dictionaries_minimization |= DOgLtracDictionaryTypeSwap;
    }
  else if (!memcmp(cmd_param,"-mphon",6)) {
    info->input->dictionaries_minimization |= DOgLtracDictionaryTypePhon;
    }
  else if (!memcmp(cmd_param,"-oall",5)) {
    info->input->dictionaries_to_export |= DOgLtracDictionaryTypeBase
                                         + DOgLtracDictionaryTypeSwap
                                         + DOgLtracDictionaryTypePhon;
    }
  else if (!memcmp(cmd_param,"-obase",6)) {
    info->input->dictionaries_to_export |= DOgLtracDictionaryTypeBase;
    }
  else if (!memcmp(cmd_param,"-oswap",6)) {
    info->input->dictionaries_to_export |= DOgLtracDictionaryTypeSwap;
    }
  else if (!memcmp(cmd_param,"-ophon",6)) {
    info->input->dictionaries_to_export |= DOgLtracDictionaryTypePhon;
    }
  else if (!memcmp(cmd_param,"-sbase",6)) {
    info->dictionaries_to_scan |= DOgLtracDictionaryTypeBase;
    }
  else if (!memcmp(cmd_param,"-sswap",6)) {
    info->dictionaries_to_scan |= DOgLtracDictionaryTypeSwap;
    }
  else if (!memcmp(cmd_param,"-sphon",6)) {
    info->dictionaries_to_scan |= DOgLtracDictionaryTypePhon;
    }
  else if (!memcmp(cmd_param,"-sall",5)) {
    info->dictionaries_to_scan |= DOgLtracDictionaryTypeBase
                               + DOgLtracDictionaryTypeSwap
                               + DOgLtracDictionaryTypePhon;
    }
  else if (!memcmp(cmd_param,"-ssi=",5)) {
    strcpy(info->param->data_directory,cmd_param+5);
    }
  else if (!strcmp(cmd_param,"-output_in_ssi")) {
    info->param->dictionaries_in_data_directory=1;
    }
  else if (cmd_param[0] != '-') {
    strcpy(info->filename,cmd_param);
    }
  }

if (must_exit) return(0);

if (param->loginfo.trace&DOgLtracTraceMinimal) {
  time(&ltime);
  OgMsg(param->hmsg,"",DOgMsgDestInLog,"Program %s.exe starting with pid %x at %.24s"
       , param->loginfo.where, getpid(), OgGmtime(&ltime));
  OgMsg(param->hmsg,"",DOgMsgDestInLog,"Command line: %s", cmd);
  OgMsg(param->hmsg,"",DOgMsgDestInLog,"%s",DOgLtracBanner);
if (prior_messages[0]) {
  OgMsg(param->hmsg,"prior_messages",DOgMsgDestInLog+DOgMsgDestInErr, "prior_messages: \n[[%s]]",prior_messages);
  }
  OgMsg(param->hmsg,"",DOgMsgDestInLog,"Current directory is '%s'",cwd);
  }

IF(OgCheckOrCreateDir(DOgDirLog,0,param->loginfo.where)) {
  OgMsg(param->hmsg,"",DOgMsgDestInLog+DOgMsgDestInErr,"Can't create directory '%s'",DOgDirLog);
  OgExit(info); return(1);
  }

if (info->input->dictionaries_to_export==0 && info->dictionaries_to_scan==0) {
  OgMsg(param->hmsg,"",DOgMsgDestInLog+DOgMsgDestInErr,"Either option -s* or -o* must be specified");
  OgExit(info); return(1);
  }

/* dealing with charsets */
memset(charset_param,0,sizeof(struct og_charset_param));
charset_param->herr=param->herr;
charset_param->hmutex=param->hmutex;
charset_param->loginfo.trace = DOgAutaTraceMinimal+DOgAutaTraceMemory;
charset_param->loginfo.where = param->loginfo.where;
IFn(hcharset=OgCharsetInit(charset_param)) return(0);

IF(found=OgCharsetGetCode(hcharset,codepage_string,&input->codepage)) {
  OgExit(info); return(1);
  }
if (!found) {
  input->codepage=DOgLtracDefaultCharset;
  IF(OgCharsetGetString(hcharset,input->codepage,charset)) {
    OgExit(info); return(1);
    }
  sprintf(msg,"OgLtrac: Input charset '%s' not found defaults to '%s'"
                 , codepage_string, charset);
  OgMsg(param->hmsg,"",DOgMsgDestInLog, "%s", msg);
  }

IF(Ltrac(info)) {
  OgExit(info); return(1);
  }

IF(OgCharsetFlush(hcharset)) {
  OgExit(info); return(1);
  }

IF(OgFlushCriticalSection(param->hmutex)) {
  OgExit(info); return(1);
  }
OgErrFlush(param->herr);

if (param->loginfo.trace&DOgLtracTraceMinimal) {
  time(&ltime);
  OgMsg(param->hmsg,"",DOgMsgDestInLog,"\nProgram %s.exe exiting at %.24s\n",param->loginfo.where,OgGmtime(&ltime));
  }

return(0);
}




STATICF(int) Ltrac(info)
  struct og_info *info;
{

  IFn(info->input->min_frequency_swap)
  {
    if (info->input->dictionaries_to_export == DOgLtracDictionaryTypeSwap)
    {
      info->input->min_frequency_swap = info->input->min_frequency;
    }
  }

  IFn(info->hltrac=OgLtracInit(info->param)) DPcErr;

  if (info->dictionaries_to_scan)
  {
    if (info->dictionaries_to_scan & DOgLtracDictionaryTypeBase)
    {
      IFE(OgLtracDicBaseLog(info->hltrac));
    }
    if (info->dictionaries_to_scan & DOgLtracDictionaryTypeSwap)
    {
      IFE(OgLtracDicSwapLog(info->hltrac));
    }
    if (info->dictionaries_to_scan & DOgLtracDictionaryTypePhon)
    {
      IFE(OgLtracDicPhonLog(info->hltrac));
    }
  }
  else
  {
    IFE(OgLtrac(info->hltrac, info->input));
  }

  IFE(OgLtracFlush(info->hltrac));

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

sprintf(buffer,               "Usage : ogltrac [-v] [-h] [options]\n");
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"    -f<frequency> minimum frequency (default 0)\n");
sprintf(buffer+strlen(buffer),"    -fswap<frequency> minimum frequency for swap dictionary(default 2*frequency)\n");
sprintf(buffer+strlen(buffer),"    -d<directory>: creates the dictionaries in <directory> (default is 'ling')\n");
sprintf(buffer+strlen(buffer),"    -output_in_ssi: creates the dictionaries in the ssi directory\n");
sprintf(buffer+strlen(buffer),"    -ssi=<ssi_dir> uses this specific ssi directory\n");
sprintf(buffer+strlen(buffer),"    -h prints this message\n");
sprintf(buffer+strlen(buffer),"    -mbase: minimizes ltra_base.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -mswap: minimizes ltra_swap.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -mphon: minimizes ltra_phon.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -mall: minimizes all 3 above automatons\n");
sprintf(buffer+strlen(buffer),"    -obase: extracts ltra_base.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -oswap: extracts ltra_swap.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -ophon: extracts ltra_phon.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -oall: extracts all 4 above automatons or source dictionary\n");
sprintf(buffer+strlen(buffer),"    -sbase: scans ltra_base.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -sswap: scans ltra_swap.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -sphon: scans ltra_phon.auf automaton\n");
sprintf(buffer+strlen(buffer),"    -sall: scans all 3 above automatons\n");
sprintf(buffer+strlen(buffer),"    -cp<encoding>: encoding of list_of_words.txt for filter option, default windows-1252\n");
sprintf(buffer+strlen(buffer),"    -t<n>: trace options for logging (default 0x%x)\n",info->param->loginfo.trace);
sprintf(buffer+strlen(buffer),"      <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"        0x1: minimal, 0x2: memory, 0x4: adding\n");
sprintf(buffer+strlen(buffer),"    -v gives version number of the program\n");
sprintf(buffer+strlen(buffer),"\n");


#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"ogltrac help",edit_buffer,420,300);
#else
OgMessageBox(0,buffer,info->param->loginfo.where,DOgMessageBoxInformation);
#endif

DONE;
}



static void OgExit(struct og_info *info)
{
  time_t ltime;
  time(&ltime);

  OgMsgErr(info->param->hmsg, "", 0, 0, 0, DOgMsgSeverityEmergency + DOgMsgDestInLog + DOgMsgDestInErr, 0);
  OgMsg(info->param->hmsg, "", DOgMsgSeverityEmergency + DOgMsgDestInLog + DOgMsgDestInErr,
      "Program %s.exe exiting on error at %.24s\n", info->param->loginfo.where, OgGmtime(&ltime));
}

