/*
 *  Flow chart function for Linguistic trf search
 *  Copyright (c) 2009,2010 Pertimm, by Patrick Constant
 *  Dev : November 2009, January 2010
 *  Version 1.1
*/
#include "ogm_ltras.h"


static int LtrasFlowChartParse(struct og_ctrl_ltras *ctrl_ltras,unsigned char *flow_chart);




PUBLIC(og_status) OgLtrasFlowChart(void *handle, struct og_ltras_input *input
  , struct og_ltra_trfs **output)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct og_ltra_trfs *trfs;

IFE(OgLtrasParse(ctrl_ltras, input, &trfs));

/** Parsing and creating the boolean tree **/
IFE(LtrasFlowChartParse(ctrl_ltras,input->flow_chart));

/** Reading the boolean tree and executing the trfs **/
IFE(LtrasFlowChartBoolean(ctrl_ltras,trfs,output));

if (trfs != *output) {
  IFE(OgLtrasTrfsDestroy(ctrl_ltras,trfs));
  }

DONE;
}




/*
 * Transforming typically :
 * m1-(m2/m3-m4/m5)-m6
 * -> m1 or (m2 and m3 or m4 and m5) or m6
 * m1-m2/m3-m4/m5-m6
 * -> m1 or m2 and m3 or m4 and m5 or m6
 * m1-m2,m3-m4,m5-m6
 * -> (m1 or m2) and (m3 or m4) and (m5 or m6)
 * then parsing with rqp and finally handling the boolean tree
 * to run the processes.
*/

static int LtrasFlowChartParse(struct og_ctrl_ltras *ctrl_ltras,unsigned char *flow_chart)
{
int iunicode; unsigned char *unicode;
int BaStart=ctrl_ltras->BaUsed;
int is; unsigned char *s;
int i,length;

s=flow_chart; is=strlen(s);

IFE(LtrasAppendBa(ctrl_ltras,1,"("));
for (i=0; i<is; i++) {
  if (s[i]==';') {
    IFE(LtrasAppendBa(ctrl_ltras,7,") and ("));
    }
  else if (s[i]=='-') {
    IFE(LtrasAppendBa(ctrl_ltras,4," or "));
    }
  else if (s[i]=='/') {
    IFE(LtrasAppendBa(ctrl_ltras,5," and "));
    }
  else {
    IFE(LtrasAppendBa(ctrl_ltras,1,s+i));
    }
  }
IFE(LtrasAppendBa(ctrl_ltras,2,")"));

length = ctrl_ltras->BaUsed - BaStart -1;
IFE(LtrasTestReallocBa(ctrl_ltras,length*2+DPcPathSize));
unicode = ctrl_ltras->Ba + ctrl_ltras->BaUsed;
IFE(OgCpToUni(length, ctrl_ltras->Ba+BaStart
  , length*2, &iunicode, unicode, DOgCodePageUTF8, 0, 0));
ctrl_ltras->BaUsed += iunicode;

IFE(OgRqpParse(ctrl_ltras->hrqp,iunicode,unicode));

if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleFlowChart) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
    , "LtrasFlowChartParse: flowchart is '%s'", s);
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
    , "LtrasFlowChartParse: boolean flowchart is '%s'", ctrl_ltras->Ba+BaStart);
  IFE(OgRqpLogPrettyTree(ctrl_ltras->hrqp));
  }

DONE;
}




