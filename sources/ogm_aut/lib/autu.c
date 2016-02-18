/*
 *	Fonction de base qui analyse un fichier en Unicode et construit l'automate 
 *	de base (non compress\'e et non minimis\'e).
 *	Copyright (c) 2001-2006	Pertimm by Patrick Constant
 *	Dev: Aout 2001, March 2006
 *	Version 1.1
*/
#include "ogm_aut.h"



/*
 *	Fonction qui analyse le ficher 'file' et qui construit
 *	un automate non compress\'e et non minimis\'e.
 *	'ha' est un handle d'automate renvoy\'e par OgAutInit();
 *	Utiliser OgAum(handle) pour minimiser et OgAuf(ha) 
 *  pour obtenir un automate rapide.
*/

PUBLIC(int) OgAutu(handle,file)
void *handle; char *file;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int empty,out[DPcAutMaxBufferSize];
char erreur[DOgErrorSize];
int oldc,c,iout,nb_char;
struct stat file_stat;
unsigned number_line;
FILE *fd;

if (stat(file,&file_stat)) {
  sprintf(erreur,"OgAutu (%s): impossible to stat `%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

if (ctrl_aut->StateNumber < file_stat.st_size) {
  ctrl_aut->StateUsed = 0;
  ctrl_aut->StartFreeState = (-1);
  ctrl_aut->StateNumber = file_stat.st_size; DPcFree(ctrl_aut->State);
  IFn(ctrl_aut->State=(struct state *)malloc(ctrl_aut->StateNumber*sizeof(struct state))) {
    sprintf(erreur,"OgAutu (%s): malloc error on State",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); return(0);
    }
  }

IFn(fd=fopen(file,"rb")) {
  sprintf(erreur,"OgAutu (%s): impossible to fopen `%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

empty=1; number_line=1; nb_char=0;

oldc=-1; iout=0;
while((c=getc(fd))!=EOF) {
  if (oldc==0 && c=='\n') {
    out[iout-1]=(-1);
    IFE(AutAline2(ctrl_aut,out));
    iout=0; empty=0; number_line++; nb_char++;
    }
  else {
    if (iout < DPcAutMaxBufferSize-1) out[iout++]=c;
    }
  oldc=c;
  }

fclose(fd);
DONE;
}

