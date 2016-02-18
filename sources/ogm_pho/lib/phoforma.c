/*
 *	Format function for Phonet function
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : December 2006
 *  Version 1.0
*/
#include "ogm_pho.h"



int PhoFormatClean(ctrl_pho)
struct og_ctrl_pho *ctrl_pho;
{
unsigned char c;
int i,j,k;

OgUniStrlwr(ctrl_pho->iinput,ctrl_pho->input,ctrl_pho->input);

if (ctrl_pho->loginfo->trace & DOgPhoTraceMain) {
  OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"OgPhonet: non_alpha_to_space = %d",ctrl_pho->non_alpha_to_space);
  }

for(i=0;i<ctrl_pho->iinput;i+=2) {
  
  for(k=0;k<ctrl_pho->appending_characters_number;k++) {
    if(  ctrl_pho->input[i] == ctrl_pho->appending_characters[k*2]
      && ctrl_pho->input[i+1] == ctrl_pho->appending_characters[k*2+1]) {
      for(j=i;j<ctrl_pho->iinput-2;j++) {
        ctrl_pho->input[j]=ctrl_pho->input[j+2];
        }
      ctrl_pho->iinput -= 2;
      }
    }
  if(!ctrl_pho->non_alpha_to_space) continue;
  c = (ctrl_pho->input[i]<<8)+ctrl_pho->input[i+1];
  if(!OgUniIsalpha(c)) {
    memcpy(ctrl_pho->input+i,ctrl_pho->space_character,2);
    }
  }

DONE;
}





int PhoFormatAppendingCharAdd(ctrl_pho,ib,b)
struct og_ctrl_pho *ctrl_pho;
int ib;
unsigned char *b;
{
if(ib<2) {
  
  DONE;
  }

if(ib>2) {
  
  }

memcpy(ctrl_pho->appending_characters+ctrl_pho->appending_characters_number*2,b,2);
ctrl_pho->appending_characters_number++;

DONE;
}

