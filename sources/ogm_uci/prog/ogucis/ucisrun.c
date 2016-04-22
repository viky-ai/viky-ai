/*
 *  Running the Uci demonstration server
 *  Copyright (c) 2006 Pertimm
 *  Dev : July 2006
 *  Version 1.0
*/
#include "ogucis.h"


STATICF(int) UcisRun1(pr_(void *) pr(struct og_socket_info *));
STATICF(int) UcisReceivedStop(pr_(struct og_ucis_ctrl *) pr(struct og_ucisr_output *));
STATICF(int) UcisAnswerStop(pr_(struct og_ucis_ctrl *) pr(int));



/*
 *  Listens to a socket and answers it.
*/

int UcisRun(ucis_ctrl)
struct og_ucis_ctrl *ucis_ctrl;
{
struct og_conf_lines *addresses = &ucis_ctrl->addresses;
struct og_addr_param caddr_param, *addr_param=&caddr_param;
time_t clock_start;
int i;

memset(addr_param,0,sizeof(struct og_addr_param));
addr_param->herr = ucis_ctrl->herr;
addr_param->hmsg = ucis_ctrl->hmsg;
addr_param->hmutex = ucis_ctrl->hmutex;
addr_param->loginfo.trace = DOgAddrTraceMinimal+DOgAddrTraceMemory;
addr_param->loginfo.where = ucis_ctrl->loginfo->where;
IFn(ucis_ctrl->haddr=OgAddrInit(addr_param)) DPcErr;

if (ucis_ctrl->loginfo->trace & DOgUciServerTraceMinimal) {
  OgThrMessageLog(ucis_ctrl->hmutex,DOgMlogInLog,ucis_ctrl->loginfo->where,0,"UcisRun starting with following adresses:");
  }

IFE(OgConfGetVarLines(DOgFileOgmConf_Txt, "UcisAddresses", addresses, ucis_ctrl->loginfo->where));
IFn(addresses->nb_values) {
  char saddress[DPcPathSize];
  sprintf(saddress,"0.0.0.0:%d",DOgUciPortNumber);
  IFE(OgAddrAdd(ucis_ctrl->haddr,saddress,DOgUciPortNumber));
  if (ucis_ctrl->loginfo->trace & DOgUciServerTraceMinimal) {
    OgMessageLog( DOgMlogInLog, ucis_ctrl->loginfo->where, 0,"  %s",saddress);
    }
  /** hostname 0.0.0.0 does not work for calling sockets **/
  sprintf(saddress,"127.0.0.1:%d",DOgUciPortNumber);
  IFE(OgParseServerAddress(saddress,ucis_ctrl->hostname,&ucis_ctrl->port_number));
  }
else {
  char *address;
  for (i=0; i<addresses->nb_values; i++) {
    address = addresses->buffer + addresses->value[i];
    IFE(OgAddrAdd(ucis_ctrl->haddr,address,DOgUciPortNumber));
    if (ucis_ctrl->loginfo->trace & DOgUciServerTraceMinimal) {
      OgMessageLog( DOgMlogInLog, ucis_ctrl->loginfo->where, 0,"  %s",address);
      }
    }
  address = addresses->buffer + addresses->value[0];
  ucis_ctrl->port_number = DOgUciPortNumber; /** Necessary if the port is not given **/
  IFE(OgParseServerAddress(address,ucis_ctrl->hostname,&ucis_ctrl->port_number));
  /** hostname 0.0.0.0 does not work for calling sockets **/
  if (!strcmp(ucis_ctrl->hostname,"0.0.0.0")) strcpy(ucis_ctrl->hostname,"127.0.0.1");
  }

OgMessageLog(DOgMlogInLog,ucis_ctrl->loginfo->where,0,"Listening ...");

ucis_ctrl->must_stop=0;
IFE(OgAddrLoop(ucis_ctrl->haddr,UcisRun1,(void *)ucis_ctrl));

clock_start=OgClock();

IFE(OgConfGetVarLinesFlush(addresses));
IFE(OgAddrFlush(ucis_ctrl->haddr));

if (ucis_ctrl->loginfo->trace & DOgUciServerTraceMinimal) {
  OgThrMessageLog(ucis_ctrl->hmutex,DOgMlogInLog,ucis_ctrl->loginfo->where,0
    ,"UcisRun: finished after %d milli-seconds",OgClock()-clock_start);
  }

DONE;
}





STATICF(int) UcisRun1(ptr,info)
void *ptr; struct og_socket_info *info;
{
struct og_ucis_ctrl *ucis_ctrl = (struct og_ucis_ctrl *)ptr;
int xml_content_length; unsigned char *xml_content;
struct og_ucisr_output coutput,*output=&coutput;
struct og_ucisw_input cwinput,*winput=&cwinput;
struct og_ucisr_input cinput,*input=&cinput;
int must_stop;

if (ucis_ctrl->must_stop) {
  if (ucis_ctrl->loginfo->trace & DOgUciServerTraceMinimal) {
    OgThrMessageLog(ucis_ctrl->hmutex, DOgMlogInLog, ucis_ctrl->loginfo->where, 0, "UcisRun must stop");
    }
  return(1);
  }

/** Here we get the IP address of the calling program (direct socket) **/
IFE(OgGetRemoteAddrSocket(&info->socket_in.sin_addr,ucis_ctrl->sremote_addr,0));
if (ucis_ctrl->loginfo->trace & DOgUciServerTraceMinimal) {
  OgThrMessageLog(ucis_ctrl->hmutex,DOgMlogInLog+DOgMlogDateIn,ucis_ctrl->loginfo->where,0
    ,"UcisRun1: receiving request on '%s' at port %d from %s"
    , info->address, info->port, ucis_ctrl->sremote_addr);
  }

/** Now we read the request and answer the request as an answer **/
memset(input,0,sizeof(struct og_ucisr_input));
input->hsocket = info->hsocket_service;
input->timeout = 30000;

IFE(OgUciServerRead(ucis_ctrl->hucis,input,output));

IFE(must_stop=UcisReceivedStop(ucis_ctrl,output));
if (must_stop) {
  OgThrMessageLog(ucis_ctrl->hmutex, DOgMlogInLog, ucis_ctrl->loginfo->where, 0, "UcisRun1 stopping");
  IFE(UcisAnswerStop(ucis_ctrl,input->hsocket));
  OgCloseSocket(info->hsocket_service);
  return(1);
  }

xml_content_length = output->content_length - output->header_length;
xml_content = output->content + output->header_length;

memset(winput,0,sizeof(struct og_ucisw_input));
winput->content_length=xml_content_length;
winput->content=xml_content;
winput->hsocket=input->hsocket;

IF(OgUciServerWrite(ucis_ctrl->hucis,winput)) {
  OgThrMessageLog(ucis_ctrl->hmutex, DOgMlogInLog+DOgMlogInErr, ucis_ctrl->loginfo->where, 0
    ,"OgUciServerWrite: connexion was prematurely closed by client, going on");
  }

return(0);
}



/*
 *  The stop message can be anything, for example:
 *    <?xml version="1.0" encoding="UTF-8">
 *    <control_commands>
 *      <control_command name="stop"/>
 *    </control_commands>
 *  The UCI api does not handle this as it should
 *  not understand the content of the XML buffer.
*/

STATICF(int) UcisReceivedStop(ucis_ctrl,output)
struct og_ucis_ctrl *ucis_ctrl;
struct og_ucisr_output *output;
{
int is=output->content_length;
unsigned char *s=output->content;
unsigned char *stop="<control_command name=\"stop\"/>"; int istop=sizeof("<control_command name=\"stop\"/>")-1;
int i,state=1;

/** We check the top level tag to make avoid confusion with other request types **/
if (Ogstricmp(output->top_level_tag,"control_commands")) return(0);

for (i=0; i<is; i++) {
  if (i+istop <is && !Ogmemicmp(s+i,stop,istop)) return(1);
  }

return(0);
}



STATICF(int) UcisAnswerStop(ucis_ctrl,hsocket)
struct og_ucis_ctrl *ucis_ctrl; int hsocket;
{
struct og_ucisw_input cwinput,*winput=&cwinput;
unsigned char *answer =
   "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
   "<control_answers>\n"
   "  <control_answer name=\"stop\">ok</control_answer>\n"
   "</control_answers>\n";
int ianswer=strlen(answer);

memset(winput,0,sizeof(struct og_ucisw_input));
winput->content_length=ianswer;
winput->content=answer;
winput->hsocket=hsocket;

IF(OgUciServerWrite(ucis_ctrl->hucis,winput)) {
  OgThrMessageLog(ucis_ctrl->hmutex, DOgMlogInLog+DOgMlogInErr, ucis_ctrl->loginfo->where, 0
    ,"OgUciServerWrite: connexion was prematurely closed by client on a stop command, giving up");
  }

DONE;
}





int UcisStop(ucis_ctrl)
struct og_ucis_ctrl *ucis_ctrl;
{
DONE;
}




