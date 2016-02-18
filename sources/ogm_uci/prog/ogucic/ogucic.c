/*
 *  This is main procedure for ogucic.exe
 *  Copyright (c) 2006-2007 Pertimm, by Patrick Constant
 *  Dev : August 2006, March 2007
 *  Version 1.1
*/
#include <time.h>
#include <loguci.h>
#include <logpath.h>


#define DOgUcicInputFile    "ucic_input.xml"
#define DOgUcicOutputFile   "ucic_output.xml"

#define DOgDefaultTrace     (DOgUciClientTraceMinimal+DOgUciClientTraceMemory+DOgUciClientTraceSocket+DOgUciClientTraceSocketSize)
#define DOgDefaultListTrace (DOgUciClientTraceMinimal+DOgUciClientTraceMemory)
#define DOgUcicTimeout      3600
#define DOgUcicSleepPeriod  0
#define DOgUciBufferSize    0x800000 /* 8mb */
#define DOgUciNbRequestsPerLog      100
#define DOgNbErrorsBeforeStopping   20

struct ogucic_info {
  struct og_uci_client_param *param;
  struct og_ucic_request *request;
  struct og_ucic_answer *answers;
  char input_file[DPcPathSize], output_file[DPcPathSize];
  char list_filename[DPcPathSize], list_filename_output[DPcPathSize];
  int port,timeout,sleep_period,nb_requests_per_log;
  char hostname[DPcPathSize];
  int send_header;
  void *hucic;
  int loop;
  };

STATICF(int) UciClient(pr(struct ogucic_info *));
STATICF(int) UciClientList(pr(struct ogucic_info *));
STATICF(int) OgUse(pr_(struct og_uci_client_param *) pr_(void *) pr(int));
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

char cmd_param[1024];
char cwd[1024],cmd[1024];
struct og_uci_client_param cparam,*param;
struct og_ucic_request crequest,*request;
struct og_ucic_answer canswers,*answers;
struct ogucic_info cinfo,*info=&cinfo;
int is_input_file=0,must_exit=0;
char *nil,*DOgPIPE;
time_t ltime;

memset(info,0,sizeof(struct ogucic_info));
param=info->param=&cparam;
request=info->request=&crequest;
answers=info->answers=&canswers;
info->sleep_period=DOgUcicSleepPeriod;
info->timeout=DOgUcicTimeout;
info->port=DOgUciPortNumber;
strcpy(info->input_file,DOgUcicInputFile);
strcpy(info->output_file,DOgUcicOutputFile);
info->nb_requests_per_log=DOgUciNbRequestsPerLog;

memset(param,0,sizeof(struct og_uci_client_param));
param->loginfo.trace = DOgDefaultTrace;
param->loginfo.where = "ogucic";

/* Si la variable d'environnement DOgPIPE est définie, on utilise le mode de log en pipe*/
if(DOgPIPE=getenv("DOgPIPE")) {
  IFn(param->hmsg=OgLogInit("ogm_ssrv",DOgPIPE, DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogPipe)) return(0);
  }
else {
  IFn(param->hmsg=OgLogInit("ogm_ssrv","ogm_ssrv", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) return(0);
  }
param->herr= OgLogGetErr(param->hmsg);
param->hmutex=OgLogGetMutex(param->hmsg);

param->socket_buffer_size=DOgUciBufferSize;

memset(request,0,sizeof(struct og_ucic_request));
sprintf(cmd,"%s",param->loginfo.where);

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
    OgMessageBox(0,DOgUciBanner,param->loginfo.where,DOgMessageBoxInformation);
    must_exit=1;
    }
  else if (!strcmp(cmd_param,"-h")) {
    OgUse(param,hInstance,nCmdShow);
    must_exit=1;
    }
  else if (!memcmp(cmd_param,"-e",2)) {
    info->sleep_period=atoi(cmd_param+2);
    }
  else if (!strcmp(cmd_param,"-header")) {
    info->send_header=1;
    }
  else if (!strcmp(cmd_param,"-loop")) {
    info->loop=1;
    }
  else if (!memcmp(cmd_param,"-i",2)) {
    strcpy(info->list_filename,cmd_param+2);
    param->loginfo.trace = DOgDefaultListTrace;
    }
  else if (!memcmp(cmd_param,"-o",2)) {
    strcpy(info->list_filename_output,cmd_param+2);
    }
  else if (!memcmp(cmd_param,"-n",2)) {
    strcpy(info->hostname,cmd_param+2);
    }
  else if (!memcmp(cmd_param,"-p",2)) {
    info->port=atoi(cmd_param+2);
    }
  else if (!memcmp(cmd_param,"-s",2)) {
    param->socket_buffer_size=OgArgSize(cmd_param+2);
    }
  else if (!memcmp(cmd_param,"-r",2)) {
    info->nb_requests_per_log=OgArgSize(cmd_param+2);
    if (info->nb_requests_per_log <= 0) info->nb_requests_per_log=1;
    }
  else if (!memcmp(cmd_param,"-t",2)) {
    param->loginfo.trace = strtol(cmd_param+2,&nil,16);
    }
  else if (!memcmp(cmd_param,"-w",2)) {
    info->timeout=atoi(cmd_param+2);
    }
  else if (cmd_param[0] != '-') {
    if (!is_input_file) {
      strcpy(info->input_file,cmd_param);
      is_input_file=1;
      }
    else {
      strcpy(info->output_file,cmd_param);
      }
    }
  }

if (must_exit) exit(0);

IF(OgCheckOrCreateDir(DOgDirLog,0,param->loginfo.where)) { 
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,param->loginfo.where,0
    ,"Can't create directory '%s'",DOgDirLog);    
  exit(1);
  }

if (param->loginfo.trace&DOgUciClientTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
	   ,"\nProgram %s.exe starting with pid %x at %.24s"
       , param->loginfo.where, getpid(), OgGmtime(&ltime));
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0, "Command line: %s", cmd);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0, "%s",DOgUciBanner);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0,"Current directory is '%s'",cwd);
  }

IFn(info->hucic=OgUciClientInit(param)) OgExit(param->herr,param->loginfo.where);

if (info->list_filename[0]) {
  if (info->loop) {
    while (1) {
      IF(UciClientList(info)) OgExit(param->herr,param->loginfo.where);
      }
    }
  else {
    IF(UciClientList(info)) OgExit(param->herr,param->loginfo.where);
    }
  }
else {
  IF(UciClient(info)) OgExit(param->herr,param->loginfo.where);
  }

IF(OgUciClientFlush(info->hucic)) OgExit(param->herr,param->loginfo.where);

OgErrFlush(param->herr);

if (param->loginfo.trace&DOgUciClientTraceMinimal) {
  time(&ltime);
  OgMessageLog(DOgMlogInLog,param->loginfo.where,0
    ,"Program %s.exe exiting at %.24s\n",param->loginfo.where,OgGmtime(&ltime));
  }

return(0);
}






STATICF(int) UciClient(info)
struct ogucic_info *info;
{
struct og_ucic_request crequest,*request=&crequest;
struct og_ucic_answer canswer,*answer=&canswer;
char *filename = info->input_file;
char erreur[DOgErrorSize];
int iB; unsigned char *B;
struct stat filestat;
FILE *fd;

filename = info->input_file;

IFn(fd=fopen(filename,"rb")) {
  sprintf(erreur,"UciClient: impossible to fopen '%s'",filename);
  OgErr(info->param->herr,erreur); DPcErr;
  }

IF(fstat(fileno(fd),&filestat)) {
  sprintf(erreur,"UciClient: impossible to fstat '%s'",filename);
  OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(B=(char *)malloc(filestat.st_size+9)) {
  sprintf(erreur,"UciClient: impossible to allocate %d bytes for '%s'",filestat.st_size,filename);
  fclose(fd); OgErr(info->param->herr,erreur); DPcErr;
  }

IFn(iB=fread(B,1,filestat.st_size,fd)) {
  sprintf(erreur,"UciClient: impossible to fread '%s'",filename);
  DPcFree(B); fclose(fd); OgErr(info->param->herr,erreur); DPcErr;
  }

fclose(fd);

memset(request,0,sizeof(struct og_ucic_request));
strcpy(request->hostname,info->hostname);
request->timeout=info->timeout;
request->port=info->port;

request->request_length=iB;
request->request=B;

IFE(OgUciClientRequest(info->hucic,request,answer));

DPcFree(B);

filename = info->output_file;

IFn(fd=fopen(filename,"wb")) {
  sprintf(erreur,"UciClient: impossible to fopen '%s' for writing",filename);
  OgErr(info->param->herr,erreur); DPcErr;
  }

if (info->send_header) {
  iB = answer->answer_length;
  B = answer->answer;
  }
else {
  iB = answer->answer_length - answer->header_length;
  B = answer->answer + answer->header_length;
  }

if (iB > 0) {
  IFn(fwrite(B,1,iB,fd)) {
    sprintf(erreur,"UciClient: impossible to fwrite '%s'",filename);
    fclose(fd); OgErr(info->param->herr,erreur); DPcErr;
    }
  }

fclose(fd);

DONE;
}




STATICF(int) UciClientList(info)
struct ogucic_info *info;
{
ogint64_t min_elapsed,max_elapsed,n_max_elapsed,mean_elapsed,elapsed,nb_elapsed;
struct og_ucic_request crequest,*request=&crequest;
struct og_ucic_answer canswer,*answer=&canswer;
char stimeout[DPcPathSize],buffer[DPcPathSize];
char *filename = info->list_filename;
ogint64_t micro_clock_start;
int n=1,nb_errors=0,first=1;
char erreur[DOgErrorSize];
char v1[64],v2[64],v3[64];
int sB; unsigned char *B;
int iBout; unsigned char *Bout;
FILE *fd,*fdout;

memset(request,0,sizeof(struct og_ucic_request));
strcpy(request->hostname,info->hostname);
request->timeout=info->timeout;
request->port=info->port;

IFn(fd=fopen(filename,"rb")) {
  sprintf(erreur,"UciClientList: impossible to fopen '%s'",filename);
  OgErr(info->param->herr,erreur); DPcErr;
  }

sB = info->param->socket_buffer_size;
IFn(B=(char *)malloc(sB+9)) {
  sprintf(erreur,"UciClientList: impossible to allocate %d bytes for '%s'",sB,filename);
  fclose(fd); OgErr(info->param->herr,erreur); DPcErr;
  }

if (info->list_filename_output[0]) {
  IFn(fdout=fopen(info->list_filename_output,"wb")) {
    sprintf(erreur,"UciClientList: impossible to fopen output file '%s'",info->list_filename_output);
    OgErr(info->param->herr,erreur); DPcErr;
    }
  }

while(fgets(B,sB,fd)) {
  request->request_length=strlen(B);
  request->request=B;
  micro_clock_start=OgMicroClock();
  IF(OgUciClientRequest(info->hucic,request,answer)) {

    if (info->param->loginfo.trace & DOgUciClientTraceMinimal) {
      int max_log_size=request->request_length,truncated=0; buffer[0]=0; stimeout[0]=0; 
      if (max_log_size>DOgMlogMaxMessageSize/2) {
        max_log_size=DOgMlogMaxMessageSize/2; truncated=1;
        sprintf(buffer," (log truncated)");
        }
      if (answer->timed_out) sprintf(stimeout," (timed out with timeout %d seconds)",request->timeout); 
      OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
        , "UciClientList: error%s on request is%s:[\n%.*s]"
        , stimeout, buffer, max_log_size, request->request);
      OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
        , "UciClientList: if the error is \"buffer full\", try -s option of ogucic to get a larger buffer");
      }  

    OgErrLog(info->param->herr,info->param->loginfo.where,0,0,0,0);
    if (nb_errors >= DOgNbErrorsBeforeStopping) {
      sprintf(erreur,"Stopping because nb_errors (%d) >= max %d"
        , nb_errors, DOgNbErrorsBeforeStopping);
      OgErr(info->param->herr,erreur); DPcErr;
      }
    nb_errors++;
    }
  elapsed=(int)(OgMicroClock()-micro_clock_start);


  if (info->list_filename_output[0]) {
    if (info->send_header) {
      iBout = answer->answer_length;
      Bout = answer->answer;
      }
    else {
      iBout = answer->answer_length - answer->header_length;
      Bout = answer->answer + answer->header_length;
      }

    if (iBout > 0) {
      IFn(fwrite(Bout,1,iBout,fdout)) {
        sprintf(erreur,"UciClientList: impossible to fwrite '%s'",filename);
        fclose(fdout); OgErr(info->param->herr,erreur); DPcErr;
        }
      }
    }

  /** Necessary because too fast for server **/
  OgSleep(info->sleep_period);
  if (first) {
    min_elapsed=elapsed;
    max_elapsed=elapsed; n_max_elapsed=n;
    mean_elapsed=elapsed;
    nb_elapsed=1;
    first=0;
    }
  else {
    if (min_elapsed > elapsed) min_elapsed=elapsed;
    if (max_elapsed < elapsed) { max_elapsed=elapsed; n_max_elapsed=n; }
    mean_elapsed+=elapsed;
    nb_elapsed++;
    }

  if (!(n%info->nb_requests_per_log)) {
    Og64FormatThousand(min_elapsed,v1,1);
    Og64FormatThousand(mean_elapsed/nb_elapsed,v2,1);
    Og64FormatThousand(max_elapsed,v3,1);
    OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
      ,"%6d: min=%s mean=%s max=%s (at %d)"
      ,n, v1, v2, v3, n_max_elapsed);
    first=1;
    }
  n++;
  }

if (!first) {
  Og64FormatThousand(min_elapsed,v1,1);
  Og64FormatThousand(mean_elapsed/nb_elapsed,v2,1);
  Og64FormatThousand(max_elapsed,v3,1);
  OgMessageLog(DOgMlogInLog,info->param->loginfo.where,0
    ,"%6d: min=%s mean=%s max=%s (at %d)"
    ,n-1, v1, v2, v3, n_max_elapsed);
  }
  
if (info->list_filename_output[0]) {
  fclose(fdout);
  }

DPcFree(B);

fclose(fd);

DONE;
}









STATICF(int) OgUse(param,hInstance,nCmdShow)
struct og_uci_client_param *param;
void *hInstance;
int nCmdShow;
{
char hostname[DPcPathSize];
char buffer[2048],edit_buffer[2048];
char sys_erreur[DPcSzErr], erreur[DPcSzErr];
#if ( DPcSystem == DPcSystemWin32)
int j,k;
#endif

IFE(OgStartupSockets(param->herr));

if (gethostname(hostname, DPcPathSize) != 0) {
  int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
  sprintf(erreur,"OgRequest: gethostname: (%d) %s",nerr,sys_erreur);
  PcErr(-1,erreur); DPcErr;
  }

sprintf(buffer,               "Usage : ogucic [-v] [-h] [options] request_file [answer_file]\n");
sprintf(buffer+strlen(buffer),"        default for anwer_file is '%s'\n",DOgUcicOutputFile);
sprintf(buffer+strlen(buffer),"options are:\n");
sprintf(buffer+strlen(buffer),"  -e<period in milli-secs> sleep between each requests (-i option) (default %d)\n", DOgUcicSleepPeriod);
sprintf(buffer+strlen(buffer),"  -h prints this message\n");
sprintf(buffer+strlen(buffer),"  -header sends also header in <answer_file>\n");
sprintf(buffer+strlen(buffer),"  -i<filename> reads full xml request (one by line)\n");
sprintf(buffer+strlen(buffer),"  -loop loops sending requests from <filename> (option -i)\n");
sprintf(buffer+strlen(buffer),"  -n<hostname> (default %s)\n", hostname);
sprintf(buffer+strlen(buffer),"  -o<filename> outputs all results from -i requests in <filename>\n");
sprintf(buffer+strlen(buffer),"    default is no output at all\n");
sprintf(buffer+strlen(buffer),"  -p<port number> (default %d)\n", DOgUciPortNumber);
sprintf(buffer+strlen(buffer),"  -s<buffer_size> (default %d)\n", DOgUciBufferSize);
sprintf(buffer+strlen(buffer),"  -r<nb_requests_per_log> number of request per log (default %d)\n", DOgUciNbRequestsPerLog);
sprintf(buffer+strlen(buffer),"  -t<n>: trace options for logging (default %x)\n");
sprintf(buffer+strlen(buffer),"    <n> has a combined hexadecimal value of:\n");
sprintf(buffer+strlen(buffer),"    0x1: minimal, 0x2: memory, 0x4: socket, 0x8: socket size\n");
sprintf(buffer+strlen(buffer),"  -v gives version number of the program\n");
sprintf(buffer+strlen(buffer),"  -w<timeout in seconds> (default %d)\n", DOgUcicTimeout);

#if ( DPcSystem == DPcSystemWin32)
for (j=k=0; buffer[j]; j++) {
  if (buffer[j]=='\n') { edit_buffer[k++]='\r'; edit_buffer[k++]='\n'; }
  else edit_buffer[k++]=buffer[j];
  }
edit_buffer[k-2]=0; /* removing last CR */
OgEditBox(hInstance,nCmdShow,"ogucic help",edit_buffer,480,280);
#else
OgMessageBox(0,buffer,param->loginfo.where,DOgMessageBoxInformation);
#endif

OgCleanupSocket();

DONE;
}





STATICF(void) OgExit(herr,where)
void *herr; char *where;
{
int is_error;
time_t ltime;
is_error=OgErrLog(herr,where,0,0,0,0);

time(&ltime);
OgMessageLog(DOgMlogInLog,where,0,"\nProgram %s.exe exiting on error at %.24s\n",where,OgGmtime(&ltime));
exit(1);
} 



