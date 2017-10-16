/*
 *  Internal header for handling automatons
 *  Copyright (c) 2006-2008 Pertimm, Inc. by Patrick Constant
 *  Dev : February,March 2006, December 2008
 *  Version 1.1
*/
#include <logaut.h>
#include <lpcmcr.h>


/** Simplifications pour les d\'eplacements sur les \'etats **/
#define GnState(x)  (((x)=ctrl_aut->State[x].next)!=0)


/*  DOgIdentificationBanner: Taille du header d'identification au d\'ebut
 *  de l'automate compress\'e. Pour des raisons de portabilit\'e
 *  entre Unix et Windows, DOgIdentificationBanner doit \^etre un multiple de 4. */
#define DOgIdentificationBanner     68


/** Value of saved flags when writing automatons **/
#define DOgFlagsLittleEndian    0x1
#define DOgFlagsIsAug           0x2

#define DOgLstminNumber   4

/*
 *  DPcMxlettre : nombre maximal de bits pour coder une lettre
 *  DPcMxindex : nombre maximal de bits pour coder une adresse
 *  2^24 = 16 777 216   2^22 = 4 194 304
 *  DPcMxgrp : taille maximal d'un groupe (en nombre de state)
*/
#define DPcMxlettre  8
// August 29th 2008 revert momentarily to 8 in order to be compatible to
// atva_in_matrices.aut in ssi for PJ. Howerever this is not compatible
// with ogm_ls (new linear sorting).
#define DPcMxExtlettre  9
//#define DPcMxExtlettre  8
#define DPcMxindex  24
#define DPcMxgrp  16


struct state {
  unsigned index;
  unsigned next;
  unsigned lettre: DPcMxExtlettre;
  unsigned start: 1;
  unsigned final: 1;
  unsigned freq: 6;
  unsigned libre:15;
  };


struct free_state {
  unsigned nx_free;
  };


struct fstate {
  unsigned lettre: DPcMxlettre;
  unsigned index: DPcMxindex-2;
  unsigned final: 1;
  unsigned last: 1;
  };


struct gstate {
  unsigned index;
  unsigned lettre: DPcMxlettre;
  unsigned final: 1;
  unsigned last: 1;
  unsigned libre:22;
  };


struct lstmin {
  unsigned grp;
  unsigned ngrp;
  unsigned hstate;
  unsigned hnext;
  unsigned nbgrp: DPcMxgrp;
  unsigned nbngrp: DPcMxgrp;
  unsigned len: 14; // DPcAutMaxBufferSize is 2^13 so 14 is good
  unsigned lenngrp: 14; // DPcAutMaxBufferSize is 2^13 so 14 is good
  unsigned hused: 1;
  unsigned dead: 1;
  unsigned standby: 2;
  };


/*
 *  Structures permettant la gestion des d\'eclarations linguistiques.
 *  DPcMaxSizeDecl : taille maximale d'une d\'eclaration ;
 *  DPcMaxNbComposante : nombre maximal de composantes ;
 *  DPcMaxNbNormal : nombre maximal de d\'eclarations construites
 *    \`a partir d'une sp\'ecification donn\'ee. les sp\'ecifications
 *    turber:=:danser donnent jusqu'\`a 91 d\'eclarations.
*/
#define DPcMaxSizeDecl      128
#define DPcMaxNbComposante  10
#define DPcMaxNbNormal      100


struct decl_normal {
  int type_attribut;
  unsigned char entry[DPcMaxSizeWord];
  unsigned char attribut[DPcMaxSizeWord];
  unsigned char value[DPcMaxSizeWord];
  int nentry,nattribut,nvalue;
  unsigned char composante_entry[DPcMaxNbComposante][DPcMaxSizeWord];
  unsigned char composante_value[DPcMaxNbComposante][DPcMaxSizeWord];
  int ncomposante_entry,ncomposante_value;
  unsigned char scanned[DPcMaxSizeWord];
  int killed;
  };


struct og_ctrl_aut {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  char name[DOgAutNameSize];

  int gauge_lstmin;
  int frequency;
  int is_aug;

  int FstateUsed;
  int FstateAllocated;
  struct fstate *Fstate;

  int GstateUsed;
  struct gstate *Gstate;

  int StateNumber,StateUsed,StartFreeState;
  struct free_state *FreeState;
  struct state *State;

  int LstminNumber,LstminUsed;
  struct lstmin *Lstmin;

  int aul_nocharcase;
  };

/* Internal function with same are appended a '2' at the end of the name
 * to avoid a naming clash under linux with the genau lib .
 * This is not a problem under windows. */

/** autstate.c **/
int FreeState(pr_(struct og_ctrl_aut *) pr(int));
int AllocState(pr(struct og_ctrl_aut *));

/** autlstmin.c **/
int AllocLstmin(pr(struct og_ctrl_aut *));

/** aut.c **/
int AutTranslateBuffer2(pr_(int) pr_(unsigned char *) pr(int *));

/** autaline.c **/
int AutAline2(pr_(struct og_ctrl_aut *) pr(int *));

/** autput.c **/
int AutPut2(pr_(struct og_ctrl_aut *) pr_(oindex) pr_(oindex) pr(oindex));

/** autget.c **/
oindex AutGet2(pr_(struct og_ctrl_aut *) pr_(oindex) pr(oindex));

/** autgline.c **/
oindex AutGline2(pr_(struct og_ctrl_aut *) pr(int *));

/** autfinal.c **/
int AutFinal2(pr_(struct og_ctrl_aut *) pr_(oindex) pr(int));

/** aumbuild.c **/
oindex AumBuild2(pr(struct og_ctrl_aut *));

/** auf.c **/
int Auf(pr(struct og_ctrl_aut *));

/** aug.c **/
int Aug(pr(struct og_ctrl_aut *));

/** aufgline.c **/
oindex AufGline2(pr_(struct fstate *) pr(int *));
oindex AugGline(pr_(struct gstate *) pr(int *));

/** aufscan.c **/
int AufScanf(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *) pr_(int *)
             pr_(unsigned char *) pr_(int *) pr_(int *) pr(oindex *));
int AufScann(pr_(struct og_ctrl_aut *) pr_(int *) pr_(unsigned char *)
             pr_(int) pr_(int *) pr(oindex *));

/** augscan.c **/
int AugScanf(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *) pr_(int *)
             pr_(unsigned char *) pr_(int *) pr_(int *) pr(oindex *));
int AugScann(pr_(struct og_ctrl_aut *) pr_(int *) pr_(unsigned char *)
             pr_(int) pr_(int *) pr(oindex *));

/** autendian.c **/
int AutEndianChangeUnsigned(pr_(struct og_ctrl_aut *) pr_(unsigned) pr(unsigned *));

/** autefstate.c **/
int AutEndianFstate(pr(struct og_ctrl_aut *));
int AutEndianChangeFstate(pr_(struct og_ctrl_aut *) pr(int));

/** autestate.c **/
int AutEndianState(pr(struct og_ctrl_aut *));
int AutEndianChangeState(pr_(struct og_ctrl_aut *) pr(int));

/** autegstate.c **/
int AutEndianGstate(pr(struct og_ctrl_aut *));
int AutEndianChangeGstate(pr_(struct og_ctrl_aut *) pr(int));

/** autefrstate.c **/
int AutEndianFrstate(pr(struct og_ctrl_aut *));
int AutEndianChangeFrstate(pr_(struct og_ctrl_aut *) pr(int));

/** autadde.c **/
int OgAutCheckLine(pr_(struct og_ctrl_aut *) pr_(char *) pr_(int) pr(int *));
int OgAutRupToExt(pr_(struct og_ctrl_aut *) pr_(int) pr_(int) pr_(unsigned char *) pr_(int *) pr(int *));

/** autscan.c **/
int AutGlineState(pr_(struct og_ctrl_aut *) pr_(int *) pr_(oindex *) pr(int *));
int AutScanf1(pr_(struct og_ctrl_aut *) pr_(int) pr(oindex *));
int AutScann1(pr_(struct og_ctrl_aut *) pr_(int) pr_(int *) pr(oindex *));

/** autpri.c **/
int AutPri(pr(struct og_ctrl_aut *));
int AutPri1(pr_(struct og_ctrl_aut *) pr(int));
int AumPri1(pr_(struct og_ctrl_aut *) pr(int));

/** auldecl.c **/
int AulDecl(pr_(struct og_ctrl_aut *) pr_(int) pr_(unsigned char *)
  pr_(unsigned char [][DPcMaxSizeDecl]) pr(int));
int AulDeclAttribut(pr(struct decl_normal *));
int AulDeclComposante(pr(struct decl_normal *));

/** aut254.c **/
int AutRead254Change(struct og_ctrl_aut *,char *);

/** autdel.c **/
int AutDline(struct og_ctrl_aut *ctrl_aut, int *line);

