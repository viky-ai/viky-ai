/*
 *	Impressions de contr\^ole sur l'automate.
 *	Copyright (c) 1996-2008 Pertimm by Patrick Constant
 *	Dev: Mars,Avril,Juillet,Septembre 1996
 *  Dev: July 2008
 *	Version 1.1
*/
#include "ogm_aut.h"


/*
 *	Fonction d'impression de l'automate dont le handle est 'h'
 *	On imprime tous les \'etats et toutes les transitions.
*/

int AutPri(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
int i;
for (i=0; i<ctrl_aut->StateUsed; i++) {
  AumPri1(ctrl_aut,i); AutPri1(ctrl_aut,i);
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"");
  }
DONE;
}




int AutPri1(ctrl_aut,i)
struct og_ctrl_aut *ctrl_aut; 
int i;
{
char bspri[DPcSprintfSize];
sprintf(bspri,"%3d %s%s - ",i,
  ctrl_aut->State[i].final?"f":" ",
  ctrl_aut->State[i].start?" ":"n");
OgMessageLog(DOgMlogInLog+DOgMlogNoCr,ctrl_aut->loginfo->where,0,"%s",bspri);
do {
  sprintf(bspri,"%d:%d ",ctrl_aut->State[i].lettre,ctrl_aut->State[i].index);
  OgMessageLog(DOgMlogInLog+DOgMlogNoCr,ctrl_aut->loginfo->where,0,"%s",bspri);
  }
while(GnState(i));
DONE;
}



/*
 *	Impression de toutes les informations n\'ecessaires
 *	pour faire la minimisation.
*/

int AumPri1(ctrl_aut,i)
struct og_ctrl_aut *ctrl_aut; 
int i;
{
oindex grp,ngrp;
char bspri[DPcSprintfSize];
IFn(ctrl_aut->Lstmin) DONE;
grp=ctrl_aut->Lstmin[i].grp;
ngrp=ctrl_aut->Lstmin[i].ngrp;
DPcSprintf(bspri,"[(%2u) %4u %4u %3u-%-3u(%2u) %4u %4u %d %d] ",
  ctrl_aut->Lstmin[i].len,
  ctrl_aut->Lstmin[i].grp,ctrl_aut->Lstmin[i].ngrp,
  ctrl_aut->Lstmin[grp].nbgrp,ctrl_aut->Lstmin[ngrp].nbngrp,
  ctrl_aut->Lstmin[i].lenngrp,
  ctrl_aut->Lstmin[i].hstate, ctrl_aut->Lstmin[i].hnext,
  ctrl_aut->Lstmin[i].dead, ctrl_aut->Lstmin[i].standby);
OgMessageLog(DOgMlogInLog+DOgMlogNoCr,ctrl_aut->loginfo->where,0,"%s",bspri);
DONE;
}




