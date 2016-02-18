/*
 *  Internal header for linguistic transformation search
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include <logltrac.h>
#include <logsidx.h>
#include <loglip.h>
#include <logaut.h>
#include <logpho.h>



/* Value for separator in automatons ha_base ha_swap and ha_phon
 * separator is a non valid unicode char, we cannot use extended
 * automaton, as we need to create fast automatons (.auf)
 * because they are 4 times smaller than normal automations (.aut)
*/
#define DOgLtracExtStringSeparator  1


struct ltrac_dic_input {
  int value_length; unsigned char *value;
  int attribute_number;
  int language_code,frequency;
  };



struct og_ctrl_ltrac {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  struct og_ltrac_input *input;

  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  char data_directory[DPcPathSize];

  int is_attrnum_positive,nb_attrnum;
  void *ha_attrnum,*ha_attrstr;

  char name_base[DPcPathSize];
  char name_swap[DPcPathSize];
  char name_phon[DPcPathSize];
  char name_aspell[DPcPathSize];
  char log_base[DPcPathSize];
  char log_swap[DPcPathSize];
  char log_phon[DPcPathSize];
  void *ha_base,*ha_swap,*ha_phon,*ha_filter,*ha_seen;
  FILE *fd_aspell;
  void *hpho;

  /** Used to scan the attributes **/
  DOgHinstance hlibogsidx; void *hsidx; int authorized;
  void * (*OgSidxInit)(pr_(struct og_sidx_param *) pr_(void **) pr(int *));
  char * (*OgSidxAuthorizationMessage)(void *);
  int (*OgSidxScan)(pr_(void *) pr(struct og_sidx_scan_input *));
  int (*OgSidxAttributeNumberToString)(void *handle, int attribute_number, char *attribute_string);
  int (*OgSidxAttributeStringToNumber)(void *handle, char *attribute_string, int *pattribute_number);
  int (*OgSidxScanAtvamat)(void *handle, int (*scan)(void *context,struct og_attval *attval, int frequency), void *context);
  int (*OgSidxLtracScan)(void *handle,int (*func)(void *context, struct og_sidx_ltrac_scan *scan), void *context);
  int (*OgSidxFeedInit)(pr_(void *) pr(struct og_sidx_handle *));
  int (*OgSidxItemIsStopped)(pr_(void *) pr(int));
  int (*OgSidxHasLtraDirectory)(pr(unsigned char *));
  int (*OgSidxFlush)(pr(void *));

  };


/** ltraca.c **/
int LtracAttributesPlugInit(struct og_ctrl_ltrac *ctrl_ltrac);
int LtracAttributes(struct og_ctrl_ltrac *ctrl_ltrac);

/** ltracdic.c **/
int LtracDicAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);
int LtracDicInit(struct og_ctrl_ltrac *ctrl_ltrac);
int LtracDicWrite(struct og_ctrl_ltrac *ctrl_ltrac);
int LtracDicFlush(struct og_ctrl_ltrac *ctrl_ltrac);
int LtracDicAddFilterWords(struct og_ctrl_ltrac *ctrl_ltra);

/** ltracdica.c **/
int LtracDicAspellAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdicb.c **/
int LtracDicBaseAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdics.c **/
int LtracDicSwapAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);

/** ltracdicp.c **/
int LtracDicPhonAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);



