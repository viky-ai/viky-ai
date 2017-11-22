/*
 *  Header for library ogm_stm.dll
 *  Copyright (c) 2009 Pertimm by Lo�s Rigouste
 *  Dev : January 2009
 *  Version 1.0
 */
#ifndef _LOGSTMALIVE_
#include <lpcgentype.h>
#include <loggen.h>
#include <logthr.h>
#include <logmsg.h>
#include <loguni.h>

#define DOgStmBanner  "ogm_stm V1.0, Copyright (c) 2009 Pertimm, Inc."
#define DOgStmVersion 100

/** Trace levels **/
#define DOgStmTraceMinimal          0x1
#define DOgStmTraceMemory           0x2
#define DOgStmTraceScore            0x4
#define DOgStmTracePath             0x8

#define DOgStmMaxWordLength         80

#define DOgStmSpaceCostLength     8

struct og_stm_param
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
};

typedef enum
{
  same, insertion, deletion, substitution, swap
} LEV_OPERATION;

struct og_stm_levenshtein_input_param
{
  double insertion_cost; /**< cht => chat */
  double same_letter_insertion_cost; /**< cht => chtt */
  double deletion_cost; /**< chait => chat */
  double same_letter_deletion_cost; /**< chaat => chat */
  double substitution_cost; /**< chot => chat */
  double accents_substitution_cost; /**< chât => chat */
  double swap_cost; /**< caht => chat */
  double case_cost; /**< Chat => chat */
  double punctuation_cost; /**< chat, => chat */
};

struct og_stm_levenshtein_output_param
{
  double score[DOgStmMaxWordLength + 9][DOgStmMaxWordLength + 9];
  int length1, length2; /**< score is of size length1+1 x length2+1 (last cell is [length1][length2] */
  LEV_OPERATION levenshtein_path[DOgStmMaxWordLength + 9];
  int path_length;
};

DEFPUBLIC(void *) OgStmInit(struct og_stm_param *param);
DEFPUBLIC(int) OgStmFlush(void *hstm);

/**
 * Init all Levenstein costs
 *
 * @param hstm handle for stm
 * @param lev_param Levenstein cost structure to initialize
 * @return DONE
 */
DEFPUBLIC(og_status) OgStmInitDefaultCosts(void *hstm, struct og_stm_levenshtein_input_param *lev_param);

/**
 * Init the space_insertion_costs and space_deletion_costs
 *
 * @param hstm handle for stm
 * @param is_insertion a boolean for insertion or deletion. If true insertion, otherwise, deletion.
 * @return DONE
 */
DEFPUBLIC(og_status) StmInitDefaultSpaceCost(void *hstm, og_bool is_insertion);

/**
 * add a space_insertion_cost or a space_deletion_cost
 *
 * @param hstm handle for stm
 * @param occurence the occurence for the space cost
 * @param cost the cost
 * @param is_insertion a boolean for insertion or deletion. If true insertion, otherwise, deletion.
 * @return DONE
 */
DEFPUBLIC(og_status) OgStmAddSpaceCost(void *hstm, int occurence, double cost, og_bool is_insertion);


/**
 * get a space_insertion_cost or a space_deletion_cost
 *
 * @param hstm handle for stm
 * @param occurence the occurence for the space cost
 * @param is_insertion a boolean for insertion or deletion. If true insertion, otherwise, deletion.
 * @param space_cost the space_insertion_cost or a space_deletion_cost returned
 * @return DONE or ERROR
 */
DEFPUBLIC(og_status) OgStmGetSpaceCost(void *hstm, int occurence, og_bool is_insertion, double *space_cost);

/**
 * Create and save an entry for two equivalent letters with their cost
 * if the entry already exist, we store it and replace the previous entry
 * only if the cost is lower
 * if order of letter1 and letter2 are inverted, it is considered as a new entry.
 *
 * @param hstm handle for stm
 * @param letter1 first equivalent letter
 * @param letter2 second equivalent letter
 * @param cost the cost of the entry
 * @return DONE
 */
DEFPUBLIC(og_status) OgStmInitCreateEquivalentLetterEntry(void *hstm, int letter1, int letter2, double cost);

/**
 * Default Levenstein version
 *
 * @param hstm handle for stm
 * @param ustring1 size of string1
 * @param string1 unicode string to compare with string2
 * @param ustring2 size of string2
 * @param string1 unicode string to compare with string1
 * @param lev_input_params Levenstein input params
 * @param lev_output_params Levenstein output params
 * @return Levenstein distance
 */
DEFPUBLIC(double) OgStmLevenshtein(void *hstm, int ustring1, unsigned char *string1, int ustring2,
    unsigned char *string2, struct og_stm_levenshtein_input_param *lev_input_params,
    struct og_stm_levenshtein_output_param *lev_output_params);

/**
 * Fast Levenstein version : do not compute the path
 *
 * @param hstm handle for stm
 * @param ustring1 size of string1
 * @param string1 unicode string to compare with string2
 * @param ustring2 size of string2
 * @param string1 unicode string to compare with string1
 * @param lev_input_params Levenstein input params
 * @return Levenstein distance
 */
DEFPUBLIC(double) OgStmLevenshteinFast(void *hstm, int ustring1, unsigned char *string1, int ustring2,
    unsigned char *string2, struct og_stm_levenshtein_input_param *lev_params);

/**
 * Levenstein version with default cost values
 *
 * @param hstm handle for stm
 * @param ustring1 size of string1
 * @param string1 unicode string to compare with string2
 * @param ustring2 size of string2
 * @param string1 unicode string to compare with string1
 * @return Levenstein distance
 */
DEFPUBLIC(double) OgStmLevenshteinFastDefaultParams(void *hstm, int ustring1, unsigned char *string1, int ustring2,
    unsigned char *string2);

/**
 * Similar to OgStmLevenshteinFast with an additional parameter specifying max distance allowed:
 * computes exactly only distances less or equal to the last parameter ; when, during computation,
 * it is evaluated that the distance is necessarily greater than this number, number+1 is returned
 *
 * @param hstm handle for stm
 * @param ustring1 size of string1
 * @param string1 unicode string to compare with string2
 * @param ustring2 size of string2
 * @param string1 unicode string to compare with string1
 * @param lev_input_params Levenstein input params
 * @return Levenstein distance
 */
DEFPUBLIC(double) OgStmLevenshteinFastWithMax(void *hstm, int ustring1, unsigned char *string1, int ustring2,
    unsigned char *string2, struct og_stm_levenshtein_input_param *lev_params, double *max);

#define _LOGSTMALIVE_
#endif

