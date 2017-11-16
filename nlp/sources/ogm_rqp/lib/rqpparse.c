/*
 *	Main function for parsing a request
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : May,June,July 2006
 *	Version 1.1
*/
#include "ogm_rqp.h"





PUBLIC(int) OgRqpParse(void *handle, int request_length, unsigned char *request)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;

ctrl_rqp->request_length = request_length;
ctrl_rqp->request = request;

IFE(OgRqpReset(handle));

IFE(RqpFirstParse(ctrl_rqp));
if (ctrl_rqp->loginfo->trace & DOgRqpTraceParse) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "OgRqpParse: after RqpFirstParse");
  IFE(OgRqpLogTree(ctrl_rqp));
  }
IFE(RqpApplyPriorities(ctrl_rqp));
if (ctrl_rqp->loginfo->trace & DOgRqpTraceParse) {
  OgMsg(ctrl_rqp->hmsg,"",DOgMsgDestInLog
    , "OgRqpParse: after RqpApplyPriorities");
  IFE(OgRqpLogTree(ctrl_rqp));
  }
IFE(RqpCreateStructure(ctrl_rqp));

DONE;
}




