/*
 *	Lecture et \'ecriture d'un automate sur fichier.
 *	Copyright (c) 1996-2006 Pertimm by Patrick Constant
 *	Dev : D\'ecembre 1996, Janvier 1997, March 2006
 *	Version 1.2
*/

#include "ogm_aut.h"


#define CORRECTION_254


struct aut_header {
  char string[DOgIdentificationBanner];
  unsigned long nb_transition;
  unsigned long is_free_chain;
  unsigned long flags;
  };


#ifdef CORRECTION_254
struct state_253 {
  unsigned index;
  unsigned next;
  unsigned lettre: DPcMxlettre;
  unsigned start: 1;
  unsigned final: 1;
  unsigned freq: 6;
  unsigned libre:15;
  };
#endif

static void AutWriteHeader(pr_(struct aut_header *) pr(char *));
static void AutReadHeader(pr_(struct aut_header *) pr(char *));
static int AutReadVersion(struct og_ctrl_aut *, unsigned char *, int *);


/*
 *	Sauvegarde d'un automate non compress\'e minimis\'e ou non.
 *	La sauvegarde se fait dans le fichier dont le nom est dans 'file'
 *	Renvoit ERREUR s'il y a eu une erreur quelconque.
*/

PUBLIC(int) OgAutWrite(handle,file)
void *handle;
char *file;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char afc[DOgIdentificationBanner+8+8+8+4];
char erreur[DOgErrorSize];
struct aut_header header;
FILE *fd;

IFn(fd=fopen(file,"wb")) {
  sprintf(erreur,"OgAutWrite (%s): impossible to write '%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** First writing the header **/
memset(&header,0,sizeof(struct aut_header));
sprintf(header.string,"AUT %s",DOgAutBanner);
header.nb_transition=ctrl_aut->StateUsed;
if (ctrl_aut->FreeState) header.is_free_chain=1;
if (OgLittleEndian()) header.flags|=DOgFlagsLittleEndian;
AutWriteHeader(&header,afc);

IFn(fwrite(afc,1,DOgIdentificationBanner+8+8+8,fd)) {
  sprintf(erreur,"OgAutWrite (%s): impossible to write header",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** Writing the automaton **/
if (ctrl_aut->StateUsed>0) {
  IFn(fwrite((void *)ctrl_aut->State,sizeof(struct state),ctrl_aut->StateUsed,fd)) {
    sprintf(erreur,"OgAutWrite (%s): impossible to write State with size %d"
      ,ctrl_aut->name,sizeof(struct state)*ctrl_aut->StateUsed);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  }

/** Writing the automaton when necessary **/
if (ctrl_aut->FreeState) {
  IFn(fwrite((void *)ctrl_aut->FreeState,sizeof(struct free_state),ctrl_aut->StateUsed,fd)) {
    sprintf(erreur,"OgAutWrite (%s): impossible to write FreeState with size %d"
      ,ctrl_aut->name,sizeof(struct free_state)*ctrl_aut->StateUsed);
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

PUBLIC(int) OgAutRead(handle,file)
void *handle;
char *file;
{
struct og_ctrl_aut *ctrl_aut = (struct og_ctrl_aut *)handle;
char afc[DOgIdentificationBanner+8+8+8+4];
int little_endian_automaton;
#ifdef CORRECTION_254
int i,found_version,version;
#endif
char erreur[DOgErrorSize];
struct aut_header header;
FILE *fd;

IFn(fd=fopen(file,"rb")) {
  sprintf(erreur,"OgAutRead (%s): impossible to open '%s'",ctrl_aut->name,file);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

/** On lit d'abord le header **/
IFn(fread(afc,1,DOgIdentificationBanner+8+8+8,fd)) {
  sprintf(erreur,"OgAutRead (%s): impossible to read header",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
AutReadHeader(&header,afc);
if (memcmp(header.string,"AUT",3)) {
  sprintf(erreur,"OgAutRead (%s): automaton format should be \"AUT\"",ctrl_aut->name);
  OgErr(ctrl_aut->herr,erreur); DPcErr;  
  }

if (header.flags & DOgFlagsLittleEndian) little_endian_automaton=1;
else little_endian_automaton=0;

DPcFree(ctrl_aut->State);
/** Lecture du reste de l'automate **/
ctrl_aut->StateUsed = header.nb_transition;
ctrl_aut->StateNumber = header.nb_transition;

if (ctrl_aut->StateNumber>0) {
  DPcFree(ctrl_aut->State);
  IFn(ctrl_aut->State=(struct state *)malloc(ctrl_aut->StateNumber*sizeof(struct state))) {
    sprintf(erreur,"OgAutRead (%s): malloc error on State",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
#ifdef CORRECTION_254
  IFE(found_version=AutReadVersion(ctrl_aut,header.string,&version));
  if (found_version && version < 254) {
    struct state_253 *State_253,*state_253;
    struct state *state;
    IFn(State_253=(struct state_253 *)malloc(ctrl_aut->StateNumber*sizeof(struct state_253))) {
      sprintf(erreur,"OgAutRead (%s): malloc error on State_253",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    IFn(fread((char *)State_253,sizeof(struct state_253),ctrl_aut->StateNumber,fd)) {
      sprintf(erreur,"OgAutRead (%s): impossible to read State_253",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    for (i=0; i<ctrl_aut->StateNumber; i++) {
      state_253 = State_253 + i;
      state = ctrl_aut->State + i;
      state->index = state_253->index;
      state->next = state_253->next;
      state->lettre = state_253->lettre;
      state->start = state_253->start;
      state->final = state_253->final;
      state->freq = state_253->freq;
      }
    DPcFree(State_253);
    }
  else {  
    IFn(fread((char *)ctrl_aut->State,sizeof(struct state),ctrl_aut->StateNumber,fd)) {
      sprintf(erreur,"OgAutRead (%s): impossible to read State",ctrl_aut->name);
      OgErr(ctrl_aut->herr,erreur); DPcErr;
      }
    }
#else
  IFn(fread((char *)ctrl_aut->State,sizeof(struct state),ctrl_aut->StateNumber,fd)) {
    sprintf(erreur,"OgAutRead (%s): impossible to read State",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
#endif

  /** Should be corrected and tested for version 2.54 and above **/
  IFE(AutEndianChangeState(ctrl_aut,little_endian_automaton));
  }

if (header.is_free_chain) {
  DPcFree(ctrl_aut->FreeState);
  IFn(ctrl_aut->FreeState=(struct free_state *)malloc(ctrl_aut->StateNumber*sizeof(struct free_state))) {
    sprintf(erreur,"OgAutRead (%s): malloc error on FreeState",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  IFn(fread((char *)ctrl_aut->FreeState,sizeof(struct free_state),ctrl_aut->StateNumber,fd)) {
    sprintf(erreur,"OgAutRead (%s): impossible to read FreeState",ctrl_aut->name);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  /** Should be corrected and tested for version 2.54 and above **/
  IFE(AutEndianChangeFrstate(ctrl_aut,little_endian_automaton));
  }

fclose(fd);
/* Used to read an .aut file written in a machine with a different endian
 * This should not happen as the .aut format is not a very remanent format
 * we would prefer the .auf format. */
/** AutOsysOsys(ctrl_aut,header.flags); **/
DONE;
}




/*
 *	Ecriture du header de l'automate rapide 'afc'
 *	On fait une \'ecriture avec des sprintf pour rester
 *	portable sur les diff\'erentes plate-formes.
 *	On utilise un format hexadecimal qui a l'avantage 
 *	de tenir sur 8 caract\`eres.
*/

static void AutWriteHeader(header,afc)
struct aut_header *header;
char *afc;
{
char nombre[20];
memcpy(afc,header->string,DOgIdentificationBanner); afc+=DOgIdentificationBanner;
sprintf(nombre,"%.8lx",header->nb_transition);
memcpy(afc,nombre,8); afc+=8;
sprintf(nombre,"%.8lx",header->is_free_chain);
memcpy(afc,nombre,8); afc+=8;
sprintf(nombre,"%.8lx",header->flags);
memcpy(afc,nombre,8); afc+=8;
}



/*
 *	Lecture du header de l'automate compress\'e 'afc'
*/

static void AutReadHeader(header,afc)
struct aut_header *header;
char *afc;
{
char nombre[20];
memcpy(header->string,afc,DOgIdentificationBanner); afc+=DOgIdentificationBanner;
memcpy(nombre,afc,8); nombre[8]=0; afc+=8;
header->nb_transition=(unsigned long)strtol(nombre,(char **)0,16);
memcpy(nombre,afc,8); nombre[8]=0; afc+=8;
header->is_free_chain=(unsigned long)strtol(nombre,(char **)0,16);
memcpy(nombre,afc,8); nombre[8]=0; afc+=8;
header->flags=(unsigned long)strtol(nombre,(char **)0,16);
}




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


