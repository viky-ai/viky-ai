/*
 *	Minimisation d'un automate.
 *	J'ai essay\'e de faire des jump sur les \'etats dead cons\'ecutifs
 *	On ne gagne qu'environ 1% en temps pour une complexit\'e
 *	du code plus importante et un champ suppl\'ementaire de 24 bits
 *	dans lstmin. J'ai donc abandonn\'e cette solution.
 *	Copyright (c) 1996-2008 Pertimm by Patrick Constant
 *	Dev: Avril,Juillet,Septembre 1996, Janvier 1997, March 2006
 *  Dev: December 2008
 *	Version 1.5
*/

#include "ogm_aut.h"


STATICF(int) Aum(pr(struct og_ctrl_aut *));
STATICF(int) AumPart(pr_(struct og_ctrl_aut *) pr_(oindex) pr(unsigned));
STATICF(int) AumChg(pr_(struct og_ctrl_aut *) pr(oindex));
STATICF(int) AumRen(pr_(struct og_ctrl_aut *) pr(oindex));
STATICF(int) AumSmp(pr_(struct og_ctrl_aut *) pr_(oindex) pr(oindex));
STATICF(int) AumLen(pr_(struct og_ctrl_aut *) pr(oindex));
STATICF(oindex) AumH(pr_(struct og_ctrl_aut *) pr_(oindex) pr(oindex));
STATICF(oindex) AumK(pr_(struct og_ctrl_aut *) pr_(oindex) pr(oindex));
STATICF(int) AumHashPri(pr(struct og_ctrl_aut *));




PUBLIC(int) OgAum(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char erreur[DOgErrorSize];
oindex rideau;

IFn(ctrl_aut->StateUsed) DONE;

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"OgAum starting");
  }
  
IFE(Aum(ctrl_aut));
rideau=AumBuild2(ctrl_aut);
ctrl_aut->gauge_lstmin=ctrl_aut->StateUsed;

if (rideau > (oindex)ctrl_aut->StateUsed) {
  sprintf(erreur,"OgAum (%s): rideau (%d) > ctrl_aut->StateUsed (%d)",ctrl_aut->name, rideau, ctrl_aut->StateUsed);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"OgAum going from %d to %d",ctrl_aut->StateUsed,rideau);
  }
  
ctrl_aut->StateUsed=rideau;

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationDetail) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"\nFinalized automaton :\n\n");
  AutPri(ctrl_aut);
  }

DPcFree(ctrl_aut->Lstmin); ctrl_aut->LstminNumber=0; ctrl_aut->LstminUsed=0;

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"OgAum finished");
  }
  
DONE;
}



/*
 * July 2008: this was originally done in OgAum, but it is not a good
 * idea to keep that piece of code in that function because
 * there are cases where we reset the automaton and re-minimize
 * again, thus leading to many useless minimizations.
*/
PUBLIC(int) OgAumMinimizeMemory(handle)
void *handle;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char erreur[DOgErrorSize];
ctrl_aut->StateNumber=ctrl_aut->StateUsed;
IFn(ctrl_aut->State=(struct state *)realloc(ctrl_aut->State,ctrl_aut->StateNumber*sizeof(struct state))) {
  sprintf(erreur,"OgAum (%s): realloc error on State",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
ctrl_aut->LstminNumber=DOgLstminNumber;
IFn(ctrl_aut->Lstmin=(struct lstmin *)realloc(ctrl_aut->Lstmin,ctrl_aut->LstminNumber*sizeof(struct lstmin))) {
  sprintf(erreur,"OgAum (%s): malloc error on Lstmin",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }  
DONE;
}





STATICF(int) Aum(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
int retour;
unsigned iteration;
char erreur[DOgErrorSize];
oindex i,itmp,rideau=ctrl_aut->StateUsed;

ctrl_aut->LstminUsed = 0;
#if 0
ctrl_aut->LstminNumber = rideau+rideau/2;
IFn(ctrl_aut->Lstmin=(struct lstmin *)malloc(ctrl_aut->LstminNumber*sizeof(struct lstmin))) {
  sprintf(erreur,"Aum (%s): malloc error on Lstmin",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); return(0);
  }
#endif 

/** Calcul de la premi\`ere partition **/
for (i=1; i<rideau; i++) {
  IFE(itmp=AllocLstmin(ctrl_aut));
  if (itmp!=i) {
    sprintf(erreur,"Aum (%s): Incoherent initialisation",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }

  if (!ctrl_aut->State[i].start) { ctrl_aut->Lstmin[i].dead=1; continue; }
  if (ctrl_aut->State[i].final) ctrl_aut->Lstmin[i].grp=2;
  else ctrl_aut->Lstmin[i].grp=1;
  }
/** Calcul du champ 'len' d'un \'etat **/
IFE(AumLen(ctrl_aut,1));
if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"Maximum number of transitions: %d",ctrl_aut->Lstmin[1].len);
  }
/** On raffine les partitions jusqu'\`a stabilisation **/
iteration=0;
if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
  OgMessageLog(DOgMlogInLog+DOgMlogNoCr,ctrl_aut->loginfo->where,0,"Minimization: ");
  }
do {
  char *dot, bigdot[20];
  IFE(retour=AumPart(ctrl_aut,rideau,iteration));
  IFE(AumRen(ctrl_aut,rideau)); iteration++;
  if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
    if (iteration%5) dot=".";
    else if (iteration%10) dot=",";
    else  { sprintf(bigdot,"%d",iteration/10); dot=bigdot; }
    OgMessageLog(DOgMlogInLog+DOgMlogNoCr,ctrl_aut->loginfo->where,0,"%s",dot);
    }
  }
while(retour);

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimization) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"\nNumber of minimization iterations: %d",iteration);
  }
  
/** hstate correspond \`a l'index du premier state du groupe **/
/** c'est ce qu'on veut pour la fonction AumBuild **/
for (i=1; i<rideau; i++) {
  if (!ctrl_aut->Lstmin[i].dead) {
    ctrl_aut->Lstmin[i].grp=ctrl_aut->Lstmin[ctrl_aut->Lstmin[i].ngrp].hstate;
    }
  }
for (i=1; i<rideau; i++) {
  struct lstmin* ilstmin;
  if (ctrl_aut->Lstmin[i].standby!=0) {
    ilstmin=ctrl_aut->Lstmin+i;
    ilstmin->standby=0;
    ilstmin->dead=0;
    }
  }
DONE;
}



/*
 *	Regarde si la minimisation est termin\'ee.
 *	La condition est que: toute les partitions
 *	de plus de 1 \'el\'ement restent les m\^emes
 *	Ce qui est \'equivalent \`a dire qu'elle gardent
 *	le m\^eme nombre d'\'el\'ement.
*/

STATICF(int) AumChg(ctrl_aut,rideau)
struct og_ctrl_aut *ctrl_aut;
oindex rideau;
{
oindex i;
struct lstmin* ilstmin;
for (i=1; i<rideau; i++) {
  ilstmin=ctrl_aut->Lstmin+i;
  if (ilstmin->dead) continue;
  if (ctrl_aut->Lstmin[ilstmin->grp].nbgrp
     != ctrl_aut->Lstmin[ilstmin->ngrp].nbngrp) return(1);
  }
return(0);
}



/*
 *	Construction de la nouvelle partition correspondant
 *	\`a l'algorithme de minimisation. Pour cette construction
 *	on utilise une table de hachage qui permet de pointer
 *	sur des cellules qui correspondent chacune \`a une partition.
 *	Ceci permet d'\'eviter le tri des \'etats pour obtenir
 *	la partition.
*/

STATICF(int) AumPart(ctrl_aut,rideau,iteration)
struct og_ctrl_aut *ctrl_aut; oindex rideau;
unsigned iteration;
{
oindex i;
int change,stop;
oindex j,pj,nj=0,rdmax,rdlstmin=ctrl_aut->StateUsed;
/** Calcul de la seconde partition **/
for (i=1; i<rideau; i++) {
  unsigned len; struct lstmin* jlstmin;
  if (ctrl_aut->Lstmin[i].dead) continue;
  len=ctrl_aut->Lstmin[i].len;
  j=AumH(ctrl_aut,i,rideau); jlstmin=ctrl_aut->Lstmin+j;
  if (!jlstmin->hused) {
    jlstmin->hstate=i; jlstmin->hused=1;
    jlstmin->nbngrp=1; jlstmin->hnext=0;
    jlstmin->lenngrp=len;
    }
  else {
    pj=j;
    do {
      if (AumSmp(ctrl_aut,ctrl_aut->Lstmin[pj].hstate,i)) {
        ctrl_aut->Lstmin[pj].nbngrp+=1;
        if (ctrl_aut->Lstmin[pj].lenngrp<len) ctrl_aut->Lstmin[pj].lenngrp=len;
        break;
        }
      }
    while((pj=ctrl_aut->Lstmin[pj].hnext)!=0);
    if (pj==0) {
      struct lstmin* njlstmin;
      IFn(nj) nj=rideau; else nj++;
      if (nj>=rdlstmin) { IFE(nj=AllocLstmin(ctrl_aut)); }
      /** necessary because reallocation in AllocLstmin **/
      jlstmin=ctrl_aut->Lstmin+j;
      njlstmin=ctrl_aut->Lstmin+nj;
      njlstmin->hstate=i; njlstmin->hused=1; njlstmin->nbngrp=1; 
      njlstmin->hnext=jlstmin->hnext; jlstmin->hnext=nj;
      njlstmin->lenngrp=len;
      }
    }
  }

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"Hashing overhead: %d (rideau=%d)\n",nj-rideau,rideau);
  }
  
/** Cr\'eation du nouveau groupe avec la table de hachage **/
for (i=1; i<rideau; i++) {
  oindex ngrp;
  if (ctrl_aut->Lstmin[i].dead) continue;
  ngrp=AumK(ctrl_aut,i,rideau);
  ctrl_aut->Lstmin[i].ngrp=ngrp;
  }
/** Mise \`a jour des champs 'dead' et 'standby' **/
for (stop=1,change=0,i=1; i<rideau; i++) {
  oindex ngrp;
  if (ctrl_aut->Lstmin[i].dead) continue;
  ngrp=ctrl_aut->Lstmin[i].ngrp;
  if (ctrl_aut->Lstmin[ngrp].nbngrp==1) {
    /** Ce type de changement n'est pas comptabilis\'e **/
    /** par la fonction AumChg et en plus on optimise **/
    /** car on n'a pas besoin d'appeler AumChg **/
    ctrl_aut->Lstmin[i].dead=1; change=1;
    }
  else {
    stop=0;
    /** On met en stand-by les groupes qui ne bougeront pas **/
    if (iteration>=ctrl_aut->Lstmin[ngrp].lenngrp) {
      ctrl_aut->Lstmin[i].dead=1; ctrl_aut->Lstmin[i].standby=2;
      ctrl_aut->Lstmin[i].grp=ctrl_aut->Lstmin[i].ngrp;
      }
    }
  }
/** A cet endroit, les deux partitions, la nouvelle et l'ancienne **/
/** cohabitent ensemble. Il est alors possible de savoir si **/
/** il y a eu un raffinement quelconque dans la partition **/
/** Si c'est le cas, il faut continuer de raffiner **/
/** le champ 'dead' indique un \'etat tout seul dans son groupe **/
/** pour optimiser, dead indique aussi les state non start **/

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"iteration %d:",iteration);
  AumHashPri(ctrl_aut); AutPri(ctrl_aut);
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"");
  }

if (stop) { return(0); }
if(!change) IFn(AumChg(ctrl_aut,rideau)) return(0);
rdmax=(nj?nj:rideau);
/** Ici on transfert les nouveaux groupes dans les groupes **/
for (i=1; i<rdmax; i++) {
  struct lstmin* plstmin; plstmin=ctrl_aut->Lstmin+i;
  /** si l'\'etat est dans un groupe en stand-by **/
  /** on garde le num\'ero du groupe **/
  if (!plstmin->standby) plstmin->grp=plstmin->ngrp; plstmin->ngrp=0;
  plstmin->nbgrp=plstmin->nbngrp; plstmin->nbngrp=0;
  plstmin->hstate=0; plstmin->hnext=0; plstmin->hused=0;
  }
return(1);
}


/*
 *	Renomage des nouveaux groupes en stand-by
 *	Le renomage consiste \`a donner au champ 'grp'
 *	l'index de la premi\`ere cellule appartenant \`a ce groupe
 *	pour toute les cellules appartenant \`a ce groupe.
 *	On utilise pour cela le champ 'hnext' qui n'est pas 
 *	utilis\'e en chevauchement sur l'appel de AumRen.
 *	Si ce champ est utilis\'e dans cette zone temporelle,
 *	il faut utiliser un nouveau champ uniquement pour cette fonction.
*/

STATICF(int) AumRen(ctrl_aut,rideau)
struct og_ctrl_aut *ctrl_aut;
oindex rideau;
{
oindex i,grp,rdlstmin;

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"Avant renommage");
  AutPri(ctrl_aut);
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"");
  }

rdlstmin=ctrl_aut->StateUsed;
for (i=1; i<rdlstmin; i++) ctrl_aut->Lstmin[i].hnext=0;

for (i=1; i<rideau; i++) {
  if (ctrl_aut->Lstmin[i].standby!=2) continue;
  grp=ctrl_aut->Lstmin[i].grp;
  IFx(ctrl_aut->Lstmin[grp].hnext) continue;
  ctrl_aut->Lstmin[grp].hnext=i;
  }
for (i=1; i<rideau; i++) {
  if (ctrl_aut->Lstmin[i].standby!=2) continue;
  grp=ctrl_aut->Lstmin[i].grp;
  ctrl_aut->Lstmin[i].grp=ctrl_aut->Lstmin[grp].hnext;
  ctrl_aut->Lstmin[i].standby=1;
  }

for (i=1; i<rdlstmin; i++) ctrl_aut->Lstmin[i].hnext=0;

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"Apres renommage");
  AutPri(ctrl_aut);
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"");
  }

DONE;
}




/*
 *	Regarde si les \'etats state1 et state2
 *	sont dans la m\^eme partition c'est-\`a-dire 
 *	si ils pointent avec les m\^emes lettres sur
 *	les m\^emes groupes respectivement.
 *	Attention, ces \'etats peuvent pointer
 *	sur des \'etats 'dead' ou en standby.
 *  December 26th 2008: function AumSmp was
 *  bugged: it has been recoded.
*/
STATICF(int) AumSmp(ctrl_aut,state1,state2)
struct og_ctrl_aut *ctrl_aut;
oindex state1,state2;
{
int same;
oindex i,j;
struct state *istate, *jstate;
struct lstmin *ilstmin, *jlstmin;

if (ctrl_aut->Lstmin[state1].grp!=ctrl_aut->Lstmin[state2].grp) return(0);

i=state1;
j=state2;
while(1) {
  istate=ctrl_aut->State+i;
  jstate=ctrl_aut->State+j;
  if (istate->lettre!=jstate->lettre) return(0);
  ilstmin=ctrl_aut->Lstmin+istate->index;
  jlstmin=ctrl_aut->Lstmin+jstate->index;
  same=0;
  if (istate->index==jstate->index) same=1;
  if ((ilstmin->standby && jlstmin->standby)
    || (!ilstmin->dead && !jlstmin->dead)) {
    if (ilstmin->grp==jlstmin->grp) same=1;
    }
  if (!same) return(0);
  i=istate->next; j=jstate->next;
  if (i==0 && j==0) break;
  else if (i==0) return(0);
  else if (j==0) return(0);
  }
return(1);
}





STATICF(int) AumLen(ctrl_aut,state)
struct og_ctrl_aut *ctrl_aut;
oindex state;
{
oindex i;
int len,ilen;
len=0; i=state;
IFx(ctrl_aut->State[i].index) {
  do {
    ilen=AumLen(ctrl_aut,ctrl_aut->State[i].index);
    if (ilen>len) len=ilen;
    }
  while(GnState(i));
  len++;
  }
ctrl_aut->Lstmin[state].len=len;
return(len);
}





/*
 *	Calcul de la clef de hachage adapt\'ee qui permet
 *	de faire pointer les \'el\'ements dans le m\^eme
 *	groupe sur la m\^eme cellule.
*/

STATICF(oindex) AumH(ctrl_aut,i,rideau)
struct og_ctrl_aut *ctrl_aut;
oindex i,rideau;
{
register unsigned int result;
register unsigned int c;
result=0;
/** On tient compte du groupe du state i **/
c=ctrl_aut->Lstmin[i].grp;
result += (result<<3) + c;
do {
  /** On tient compte de la lettre et du groupe **/
  c=ctrl_aut->State[i].lettre<<DPcMxindex;
  c|=ctrl_aut->Lstmin[ctrl_aut->State[i].index].grp;
  result += (result<<3) + c;
  }
while(GnState(i));
result = (result%(rideau-1)) + 1;
return((oindex)result);
}



/*
 *	Recherche de la clef avec la fonction de hachage
 *	permettant d'avoir une clef par nouvelle partition
 *	pour la minimisation.
*/

STATICF(oindex) AumK(ctrl_aut,i,rideau)
struct og_ctrl_aut *ctrl_aut;
oindex i,rideau;
{
oindex pj=AumH(ctrl_aut,i,rideau);
do {
  if (AumSmp(ctrl_aut,ctrl_aut->Lstmin[pj].hstate,i)) return(pj);
  }
while((pj=ctrl_aut->Lstmin[pj].hnext)!=0);
return(0);
}



STATICF(int) AumHashPri(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
oindex i,j;
oindex rideau=ctrl_aut->StateUsed;
int hashed=0,collisions=0;
for (i=1; i<rideau; i++) {
  if (ctrl_aut->Lstmin[i].dead) continue;
  j=AumH(ctrl_aut,i,rideau);
  collisions-=1; hashed++;
  do collisions++;
  while((j=ctrl_aut->Lstmin[j].hnext)!=0);
  }
OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
  , "rideau=%d, hashed=%d, collisions=%d"
  , rideau, hashed, collisions);
DONE;
}




