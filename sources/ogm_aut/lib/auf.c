/*
 *  Construction de l'automate rapide.
 *  Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *  Dev : D\'ecembre 1996, Janvier 1997, March 2006
 *  Version 1.2
*/

#include "ogm_aut.h"



int Auf(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
char erreur[DOgErrorSize];
struct fstate *af;
oindex i,pi,j;
oindex *posi;

IFn(ctrl_aut->StateUsed) DONE;

ctrl_aut->FstateAllocated=1;
ctrl_aut->FstateUsed=ctrl_aut->StateUsed;
IFn(af=(struct fstate *)realloc(ctrl_aut->Fstate, ctrl_aut->FstateUsed*sizeof(struct fstate))) {
  sprintf(erreur,"OgAuf (%s): realloc error on af",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
/** We need to set only the first cell **/
memset(af,0,sizeof(struct fstate));

IFn(posi=(oindex *)malloc(ctrl_aut->FstateUsed*sizeof(oindex))) {
  sprintf(erreur,"OgAuf (%s): malloc error on posi",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** Mise \`a jour de la correspondance entre la position de la cellule **/
/** et la position de la cellule pour l'automate rapide **/
/** on utilise un tableau temporaire 'posi' pour avoir **/
/** la correspondance avec les \'etats de l'automate **/
posi[0]=0; /** Important pour les index \`a z\'ero **/
for (i=j=1; i<(oindex)ctrl_aut->FstateUsed; i++) {
  posi[i]=j;
  if (ctrl_aut->State[i].start) { pi=i; do j++; while(GnState(pi)); }
  }
/** Construction de l'automate rapide **/
for (i=j=1; i<(oindex)ctrl_aut->FstateUsed; i++) {
  if (!ctrl_aut->State[i].start) continue;
  pi=i;
  do {
    af[j].final=ctrl_aut->State[pi].final;
    af[j].lettre=ctrl_aut->State[pi].lettre;
    af[j].index=posi[ctrl_aut->State[pi].index];
    IFn(ctrl_aut->State[pi].next) af[j].last=1;
    else af[j].last=0;
    j++;
    }
  while(GnState(pi));
  }
ctrl_aut->Fstate=af;
DPcFree(posi);
DONE;
}


