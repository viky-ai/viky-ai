/*
 *  Scanning de l'automate pour r\'ecup\'erer toutes les cha\^ines
 *  de caract\`eres contenus dans celui-ci.
 *  Copyright (c) 1997-2006 Pertimm by Patrick Constant
 *  Dev : Janvier 1997, Septembre 1998, March 2006
 *  Version 1.2
*/

#include "ogm_aut.h"


/*
 *  Regarde si une ligne de "lettres" est dans l'automate.
 *  Renvoit 0 si la cha\^ine n'existe pas dans l'automate.
 *  Renvoit 1 si la cha\^ine est au moins une sous-cha\^ine
 *  et renvoit dans 'out' le reste de la cha\^ine.
 *  Si on a dans l'automate :
 *    danse:nms
 *    danse:vip1s
 *    danse:vip2s
 *  et si l'on a line="danse:", on aura "nms" dans out.
*/

PUBLIC(int) OgAufScanf(handle,iline,line,iout,out,nstate0,nstate1,states)
void *handle; int iline; unsigned char *line;
int *iout; unsigned char *out;
int *nstate0,*nstate1; oindex *states;
{
char erreur[DOgErrorSize];
struct og_ctrl_aut *ctrl_aut;

IFn(handle) return(0);
ctrl_aut = (struct og_ctrl_aut *)handle;

if (ctrl_aut->Fstate) {
  return(AufScanf(ctrl_aut,iline,line,iout,out,nstate0,nstate1,states));
  }
else if (ctrl_aut->Gstate) {
  return(AugScanf(ctrl_aut,iline,line,iout,out,nstate0,nstate1,states));
  }
return(0);
}



PUBLIC(int) OgAufScann(handle,iout,out,nstate0,nstate1,states)
void *handle; int *iout; unsigned char *out;
int nstate0,*nstate1;
oindex *states;
{
char erreur[DOgErrorSize];
struct og_ctrl_aut *ctrl_aut;

IFn(handle) return(0);
ctrl_aut = (struct og_ctrl_aut *)handle;

if (ctrl_aut->Fstate) {
  return(AufScann(ctrl_aut,iout,out,nstate0,nstate1,states));
  }
else if (ctrl_aut->Gstate) {
  return(AugScann(ctrl_aut,iout,out,nstate0,nstate1,states));
  }
return(0);
}




