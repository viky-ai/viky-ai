/*
 *  General include for automaton handling
 *  Copyright (c) 1996-2006 Pertimm par Patrick Constant
 *  Dev: Mars,Juin,Octobre,D\'ecembre 1996, Janvier, F\'evrier 1997
 *  Dev: Mars,Mai 1997, Septembre 1998, August 1999, March,August 2000, May 2003
 *  Dev: April 2004, November 2005, February, March 2006, June 2007
 *  Dev: May,July,August 2008, March 2009, February 2011
 *  Dev: July 2013, April 2014
 *  Version 3.4
*/

#ifndef _LOGAUTALIVE_
#include <lpcgentype.h>
#include <loggen.h>
#include <logthr.h>


#define DOgAutBanner  "ogm_aut V2.67, Copyright (c) 1996-2014 Pertimm"
#define DOgAutVersion  267



/** Trace levels **/
#define DOgAutTraceMinimal            0x1
#define DOgAutTraceMemory             0x2
#define DOgAutTraceMinimization       0x4
#define DOgAutTraceMinimizationDetail 0x8
#define DOgAutTraceMinimizationBuild  0x10


/** For debugging purposes it is nice to have a name for the automaton **/
#define DOgAutNameSize      128

struct og_aut_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char name[DOgAutNameSize];
  int state_number;
  };



/*
 *  Taille maximale des buffers de l'API
 *  Cela signifie qu'on ne peut pas entrer
 *  de mots plus grands que cette taille.
 *  Size changed because DOgFdMaxLoginSize (logfind.h)
 *  is changed to 8192 bytes march 9th 2005
 *
 *  WARNING : This value must be sync with DOgSsilAutMaxBufferSize in log_ssil.h
*/
#define DPcAutMaxBufferSize    8192


/**  Possible types of automatons **/
#define DOgAutNil       0
#define DOgAutNormal    1
#define DOgAutFast      2


struct aut_stat {
  int size_cell_min,size_cell_automat;
  int allocated_fast_automaton,size_cell_fast;
  int gauge_min,gauge_automat;
  int max_min,max_automat;
  int max_index, size_cell_index;
  int header_size,total_size;
  int dist_letter[256+9];
  int dist_index[32+9];
  int mxbit_index;
  int nb_letter;
  };

struct aut_memory {
  size_t aut_used, aut_number, aut_size_cell;
  size_t auf_used, auf_number, auf_size_cell;
  int large_fast_automaton;
  };

/*
 *  Liste des flags pour la fonction OgAul :
 *  DPcAulDirect : analyse directe des entr\'ees et construction
 *    directe de l'automate correspondant ;
 *  DPcAulFile : construction du fichier des entr\'ees
 *    interne de l'automate.
 *  DPcAulTermination : construction du fichier des entr\'ees avec
 *    des terminaisons simplifi\'ees et du fichier de ces terminaisons ;
 *  DPcAulIndirect : DPcAulFile et DPcAulTermination avec
 *    construction de l'automate correspondant.
*/
#define DPcAulDirect        0x1
#define DPcAulFile          0x2
#define DPcAulTermination   0x4
#define DPcAulIndirect      0x8


/*
 *  Types des attributs linguistiques pr\'ed\'efinis.
*/
#define DPcAttribut_cs  0x1
#define DPcAttribut_c   0x2
#define DPcAttribut_s   0x4
#define DPcAttribut_f   0x8
#define DPcAttribut_m   0x10
#define DPcAttribut_u   0x20
#define DPcAttribut_t   0x40
#define DPcAttribut_v   0x80
#define DPcAttribut_i   0x100
#define DPcAttribut_b   0x200
#define DPcAttribut_autre   0x400
#define DPcAttribut_delete  0x800
#define DPcAttribut_equal   0x1000
#define DPcAttribut_all     0xffff


/** value given by OgAutType, binary combined **/
#define DOgAutTypeAut   0x1
#define DOgAutTypeAuf   0x2
#define DOgAutTypeAug   0x4


/*
 *  Structure repr\'esentant les informations fournies
 *  par la fonction d'extraction linguistique PcAulScan :
 *  type_attribut : type de l'attribut ;
 *  entry : forme r\'eelle de l'entr\'ee (majuscule et tirets ajout\'es)
 *  attribut : cha\^ine de caract\`eres repr\'esentant l'attribut ;
 *  value : valeur explicite de l'attribut.
*/
#define DPcMaxSizeWord  100

struct att_value {
  int type_attribut;
  unsigned char entry[DPcMaxSizeWord];
  unsigned char attribut[DPcMaxSizeWord];
  unsigned char value[DPcMaxSizeWord];
  };


DEFPUBLIC(char *) OgAutBanner(pr(void));
DEFPUBLIC(void *) OgAutInit(pr(struct og_aut_param *));
DEFPUBLIC(int)    OgAut(pr_(void *) pr(char *));
DEFPUBLIC(int)    OgAutu(pr_(void *) pr(char *));
DEFPUBLIC(int) OgAutFlush(pr(void *));
DEFPUBLIC(int) OgAutWrite(pr_(void *) pr(char *));
DEFPUBLIC(int) OgAutRead(pr_(void *) pr(char *));
DEFPUBLIC(int) OgAutTry(pr_(void *) pr_(int) pr(unsigned char *));
DEFPUBLIC(int) OgAutLet(pr_(void *) pr_(int) pr_(unsigned char *) pr_(int *) pr(unsigned char *));
DEFPUBLIC(int) OgAutAdd(pr_(void *) pr_(int) pr(unsigned char *));
DEFPUBLIC(int) OgAutDel(pr_(void *) pr_(int) pr(unsigned char *));
DEFPUBLIC(int) OgAutFreqCtrl(pr_(void *) pr(int));
DEFPUBLIC(int) OgAutFreq(pr_(void *) pr(oindex));
DEFPUBLIC(int) OgAum(pr(void *));
DEFPUBLIC(int) OgAumMinimizeMemory(pr(void *));
DEFPUBLIC(int) OgAutScanf(pr_(void *) pr_(int) pr_(unsigned char *) pr_(int *) pr_(unsigned char *)
                          pr_(int *) pr_(int *) pr(oindex *));
DEFPUBLIC(int) OgAutScann(pr_(void *) pr_(int *) pr_(unsigned char *) pr_(int)
                          pr_(int *) pr(oindex *));
DEFPUBLIC(int) OgAutIsFinal(pr_(void *) pr(oindex));

DEFPUBLIC(int) OgAuf(pr_(void *) pr(int));
DEFPUBLIC(int) OgAufClean(void * handle);
DEFPUBLIC(int) OgAufWrite(pr_(void *) pr(char *));
DEFPUBLIC(int) OgAufRead(pr_(void *) pr(char *));
DEFPUBLIC(int) OgAufTry(pr_(void *) pr_(int) pr(unsigned char *));
DEFPUBLIC(int) OgAufLet(pr_(void *) pr_(int) pr_(unsigned char *) pr_(int *) pr(unsigned char *));
DEFPUBLIC(int) OgAutReset(pr(void *));
DEFPUBLIC(int) OgAufScanf(pr_(void *) pr_(int) pr_(unsigned char *) pr_(int *) pr_(unsigned char *)
                          pr_(int *) pr_(int *) pr(oindex *));
DEFPUBLIC(int) OgAufScann(pr_(void *) pr_(int *) pr_(unsigned char *) pr_(int)
                          pr_(int *) pr(oindex *));
DEFPUBLIC(int) OgAufBufferSize(pr(void *));
DEFPUBLIC(int) OgAufBufferRead(pr_(void *) pr_(int) pr(char *));
DEFPUBLIC(int) OgAufBufferWrite(pr_(void *) pr_(int) pr(char *));

DEFPUBLIC(int) OgAutStat(pr_(void *) pr(struct aut_stat *));
DEFPUBLIC(int) OgAufStat(pr_(void *) pr(struct aut_stat *));

DEFPUBLIC(int) OgAutMem(pr_(void *) pr_(int) pr_(int) pr(ogint64_t *));
DEFPUBLIC(int) OgAutMemory(void *handle, struct aut_memory *memory);
DEFPUBLIC(int) OgAutSize(void *handle);
DEFPUBLIC(int) OgAutUsed(void *handle);
DEFPUBLIC(int) OgAutNumber(void *handle);
DEFPUBLIC(og_string) OgAutName(void *handle);
/** Free the 'aut' part of the automaton, leaving the 'auf' part operational **/
DEFPUBLIC(int) OgAutFree(pr(void *));
/** Free the 'auf' part of the automaton, leaving the 'aut' part operational **/
DEFPUBLIC(int) OgAufFree(pr(void *));
DEFPUBLIC(int) OgAutResize(void *handle, int max_state_number);

DEFPUBLIC(int) OgAutEndian(pr(void *));

DEFPUBLIC(int) OgAutAddExt(pr_(void *) pr_(int) pr(int *));
DEFPUBLIC(int) OgAutScanfExt(pr_(void *) pr_(int) pr_(int *) pr_(int *) pr_(int *) pr_(int *) pr_(int *) pr(oindex *));
DEFPUBLIC(int) OgAutScannExt(pr_(void *) pr_(int *) pr_(int *) pr_(int) pr_(int *) pr(oindex *));

DEFPUBLIC(int) OgAutAddRup(pr_(void *) pr_(int) pr_(int) pr(unsigned char *));
DEFPUBLIC(int) OgAutDelRup(void *handle, int iline, int rupture, unsigned char *line);
DEFPUBLIC(int) OgAutScanfRup(pr_(void *) pr_(int) pr_(int) pr_(unsigned char *) pr_(int *) pr_(int *)  pr_(unsigned char *) pr_(int *) pr_(int *) pr(oindex *));
DEFPUBLIC(int) OgAutScannRup(pr_(void *) pr_(int *) pr_(int *)  pr_(unsigned char *) pr_(int) pr_(int *) pr(oindex *));

DEFPUBLIC(int) OgAul(pr_(void *) pr_(char *) pr_(int) pr(int));
DEFPUBLIC(int) OgAulScan(pr_(void *) pr_(int) pr_(unsigned char *)
                         pr_(struct att_value *) pr(int));
DEFPUBLIC(int) OgAulAdd(pr_(void *) pr(unsigned char *));
DEFPUBLIC(int) OgAulAddScan(pr_(void *) pr_(unsigned char *)
                            pr_(struct att_value *) pr(int));

DEFPUBLIC(int) OgAufToAut(void *);
DEFPUBLIC(int) OgAutGetType(void *,int *);

DEFPUBLIC(int) OgAutCopy(void *hfrom,void *hto);


#define _LOGAUTALIVE_
#endif
