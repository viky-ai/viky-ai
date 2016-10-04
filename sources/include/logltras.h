/*
 *  Header for library ogm_ltras.dll
 *  Copyright (c) 2009-2014 Pertimm by P.Constant
 *  Dev : November 2009, January 2010, Juiy 2014
 *  Version 1.1
*/
#ifndef _LOGLTRASALIVE_
#include <loggen.h>
#include <logthr.h>
#include <logxml.h>
#include <loguni.h>
#include <logmsg.h>
#include <logstm.h>
#include <logheap.h>


#define DOgLtrasBanner  "ogm_ltras V1.00, Copyright (c) 2009-2010 Pertimm, Inc."
#define DOgLtrasVersion 100

/** Trace levels **/
#define DOgLtrasTraceMinimal              0x1
#define DOgLtrasTraceMemory               0x2
#define DOgLtrasTraceInformation          0x4
#define DOgLtrasTraceModuleConf           0x8
#define DOgLtrasTraceModuleFlowChart      0x10
#define DOgLtrasTraceModuleCalls          0x20
#define DOgLtrasTraceSelection            0x40
#define DOgLtrasTraceModuleCut            0x100
#define DOgLtrasTraceModuleDel            0x200
#define DOgLtrasTraceModulePaste          0x400
#define DOgLtrasTraceModulePhon           0x800
#define DOgLtrasTraceModuleSwap           0x1000
#define DOgLtrasTraceModuleTerm           0x2000
#define DOgLtrasTraceModuleExc            0x4000
#define DOgLtrasTraceModuleLem            0x8000
#define DOgLtrasTraceModuleTra            0x10000
#define DOgLtrasTraceModuleRef            0x20000

#define DOgLtrasModuleConfiguration   "conf/ltras_conf.xml"
#define DOgLtrasOutputFile            "log/ogltras.xml"

/** Used to copy a double levenshtein distance into a int **/
#define DOgLtrasLevenshteinDistancePrecision   100000000


struct og_ltra_trf {
  struct og_ltra_trfs *trfs;
  int basic,start,length;
  int nb_words,start_word;
  int nb_modules,start_module;
  /** used only in the term module */
  int span_start_trf,span_nb_trfs,history_trf;
  int global_frequency;
  double global_score;
  int final,total;
  double final_score;
  int language;
  /** used only in the lem1 module */
  int compound_prefix_length;
  /** used only in the ref module */
  int marker;

  /** transposition start and length in the heap htransposition of all trf. It is used after a paste tranformation
   * to keep the corresponding start and length of each word of the original string inside the created pasted string*/
  int start_transposition, length_transposition;
  };

struct og_ltra_word {
  int start_position,length_position;
  int base_frequency,frequency;
  int start,length;
  int language;
  };

struct og_ltra_module{
    int module_id;
};

struct og_ltra_trfs {
  int length_text, start_text;

  int TrfNumber;
  int TrfUsed,TrfBasicUsed;
  struct og_ltra_trf *Trf;

  int WordNumber;
  int WordUsed;
  struct og_ltra_word *Word;

  int ModuleNumber;
  int ModuleUsed;
  struct og_ltra_module *Module;

  int BaSize,BaUsed;
  unsigned char *Ba;

  int originSize,originUsed;
  unsigned char *origin;

  /** Heap to store transpositions (int) from request words to pasted words. When we make a paste, we lose information
   * of start_position and lenght_position in the original string thus we store a transposition tab*/
  og_heap htransposition;
  };


#define  DOgLtrasAddTrfMaxNbWords  50

/** Informations about the transformed word*/
struct og_ltra_add_trf_word {
  int string_length; unsigned char *string;
  /**start of the word in the original string (before transformation)*/
  int start_position;
  /**length_position size of the word in the original string (before transformation)*/
  int length_position;
  int base_frequency,frequency;
  int language;
  };

/** Informations to add to a new transformation*/
struct og_ltra_add_trf_input {
  int basic,start,length;
  int nb_words; struct og_ltra_add_trf_word word[DOgLtrasAddTrfMaxNbWords];
  int module_id; double score;
  int from_trf,final,total;
  int language;

  /** transposition start and lenght in the heap htransposition of all trf*/
  int start_transposition, length_transposition;
  };

/** External parameters to initialize ltras library*/
struct og_ltras_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  char dictionaries_directory[DPcPathSize];
  char caller_label[DPcPathSize];
  char output_file[DPcPathSize];
  void *hltras_to_inherit;
  };

/** Input of an ltras module (for instance input of module del)*/
struct og_ltras_input {
  int request_length; unsigned char *request;
  int language_code, must_calculate_language;
  unsigned char *flow_chart;
  double frequency_ratio;
  double score_factor;

  /** activate logs of start_position and length_position for each word corrected */
  og_bool log_pos;
  };


/** Parameters to initialize a module*/
struct og_ltra_module_param {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  char configuration_file[DPcPathSize];
  void *hmodule_to_inherit;
  };

/** Input parameters of a module. It is a callback context since the module is called as a callback*/
struct og_ltra_module_input {
  void *handle; int id;
  int argc; char **argv;
  };


/**
 * Initialize ltras library
 *
 * @param og_ltras_param parameters structure for ltras
 * @return handle for ltras.
 */
DEFPUBLIC(void *) OgLtrasInit(pr(struct og_ltras_param *));

/**
 * Clean ltras library
 *
 * @param handle for ltras
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasFlush(pr(void *));

/**
 * Linguistic Transformation function
 *
 * @param hltras handle for ltras
 * @param input input structure for ltras (request, frequency_ratio, score_factor...)
 * @param ptrfs list of all transformations
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtras(void *hltras, struct og_ltras_input *input, struct og_ltra_trfs **ptrfs);

/**
 * Parses the unicode text contained in input->string and creates a list of trfs in output.
 *
 * @param hltras handle for ltras
 * @param input input structure for ltras (request, frequency_ratio, score_factor...)
 * @param ptrfs list of all transformations
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasParse(void *hltras, struct og_ltras_input *input, struct og_ltra_trfs **ptrfs);

/**
 * Parses the flowchart (fccut-(add-del/phon)-term for instance) and create a boolean tree
 * here : cut AND ((add AND del) OR phon) AND term
 *
 * @param hltras handle for ltras
 * @param input input structure for ltras (request, frequency_ratio, score_factor...)
 * @param ptrfs list of all transformations
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasFlowChart(void *hltras, struct og_ltras_input *input, struct og_ltra_trfs **output);

/**
 * Initialize handle of base automaton
 *
 * @param hltras handle for ltras
 * @return handle of base automaton.
 */
DEFPUBLIC(void *) OgLtrasHaBase(void *hltras);

/**
 * Initialize handle of swap automaton
 *
 * @param hltras handle for ltras
 * @return handle of swap automaton.
 */
DEFPUBLIC(void *) OgLtrasHaSwap(void *hltras);

/**
 * Initialize handle of phon automaton
 *
 * @param hltras handle for ltras
 * @return handle of phon automaton.
 */
DEFPUBLIC(void *) OgLtrasHaPhon(void *hltras);

/**
 * Get ltras automaton of false transformations rules contained in ltras_false.xml
 *
 * @param hltras handle for ltras
 * @return handle of false transformations automaton.
 */
DEFPUBLIC(void *) OgLtrasHaFalse(void *hltras);

/**
 * Get phonetics dictionary created according to phonet_ltra_conf.xml
 *
 * @param hltras handle for ltras
 * @return handle of false phonetics dictionary.
 */
DEFPUBLIC(void *) OgLtrasHpho(void *hltras);

/**
 * Get handle of language dictionaries (defined in ldi_conf.xml)
 *
 * @param hltras handle for ltras
 * @return handle of language dictionaries.
 */
DEFPUBLIC(void *) OgLtrasHldi(void *hltras);

/**
 * Get handle of Levenshtein library
 *
 * @param hltras handle for ltras
 * @return handle of Levenshtein library.
 */
DEFPUBLIC(void *) OgLtrasHstm(void *hltras);


/**
 * Get score factor
 *
 * @param hltras handle for ltras
 * @return score factor.
 */
DEFPUBLIC(double) OgLtrasScoreFactor(void *hltras);


/**
 * Get frequency ratio
 *
 * @param hltras handle for ltras
 * @return frequency ratio.
 */
DEFPUBLIC(double) OgLtrasFrequencyRatio(void *handle);

/**
 * Get information if position log is activated. If yes, start_position and length_position will be printed in result
 * for each corrected word
 *
 * @param hltras handle for ltras
 * @return position log boolean.
 */
DEFPUBLIC(og_bool) OgLtrasScoreFactorIsLogPosActivated(void *handle);

/**
 * Get the maximum frequency of all the words contained base_automaton.
 * If base automaton doesn't exist, default value is set to 10.
 *
 * @param hltras handle for ltras
 * @return maximum frequency.
 */
DEFPUBLIC(int) OgLtrasMaxWordFrequency(void *hltras);

/**
 * Get current working directory
 *
 * @param hltras handle for ltras
 * @return working directory.
 */
DEFPUBLIC(char *) OgLtrasWorkingDirectory(void *hltras);

/**
 * Get frequency from normalized frequency with log10
 *
 * @param hltras handle for ltras
 * @param normalized_frequency normalized frequency with log10
 * @param pfrequency reverse of the normalized frequency
 * @return working directory.
 */
DEFPUBLIC(og_status) OgLtrasGetFrequencyFromNormalizedFrequency(void *hltras, double normalized_frequency, double *pfrequency);

/**
 * Get the maximum number of transformations allowed and the maximum transformation length.
 * The maximum number of transformations allowed depends on the number of words inside the string to transform.
 * default values for maximum number of transformations are:
 * for 1 word : 100 transformations,
 * for 2 words : 8 transformations,
 * for 3 words : 5 transformations,
 * for 4 words : 4 transformations,
 * for 5 words : 3 transformations,
 * for 6 words : 3 transformations,
 * for 7 to 9 words : 2 transformations,
 * for 10 words : 1 transformations.
 * default value for maximum transformation length is 1.
 *
 * @param hltras handle for ltras
 * @param pmaximum_transformation_length normalized frequency with log10
 * @param pmaximum_transformation maximum number of transformations allowed
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasGetMaximumTransformation(void *hltras,int *pmaximum_transformation_length,int **pmaximum_transformation);

/**
 * Create a new transformations list
 *
 * @param hltras handle for ltras
 * @param ptrfs pointer on the new transformations list
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsCreate(void *hltras,struct og_ltra_trfs **ptrfs);

/**
 * Destroy a transformations list
 *
 * @param hltras handle for ltras
 * @param ptrfs pointer on the transformations list to destroy
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsDestroy(void *hltras,struct og_ltra_trfs *trfs);

/**
 * Duplicate a transformations list
 *
 * @param hltras handle for ltras
 * @param input transformations list to duplicate
 * @param output duplicated transformations list
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsDuplicate(void *hltras,struct og_ltra_trfs *input,struct og_ltra_trfs **output);

/**
 * Add original text to a transformations list
 *
 * @param hltras handle handle for ltras
 * @param trfs transformations list
 * @param length_text the length of the text
 * @param text the text to add to the transformations list
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsAddText(void *hltras, struct og_ltra_trfs *trfs, int length_text, unsigned char *text);

/**
 * Merges two tranformations list. If two transformations are the same in both lists,
 * the transformation with the highest score is kept.
 * otherwise the transformation is copied from the first list to the end of the second one.
 *
 * @param hltras handle for ltras
 * @param trfs transformations list
 * @param mother_trfs the merged transformations list
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsMerge(void *hltras, struct og_ltra_trfs *trfs, struct og_ltra_trfs *mother_trfs);


/**
 * Add the result information on the transformations list after extracting the valid transformations inside the list.
 * the transformation with the highest score is kept.
 * otherwise the transformation is copied from the first list to the end of the second one.
 *
 * @param hltras handle for ltras
 * @param trfs transformations list
 * @param send_result indicates if must print the result
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsAddResult(void *hltras, struct og_ltra_trfs *trfs, int send_result);

/**
 * Calculate common information on results, that is to say mean frequency, mean final_score...
 *
 * @param hltras handle for ltras
 * @param send_result indicates if must print the result inside ogltras.xml
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsConsolidateResults(void *hltras, int send_result);

/**
 * Log a transformations list
 *
 * @param hltras handle for ltras
 * @param trfs transformations list to log
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfsLog(void *hltras, struct og_ltra_trfs *trfs);

/**
 * Add a new transformation
 *
 * @param hltras handle for ltras
 * @param trfs transformations structure in which the new transformation is added
 * @param input all informations about the new transformation
 * @param pIntrf the position of the new transformation in trfs
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfAdd(void *hltras, struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input, int *pIntrf);

/**
 * Copy a transformation at a certain position from a trfs at the end of another trfs
 *
 * @param hltras handle for ltras
 * @param trfs1 transformations structure in which the transformation at the position Itrf1 is copied
 * @param Itrf1 position of the transformation to copy
 * @param trfs2 transformation structure in witch the transformation copied is added
 * @param copy_scores indicates if it is necessary to copy score and module informations. If false, score and module are set to 0
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfCopy(void *hltras, struct og_ltra_trfs *trfs1, int Itrf1, struct og_ltra_trfs *trfs2, int copy_scores);

/**
 * Compute the global score, global frequency and final score of a transformation
 *
 * @param hltras handle for ltras
 * @param trfs the transformations structure where to extract the desired transformation
 * @param Itrf position of the transformation to compute scores and frequency
 * @param pglobal_frequency global frequency of the transformation :
 * if there is more than one word in the transformation, global frequency is the frequency of the least frequent word.
 * @param pglobal_score levenshtein score of the transformation
 * @param pfinal_score calculated according to global score, global frequency normalized and score factor
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfCalculateGlobal(void *hltras, struct og_ltra_trfs *trfs, int Itrf
  , int *pglobal_frequency, double *pglobal_score, double *pfinal_score);

/**
 * Get frequency of a word inside base automaton ha_base
 *
 * @param hltras handle for ltras
 * @param string_length size of the word
 * @param string the word to get the frequency
 * @param pfrequency frequency of the word. If the word appears more than once in the automaton,
 * the frequencies are added.
 * @return function status : TRUE, else FALSE if word is not found, else ERREUR
 */
DEFPUBLIC(og_bool) OgLtrasTrfCalculateFrequency(void *hltras, int string_length, unsigned char *string, int *pfrequency);

/**
 * Get the start position and end position in the original string of the tranformation
 *
 * @param trfs the transformations structure
 * @param Itrf position of the transformation
 * @param ptrf_start_position pointer to the start position on the original string
 * @param ptrf_end_position pointer to the end position on the original string
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(int) OgLtrasTrfGetBoundaries(struct og_ltra_trfs *trfs, int Itrf, int *ptrf_start_position,
    int *ptrf_end_position);

/**
 * Get the original string before any transformation
 *
 * @param trfs the transformations structure
 * @param Itrf position of the actual transformation
 * @param origin original string to get
 * @param origin_length length of the original string.
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(int) OgLtrasTrfGetOriginalString(void *handle, struct og_ltra_trfs *trfs, int Itrf, unsigned char *origin, int origine_size,
    int *porigin_length);

/**
 * Initialize a tinput to the right words to prepare transformations
 *
 * @param trfs the transformations structure
 * @param Itrf position of the actual transformation
 * @param tinput the tinput to fill
 * @param module_id the id of the module calling the function
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(int) OgLtrasTrfInitTinput(struct og_ltra_trfs *trfs, int Itrf, struct og_ltra_add_trf_input *tinput, int module_id, unsigned char *words);
/**
 * Log a transformation
 *
 * @param hltras handle for ltras
 * @param trfs transormations list where the transformation is contained
 * @param Itrf index of the transformation inside the transformations list trfs
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasTrfLog(void *hltras, struct og_ltra_trfs *trfs, int Itrf);

/**
 * Log informations of a new transformation input
 *
 * @param hltras handle for ltras
 * @param trfs transformations list
 * @param input new transformation input
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasInputTrfLog(void *hltras, struct og_ltra_trfs *trfs, struct og_ltra_add_trf_input *input);

/**
 * Get the string of the input information of a new transformation
 *
 * @param hltras handle for ltras
 * @param input input information of a new transformation
 * @param string_size maximum size of the string to return
 * @param string the input information string
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasInputTrfString(void *hltras, struct og_ltra_add_trf_input *input, int string_size, unsigned char *string);

/**
 * Get the default levenshtein cost for ltras library
 *
 * @param hltras handle for ltras
 * @param levenshtein_costs to initialize
 * @return function status : CORRECT else ERREUR.
 */
DEFPUBLIC(og_status) OgLtrasGetLevenshteinCosts(void *hltras, struct og_stm_levenshtein_input_param *levenshtein_costs);

/**
 * Get module name from module id
 *
 * @param hltras handle for ltras
 * @param module_id module id
 * @return module name.
 */
DEFPUBLIC(char *) OgLtrasModuleName(void *hltras,int module_id);

/**
 * Get module id from module name
 *
 * @param hltras handle for ltras
 * @param module_name module name
 * @return module id.
 */
DEFPUBLIC(int) OgLtrasModuleId(void *hltras,char *module_name,int *pmodule_id);

/**
 * Read an XML configuration file, whose format is as follows :
 *   <?xml version="1.0" encoding="UTF-8" ?>
 *   <ltras>
 *   <parameters>
 *     <parameter name="toto">value</parameter>
 *   </parameters>
 *   <minimum_scores>
 *     <minimum_score length="3" score="0.90" />
 *     <minimum_score length="4" score="0.85" />
 *   </minimum_scores>
 *   <minimum_final_scores>
 *     <minimum_final_score length="3" score="0.90" />
 *     <minimum_final_score length="4" score="0.85" />
 *   </minimum_final_scores>
 *   <maximum_transformations>
 *     <maximum_transformation nb_request_words="1" number="100"/>
 *     <maximum_transformation nb_request_words="2" number="8"/>
 *   </maximum_transformations>
 *   <ltras_modules>
 *    <ltras_module>
 *     <name>xxxx</name>
 *     <function_name>OgLtrasModuleXxxx</function_name>
 *     <library_name>my_library</library_name>
 *    </ltras_module>
 *   </ltras_modules>
 *   </ltras>
 *
 * Encoding is supposed to be UTF-8.
 *
 * @param hltras handle for ltras
 * @param filename configuration file path
 * @return function status : CORRECT else ERREUR
 */
DEFPUBLIC(og_status) OgLtrasModuleReadConf(void *hltras, char *filename);


/**
 * Reset suggestion automaton and suggestions counter
 *
 * @param hltras handle for ltras
 * @return function status : CORRECT else ERREUR
 */
DEFPUBLIC(og_status) OgLtrasSuggestionInit(void *hltras);

/**
 * Add a new spelling suggestion in the suggestion automaton and in the suggestion counter
 *
 * @param handle handle for ltras
 * @param is size of the suggestion string
 * @param s the suggestion string
 * @return function status : CORRECT else ERREUR
 */
DEFPUBLIC(og_status) OgLtrasSuggestionAdd(void *handle, int is, unsigned char *s);

/**
 * Reset operations automaton. (an operation corresponds to a linguistic transformation to compute)
 *
 * @param handle handle for ltras
 * @return function status : CORRECT else ERREUR
 */
DEFPUBLIC(og_status) OgLtrasOperationInit(void *handle);

/**
 * Add an operation inside operations automaton provided that it doesn't already exist inside the automaton
 *
 * @param handle handle for ltras
 * @param is operation string size
 * @param s operation string
 * @return function status : CORRECT else ERREUR
 */
DEFPUBLIC(og_status) OgLtrasOperationGet(void *handle, int is, unsigned char *s);

/**
 * Test if the transformation is a false tranformation recorded in ltra_false.xml
 *
 * @param handle handle for ltras
 * @param from_length size of the original string
 * @param from the original string
 * @param to_length size of the transformed string
 * @param to the transformed string
 * @return function status : TRUE, else FALSE if word is not found, else ERREUR
 */
DEFPUBLIC(og_bool) OgLtrasIsFalseTransformation(void *handle, int from_length, unsigned char *from, int to_length, unsigned char *to);

/**
 * Get the value of the parameter specified
 *
 * @param handle handle for ltras
 * @param name name of the parameter
 * @param value_size size of the parameter value
 * @param value value of the parameter
 * @return function status : TRUE, else FALSE if word is not found, else ERREUR
 */
DEFPUBLIC(og_bool) OgLtrasGetParameterValue(void *handle, unsigned char *name, int value_size, unsigned char *value);

/**
 * Indicates memory consumption of the ltras library
 *
 * @param handle handle for ltras
 * @param must_log if must log the information
 * @param module_level module level
 * @param pmem memory consumption
 * @return function status : TRUE, else FALSE if word is not found, else ERREUR
 */
DEFPUBLIC(int) OgLtrasMem(void *hltras, int must_log, int module_level, ogint64_t *pmem);

/** add module **/
DEFPUBLIC(void *) OgLtrasModuleAddInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleAddFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleAdd(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** cut module **/
DEFPUBLIC(void *) OgLtrasModuleCutInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleCutFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleCut(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** split module **/
DEFPUBLIC(void *) OgLtrasModuleSplitInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleSplitFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleSplit(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);


/** del module **/
DEFPUBLIC(void *) OgLtrasModuleDeleteInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleDeleteFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleDelete(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** except module **/
DEFPUBLIC(void *) OgLtrasModuleExceptInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleExceptFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleExcept(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** lem1 module **/
DEFPUBLIC(void *) OgLtrasModuleLem1Init(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleLem1Flush(void *handle);
DEFPUBLIC(int) OgLtrasModuleLem1(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** lem2 module **/
DEFPUBLIC(void *) OgLtrasModuleLem2Init(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleLem2Flush(void *handle);
DEFPUBLIC(int) OgLtrasModuleLem2(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** tra module **/
DEFPUBLIC(void *) OgLtrasModuleTraInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleTraFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleTra(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** paste module **/
DEFPUBLIC(void *) OgLtrasModulePasteInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModulePasteFlush(void *handle);
DEFPUBLIC(int) OgLtrasModulePaste(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** phon module **/
DEFPUBLIC(void *) OgLtrasModulePhonInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModulePhonFlush(void *handle);
DEFPUBLIC(int) OgLtrasModulePhon(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** swap module **/
DEFPUBLIC(void *) OgLtrasModuleSwapInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleSwapFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleSwap(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** ref module **/
DEFPUBLIC(void *) OgLtrasModuleReferenceInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleReferenceFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleReference(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** test module **/
DEFPUBLIC(void *) OgLtrasModuleTestInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleTestFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleTest(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);

/** term module **/
DEFPUBLIC(void *) OgLtrasModuleTermInit(struct og_ltra_module_param *param);
DEFPUBLIC(int) OgLtrasModuleTermFlush(void *handle);
DEFPUBLIC(int) OgLtrasModuleTerm(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,  struct og_ltra_trfs **output, ogint64_t *elapsed);


#define _LOGLTRASALIVE_
#endif


