/*
 *	Matching for ogm_matching functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"



STATICF(int) AllocMatching(pr_(struct og_ctrl_pho *) pr(struct matching **));



int PhoMatchingRules(ctrl_pho,step)
struct og_ctrl_pho *ctrl_pho;
int step;
{
int i;

for(i=0;i<ctrl_pho->ibufferIn;i+=2) {
  IFE(PhoRulesRuleGet(ctrl_pho,i,step));
  }

DONE;
}








int PhoMatchingAdd(ctrl_pho,offset,Irule)
struct og_ctrl_pho *ctrl_pho;
int offset,Irule;
{
struct matching *matching;
struct rule *rule=ctrl_pho->Rule+Irule;
int Imatching;

IFE(Imatching=AllocMatching(ctrl_pho,&matching));
matching->offset=offset+rule->ileft;
matching->Irule=Irule;

DONE;
}



STATICF(int) AllocMatching(ctrl_pho,pmatching)
struct og_ctrl_pho *ctrl_pho;
struct matching **pmatching;
{
char erreur[DOgErrorSize];
struct matching *matching = 0;
int i=ctrl_pho->MatchingNumber;

beginAllocMatching:

if (ctrl_pho->MatchingUsed < ctrl_pho->MatchingNumber) {
  i = ctrl_pho->MatchingUsed++; 
  }

if (i == ctrl_pho->MatchingNumber) {
  unsigned a, b; struct matching *og_l;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocMatching: max Matching number (%d) reached", ctrl_pho->MatchingNumber);
    }
  a = ctrl_pho->MatchingNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct matching *)malloc(b*sizeof(struct matching))) {
    sprintf(erreur,"AllocMatching: malloc error on Matching");
    OgErr(ctrl_pho->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_pho->Matching, a*sizeof(struct matching));
  DPcFree(ctrl_pho->Matching); ctrl_pho->Matching = og_l;
  ctrl_pho->MatchingNumber = b;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMemory) {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"AllocMatching: new Matching number is %d\n", ctrl_pho->MatchingNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocMatching: MatchingNumber reached (%d)",ctrl_pho->MatchingNumber);
  OgErr(ctrl_pho->herr,erreur); DPcErr;
#endif

  goto beginAllocMatching;
  }

matching = ctrl_pho->Matching + i;
memset(matching,0,sizeof(struct matching));
matching->ctrl_pho = ctrl_pho;

if (pmatching) *pmatching = matching;
return(i);
}











int MatchingLog(ctrl_pho,step)
struct og_ctrl_pho *ctrl_pho;
int step;
{
int i,j,last_offset=(-2),iB1,iB;
unsigned char B[DPcPathSize];
struct matching *matching;
char B1[DPcPathSize*2];
struct rule *rule;
char message[25];

IFE(OgUniToCp(ctrl_pho->ibufferIn,ctrl_pho->bufferIn,DPcPathSize*2,&iB1,B1,DOgCodePageANSI,0,0));
OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog
     ,"MatchingLog: -- *STEP %d* listing matching rules for '%.*s' --",step,iB1,B1);

for(i=0;i<ctrl_pho->MatchingUsed;i++) {
  matching=ctrl_pho->Matching+i;
  rule=ctrl_pho->Rule+matching->Irule;

  if(matching->offset>last_offset) {
    for(j=last_offset+2;j<matching->offset+1;j+=2) {
      IFE(OgUniToCp(2,ctrl_pho->bufferIn+j,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
      OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"MatchingLog: -- OFFSET %.*s",iB1,B1);
      }
    last_offset=matching->offset;
    }
  
  if(matching->selected) strcpy(message," SELECTED");
  else strcpy(message,"");

  /*
  %s:%s:%s::%s
  rule->left,rule->key,rule->right,rule->phonetic
  */
  iB=0;
  memcpy(B+iB,rule->left,rule->ileft); iB+=rule->ileft;
  B[iB++]='\0';B[iB++]=':';
  memcpy(B+iB,rule->key,rule->ikey); iB+=rule->ikey;
  B[iB++]='\0';B[iB++]=':';
  memcpy(B+iB,rule->right,rule->iright); iB+=rule->iright;
  B[iB++]='\0';B[iB++]='-';
  memcpy(B+iB,rule->phonetic,rule->iphonetic); iB+=rule->iphonetic;
  
  IFE(OgUniToCp(iB,B,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
  
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog
    ,"MatchingLog: RULE Irule=%.3d (%d*%.*s*%d) %s"
    ,matching->Irule,rule->step,iB1,B1,rule->replace,message);
  }

for(j=last_offset+2;j<ctrl_pho->ibufferIn;j+=2) {
  IFE(OgUniToCp(2,ctrl_pho->bufferIn+j,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"MatchingLog: -- OFFSET %.*s",iB1,B1);
  }

DONE;
}