/*
 *	Automaton for ogm_pho functions
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006
 *  Version 1.0
*/
#include "ogm_pho.h"






int PhoRulesAutRuleAdd(ctrl_pho,iword,word,Irule)
struct og_ctrl_pho *ctrl_pho;
char *word;
int Irule;
{
char buffer[DPcPathSize*2],*p;
struct rule *rule=ctrl_pho->Rule+Irule;
int ibuffer=0,iB=0;

/* step:word:Irule tout en Unicode */
p=buffer;
OggNout(rule->step,&p);
memcpy(p,word,iword); p+=iword;
*p++=0;
*p++=1;
OggNout(Irule,&p);
ibuffer = p-buffer;
IFE(OgAutAdd(ctrl_pho->ha_rules,ibuffer,buffer));

DONE;
}

