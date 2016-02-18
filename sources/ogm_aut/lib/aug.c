/*
 *	Construction de l'automate rapide.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : D\'ecembre 1996, Janvier 1997, March 2006
 *	Version 1.2
*/

#include "ogm_aut.h"



int Aug(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
char erreur[DOgErrorSize];
struct gstate *ag;
oindex i,pi,j;
oindex *posi;

IFn(ctrl_aut->StateUsed) DONE;

DPcFree(ctrl_aut->Gstate);
ctrl_aut->GstateUsed=ctrl_aut->StateUsed;
IFn(ag=(struct gstate *)malloc(ctrl_aut->GstateUsed*sizeof(struct gstate))) {
  sprintf(erreur,"Aug (%s): malloc error on ag",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
/** We need to set only the first cell **/
memset(ag,0,sizeof(struct gstate));

IFn(posi=(oindex *)malloc(ctrl_aut->GstateUsed*sizeof(oindex))) {
  sprintf(erreur,"Aug (%s): malloc error on posi",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** Mise \`a jour de la correspondance entre la position de la cellule **/
/** et la position de la cellule pour l'automate rapide **/
/** on utilise un tableau temporaire 'posi' pour avoir **/
/** la correspondance avec les \'etats de l'automate **/
posi[0]=0; /** Important pour les index \`a z\'ero **/
for (i=j=1; i<(oindex)ctrl_aut->GstateUsed; i++) {
  posi[i]=j;
  if (ctrl_aut->State[i].start) { pi=i; do j++; while(GnState(pi)); }
  }
/** Construction de l'automate rapide **/
for (i=j=1; i<(oindex)ctrl_aut->GstateUsed; i++) {
  if (!ctrl_aut->State[i].start) continue;
  pi=i;
  do {
    ag[j].final=ctrl_aut->State[pi].final;
    ag[j].lettre=ctrl_aut->State[pi].lettre;
    ag[j].index=posi[ctrl_aut->State[pi].index];
    IFn(ctrl_aut->State[pi].next) ag[j].last=1;
    else ag[j].last=0;
    j++;
    }
  while(GnState(pi));
  }
ctrl_aut->Gstate=ag;
DPcFree(posi);
DONE;
}


