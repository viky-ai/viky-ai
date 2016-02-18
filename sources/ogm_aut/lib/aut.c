/*
 * Fonction de base qui analyse un fichier et construit l'automate 
 * de base (non compress\'e et non minimis\'e).
 * Copyright (c) 1996-2006  Pertimm by Patrick Constant
 * Dev: Juillet,Septembre,D\'ecembre 1996
 * Dev: Avril 2004, February 2006
 * Version 1.5
*/

#include "ogm_aut.h"




/*
 *  Fonction qui analyse le ficher 'file' et qui construit
 *  un automate non compress\'e et non minimis\'e.
 *  'ha' est un handle d'automate renvoy\'e par OgAutInit();
 *  Utiliser OgAum(handle) pour minimiser et OgAuf(ha) 
 *  pour obtenir un automate rapide.
*/

PUBLIC(int) OgAut(handle,file)
void *handle; char *file;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
unsigned char ligne[DPcAutMaxBufferSize];
int out[DPcAutMaxBufferSize];
char erreur[DOgErrorSize];
struct stat file_stat;
unsigned number_line;
FILE *fd;

if (stat(file,&file_stat)) {
  sprintf(erreur,"OgAut (%s): impossible to stat `%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

if (ctrl_aut->StateNumber < file_stat.st_size) {
  ctrl_aut->StateUsed = 0;
  ctrl_aut->StartFreeState = (-1);
  ctrl_aut->StateNumber = file_stat.st_size; DPcFree(ctrl_aut->State);
  IFn(ctrl_aut->State=(struct state *)malloc(ctrl_aut->StateNumber*sizeof(struct state))) {
    sprintf(erreur,"OgAut (%s): malloc error on State",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); return(0);
    }
  }

IFn(fd=fopen(file,"r")) {
  sprintf(erreur,"OgAut (%s): impossible to fopen `%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

number_line=1;

while(fgets(ligne,DPcAutMaxBufferSize,fd)) {
  /** removing all '\r' and '\n' at end of line **/
  int i,iligne=strlen(ligne);
  for (i=iligne-1; i>=0; i--) {
    if (ligne[i]!='\r' && ligne[i]!='\n') { iligne=i+1; break; }
    }
  IF(AutTranslateBuffer2(iligne,ligne,out)) continue;
  IFE(AutAline2(ctrl_aut,out));
  number_line++;
  }

fclose(fd);
DONE;
}




/*
 *  Le format interne d'une entr\'ee sur l'automate
 *  est une suite d'entiers qui se termine par -1.
 *  Cette fonction permet de passer au format interne.
 *  Elle est utilisée par la majeure partie des fonctions
 *  de l'API. Lorsque l'on d\'epasse la taille du buffer
 *  utilis\'e, cette fonction renvoit -1.
*/

int AutTranslateBuffer2(iline,line,out)
int iline; unsigned char *line;
int *out;
{
int i;
if (iline<0) {
  for (i=0; line[i]!=0; i++) {
    if (i>=DPcAutMaxBufferSize-1) return(-1);
    out[i]=line[i];
    }
  }
else if (iline>=DPcAutMaxBufferSize) return(-1);
else { 
  for (i=0; i<iline; i++) out[i]=line[i];
  }
out[i]=(-1);
return(i);
}

