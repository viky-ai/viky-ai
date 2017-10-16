/*
 *  Initialization for ogm_uci functions
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : July,September 2006
 *  Version 1.1
*/
#include "ogm_uci.h"




PUBLIC(int) OgUciClientRequest(handle,request,answer)
void *handle; struct og_ucic_request *request;
struct og_ucic_answer *answer;
{
struct og_ctrl_ucic *ctrl_ucic = (struct og_ctrl_ucic *)handle;
char sys_erreur[DOgErrorSize], erreur[DOgErrorSize];
struct og_ucir_output coutput,*output=&coutput;
struct og_ucir_input cinput,*input=&cinput;
int iheader; char header[DPcPathSize];
int nb_sent_chars;

memset(answer,0,sizeof(struct og_ucic_answer));

IFn(request->port) request->port=DOgUciPortNumber;
if (request->hostname[0]==0) {
  if (gethostname(request->hostname, DPcPathSize) != 0) {
    int nerr = OgSysErrMes(DOgSocketErrno,DPcSzSysErr,sys_erreur);
    sprintf(erreur,"OgUciClientRequest: gethostname: (%d) %s",nerr,sys_erreur);
    OgErr(ctrl_ucic->herr,erreur); DPcErr;
    }
  }

ctrl_ucic->request=request;

/** Encapsulated socket creation function **/
IFE(ctrl_ucic->hsocket=OgCallSocket(ctrl_ucic->herr,request->hostname,request->port));

/** Sending the header **/
sprintf(header,
  "POST /ssrv HTTP/1.1\r\n"
  "User-Agent: Pertimm UCI/1.0\r\n"
  "Content-Type: text/xml\r\n"
  "Content-length: %d\r\n"
  "\r\n"
  , request->request_length);
iheader=strlen(header);

ctrl_ucic->BaUsed=0;
IFE(UcicAppendBa(ctrl_ucic,iheader,header));
IFE(UcicAppendBa(ctrl_ucic,request->request_length,request->request));

if (ctrl_ucic->loginfo->trace & DOgUciClientTraceSocket) {
  OgMsg(ctrl_ucic->hmsg, "", DOgMsgDestInLog
    , "OgUciClientRequest: sending %d bytes to socket", ctrl_ucic->BaUsed);
  }

IF(nb_sent_chars=OgSendSocket(ctrl_ucic->herr,ctrl_ucic->hsocket,ctrl_ucic->Ba,ctrl_ucic->BaUsed)) {
  OgCloseSocket(ctrl_ucic->hsocket);
  DPcErr;
  }

if (ctrl_ucic->loginfo->trace & DOgUciClientTraceSocket) {
  OgMsg(ctrl_ucic->hmsg, "", DOgMsgDestInLog
    , "OgUciClientRequest: sent %d bytes to socket", nb_sent_chars);
  }

memset(input, 0, sizeof(struct og_ucir_input));
input->hsocket=ctrl_ucic->hsocket;
input->timeout=request->timeout;

if (ctrl_ucic->loginfo->trace & DOgUciClientTraceSocket) {
  OgMsg(ctrl_ucic->hmsg, "", DOgMsgDestInLog
    , "OgUciClientRequest: reading answer with timeout %d seconds", input->timeout);
  }

IF(OgUciRead(ctrl_ucic->huci,input,output)) {
  if (output->timed_out) answer->timed_out=1;
  OgCloseSocket(ctrl_ucic->hsocket);
  DPcErr;
  }

OgCloseSocket(ctrl_ucic->hsocket);

answer->header_length=output->header_length;
answer->answer_length=output->content_length;
answer->answer=output->content;

DONE;
}




