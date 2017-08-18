/*
 *  Logs whatever is possible to log on emergency
 *  Copyright (c) 2007 Pertimm by Patrick Constant
 *  Dev : September 2007
 *  Version 1.0
*/
#include "ogm_nls.h"



static int NlsOnEmergency1(struct og_ctrl_nls *, int);



/*
 *  We logs all the current requests.
*/

int NlsOnEmergency(struct og_ctrl_nls *ctrl_nls)
{
int i;

if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal) {
  OgMsg(ctrl_nls->hmsg,"",DOgMsgDestInLog,"NlsOnEmergency: starting");
  }

if (ctrl_nls->LtNumber > 0) {
  for (i=0; i<ctrl_nls->LtNumber; i++) {
    IFE(NlsOnEmergency1(ctrl_nls,i));
    }
  }

if (ctrl_nls->loginfo->trace & DOgNlsTraceMinimal) {
  OgMsg(ctrl_nls->hmsg,"",DOgMsgDestInLog,"NlsOnEmergency: finished");
  }

DONE;
}





static int NlsOnEmergency1(struct og_ctrl_nls *ctrl_nls, int Ilt)
{
  if (ctrl_nls->Lt != NULL)
  {

    struct og_listening_thread *lt = ctrl_nls->Lt + Ilt;

    IFn(lt) DONE;
    IFn(lt->output->content) DONE;
    if (!lt->running) DONE;

    if (lt->loginfo->trace & DOgNlsTraceMinimal)
    {
      IFE(NlsRequestLog(lt, "NlsOnEmergency1", NULL, DOgMsgDestInErr));
    }

  }
  DONE;
}



