/*
 *	Lecture et \'ecriture d'un automate sur fichier.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : D\'ecembre 1996, Janvier 1997, March 2006, May 2008
 *	Version 1.3
*/

#include "ogm_aut.h"


struct auf_header {
  char string[DOgIdentificationBanner];
  unsigned long nb_transition;
  unsigned long flags;
  };


STATICF(void) AufWriteHeader(pr_(struct auf_header *) pr(char *));
STATICF(void) AufReadHeader(pr_(struct auf_header *) pr(char *));


/*
 *	Sauvegarde d'un automate non compress\'e minimis\'e ou non.
 *	La sauvegarde se fait dans le fichier dont le nom est dans 'file'
 *	Renvoit ERREUR s'il y a eu une erreur quelconque.
*/

PUBLIC(int) OgAufWrite(handle,file)
void *handle;
char *file;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char afc[DOgIdentificationBanner+8+8+4];
char erreur[DOgErrorSize];
struct auf_header header;
FILE *fd;

IFn(fd=fopen(file,"wb")) {
  sprintf(erreur,"OgAufWrite (%s): impossible to write '%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** First writing the header **/
memset(&header,0,sizeof(struct auf_header));
sprintf(header.string,"AUF %s",DOgAutBanner);
if (ctrl_aut->Fstate) header.nb_transition=ctrl_aut->FstateUsed;
else header.nb_transition=ctrl_aut->GstateUsed;
if (OgLittleEndian()) header.flags|=DOgFlagsLittleEndian;
if (ctrl_aut->Gstate) header.flags|=DOgFlagsIsAug;
AufWriteHeader(&header,afc);

IFn(fwrite(afc,1,DOgIdentificationBanner+8+8,fd)) {
  sprintf(erreur,"OgAufWrite (%s): impossible to write header",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** Then writing the automaton **/
if (ctrl_aut->Fstate) {
  IFn(fwrite((char *)ctrl_aut->Fstate,sizeof(struct fstate),ctrl_aut->FstateUsed,fd)) {
    sprintf(erreur,"OgAufWrite (%s): impossible to write Fstate",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  }
else if (ctrl_aut->Gstate) {
  IFn(fwrite((char *)ctrl_aut->Gstate,sizeof(struct gstate),ctrl_aut->GstateUsed,fd)) {
    sprintf(erreur,"OgAufWrite (%s): impossible to write Gstate",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  }

fclose(fd);
DONE;
}




/*
 *	Lecture d'un automate \`a partir d'un fichier dont 
 *	le nom est dans 'file'. Renvoit ERREUR s'il y a eu une 
 *	erreur quelconque. sinon renvoit le descripteur
 *	de cet automate.
*/

PUBLIC(int) OgAufRead(handle,file)
void *handle;
char *file;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char afc[DOgIdentificationBanner+8+8+4];
int little_endian_automaton;
char erreur[DOgErrorSize];
struct auf_header header;
FILE *fd;

IFn(fd=fopen(file,"rb")) {
  sprintf(erreur,"OgAufRead (%s): impossible to open '%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** On lit d'abord le header **/
IFn(fread(afc,1,DOgIdentificationBanner+8+8,fd)) {
  sprintf(erreur,"OgAufRead (%s): impossible to read header",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
AufReadHeader(&header,afc);
if (memcmp(header.string,"AUF",3)) {
  sprintf(erreur,"OgAufRead (%s): automaton format should be \"AUF\"",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;  
  }

if (header.flags & DOgFlagsLittleEndian) little_endian_automaton=1;
else little_endian_automaton=0;

if (header.flags & DOgFlagsIsAug) {
  /** Lecture du reste de l'automate **/
  ctrl_aut->GstateUsed=header.nb_transition; DPcFree(ctrl_aut->Gstate);
  if (ctrl_aut->GstateUsed > 0) {
    IFn(ctrl_aut->Gstate=(struct gstate *)malloc(ctrl_aut->GstateUsed*sizeof(struct gstate))) {
      sprintf(erreur,"OgAufRead (%s): malloc error on Gstate",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    IFn(fread((char *)ctrl_aut->Gstate,sizeof(struct gstate),ctrl_aut->GstateUsed,fd)) {
      sprintf(erreur,"OgAufRead (%s): impossible to read Gstate",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    }
  IFE(AutEndianChangeGstate(ctrl_aut,little_endian_automaton));
  ctrl_aut->is_aug=1;
  }
else {
  /** Lecture du reste de l'automate **/
  ctrl_aut->FstateAllocated=1;
  ctrl_aut->FstateUsed=header.nb_transition; DPcFree(ctrl_aut->Fstate);
  if (ctrl_aut->FstateUsed > 0) {
    IFn(ctrl_aut->Fstate=(struct fstate *)malloc(ctrl_aut->FstateUsed*sizeof(struct fstate))) {
      sprintf(erreur,"OgAufRead (%s): malloc error on Fstate",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    IFn(fread((char *)ctrl_aut->Fstate,sizeof(struct fstate),ctrl_aut->FstateUsed,fd)) {
      sprintf(erreur,"OgAufRead (%s): impossible to read Fstate",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    }
  IFE(AutEndianChangeFstate(ctrl_aut,little_endian_automaton));
  ctrl_aut->is_aug=0;
  }

fclose(fd);
DONE;
}




/*
 *	Ecriture du header de l'automate rapide 'afc'
 *	On fait une \'ecriture avec des sprintf pour rester
 *	portable sur les diff\'erentes plate-formes.
 *	On utilise un format hexadecimal qui a l'avantage 
 *	de tenir sur 8 caract\`eres.
*/

STATICF(void) AufWriteHeader(header,afc)
struct auf_header *header;
char *afc;
{
char nombre[20];
memcpy(afc,header->string,DOgIdentificationBanner); afc+=DOgIdentificationBanner;
DPcSprintf(nombre,"%.8lx",header->nb_transition);
memcpy(afc,nombre,8); afc+=8;
DPcSprintf(nombre,"%.8lx",header->flags);
memcpy(afc,nombre,8); afc+=8;
}



/*
 *	Lecture du header de l'automate compress\'e 'afc'
*/

STATICF(void) AufReadHeader(header,afc)
struct auf_header *header;
char *afc;
{
char nombre[20];
memcpy(header->string,afc,DOgIdentificationBanner); afc+=DOgIdentificationBanner;
memcpy(nombre,afc,8); nombre[8]=0; afc+=8;
header->nb_transition=(unsigned long)strtol(nombre,(char **)0,16);
memcpy(nombre,afc,8); nombre[8]=0; afc+=8;
header->flags=(unsigned long)strtol(nombre,(char **)0,16);
}



