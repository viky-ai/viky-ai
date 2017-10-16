/*
 *	Handling mask
 *	Copyright (c) 2007 Pertimm by Patrick Constant and Guillaume Logerot
 *	Dev : April 2007
 *	Version 1.0
*/
#include "ogm_msg.h"



static int OgMsgInterpretFlag(struct og_ctrl_msg *, char *, int *);




int MsgMaskMix(struct og_ctrl_msg *ctrl_msg, int mask, int *pgmask)
{
int gmask=0;

if (mask & 0xf) gmask = (mask & 0xf);
else gmask = (ctrl_msg->default_mask & 0xf);

gmask |= (mask | ctrl_msg->default_mask) & (~ 0xf);

*pgmask = gmask;

DONE;
}





PUBLIC(int) OgMsgMaskInterpret(void *handle, char *s, int *pmask)
{
struct og_ctrl_msg *ctrl_msg = (struct og_ctrl_msg *)handle;
int i,c,start=0,end=0,is = strlen(s);
char sflag[DPcPathSize];
int found,flag,mask=0;

for (i=0; !end; i++) {
  if (i>=is) { c='+'; end=1; }
  else c=s[i];
  if (PcIspunct(c)) {
    memcpy(sflag,s+start  ,i-start); sflag[i-start]=0; 
    OgTrimString(sflag,sflag);
    IFE(found=OgMsgInterpretFlag(ctrl_msg,sflag,&flag));
    if (found) {
      if (flag & 0xf) mask &= ~0xf;
      mask |= flag;
      }
    start=i+1;
    }
  }

*pmask = mask;

DONE;
}





static int OgMsgInterpretFlag(struct og_ctrl_msg *ctrl_msg, char *sflag, int *pflag)
{
int found=1;

if (!Ogstricmp(sflag,"SeverityEmergency")) *pflag = DOgMsgSeverityEmergency;
else if (!Ogstricmp(sflag,"SeverityAlert")) *pflag = DOgMsgSeverityAlert;
else if (!Ogstricmp(sflag,"SeverityCritical")) *pflag = DOgMsgSeverityCritical;
else if (!Ogstricmp(sflag,"SeverityError")) *pflag = DOgMsgSeverityError;
else if (!Ogstricmp(sflag,"SeverityWarning")) *pflag = DOgMsgSeverityWarning;
else if (!Ogstricmp(sflag,"SeverityNotice")) *pflag = DOgMsgSeverityNotice;
else if (!Ogstricmp(sflag,"SeverityInfo")) *pflag = DOgMsgSeverityInfo;
else if (!Ogstricmp(sflag,"SeverityDebug")) *pflag = DOgMsgSeverityDebug;

else if (!Ogstricmp(sflag,"DestInLog")) *pflag = DOgMsgDestInLog;
else if (!Ogstricmp(sflag,"DestMBox")) *pflag = DOgMsgDestMBox;
else if (!Ogstricmp(sflag,"DestInErr")) *pflag = DOgMsgDestInErr;
else if (!Ogstricmp(sflag,"DestSysErr")) *pflag = DOgMsgDestSysErr;

else if (!Ogstricmp(sflag,"ParamLogDate")) *pflag = DOgMsgParamLogDate;
else if (!Ogstricmp(sflag,"ParamDateIn")) *pflag = DOgMsgParamDateIn;
else if (!Ogstricmp(sflag,"ParamNoCr")) *pflag = DOgMsgParamNoCr;
else if (!Ogstricmp(sflag,"ParamUnicode")) *pflag = DOgMsgParamUnicode;

else found=0;

return(found);
}





PUBLIC(int) OgMsgMaskString(void *handle, int mask, char *smask)
{
int ismask,severity_mask = mask & 0xf;
smask[0]=0;

switch(severity_mask) {
  case DOgMsgSeverityEmergency:
    sprintf(smask+strlen(smask),"SeverityEmergency");
    break;
  case DOgMsgSeverityAlert:
    sprintf(smask+strlen(smask),"SeverityAlert");
    break;
  case DOgMsgSeverityCritical:
    sprintf(smask+strlen(smask),"SeverityCritical");
    break;
  case DOgMsgSeverityError:
    sprintf(smask+strlen(smask),"SeverityError");
    break;
  case DOgMsgSeverityWarning:
    sprintf(smask+strlen(smask),"SeverityWarning");
    break;
  case DOgMsgSeverityNotice:
    sprintf(smask+strlen(smask),"SeverityNotice");
    break;
  case DOgMsgSeverityInfo:
    sprintf(smask+strlen(smask),"SeverityInfo");
    break;
  case DOgMsgSeverityDebug:
    sprintf(smask+strlen(smask),"SeverityDebug");
    break;
  }

if (mask & DOgMsgDestInLog) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sDestInLog",ismask?"+":"");
  }
if (mask & DOgMsgDestMBox) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sDestMBox",ismask?"+":"");
  }
if (mask & DOgMsgDestInErr) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sDestInErr",ismask?"+":"");
  }
if (mask & DOgMsgDestSysErr) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sDestSysErr",ismask?"+":"");
  }

if (mask & DOgMsgParamLogDate) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sParamLogDate",ismask?"+":"");
  }
if (mask & DOgMsgParamDateIn) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sParamDateIn",ismask?"+":"");
  }
if (mask & DOgMsgParamNoCr) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sParamNoCr",ismask?"+":"");
  }
if (mask & DOgMsgParamUnicode) {
  ismask=strlen(smask); sprintf(smask+ismask,"%sParamUnicode",ismask?"+":"");
  }

DONE;
}






