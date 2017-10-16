/*
 *	Scanning de l'automate pour r\'ecup\'erer toutes les cha\^ines
 *	de caract\`eres contenus dans celui-ci.
 *	Copyright (c) 1997-2006 Pertimm by Patrick Constant
 *	Dev : Janvier 1997, Septembre 1998, March 2006
 *	Version 1.2
*/

#include "ogm_aut.h"


STATICF(int) AufGlineState(pr_(struct fstate *) pr_(int *) pr_(oindex *) pr(int *));
STATICF(int) AufScanf1(pr_(struct fstate *) pr_(int) pr(oindex *));
STATICF(int) AufScann1(pr_(struct fstate *) pr_(int) pr_(int *) pr(oindex *));


/*
 *	Regarde si une ligne de "lettres" est dans l'automate.
 *	Renvoit 0 si la cha\^ine n'existe pas dans l'automate.
 *	Renvoit 1 si la cha\^ine est au moins une sous-cha\^ine
 *	et renvoit dans 'out' le reste de la cha\^ine.
 *	Si on a dans l'automate :
 *	  danse:nms
 *	  danse:vip1s
 *	  danse:vip2s
 *	et si l'on a line="danse:", on aura "nms" dans out.
*/

int AufScanf(ctrl_aut,iline,line,iout,out,nstate0,nstate1,states)
struct og_ctrl_aut *ctrl_aut; int iline; unsigned char *line;
int *iout; unsigned char *out;
int *nstate0,*nstate1; oindex *states;
{
int linei[DPcAutMaxBufferSize];
int i,nstate0i,nstate1i,istates;
struct fstate *af=ctrl_aut->Fstate;

IFn(af) return(0);

IF(AutTranslateBuffer2(iline,line,linei)) return(0);

IFn(nstate0i=AufGlineState(af,linei,states,&istates)) { *nstate0=istates; return(0); }
nstate1i=AufScanf1(af,nstate0i,states);
for (i=nstate0i-1; i<nstate1i-1; i++)
  out[i-nstate0i+1]=af[states[i]].lettre; out[i-nstate0i+1]=0;
IFx(iout) *iout=nstate1i-nstate0i;
*nstate0=nstate0i;
*nstate1=nstate1i;
return(1);
}



/*
 *	Mets dans 'states', la liste des \'etats ayant permis
 *	d'analyser 'line' et renvoit le nombre d'\'etats dans 'states'.
 *	Ce nombre vaut 0 si 'line' n'a pas \'et\'e reconnu.
 *	La cha\^ine vide donne le nombre 1 avec dans 'states' 
 *	l'\'etat 1, donc il n'y a pas d'ambigu\"it\'e sur
 *	la signification de ce nombre.
*/

STATICF(int) AufGlineState(af,line,states,istates)
struct fstate *af; int *line;
oindex *states;
int *istates;
{
int cstate=0;
register int i=0;
register oindex state=1;
states[cstate++]=state;
etiquette:
if (line[i]==(-1)) return(cstate);
do if (af[state].lettre==(unsigned)line[i])
    { state=af[state].index; states[cstate++]=state; i++; goto etiquette; }
while(!af[state++].last);
*istates=cstate;
return(0);
}



/*
 *	Fonction qui va trouver, \`a partir de l'\'etat 'states[nstate0-1]'
 *	la premi\`ere suite d'\'etats possibles dans l'automate.
 *	Renvoit le nouveau nombre d\'etats dans states.
*/

STATICF(int) AufScanf1(af,nstate0,states)
struct fstate *af; int nstate0; oindex *states;
{
int nstate=nstate0-1;
oindex state=states[nstate];
while(1) {
  states[nstate++]=state;
  if (af[state].final) break;
  state=af[state].index;
  }
return(nstate);
}




/*
 *	Recherche le r\'esultat suivant it\'erativement 
 *	apr\`es un appel \`a PcAufScanf().
 *	Par exemple, si on a appel\'e PcAufScanf(ha,"danse:",out);
 *	On aura successivement "vip1s" et "vip2s" dans out.
*/

int AufScann(ctrl_aut,iout,out,nstate0,nstate1,states)
struct og_ctrl_aut *ctrl_aut; int *iout; unsigned char *out;
int nstate0,*nstate1; 
oindex *states;
{
struct fstate *af=ctrl_aut->Fstate;
int i,retour;
IFE(retour=AufScann1(af,nstate0,nstate1,states));
for (i=nstate0-1; i<(*nstate1)-1; i++)
  out[i-nstate0+1]=af[states[i]].lettre; out[i-nstate0+1]=0;
IFx(iout) *iout=(*nstate1)-nstate0;
return(retour);
}



/*
 *	Fonction qui va trouver, \`a partir de la suite
 *	de caract\`eres 'line', la suite de caract\`eres
 *	suivante dans l'automate. Cette suite doit avoir 
 *	les m\^eme 'i' premiers caract\`eres.
 *	Renvoit 1 si elle a trouve une suite suivante, 0 sinon.
 *	avec la suite :
 *	  3 2 06 5 2 4 -> `samoan'
 *	  3 2 06 5 05 6 -> `samole'
 *	  3 2 06 5 05 6 3 -> `samoles'
 *	  3 2 06 14 12 2 02 -> `samurai'
 *	et i=5, on trouve les trois premiers
 *	(suite 3 2 0 6 5 identique) en appelant la fonction
 *	jusqu'\`a ce quelle renvoit 0.
*/

STATICF(int) AufScann1(af,nstate0,nstate1,states)
struct fstate *af;
int nstate0,*nstate1;
oindex *states;
{
int i;
oindex nstate1i=(*nstate1);
oindex state1,state;

state1=states[nstate1i-1];
IFx(state=af[state1].index) {
  states[nstate1i++]=state;
  *nstate1=AufScanf1(af,nstate1i,states);
  return(1);
  }

/** La derni\`ere transition va de states[nstate1i-2] \`a states[nstate1i-1] **/
/** soit cette transition est la derni\`ere et on remonte d'une transition **/
/** soit elle n'est pas la derni\`ere et on suit la transition suivante **/

for(i=nstate1i-2; i>=nstate0-1; i--) {
  if (af[states[i]].last) continue;
  states[i]=states[i]+1;
  i++;
  *nstate1=AufScanf1(af,i,states);
  return(1);
  }
return(0);
}



