/**
 *  This is the prefix we add at the beginning of each message
 *  Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *  Dev : March,April 2007
 *  Version 1.1
 */
#include "ogm_msg.h"

int OgMsgTemplateBuild(struct og_ctrl_msg *ctrl_msg, char *id, char *stemplate, int mask)
{
  char sdate[DPcPathSize], sseverity[DPcPathSize], sid[DPcPathSize];
  char module_name[DPcPathSize];


  sdate[0] = 0;
  if (ctrl_msg->show_timestamp)
  {
    time_t ltime;

    time(&ltime);
    struct tm *gmt = gmtime(&ltime);
    strftime(sdate, DPcPathSize, "%Y-%m-%dT%H:%M:%SZ", gmt);
  }

  sseverity[0] = 0;
  switch (mask & 0xf)
  {
    case DOgMsgSeverityEmergency:
      strcpy(sseverity, "EMERG");
      break;
    case DOgMsgSeverityAlert:
      strcpy(sseverity, "ALERT");
      break;
    case DOgMsgSeverityCritical:
      strcpy(sseverity, "CRITI");
      break;
    case DOgMsgSeverityError:
      strcpy(sseverity, "ERROR");
      break;
    case DOgMsgSeverityWarning:
      strcpy(sseverity, "WARNG");
      break;
    case DOgMsgSeverityNotice:
      strcpy(sseverity, "NOTIC");
      break;
    case DOgMsgSeverityInfo:
      strcpy(sseverity, "INFOR");
      break;
    case DOgMsgSeverityDebug:
      strcpy(sseverity, "DEBUG");
      break;
  }

  module_name[0] = 0;
  if (ctrl_msg->show_id & DOgMsgTuneShowIdModule)
  {
    strcpy(module_name, ctrl_msg->module_name);
  }

  sid[0] = 0;
  if (ctrl_msg->show_id & DOgMsgTuneShowIdMessage)
  {
    if (id[0])
    {
      if (module_name[0])
      {
        sprintf(sid, " %s:%s", module_name, id);
      }
      else
      {
        sprintf(sid, " %s", id);
      }
    }
    else if (module_name[0])
    {
      sprintf(sid, " %s", module_name);
    }
  }
  else if (module_name[0])
  {
    sprintf(sid, " %s", module_name);
  }

  stemplate[0] = 0;
  if (sdate[0] && sseverity[0])
  {
    sprintf(stemplate, "[%s] [%s%s] ", sdate, sseverity, sid);
  }
  else if (sdate[0] && sid[0])
  {
    sprintf(stemplate, "[%s] [%s] ", sdate, sid);
  }
  else if (sdate[0])
  {
    sprintf(stemplate, "[%s] ", sdate);
  }
  else if (sseverity[0] && sid[0])
  {
    sprintf(stemplate, "[%s] [%s] ", sseverity, sid);
  }
  else if (sseverity[0])
  {
    sprintf(stemplate, "[%s] ", sseverity);
  }
  else if (sid[0])
  {
    sprintf(stemplate, "[%s] ", sid + 1);
  }

  DONE;
}

