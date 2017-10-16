/*
 *	Handling little endian and big endian.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev: October 2007
 *	Version 1.0
*/

#include "ogm_aut.h"


STATICF(int) AutEndianChangeGstate1(pr_(struct og_ctrl_aut *) pr_(int) pr_(struct gstate *) pr(struct gstate *));






int AutEndianGstate(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
struct gstate cgstate1,*gstate1=&cgstate1;
struct gstate cgstate2,*gstate2=&cgstate2;
struct gstate cgstate3,*gstate3=&cgstate3;
char alter_filename[DPcPathSize],*alter_endian;
char filename[DPcPathSize],*endian;
char erreur[DOgErrorSize];
int little_endian;
FILE *fd;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian) { little_endian=1; endian="little_endian"; alter_endian="big_endian"; }
else { endian="big_endian"; alter_endian="little_endian"; }

OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianGstate: endian of current machine is %s, sizeof(struct gstate)=%d",endian,sizeof(struct gstate));

sprintf(filename,"%s_gstate.bin",endian);
OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianGstate: writing '%s'",filename);
IFn(fd=fopen(filename,"wb")) {
  sprintf(erreur,"AutEndianGstate: impossible to fopen for writing '%s'",filename);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
gstate1->index=32; 
gstate1->lettre=8; 
gstate1->final=1; 
gstate1->last=1; 
gstate1->libre=0; 
fwrite((void *)gstate1,sizeof(struct gstate),1,fd);
fclose(fd);

sprintf(alter_filename,"%s_gstate.bin",alter_endian);
if (OgFileExists(alter_filename)) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianGstate: reading '%s'",alter_filename);
  IFn(fd=fopen(alter_filename,"rb")) {
    sprintf(erreur,"AutEndianGstate: impossible to fopen for reading '%s'",alter_filename);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  fread((void *)gstate2,sizeof(struct gstate),1,fd);
  fclose(fd);
  IFE(AutEndianChangeGstate1(ctrl_aut,little_endian,gstate2,gstate3));
  if (!memcmp(gstate1,gstate3,sizeof(struct gstate))) {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianGstate: endian change ok");
    }
  else {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianGstate: endian change not ok");
    }  
  }

DONE;
}






int AutEndianChangeGstate(ctrl_aut,little_endian_automaton)
struct og_ctrl_aut *ctrl_aut;
int little_endian_automaton;
{
struct gstate *gstate;
int i,little_endian;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian_automaton == little_endian) DONE;

for (i=0; i<ctrl_aut->GstateUsed; i++) {
  gstate = ctrl_aut->Gstate + i;
  IFE(AutEndianChangeGstate1(ctrl_aut,little_endian,gstate,gstate));
  }

DONE;
}





STATICF(int) AutEndianChangeGstate1(ctrl_aut,little_endian,gstate1,gstate2)
struct og_ctrl_aut *ctrl_aut; int little_endian;
struct gstate *gstate1,*gstate2;
{
unsigned buffin[2],buffout;
char erreur[DOgErrorSize];

if (sizeof(struct gstate) != 2*sizeof(unsigned)) {
  sprintf(erreur,"AutEndianChangeGstate1: sizeof(struct gstate) (%d) != 2*sizeof(unsigned) (%d)",sizeof(struct gstate),2*sizeof(unsigned));
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

memcpy(buffin,gstate1,sizeof(struct gstate)); 

IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[0],&buffout));
memcpy(&gstate2->index,&buffout,sizeof(unsigned)); 

IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[1],&buffout));

if (little_endian) {
  gstate2->lettre=(buffout & 0xff000000) >> 24;
  gstate2->final=(buffout & 0x800000) >> 23;
  gstate2->last=(buffout & 0x400000) >> 22;
  gstate2->libre=(buffout & 0x3fffff);
  }
else {
  gstate2->lettre=(buffout & 0xff);
  gstate2->final=(buffout & 0x100) >> 8;
  gstate2->last=(buffout & 0x200) >> 9;
  gstate2->libre=(buffout & 0xfffffc00) >> 10;
  }
  
DONE;
}




