/*
 * Fonction qui analyse un fichier et construit l'automate
 * de base (non compress\'e et non minimis\'e). Le fichier
 * contient des informations linguistiques sous un format particulier.
 * Copyright (c) 1997-2008 Pertimm by Patrick Constant
 * Dev : January 1997, July 2008
 * Version 1.1
*/
#include "ogm_aut.h"


/*
 *  Fonction qui analyse le ficher 'file' et qui construit
 *  un automate non compress\'e et non minimis\'e. Le fichier
 *  contient des informations linguistiques sous un format particulier.
 *  'ha' est un handle d'automate renvoy\'e par PcAutInit();
 *  Utiliser PcAum(ha) pour minimiser, PcAuz(ha) pour compresser
 *  et PcAuf(ha) pour obtenir un automate rapide.
 *  Voir lpcaut.h pour les valeurs/significations des flags.
*/

PUBLIC(int) OgAul(handle,file,nocharcase,flags)
void *handle; char *file;
int nocharcase,flags;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
int opened_filei=0;
FILE *fd, *fd_filei;
int i,j,empty,nline;
int out[DPcAutMaxBufferSize];
unsigned char in[DPcMaxSizeDecl];
char basefile[DPcPathSize],filei[DPcPathSize];
unsigned char buffer[DPcMaxNbNormal][DPcMaxSizeDecl];
char erreur[DOgErrorSize];

ctrl_aut->aul_nocharcase=nocharcase;

IFn(fd=fopen(file,"r")) {
  DPcSprintf(erreur,"OgAul : impossible to open `%s'",file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** On enl\`eve l'extension .aus si elle existe pour avoir le nom de base **/
for (i=0; file[i]!=0 && file[i]!='.'; i++) basefile[i]=file[i]; basefile[i]=0;

empty=1; nline=1; fd_filei=0;
while(fgets(in,DPcMaxSizeDecl,fd)) {
  in[strlen(in)-1]=0; /* On enl\`eve le '\n' \`a la fin */
  IF(AulDecl(ctrl_aut,nline,in,buffer,DPcMaxNbNormal)) {
    OgErrLast(ctrl_aut->herr,erreur,0);
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"%s",erreur);
    continue;
    }
  for (i=0; buffer[i][0]!=0; i++) {
    for (j=0; buffer[i][j]!=0; j++) out[j]=buffer[i][j]; out[j]=(-1);
    if (flags&DPcAulDirect) {
      IFE(AutAline2(ctrl_aut,out));
    }
    else if (flags>=DPcAulFile) {
      if (!opened_filei) {
        opened_filei=1; sprintf(filei,"%si.aus",basefile);
        IFn(fd_filei=fopen(filei,"w")) {
          sprintf(erreur,"PcAul : impossible to open `%s'",file);
          OgErr(ctrl_aut->herr,erreur); DPcErr;
          }
        }
      fprintf(fd_filei,"%s\n",buffer[i]);
      }
    }
  empty=0; nline++;
  }

if (opened_filei) fclose(fd_filei);

if (empty) {
  DPcSprintf(erreur,"OgAul : file empty `%s'",file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

fclose(fd);
DONE;
}




