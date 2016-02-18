/*
 *	Initialization for ogm_uci functions
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : July,November 2006
 *	Version 1.1
*/
#include "ogm_uci.h"



PUBLIC(int) OgUciServerWrite(handle,input)
void *handle; struct og_ucisw_input *input;
{
struct og_ctrl_ucis *ctrl_ucis = (struct og_ctrl_ucis *)handle;
int max_content_length=DOgMlogMaxMessageSize/2;
int iheader; char header[DPcPathSize];
int limited_content_length;

// Default content type
if(input->content_type == NULL)
{
  input->content_type = "text/xml";
}


/** Sending the header **/
sprintf(header,
  "HTTP/1.1 200 OK\r\n"
  "Connection: close\r\n"
  "Content-Type: %s\r\n"
  "Content-length: %d\r\n"
  "\r\n"
  , input->content_type, input->content_length);
iheader=strlen(header);

ctrl_ucis->BaUsed=0;
IFE(UcisAppendBa(ctrl_ucis,iheader,header));
IFE(UcisAppendBa(ctrl_ucis,input->content_length,input->content));

IF(OgSendSocket(ctrl_ucis->herr,input->hsocket, ctrl_ucis->Ba, ctrl_ucis->BaUsed)) {
  limited_content_length=input->content_length;
  if (limited_content_length>max_content_length) limited_content_length=max_content_length;
  /* This error happens when the client decides to close the connection 
   * before the server has sent all the data. From that point on, all
   * data sent will be refused, so we decide to stop sending anything. */
  OgMsgErr(ctrl_ucis->hmsg,"OgUciServerWrite",0,0,0,DOgMsgSeverityError
    ,DOgErrLogFlagNoSystemError+DOgErrLogFlagNotInErr);
  OgMsg(ctrl_ucis->hmsg,"",DOgMlogInLog+DOgMlogInErr+DOgMlogDateIn
    ,"OgUciServerWrite: error sending [\n%.*s] hsocket=%d",limited_content_length,input->content,input->hsocket);
  DPcErr;
  }

if (ctrl_ucis->loginfo->trace & DOgUciServerTraceSocket) {
  limited_content_length=input->content_length;
  if (limited_content_length>max_content_length) limited_content_length=max_content_length;
  OgMsg(ctrl_ucis->hmsg, "", DOgMlogInLog
    , "OgUciServerWrite: sending [\n%.*s]", limited_content_length, input->content);
  }

DONE;
}






