/*
 *  Regarde si une ligne de "lettres" est dans l'automate.
 *  Copyright (c) 2000-2006 Pertimm by Patrick Constant
 *  Dev : D\'ecembre 1996, Septembre 1998, March 2006
 *  Version 1.2
*/

#include "ogm_aut.h"


STATICF(int) AufLet(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *) pr_(int *) pr(unsigned char*));
STATICF(int) AugLet(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *) pr_(int *) pr(unsigned char*));


/*  PUBLIC
 *  Regarde si une ligne de "lettres" est dans l'automate.
 *  Renvoit 0 si la cha\^ine n'existe pas dans l'automate.
 *  Renvoit 1 si la cha\^ine existe mais n'est pas finale,
 *  Renvoit 2 si la cha\^ine est une cha\^ine finale.
 *    dans ces deux derniers cas, renvoit dans 'letters'
 *    la liste des lettres qui suivent cette cha\^ine.
*/

PUBLIC(int) OgAufLet(handle,iline,line,iletters,letters)
void *handle; int iline; unsigned char *line;
int *iletters; unsigned char* letters;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char erreur[DOgErrorSize];

if (ctrl_aut->Fstate) {
  return(AufLet(ctrl_aut,iline,line,iletters,letters));
  }
else if (ctrl_aut->Gstate) {
  return(AugLet(ctrl_aut,iline,line,iletters,letters));
  }
return(0);
}





STATICF(int) AufLet(ctrl_aut,iline,line,iletters,letters)
struct og_ctrl_aut *ctrl_aut; int iline; unsigned char *line;
int *iletters; unsigned char* letters;
{
struct fstate *af=ctrl_aut->Fstate;
int out[DPcAutMaxBufferSize];
int i,state,retour;
IF(AutTranslateBuffer2(iline,line,out)) return(0);
IFn(state=AufGline2(af,out)) return(0);
else if (af[state].final) retour=2; else retour=1;
i=0; do letters[i++]=af[state].lettre;
while(!af[state++].last);
letters[i]=0;
IFx(iletters) *iletters=i;
return(retour);
}






STATICF(int) AugLet(ctrl_aut,iline,line,iletters,letters)
struct og_ctrl_aut *ctrl_aut; int iline; unsigned char *line;
int *iletters; unsigned char* letters;
{
struct gstate *ag=ctrl_aut->Gstate;
int out[DPcAutMaxBufferSize];
int i,state,retour;
IF(AutTranslateBuffer2(iline,line,out)) return(0);
IFn(state=AugGline(ag,out)) return(0);
else if (ag[state].final) retour=2; else retour=1;
i=0; do letters[i++]=ag[state].lettre;
while(!ag[state++].last);
letters[i]=0;
IFx(iletters) *iletters=i;
return(retour);
}

