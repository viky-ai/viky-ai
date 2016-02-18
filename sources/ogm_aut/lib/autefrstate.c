/*
 *	Handling little endian and big endian.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev: October 2007
 *	Version 1.0
*/

#include "ogm_aut.h"


STATICF(int) AutEndianChangeFrstate1(pr_(struct og_ctrl_aut *) pr_(int) pr_(struct free_state *) pr(struct free_state *));






int AutEndianFrstate(ctrl_aut)
struct og_ctrl_aut *ctrl_aut;
{
struct free_state cfrstate1,*frstate1=&cfrstate1;
struct free_state cfrstate2,*frstate2=&cfrstate2;
struct free_state cfrstate3,*frstate3=&cfrstate3;
char alter_filename[DPcPathSize],*alter_endian;
char filename[DPcPathSize],*endian;
char erreur[DOgErrorSize];
int little_endian;
FILE *fd;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian) { little_endian=1; endian="little_endian"; alter_endian="big_endian"; }
else { endian="big_endian"; alter_endian="little_endian"; }

OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFrstate: endian of current machine is %s, sizeof(struct free_state)=%d",endian,sizeof(struct free_state));

sprintf(filename,"%s_frstate.bin",endian);
OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFrstate: writing '%s'",filename);
IFn(fd=fopen(filename,"wb")) {
  sprintf(erreur,"AutEndianFrstate: impossible to fopen for writing '%s'",filename);
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }
frstate1->nx_free=32; 
fwrite((void *)frstate1,sizeof(struct free_state),1,fd);
fclose(fd);

sprintf(alter_filename,"%s_frstate.bin",alter_endian);
if (OgFileExists(alter_filename)) {
  OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFrstate: reading '%s'",alter_filename);
  IFn(fd=fopen(alter_filename,"rb")) {
    sprintf(erreur,"AutEndianFrstate: impossible to fopen for reading '%s'",alter_filename);
    OgErr(ctrl_aut->herr,erreur); DPcErr;
    }
  fread((void *)frstate2,sizeof(struct free_state),1,fd);
  fclose(fd);
  IFE(AutEndianChangeFrstate1(ctrl_aut,little_endian,frstate2,frstate3));
  if (!memcmp(frstate1,frstate3,sizeof(struct free_state))) {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFrstate: endian change ok");
    }
  else {
    OgMessageLog(DOgMlogInLog,ctrl_aut->loginfo->where,0,"AutEndianFrstate: endian change not ok");
    }  
  }

DONE;
}






int AutEndianChangeFrstate(ctrl_aut,little_endian_automaton)
struct og_ctrl_aut *ctrl_aut;
int little_endian_automaton;
{
struct free_state *frstate;
int i,little_endian;

if (OgLittleEndian()) little_endian=1;
else little_endian=0;

if (little_endian_automaton == little_endian) DONE;

for (i=0; i<ctrl_aut->StateUsed; i++) {
  frstate = ctrl_aut->FreeState + i;
  IFE(AutEndianChangeFrstate1(ctrl_aut,little_endian,frstate,frstate));
  }

DONE;
}





STATICF(int) AutEndianChangeFrstate1(ctrl_aut,little_endian,frstate1,frstate2)
struct og_ctrl_aut *ctrl_aut; int little_endian;
struct free_state *frstate1,*frstate2;
{
unsigned buffin[1],buffout;
char erreur[DOgErrorSize];

if (sizeof(struct free_state) != 1*sizeof(unsigned)) {
  sprintf(erreur,"AutEndianChangeFrstate1: sizeof(struct free_state) (%d) != 2*sizeof(unsigned) (%d)",sizeof(struct free_state),1*sizeof(unsigned));
  OgErr(ctrl_aut->herr,erreur); DPcErr;
  }

memcpy(buffin,frstate1,sizeof(struct free_state)); 

IFE(AutEndianChangeUnsigned(ctrl_aut,buffin[0],&buffout));
memcpy(&frstate2->nx_free,&buffout,sizeof(unsigned)); 
  
DONE;
}




