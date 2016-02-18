/*
 *	Ajustement des fichier .aut lus dans des version < à la version 2.54.
 *	Copyright (c) 2008 Pertimm by Patrick Constant
 *	Dev : August 2008
 *	Version 1.0
*/

#include "ogm_aut.h"


struct state_253 {
  unsigned index;
  unsigned next;
  unsigned lettre: DPcMxlettre;
  unsigned start: 1;
  unsigned final: 1;
  unsigned freq: 6;
  unsigned libre:15;
  };
  

static int AutReadVersion(struct og_ctrl_aut *, unsigned char *, int *);


  

/*
 * Banner is the aut banner defined in logaut.h, typically
 * "ogm_aut V2.54, Copyright (c) 1996-2008 Pertimm, Inc."
 * we get the version as 254. returns 1 if version is found
 * and zero otherwise.
*/

static int AutReadVersion(ctrl_aut,banner,pversion)
struct og_ctrl_aut *ctrl_aut;
unsigned char *banner;
int *pversion;
{
int i,ibanner=strlen(banner);
int inumber=0; char number[DPcPathSize];
int v=(-1);

*pversion=0;

for (i=0; i<ibanner; i++) {
  if (banner[i]=='v' || banner[i]=='V') { v=i; break; }
  }
if (v<0) return(0);

for (i=v+1; i<ibanner; i++) {
  if (PcIsspace(banner[i])) break;
  else if (PcIsdigit(banner[i])) number[inumber++]=banner[i];
  }
number[inumber]=0;
*pversion=atoi(number);
return(1);
}





int AutRead254Change(ctrl_aut,banner)
struct og_ctrl_aut *ctrl_aut;
char *banner;
{
struct state_253 cstate_253,*state_253=&cstate_253;
int i,found_version,version;
char erreur[DOgErrorSize];
struct state *state;

if (sizeof(struct state) != sizeof(struct state_253)) {
  sprintf(erreur,"AutRead254Change (%s): sizeof(struct state) %d != sizeof(struct state_253) %d"
    ,ctrl_aut->name, sizeof(struct state), sizeof(struct state_253));
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

IFE(found_version=AutReadVersion(ctrl_aut,banner,&version));
if (!found_version) DONE; /** No version, very strange **/
if (version >= 254) DONE; /** version is new, automaton is ok **/

for (i=0; i<ctrl_aut->StateNumber; i++) {
  state = ctrl_aut->State + i;
  memcpy(state_253,state,sizeof(state));
  state->index = state_253->index;
  state->next = state_253->next;
  state->lettre = state_253->lettre;
  state->start = state_253->start;
  state->final = state_253->final;
  state->freq = state_253->freq;
  state->index = state_253->index;
  state->index = state_253->index;
  state->index = state_253->index;
  }

DONE;
}



