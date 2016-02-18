/*
 *	Changes the mask for the OgMessageLog function.
 * 	Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *	Dev : March,April 2007
 *	Version 1.1
*/
#include "ogm_msg.h"



int OgMsgMessageLogMask(int mask)
{
int mlog_mask=0;

if (mask & DOgMsgDestInLog)    mlog_mask |= DOgMlogInLog;
if (mask & DOgMsgDestMBox)     mlog_mask |= DOgMlogMBox;
if (mask & DOgMsgDestInErr)    mlog_mask |= DOgMlogInErr;
if (mask & DOgMsgDestSysErr)   mlog_mask |= DOgMlogSysErr;
if (mask & DOgMsgParamLogDate) mlog_mask |= DOgMlogDate;
if (mask & DOgMsgParamDateIn)  mlog_mask |= DOgMlogDateIn;
if (mask & DOgMsgParamNoCr)    mlog_mask |= DOgMlogNoCr;
if (mask & DOgMsgParamUnicode) mlog_mask |= DOgMlogUnicode;

return(mlog_mask);
}




