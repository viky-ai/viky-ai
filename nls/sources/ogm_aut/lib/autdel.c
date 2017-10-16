/*
 * Retrait d'une cha\^ine de caract\`eres dans l'automate.
 * Copyright (c) 1997-2006 Pertim by Patrick Constant
 * Dev: February 1997, August 1999, March 2006
 * Version 1.2
*/
#include "ogm_aut.h"



struct del_info {
  oindex start_state;
  oindex state;
  int single;
  int final;
  int start;
};

/*
 *    Retrait d'une cha\^ine de caract\`eres dans l'automate.
 *    Attention, il n'est pas possible de supprimer des lignes de
 *    description dans le cas o\`u l'automate est minimis�.
 *    Cependant, cette v�rification n'est pas effectu�e.
*/

PUBLIC(int) OgAutDel(handle,iline,line)
void *handle; int iline; unsigned char *line;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int out[DPcAutMaxBufferSize];

IF(AutTranslateBuffer2(iline,line,out)) DONE;
IFE(AutDline(ctrl_aut,out));
DONE;
}






/*
 *    Retire une ligne de "lettres" dans l'automate.
 *    Cette ligne correspond \`a un "mot" de l'automate.
 *    la ligne est une suite d'entiers qui correspond
 *    \`a un codage d'une ligne (ISO-latin-1, Unicode ou autres).
 *    Returns 1 if the line has been deleted, 0 otherwise.
 *    Returns -1 on error.
*/

int AutDline(struct og_ctrl_aut *ctrl_aut, int *line)
{
struct del_info info[DPcAutMaxBufferSize];
int i,start=0,state,nstate,start_state=0;
int istate,bf_state,dest_state;

IFn(ctrl_aut->StateUsed) return(0);

istate=0; state=1;
for (i=0; line[i]!=(-1); i++) {
  start=1; start_state=state; nstate=0;
  do {
    if (ctrl_aut->State[state].lettre==(unsigned)line[i])
      { nstate=ctrl_aut->State[state].index; break; }
    start=0;
    }
  while(GnState(state));
  if (nstate==0) return(0);

  info[istate].state=state;
  info[istate].start_state=start_state;
  if (start && !ctrl_aut->State[state].next) info[istate].single=1; else info[istate].single=0;
  if (ctrl_aut->State[state].final) info[istate].final=1; else info[istate].final=0;
  if (start) info[istate].start=1; else info[istate].start=0;
  istate++;

  state=nstate;
  }

if (!ctrl_aut->State[state].final) return(0);

if (ctrl_aut->State[state].index) {
  ctrl_aut->State[state].final=0;
  return(1);
  }

info[istate].state=state;
/** Bug correction 17/8/99 this line added **/
info[istate].start_state=start_state;
if (start && !ctrl_aut->State[state].next) info[istate].single=1; else info[istate].single=0;
if (ctrl_aut->State[state].final) info[istate].final=1; else info[istate].final=0;
if (start) info[istate].start=1; else info[istate].start=0;
istate++;



/** On s'arrete soit sur un element final qui n'est pas le dernier **/
/**             soit sur un noeud **/
for (i=istate-1; i>0; i--) {
  if (!info[i].single) break;
  if (i<istate-1 && info[i].final) break;
  }

if (!info[i].single) {
  /** Mise \`a jour du noeud **/
  bf_state=0;
  state=info[i].start_state;
  dest_state=info[i+1].state;
  do {
    if (state==(int)info[i].state && (int)ctrl_aut->State[state].index==dest_state) {
      if (bf_state) {
        /** On enl\`eve le maillon dans la cha\^ine des next **/
        ctrl_aut->State[bf_state].next=ctrl_aut->State[state].next;
        IFE(FreeState(ctrl_aut,state));
        break;
        }
      else {
        /** On enl\`eve le maillon dans la cha\^ine des next **/
        if (i>0) {
          ctrl_aut->State[info[i-1].state].index=ctrl_aut->State[state].next;
          ctrl_aut->State[ctrl_aut->State[state].next].start=1;
          IFE(FreeState(ctrl_aut,state));
          break;
          }
        /** On se trouve sur l'\'etat state=1 **/
        else {
          int nx_state=ctrl_aut->State[state].next;
          if (!nx_state) {
            IFE(OgAutReset(ctrl_aut)); return(1);
            }
          ctrl_aut->State[state]=ctrl_aut->State[nx_state];
          ctrl_aut->State[state].start=1;
          IFE(FreeState(ctrl_aut,nx_state));
          break;
          }
        }
      }
    bf_state=state;
    }
  while(GnState(state));
  }
else if (i<istate-1 && info[i].final) {
  if (i>0) {
    ctrl_aut->State[info[i].state].index=0;
    ctrl_aut->State[state].lettre=0;
    }
  else { IFE(OgAutReset(ctrl_aut)); return(1); }
  }
else { IFE(OgAutReset(ctrl_aut)); return(1); }


/** Lib\'eration des \'etats simples **/
for (i=istate-1; i>=0; i--) {
  if (!info[i].single) break;
  if (i<istate-1 && info[i].final) break;
  IFE(FreeState(ctrl_aut,info[i].state));
  }

return(1);
}




