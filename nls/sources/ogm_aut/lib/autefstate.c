/*
 *	Handling little endian and big endian.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev: October 2007
 *	Version 1.0
*/

#include "ogm_aut.h"


STATICF(int) AutEndianChangeFstate1(pr_(struct og_ctrl_aut *) pr_(int) pr_(struct fstate *) pr(struct fstate *));





int AutEndianFstate(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
struct fstate cfstate1,*fstate1=&cfstate1;
struct fstate cfstate2,*fstate2=&cfstate2;
struct fstate cfstate3,*fstate3=&cfstate3;
char alter_filename[DPcPathSize],*alter_endian;
char filename[DPcPathSize],*endian;
char erreur[DOgErrorSize];
int little_endian;
FILE *fd;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian) { little_endian=1; endian="little_endian"; alter_endian="big_endian"; }
else { endian="big_endian"; alter_endian="little_endian"; }

OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFstate: endian of current machine is %s, sizeof(struct fstate)=%d",endian,sizeof(struct fstate));

sprintf(filename,"%s_fstate.bin",endian);
OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFstate: writing '%s'",filename);
IFn(fd=fopen(filename,"wb")) {
  sprintf(erreur,"AutEndianFstate: impossible to fopen for writing '%s'",filename);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
fstate1->lettre=8; 
fstate1->index=22; 
fstate1->final=1; 
fstate1->last=1; 
fwrite((void *)fstate1,sizeof(struct fstate),1,fd);
fclose(fd);

sprintf(alter_filename,"%s_fstate.bin",alter_endian);
if (OgFileExists(alter_filename)) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFstate: reading '%s'",alter_filename);
  IFn(fd=fopen(alter_filename,"rb")) {
    sprintf(erreur,"AutEndianFstate: impossible to fopen for reading '%s'",alter_filename);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  fread((void *)fstate2,sizeof(struct fstate),1,fd);
  fclose(fd);
  IFE(AutEndianChangeFstate1(ctrl_aut,little_endian,fstate2,fstate3));
  if (!memcmp(fstate1,fstate3,sizeof(struct fstate))) {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFstate: endian change ok");
    }
  else {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFstate: endian change not ok");
    }  
  }

DONE;
}






int AutEndianChangeFstate(ctrl_aut,little_endian_automaton)
struct og_ctrl_aut *ctrl_aut;
int little_endian_automaton;
{
struct fstate *fstate;
int i,little_endian;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian_automaton == little_endian) DONE;

for (i=0; i<ctrl_aut->FstateUsed; i++) {
  fstate = ctrl_aut->Fstate + i;
  IFE(AutEndianChangeFstate1(ctrl_aut,little_endian,fstate,fstate));
  }

DONE;
}





STATICF(int) AutEndianChangeFstate1(ctrl_aut,little_endian,fstate1,fstate2)
struct og_ctrl_aut *ctrl_aut; int little_endian;
struct fstate *fstate1,*fstate2;
{
unsigned buffin[1],buffout;
char erreur[DOgErrorSize];

if (sizeof(struct fstate) != 1*sizeof(unsigned)) {
  sprintf(erreur,"AutEndianChangeFstate1: sizeof(struct fstate) (%d) != 1*sizeof(unsigned) (%d)",sizeof(struct fstate),1*sizeof(unsigned));
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

memcpy(buffin,fstate1,sizeof(struct fstate)); 
IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[0],&buffout));

if (little_endian) {
  fstate2->lettre=(buffout & 0xff000000) >> 24;
  fstate2->index=(buffout & 0xffffff) >> 2;
  fstate2->final=(buffout & 0x2) >> 1;
  fstate2->last=(buffout & 0x1);
  }
else {
  fstate2->lettre=(buffout & 0xff);
  fstate2->index=(buffout & 0x3fffff00) >> 8;
  fstate2->final=(buffout & 0x40000000) >> 30;
  fstate2->last=(buffout & 0x80000000) >> 31;
  }
  
DONE;
}




