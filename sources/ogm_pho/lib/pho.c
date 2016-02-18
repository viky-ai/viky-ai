/*
 *	Main program for Phonet function
 *  Copyright (c) 2008 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006, april 2008
 *  Version 1.1
*/
#include "ogm_pho.h"



STATICF(int) PhoResultSendClean(pr_(struct og_ctrl_pho *) pr_(struct og_pho_output *) pr(int));



PUBLIC(int) OgPhonet(handle,input,output)
void *handle;
struct og_pho_input *input;
struct og_pho_output *output;
{
struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *)handle;
char B1[DPcPathSize*2],B2[DPcPathSize*2];
int iB1=0,iB2=0,step;

output->iB=0;
IFn(ctrl_pho) DONE;

if(input->iB>4*DPcPathSize) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"OgPhonet: input string too long (%d>%d)",input->iB,4*DPcPathSize);
  DONE;
  }

memcpy(ctrl_pho->input,input->B,input->iB);
ctrl_pho->iinput=input->iB;

IFE(PhoFormatClean(ctrl_pho));

if (ctrl_pho->loginfo->trace & DOgPhoTraceMain) {
    IFE(OgUniToCp(input->iB,input->B,DPcPathSize*2,&iB1,B1,DOgCodePageANSI,0,0));
    IFE(OgUniToCp(ctrl_pho->iinput,ctrl_pho->input,DPcPathSize*2,&iB2,B2,DOgCodePageANSI,0,0));
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog
         ,"OgPhonet: Entry is '%.*s' (%d) cleaned to '%.*s'",iB1,B1,iB1,iB2,B2);
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"OgPhonet: number of steps : %d",ctrl_pho->max_steps);
  }

/* here doing the loop for rules steps */
for(step=1;step<ctrl_pho->max_steps+1;step++) {

  ctrl_pho->MatchingUsed=0;
  ctrl_pho->BaUsed=0;

  output->iB=0;

  memcpy(ctrl_pho->bufferIn,ctrl_pho->space_character,2);
  memcpy(ctrl_pho->bufferIn+2,ctrl_pho->space_character,2);
  memcpy(ctrl_pho->bufferIn+4,ctrl_pho->input,ctrl_pho->iinput);
  memcpy(ctrl_pho->bufferIn+4+ctrl_pho->iinput,ctrl_pho->space_character,2);
  memcpy(ctrl_pho->bufferIn+6+ctrl_pho->iinput,ctrl_pho->space_character,2);
  memcpy(ctrl_pho->bufferIn+8+ctrl_pho->iinput,ctrl_pho->space_character,2);
  ctrl_pho->ibufferIn=ctrl_pho->iinput+10;
  
  if (ctrl_pho->loginfo->trace & DOgPhoTraceMain) {
    IFE(OgUniToCp(ctrl_pho->ibufferIn,ctrl_pho->bufferIn,DPcPathSize*2,&iB2,B2,DOgCodePageANSI,0,0));
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog
         ,"OgPhonet: *STEP %d* starting to phonetize '%.*s'",step,iB2,B2);
    }

  IFE(PhoMatchingRules(ctrl_pho,step));

  IFE(PhoAnalysing(ctrl_pho));

  if (ctrl_pho->loginfo->trace & DOgPhoTraceRulesMatch) {
    IFE(MatchingLog(ctrl_pho,step));
    }

  IFE(PhoWriting(ctrl_pho));

  IFE(PhoResultSendClean(ctrl_pho,output,step));

  memcpy(ctrl_pho->input,output->B,output->iB);
  ctrl_pho->iinput=output->iB;
  }

DONE;
}




STATICF(int) PhoResultSendClean(ctrl_pho,output,step)
struct og_ctrl_pho *ctrl_pho;
struct og_pho_output *output;
int step;
{
int i,start=0,end=0;

for(i=0;i<ctrl_pho->BaUsed;i+=2) {
  if(!memcmp(ctrl_pho->Ba+i,ctrl_pho->space_character,2)) continue;
  start=i;
  break;
  }
for(i=ctrl_pho->BaUsed-2;i>=0;i-=2) {
  if(!memcmp(ctrl_pho->Ba+i,ctrl_pho->space_character,2)) continue;
  end=i;
  break;
  }

if(start>=end) {
  output->B=ctrl_pho->Ba;
  output->iB=0;
  }
  
if(step==ctrl_pho->max_steps) {
  for(i=start;i<=end;i+=2) {
    if(!memcmp(ctrl_pho->Ba+i,ctrl_pho->space_character,2)) {
      ctrl_pho->Ba[i]='\0';
      ctrl_pho->Ba[i+1]=' ';
      }
    }
  }
  
output->B=ctrl_pho->Ba+start;
output->iB=end-start+2;

DONE;
}


