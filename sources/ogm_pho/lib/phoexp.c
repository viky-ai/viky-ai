/*
 *	Rules expansion
 *  Copyright (c) 2008 Pertimm by G.Logerot
 *  Dev : May 2008
 *  Version 1.0
*/
#include "ogm_pho.h"



int PhoRulesRuleAddExpense(ctrl_pho,iword,word,indice,Irule)
struct og_ctrl_pho *ctrl_pho;
int iword;
char *word;
int indice,Irule;
{
struct char_class *char_class;
char buffer[DPcPathSize*2];
int i,j,k,expanded=0;

memcpy(buffer,word,iword);

for(i=indice;i<iword && !expanded;i+=2) {
  for(j=0;j<ctrl_pho->Char_classUsed;j++) {
    char_class = ctrl_pho->Char_class + j;
    if(!memcmp(char_class->character,word+i,2)) {
      for(k=0;k<char_class->number;k++) {
        memcpy(buffer+i,ctrl_pho->BaClass+char_class->start+k*2,2);
        IFE(PhoRulesRuleAddExpense(ctrl_pho,iword,buffer,i+2,Irule));
        expanded=1;
        }
      break;
      }
    }
  }

if(i>=iword && !expanded) {
  IFE(PhoRulesAutRuleAdd(ctrl_pho,iword,word,Irule));
  DONE;
  }

DONE;
}