/*
 *  Internal header for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev: September 2017
 *  Version 1.0
 */
#include <lognlp.h>
#include <logaut.h>
#include <loguni.h>
#include <logheap.h>
#include <logsysi.h>
#include <logis639_3166.h>
#include <glib-2.0/glib.h>
#include <duktape.h>
#include <math.h>

#define DOgNlpPackageNumber 0x10
#define DOgNlpPackageBaNumber 0x100
#define DOgNlpPackageInterpretationNumber 0x10
#define DOgNlpPackageExpressionNumber (DOgNlpPackageInterpretationNumber*0x10)
#define DOgNlpPackageAliasNumber DOgNlpPackageExpressionNumber
#define DOgNlpPackageInputPartNumber DOgNlpPackageInterpretationNumber

#define DOgNlpRequestWordNumber 0x10
#define DOgNlpAcceptLanguageNumber 0x8
#define DOgNlpRequestInputPartNumber 0x10
#define DOgNlpRequestExpressionNumber 0x10
#define DOgNlpRequestPositionNumber (DOgNlpRequestExpressionNumber*2+DOgNlpRequestInputPartNumber)
#define DOgNlpBaNumber 0x100

#define DOgNlpInterpretationExpressionMaxLength 0x800

#define DOgNlpMaximumOwnedLock      16

#define NlpLog(nlptrace,nlpformat, ...) if (ctrl_nlp_th->loginfo->trace & nlptrace) \
  { \
    NlpLogImplementation(ctrl_nlp_th, nlpformat, ##__VA_ARGS__);\
  }

struct ltra_dictionaries
{
  void *ha_base;
  void *ha_swap;
  void *ha_phon;
};

struct package
{
  struct og_ctrl_nlp *ctrl_nlp;

  // =======================================
  // memory management

  /**
   * Each time a package is obtain by #NlpPackageGet ref_counter is incremented by one.
   * NlpPackageMarkAsUnused function decrement counter by one and free it it as been marked as remove.
   *
   * Warn : ref_counter can be updated currently (see: https://developer.gnome.org/glib/stable/glib-Atomic-Operations.html)
   */
  int ref_counter;

  /** Package has been remove, it is waiting to bee freed, until ref_counter == 0 */
  og_bool is_removed;

  // =======================================

  og_bool consolidate_done;

  og_string id;
  og_string slug;

  /** hinterpretation_ba : String heap for interpretation, read-only after compilation step */
  og_heap hinterpretation_ba;
  og_heap hinterpretation_compile;
  og_heap hinterpretation;

  /** hexpression_ba : String heap for expression, read-only after compilation step */
  og_heap hexpression_ba;
  og_heap hexpression_compile;
  og_heap hexpression;

  /** halias_ba : String heap for alias, read-only after compilation step */
  og_heap halias_ba;
  og_heap halias_compile;
  og_heap halias;

  /** hinput_part_ba : String heap for input_part, read-only after compilation step */
  og_heap hinput_part_ba;
  og_heap hinput_part;

  og_heap hdigit_input_part;

  /** Automaton : "<string_word>\1<Iinput_part>" */
  void *ha_word;

  /** Automaton : "<interpretation_id>\1<Iinput_part>" */
  void *ha_interpretation_id;

  /** Automatons for ltrac/ltraf */
  struct ltra_dictionaries ltra_dictionaries[1];
};

typedef struct package *package_t;

enum nlp_alias_type
{
  nlp_alias_type_Nil = 0, nlp_alias_type_type_Interpretation, nlp_alias_type_Any, nlp_alias_type_Digit
};

struct alias_compile
{
  enum nlp_alias_type type;
  int alias_start, alias_length;
  int slug_start, slug_length;       // interpretation slug
  int id_start, id_length;           // interpretation id
  int package_id_start, package_id_length;
};

struct alias
{
  enum nlp_alias_type type;

  /** alias name */
  og_string alias;
  int alias_length;

  /** interpretation */
  og_string slug;
  og_string id;
  og_string package_id;
};

struct expression_compile
{
  int text_start;
  og_bool keep_order;
  og_bool glued;
  int alias_start, aliases_nb;
  int locale;
  int input_part_start, input_parts_nb;
  json_t *json_solution;
};

struct expression
{
  /** Parent */
  struct interpretation *interpretation;

  og_string text;

  og_bool keep_order;
  og_bool glued;

  int locale;

  int aliases_nb;
  struct alias *aliases;

  int input_parts_nb;

  union
  {
    /** during consolidation process */
    int input_part_start;

    /** after consolidation process */
    struct input_part *input_parts;
  };

  // if value is 0 or more, it means the expression contains an any alias
  // otherwise value is -1
  int alias_any_input_part_position;

  og_bool is_recursive;

  json_t *json_solution;
};

struct interpretation_compile
{
  package_t package;
  int id_start, id_length;
  int slug_start, slug_length;
  int expression_start, expressions_nb;
  json_t *json_solution;
};

struct interpretation
{
  /** Parent */
  package_t package;

  og_string id;
  og_string slug;
  json_t *json_solution;

  int expressions_nb;
  struct expression *expressions;

  og_bool is_recursive;
};

struct interpret_package
{
  int self_index;
  package_t package;
};

enum nlp_input_part_type
{
  nlp_input_part_type_Nil = 0, nlp_input_part_type_Word, nlp_input_part_type_Interpretation, nlp_input_part_type_Digit
};

struct input_part_word
{
  int raw_word_start;
  int word_start;
};

struct input_part
{
  /** Parent */
  struct expression *expression;

  enum nlp_input_part_type type;
  union
  {
    /** nlp_input_part_type_Word */
    struct input_part_word word[1];

    /** nlp_input_part_type_Interpretation */
    struct alias *alias;

  };

};

struct digit_input_part
{
  int Iinput_part;
};

enum nlp_synchro_test_timeout_in
{
  nlp_timeout_in_NONE = 0,
  nlp_timeout_in_NlpPackageAddOrReplace,
  nlp_timeout_in_NlpPackageGet,
  nlp_timeout_in_NlpInterpretRequestParse,
  nlp_timeout_in_NlpMatchExpressions,
  nlp_timeout_in_NlpRequestInterpretationBuild
};

enum nlp_synchro_lock_type
{
  nlp_synchro_lock_type_read, nlp_synchro_lock_type_write
};

struct nlp_synchro_lock
{
  enum nlp_synchro_lock_type type;

  ogsysi_rwlock rwlock;
};

struct nlp_synchro_current_lock
{
  int used;
  struct nlp_synchro_lock lock[DOgNlpMaximumOwnedLock];
};

struct request_word
{
  int start;
  int length;
  int raw_start;
  int raw_length;
  int start_position;
  int length_position;
  og_bool is_digit;
  int digit_value;
  double spelling_score;
};

struct accept_language
{
  int locale;
  float quality_factor;
};

struct request_input_part
{
  /** from zero (only words) to N */
  int level;

  enum nlp_input_part_type type;
  union
  {
    /** sliced heap, used when type=nlp_input_part_type_Word */
    struct request_word *request_word;

    /** used when type=nlp_input_part_type_Interpretation */
    int Irequest_expression;
  };

  /** direct link to input_part, package can be retrieved in the structure */
  struct input_part *input_part;
  struct interpret_package *interpret_package;   // used for sorting only
  int Iinput_part;   // used for sorting only

  int request_position_start;
  int request_positions_nb;
  int request_position_distance;

  int Ioriginal_request_input_part;

  og_bool interpret_word_as_digit;
};

struct request_position
{
  int start;
  int length;
};

struct original_request_input_part
{
  int Irequest_input_part;
};

struct orip
{
  int Ioriginal_request_input_part;
};

struct request_any
{
  int request_word_start;
  int request_words_nb;

  /** used to optimize the attachement any <-> request_expression */
  GQueue queue_request_expression[1];
  int consumed;
};

struct request_score
{
  double coverage;
  double locale;
  double spelling;
  double overlap;
  double any;
};

struct request_expression
{
  int self_index;

  /** from zero (only words) to N */
  int level;

  /** matched expression */
  struct expression *expression;

  int request_position_start;
  int request_positions_nb;

  // overlapping rate of the tree of the request_expression
  int overlap_mark;

  int orip_start;
  int orips_nb;

  /** all possible any for request expression */
  int request_any_start;
  int request_anys_nb;

  int Irequest_any;

  /** used locally for various scanning */
  int analyzed;

  /** Mark of an expression as not to be use anymore **/
  int deleted;

  og_bool keep_as_result;
  og_bool contains_any;

  struct request_score score[1];

  GQueue tmp_solutions[1];

  json_t *json_solution;
};

#define DOgMatchZoneInputPartSize 0x100
struct match_zone_input_part
{
  int start;
  int length;
  int current;
};

#define DOgAliasSolutionSize 0x100
struct alias_solution
{
  struct alias *alias;
  json_t *json_solution;
};

struct og_nlp_punctuation_word
{
  /** string bytes lentgh */
  int length;

  /** UTF-8 punctuation word value*/
  og_string string;
};

#define DOgNlpParsePunctCharMaxNb 16
#define DOgNlpParsePunctWordMaxNb 32
struct og_nlp_parse_conf
{
  /** Single unicode char skipped */
  gunichar punct_char[DOgNlpParsePunctCharMaxNb];
  int punct_char_used;

  /** Single unicode char treated as word */
  gunichar punct_char_word[DOgNlpParsePunctCharMaxNb];
  int punct_char_word_used;

  /** Multiple char ward in utf-8  treated as word */
  struct og_nlp_punctuation_word punct_word[DOgNlpParsePunctWordMaxNb];
  int punct_word_used;
};

/** Glue status for positions :
 * stuck: equal position (no character in between)
 * glued: no words in between
 * loose means some words in between
 **/
enum nlp_glue_status
{
  nlp_glue_status_Loose = 0, nlp_glue_status_Glued, nlp_glue_status_Stuck
};

struct og_ctrl_nlp_js
{
  duk_context *duk_context;
  duk_idx_t init_stack_idx;

  /** For better error message list current defined variable */
  GStringChunk *varibale_values;
  GQueue variable_list[1];
};

struct og_ctrl_nlp_threaded
{
  og_nlp ctrl_nlp;
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  // current nlp_th_name
  og_char_buffer name[DPcPathSize];

  /** Use for testing feature : trigger timeout in a specific function */
  enum nlp_synchro_test_timeout_in timeout_in;

  /** common request */
  json_t *json_answer;

  /** Stack of current lock (struct nlp_synchro_lock) owned by the thread */
  struct nlp_synchro_current_lock current_lock[1];

  /** Package beeing created */
  package_t package_in_progress;

  /** Loop level for the matching **/
  int level;

  /** interpret request */
  og_heap hinterpret_package;
  og_string request_sentence;
  int basic_request_word_used;
  og_heap haccept_language;
  og_bool show_explanation;
  unsigned int regular_trace;
  og_heap hrequest_word;
  og_heap hba;

  /** Heap of struct request_input_part */
  og_heap hrequest_input_part;
  og_heap horiginal_request_input_part;
  og_heap horip;

  /** Heap of struct request_expression */
  og_heap hrequest_expression;

  /** Heap of struct request_position */
  og_heap hrequest_position;

  og_heap hrequest_any;

  /**
   * List of package_t currently used by the og_ctrl_nlp_threaded
   * (reset clean this list at the end of the request), it is better
   * to mark it as used as soon as possible
   */
  GQueue package_in_used[1];
  GQueue sorted_request_expressions[1];

  int new_request_expression_start;
  int new_request_input_part_start;

  /** js intepreter */
  struct og_ctrl_nlp_js js[1];

  /** HashTable key: int (word position) , value: int (word position) */
  GHashTable *glue_hash;

  void *hltrac;
  void *hltras;

};

struct og_ctrl_nlp
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];

  /** HashTable key: string (package id) , value: package (package_t) */
  GHashTable *packages_hash;
  ogsysi_rwlock rw_lock_packages_hash;

  /** Parsing configuration */
  struct og_nlp_parse_conf parse_conf[1];

};

/* nlperr.c */
og_status NlpThrowError(og_nlp ctrl_nlp, og_string format, ...);
og_status NlpThrowErrorTh(og_nlp_th ctrl_nlp_th, og_string format, ...);

/* nlplog.c */
og_status NlpLogImplementation(og_nlp_th ctrl_nlp_th, og_string format, ...);
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated,
    size_t flags);
og_status NlpPackageLog(og_nlp_th ctrl_nlp_th, og_string label, package_t package);
og_status NlpPackageInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package, struct interpretation *interpretation);
og_status NlpPackageInterpretationSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation *interpretation);
og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);
og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias);
og_status NlpPackageInputPartLog(og_nlp_th ctrl_nlp_th, package_t package, struct input_part *input_part);
og_status NlpPackageExpressionSolutionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);

og_status NlpPackageCompileLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpPackageCompileInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation);
og_status NlpPackageCompileInterpretationSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation);
og_status NlpPackageCompileExpressionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression);
og_status NlpPackageCompileExpressionSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression);
og_status NlpPackageCompileAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias_compile *alias);

og_status NlpLogRequestWords(og_nlp_th ctrl_nlp_th);
og_status NlpLogRequestWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
const char *NlpAliasTypeString(enum nlp_alias_type type);

/* nlpsynchro.c */
og_status OgNlpSynchroUnLockAll(og_nlp_th ctrl_nlp_th);
og_status OgNlpSynchroReadLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroReadUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroWriteLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroWriteUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroTestSleepIfTimeoutNeeded(og_nlp_th ctrl_nlp_th, enum nlp_synchro_test_timeout_in timeout_in);

/* nlpackage.c */
package_t NlpPackageCreate(og_nlp_th ctrl_nlp_th, og_string string_id, og_string string_slug);
og_status NlpPackageAddOrReplace(og_nlp_th ctrl_nlp_th, package_t package);
package_t NlpPackageGet(og_nlp_th ctrl_nlp_th, og_string package_id);
og_status NlpPackageMarkAsUnused(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpPackageMarkAllInUsedAsUnused(og_nlp_th ctrl_nlp_th);
void NlpPackageDestroyIfNotUsed(gpointer package_void);

/* nlpinterpret.c */
og_status NlpInterpretInit(og_nlp_th ctrl_nlp_th, struct og_nlp_threaded_param *param);
og_status NlpInterpretReset(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretFlush(og_nlp_th ctrl_nlp_th);

/* nlpcompile.c */
og_status NlpCompilePackageInterpretation(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretation);
og_status NlpCompilePackage(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_package);

/* nlpconsolidate.c */
og_status NlpConsolidatePackage(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpipword.c */
og_status NlpInputPartWordInit(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpInputPartWordFlush(package_t package);
og_status NlpInputPartWordAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string string_word, int length_string_word,
    int Iinput_part);
og_status NlpInputPartAliasDigitAdd(og_nlp_th ctrl_nlp_th, package_t package, size_t Iinput_part);
og_status NlpInputPartWordLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpDigitInputPartLog(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpipalias.c */
og_status NlpInputPartAliasInit(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpInputPartAliasFlush(package_t package);
og_status NlpInputPartAliasAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string interpretation_id,
    size_t Iinput_part);
og_status NlpInputPartAliasLog(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpmatch.c */
og_status NlpMatch(og_nlp_th ctrl_nlp_th);

/* nlpmatch_word.c */
og_status NlpMatchWords(og_nlp_th ctrl_nlp_th);

/* nlpmatch_expression.c */
og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th);

/* nlpmatch_interpretation.c */
og_bool NlpMatchInterpretations(og_nlp_th ctrl_nlp_th);

/* nlpparse.c */
og_status NlpParseConfInit(og_nlp ctrl_nlp);
og_status NlpParseConfFlush(og_nlp ctrl_nlp);
og_status NlpParseRequestSentence(og_nlp_th ctrl_nlp_th);
og_bool NlpParseIsPunctuation(og_nlp_th ctrl_nlp_th, int max_word_size, og_string current_word, og_bool *p_skip,
    int *p_punct_length_bytes);

/* nlprip.c */
og_status NlpRequestInputPartAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    struct interpret_package *interpret_package, int Iinput_part, og_bool word_as_digit);
og_status NlpRequestInputPartAddInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct interpret_package *interpret_package, int Iinput_part);
struct request_input_part *NlpGetRequestInputPart(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Iorip);
og_bool NlpRequestInputPartsAreOrdered(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2);
og_bool NlpRequestInputPartsAreGlued(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2);
og_status NlpRequestInputPartsLog(og_nlp_th ctrl_nlp_th, int request_input_part_start, char *title);
og_status NlpRequestInputPartLog(og_nlp_th ctrl_nlp_th, int Irequest_input_part);

/* nlprexpression.c */
og_bool NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct match_zone_input_part *match_zone_input_part, struct request_expression **prequest_expression);
og_status NlpRequestExpressionsExplicit(og_nlp_th ctrl_nlp_th);
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations);
og_status NlpSortedRequestExpressionsLog(og_nlp_th ctrl_nlp_th, char *title);
og_status NlpRequestExpressionsLog(og_nlp_th ctrl_nlp_th, int request_expression_start, char *title);
og_status NlpRequestExpressionLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int offset);

/* nlprposition.c */
og_status NlpRequestPositionAdd(og_nlp_th ctrl_nlp_th, int start, int length, size_t *pIrequest_position);
og_status NlpRequestPositionSort(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb);
og_bool NlpRequestPositionSame(og_nlp_th ctrl_nlp_th, int request_position_start1, int request_positions_nb1,
    int request_position_start2, int request_positions_nb2);
og_bool NlpRequestPositionOverlap(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb);
og_status NlpRequestPositionDistance(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb);
og_bool NlpRequestPositionsAreOrdered(og_nlp_th ctrl_nlp_th, int request_position_start1, int request_positions_nb1,
    int request_position_start2, int request_positions_nb2);
og_bool NlpRequestPositionsAreGlued(og_nlp_th ctrl_nlp_th, int request_position_start1, int request_positions_nb1,
    int request_position_start2, int request_positions_nb2);
int NlpRequestPositionString(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb, int size,
    char *string);
int NlpRequestPositionStringHighlight(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb,
    int size, char *string);

/* nlporip.c */
og_status NlpOriginalRequestInputPartsCalculate(og_nlp_th ctrl_nlp_th);
og_status NlpRequestExpressionAddOrip(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Ioriginal_request_input_part);

/* nlptree.c */
og_status NlpInterpretTreeLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpany.c */
og_status NlpInterpretAnyFlush(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretAnyReset(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretTreeAttachAny(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpRequestAnysAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpRequestAnyAddRequestExpression(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
int NlpRequestExpressionAnysLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpRequestAnyAddClosest(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression);
int NlpRequestAnyString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string);
int NlpRequestAnyPositionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string);
int NlpRequestAnyStringPretty(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string);

/* nlpanyopt.c */
int NlpRequestAnyOptimizeMatch(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression);

/* nlpreopt.c */
og_status NlpRequestExpressionsOptimize(og_nlp_th ctrl_nlp_th);

/* nlptreejson.c */
og_status NlpInterpretTreeJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretation);

/* nlpsol.c */
og_status NlpSolutionCalculate(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpSolutionMergeObjects(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpRequestSolutionString(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int size,
    char *string);
og_status NlpSolutionString(og_nlp_th ctrl_nlp_th, json_t *json_solution, int size, char *string);

/* nlpduk.c */
char *NlpDukTypeString(duk_int_t type);

/* nlpjavascript.c */
og_status NlpJsInit(og_nlp_th ctrl_nlp_th);
og_status NlpJsReset(og_nlp_th ctrl_nlp_th);
og_bool NlpJsStackWipe(og_nlp_th ctrl_nlp_th);
og_status NlpJsFlush(og_nlp_th ctrl_nlp_th);
og_status NlpJsAddVariable(og_nlp_th ctrl_nlp_th, og_string variable_name, og_string variable_eval);
og_status NlpJsAddVariableJson(og_nlp_th ctrl_nlp_th, og_string variable_name, json_t *variable_value);
og_status NlpJsEval(og_nlp_th ctrl_nlp_th, int js_script_size, og_string js_script, json_t **p_json_anwser);

/* nlpglue.c */
og_status NlpGlueInit(og_nlp_th ctrl_nlp_th);
og_status NlpGlueFlush(og_nlp_th ctrl_nlp_th);
og_status NlpGlueReset(og_nlp_th ctrl_nlp_th);
og_status NlpGlueBuild(og_nlp_th ctrl_nlp_th);
enum nlp_glue_status NlpGluedGetStatusForPositions(og_nlp_th ctrl_nlp_th, int position1, int position2);

/* nlpcheck.c */
og_status NlpCheckPackages(og_nlp_th ctrl_nlp_th);

/* nlplocale.c */
og_status NlpInterpretRequestBuildAcceptLanguage(og_nlp_th ctrl_nlp_th, json_t *json_accept_language);
int NlpAcceptLanguageString(og_nlp_th ctrl_nlp_th, int size, char *string);
og_status NlpAdjustLocaleScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpscore.c */
og_status NlpCalculateScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpclean.c */
og_status NlpRequestExpressionsClean(og_nlp_th ctrl_nlp_th);

/* nlpltras.c */
og_status NlpLtras(og_nlp_th ctrl_nlp_th);

/* nlpltrac.c */
og_status NlpLtracInit(og_nlp_th ctrl_nlp_th);
og_status NlpLtracFlush(og_nlp_th ctrl_nlp_th);
og_status NlpLtracPackage(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpLtracPackageFlush(package_t package);

/* nlpltras.c */
og_status NlpLtrasInit(og_nlp_th ctrl_nlp_th);
og_status NlpLtrasFlush(og_nlp_th ctrl_nlp_th);

