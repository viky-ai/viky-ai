/*
 *	Statistiques sur tous les automates.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev: D\'ecembre 1996, Janvier 1997, August 2004, January 2005, March 2006
 *	Version 1.4
*/

#include "ogm_aut.h"





PUBLIC(int) OgAutStat(handle,autstat)
void *handle; struct aut_stat *autstat;
{
struct og_ctrl_aut *ctrl_aut;
int i,rideau;

IFn(handle) return(0);
ctrl_aut = (struct og_ctrl_aut *)handle;
rideau=ctrl_aut->StateUsed;
memset(autstat,0,sizeof(struct aut_stat));

/* Normalement, c'est bien i<=(int)rideau, mais on a pu 
 * remarquer un plantage avec un rideau à 15000 mais pas 
 * d'allocation du troisieme segment dont le pointeur était
 * à zéro. on met donc i<(int)rideau, mais on n'a pas corrigé
 * d'où venait le problème. Si on a encore le même problème
 * on reviendra dessus plus profondément (16 janvier 2005) 
 * 4 mars 2006, la refonte de genau en ogm_aut, devrait avoir
 * totalement supprimé ce problème. En effet, cette refonte
 * rend la librairie thread safe ce qui n'est pas le cas
 * genau, ce qui explique sans doute le problème. */
for (i=1; i<rideau; i++) {
  int nbbits; int offset=ctrl_aut->State[i].index;
  autstat->dist_letter[ctrl_aut->State[i].lettre]++;
  nbbits=0; while(offset) { offset>>=1; nbbits++; }
  autstat->dist_index[nbbits]++;
  }
for (i=0; i<256; i++) if (autstat->dist_letter[i]) autstat->nb_letter++;
for (i=31; i>=0; i--)
  if (autstat->dist_index[i]) { autstat->mxbit_index=i; break; }
autstat->gauge_min=ctrl_aut->gauge_lstmin; autstat->max_min=ctrl_aut->LstminNumber; autstat->size_cell_min=sizeof(struct lstmin);
autstat->gauge_automat=rideau; autstat->max_automat=1<<DPcMxindex;
autstat->max_index=rideau; autstat->size_cell_index=sizeof(struct state);
autstat->header_size=sizeof(struct og_ctrl_aut); autstat->size_cell_fast=sizeof(struct fstate);
if (ctrl_aut->Fstate) autstat->allocated_fast_automaton=1; else autstat->allocated_fast_automaton=0;
autstat->total_size = autstat->header_size
                    + autstat->max_index * autstat->size_cell_index
                    + autstat->gauge_min * autstat->size_cell_min
                    + (autstat->allocated_fast_automaton?autstat->max_index * autstat->size_cell_fast:0);
DONE;
}




PUBLIC(int) OgAufStat(handle,autstat)
void *handle; struct aut_stat *autstat;
{
struct og_ctrl_aut *ctrl_aut;
struct fstate *af;
int i;

IFn(handle) return(0);
ctrl_aut = (struct og_ctrl_aut *)handle;
memset(autstat,0,sizeof(struct aut_stat));
af=ctrl_aut->Fstate;

for (i=1; i<(int)ctrl_aut->FstateUsed; i++) {
  int nbbits; int offset=af[i].index;
  autstat->dist_letter[af[i].lettre]++;
  nbbits=0; while(offset) { offset>>=1; nbbits++; }
  autstat->dist_index[nbbits]++;
  }
for (i=0; i<256; i++) if (autstat->dist_letter[i]) autstat->nb_letter++;
for (i=31; i>=0; i--)
  if (autstat->dist_index[i]) { autstat->mxbit_index=i; break; }
autstat->gauge_min=ctrl_aut->gauge_lstmin; autstat->max_min=ctrl_aut->LstminNumber;
autstat->gauge_automat=ctrl_aut->FstateUsed; autstat->max_automat=1<<(DPcMxindex-2);
autstat->max_index=ctrl_aut->FstateUsed;
DONE;
}


