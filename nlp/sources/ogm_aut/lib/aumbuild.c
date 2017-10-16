/*
 *	Construction de l'automate minimis\'e \`a partir
 *	des informations fournies par la fonction de minimisation.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : Mai,Juillet 1996, Janvier 1997, March 2006
 *	Version 1.3
*/
#include "ogm_aut.h"

/*
 *	Construction effective de l'automate minimis\'e.
 *	On commence par faire pointer les transitions
 *	uniquement sur les \'etats repr\'esentatifs de la classe.
 *	Ensuite on d\'ecale tous les \'etats et on refait
 *	pointer les transitions au bon endroit.
 *	Renvoit le nouveau rideau.
*/

oindex AumBuild2(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
oindex rideau=ctrl_aut->StateUsed;
oindex i,j,pi,state,grp,nxstate;

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"\nDebut de AumBuild :\n\n");
  AutPri(ctrl_aut);
  }

/** Remise \`a niveau des valeurs des transitions **/
/** On pointe sur les \'etats repr\'esentatifs **/
for (i=1; i<rideau; i++) {
  /** initialisation pour la phase suivante **/
  /** de marquage des \'etats inutiles **/
  /** hnext est utilis\'es dans la construction **/
  /** de la table de hachage et ne sert plus ici **/
  ctrl_aut->Lstmin[i].hnext=1;
  if (!ctrl_aut->State[i].start) continue;
  pi=i; do {
    state=ctrl_aut->State[pi].index;
    if (ctrl_aut->Lstmin[state].dead) continue;
    grp=ctrl_aut->Lstmin[state].grp;
    /* July 29th 2008, groups that are beyond rideau
     * should not be taken into account */
    if (grp >= rideau) {
      ctrl_aut->Lstmin[state].grp=ctrl_aut->State[pi].index;
      continue;
      }
    ctrl_aut->State[pi].index=grp;
    }
  while(GnState(pi));
  }

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"\nApres remise a niveau des transition :\n\n");
  AutPri(ctrl_aut);
  }

/** On marque les \'etats qui sont inutiles en mettant **/
/** le champ ctrl_aut->Lstmin[i].hnext \`a 0 **/
for (i=1; i<rideau; i++) {
  if (ctrl_aut->Lstmin[i].dead) continue;
  if (ctrl_aut->Lstmin[i].grp!=i) {
    pi=i; do ctrl_aut->Lstmin[pi].hnext=0;
    while(GnState(pi));
    }
  }

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"\nApres marquage des \'etats inutiles :\n\n");
  AutPri(ctrl_aut);
  }

/** On calcule les d\'ecalages des \'etats utiles en stockant la nouvelle **/
/** valeur de l'\'etat dans le champ ctrl_aut->Lstmin[i].hnext **/
/** On oublie simplement les \'etats inutiles **/

for (i=j=1; i<rideau; i++) {
  IFn(ctrl_aut->Lstmin[i].hnext) continue;
  ctrl_aut->Lstmin[i].hnext=j; j++;
  }

if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
    ,"\nApres calcul du d\'ecalage sur les \'etats utiles :\n\n");
  AutPri(ctrl_aut);
  }

/** Remise \`a niveau des valeurs des transitions d\'ecal\'es **/
/** Il faut remettre \`a jour les transitions, **/
/** mais aussi les champs next **/
for (i=1; i<rideau; i++) {
  IFn(ctrl_aut->Lstmin[i].hnext) continue;
  if (!ctrl_aut->State[i].start) continue;
  pi=i; do {
    ctrl_aut->State[pi].index=ctrl_aut->Lstmin[ctrl_aut->State[pi].index].hnext;
    nxstate=ctrl_aut->State[pi].next;
    ctrl_aut->State[pi].next=ctrl_aut->Lstmin[nxstate].hnext;
    }
  while((pi=nxstate)!=0);
  }

/** D\'eplacement physique des \'etats **/
for (i=j=1; i<rideau; i++) {
  IFn(ctrl_aut->Lstmin[i].hnext) continue;

  if (ctrl_aut->loginfo->trace & DOgAutTraceMinimizationBuild) {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0
      ,"i=%d -> j=%d",i,j);
    }

  /** 13 juin 2004: rajout du test i != j pour eviter les overlay (de valgrind) **/
  if (j != i) ctrl_aut->State[j]=ctrl_aut->State[i]; j++;
  }

/** 'j' donne le nouveau rideau **/
return(j);
}


