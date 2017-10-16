/*
 *	Handling little endian and big endian.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev: October 2007
 *	Version 1.0
*/

#include "ogm_aut.h"


STATICF(int) AutEndianChangeState1(pr_(struct og_ctrl_aut *) pr_(int) pr_(struct state *) pr(struct state *));







int AutEndianState(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
struct state cstate1,*state1=&cstate1;
struct state cstate2,*state2=&cstate2;
struct state cstate3,*state3=&cstate3;
char alter_filename[DPcPathSize],*alter_endian;
char filename[DPcPathSize],*endian;
char erreur[DOgErrorSize];
int little_endian;
FILE *fd;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian) { little_endian=1; endian="little_endian"; alter_endian="big_endian"; }
else { endian="big_endian"; alter_endian="little_endian"; }

OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianState: endian of current machine is %s, sizeof(struct state)=%d",endian,sizeof(struct state));

sprintf(filename,"%s_state.bin",endian);
OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianState: writing '%s'",filename);
IFn(fd=fopen(filename,"wb")) {
  sprintf(erreur,"AutEndianState: impossible to fopen for writing '%s'",filename);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
state1->index=32; 
state1->next=33; 
state1->lettre=8; 
state1->start=1; 
state1->final=1; 
state1->freq=6; 
state1->libre=0; 
fwrite((void *)state1,sizeof(struct state),1,fd);
fclose(fd);

sprintf(alter_filename,"%s_state.bin",alter_endian);
if (OgFileExists(alter_filename)) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianState: reading '%s'",alter_filename);
  IFn(fd=fopen(alter_filename,"rb")) {
    sprintf(erreur,"AutEndianState: impossible to fopen for reading '%s'",alter_filename);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  fread((void *)state2,sizeof(struct state),1,fd);
  fclose(fd);
  IFE(AutEndianChangeState1(ctrl_aut,little_endian,state2,state3));
  if (!memcmp(state1,state3,sizeof(struct state))) {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianState: endian change ok");
    }
  else {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianState: endian change not ok");
    }  
  }

DONE;
}






int AutEndianChangeState(ctrl_aut,little_endian_automaton)
struct og_ctrl_aut *ctrl_aut;
int little_endian_automaton;
{
struct state *state;
int i,little_endian;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian_automaton == little_endian) DONE;

for (i=0; i<ctrl_aut->StateUsed; i++) {
  state = ctrl_aut->State + i;
  IFE(AutEndianChangeState1(ctrl_aut,little_endian,state,state));
  }

DONE;
}





STATICF(int) AutEndianChangeState1(ctrl_aut,little_endian,state1,state2)
struct og_ctrl_aut *ctrl_aut; int little_endian;
struct state *state1,*state2;
{
unsigned buffin[3],buffout;
char erreur[DOgErrorSize];

if (sizeof(struct state) != 3*sizeof(unsigned)) {
  sprintf(erreur,"AutEndianChangeState1: sizeof(struct state) (%d) != 3*sizeof(unsigned) (%d)",sizeof(struct state),3*sizeof(unsigned));
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

memcpy(buffin,state1,sizeof(struct state)); 

IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[0],&buffout));
memcpy(&state2->index,&buffout,sizeof(unsigned)); 

IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[1],&buffout));
memcpy(&state2->next,&buffout,sizeof(unsigned)); 

IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[2],&buffout));

if (little_endian) {
  state2->lettre=(buffout & 0xff000000) >> 24;
  state2->start=(buffout & 0x800000) >> 23;
  state2->final=(buffout & 0x400000) >> 22;
  state2->freq=(buffout & 0x3f0000) >> 16;
  state2->libre=(buffout & 0xffff);
  }
else {
  state2->lettre=(buffout & 0xff);
  state2->start=(buffout & 0x100) >> 8;
  state2->final=(buffout & 0x200) >> 9;
  state2->freq=(buffout & 0xfc00) >> 10;
  state2->libre=(buffout & 0xffff0000) >> 16;
  }
  
DONE;
}



