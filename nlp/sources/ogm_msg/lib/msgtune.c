/*
 *	Tuning parameters for msg library.
 *	Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *	Dev : March,April 2007
 *	Version 1.1
*/
#include "ogm_msg.h"




PUBLIC(int) OgMsgTune(void *handle, int type, void *value)
{
struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;

switch (type) {
  case DOgMsgTuneMask:
    ctrl_msg->default_mask = (int)value;
    break;  
  case DOgMsgTuneShowTimestamp:
    ctrl_msg->show_timestamp = (int)value;
    break;  
  case DOgMsgTuneShowId:
    ctrl_msg->show_id = (int)value;
    break;  
  }

DONE;
}




PUBLIC(int) OgMsgTuneInherit(void *handle1, void *handle2)
{
struct og_ctrl_msg *ctrl_msg1 = (struct og_ctrl_msg *)handle1;
struct og_ctrl_msg *ctrl_msg2 = (struct og_ctrl_msg *)handle2;
char module_name[DPcPathSize];

IFn(handle2) DONE;

ctrl_msg1->default_mask = ctrl_msg2->default_mask;
ctrl_msg1->show_timestamp = ctrl_msg2->show_timestamp;
ctrl_msg1->show_id = ctrl_msg2->show_id;

strcpy(ctrl_msg1->prog_name,ctrl_msg2->prog_name);

snprintf(module_name,DPcPathSize-1,"%s/%s",ctrl_msg2->module_name,ctrl_msg1->module_name);
module_name[DPcPathSize-1]=0;
strcpy(ctrl_msg1->module_name,module_name);

ctrl_msg1->pipe = ctrl_msg2->pipe; // TODO : pipe
DONE;
}



