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
#include <logis639.h>
#include <logis3166.h>
#include <glib-2.0/glib.h>
#include <duktape.h>
#include <math.h>

#define DOgNlpPackageNumber 0x10
#define DOgNlpPackageBaNumber 0x100
#define DOgNlpPackageInterpretationNumber 0x10
#define DOgNlpPackageContextNumber (DOgNlpPackageInterpretationNumber*0x1)
#define DOgNlpPackageExpressionNumber (DOgNlpPackageInterpretationNumber*0x10)
#define DOgNlpPackageAliasNumber DOgNlpPackageExpressionNumber
#define DOgNlpPackageInputPartNumber DOgNlpPackageInterpretationNumber

#define DOgNlpRequestContextNumber 0x2
#define DOgNlpRequestWordNumber 0x10
#define DOgNlpAcceptLanguageNumber 0x8
#define DOgNlpRequestInputPartNumber 0x10
#define DOgNlpRequestExpressionNumber 0x10
#define DOgNlpRequestPositionNumber (DOgNlpRequestExpressionNumber*2+DOgNlpRequestInputPartNumber)
#define DOgNlpBaNumber 0x100

#define DOgNlpInterpretationSentenceMaxLength     0x10000
#define DOgNlpInterpretationExpressionMaxLength   0x800
#define DOgNlpInterpretationRegexMaxLength        0x1000
#define DOgNlpInterpretationContextFlagMaxLength  0x400
#define DOgNlpInterpretationSolutionMaxLength     0x2000

#define DOgNlpMaximumRegex          100
#define DOgNlpMaximumRegexStringSizeLogged  512 // Must be smaller than DPcPathSize
#define DOgNlpMaxEntitySize   512
#define DOgNlpMaxNbWordsPerEntity 10

#define DOgNlpMaximumOwnedLock      16

// Max default distance (in number of chars) between two input parts in an expression
#define DOgNlpDefaultGlueDistance   20

#define DOgNlpMinEntityNumber       500
#define DOgNlpEntitySeparator       0x80

#define NlpLog(nlptrace,nlpformat, ...) if (ctrl_nlp_th->loginfo->trace & nlptrace) \
  { \
    NlpLogImplementation(ctrl_nlp_th, nlpformat, ##__VA_ARGS__);\
  }

// %.15g means, 15 significant figures not decimal, default is 6.
#define DOgPrintDouble "%.15g"

/** Nlp configuration set by env variables*/
struct og_nlp_env
{
  int NlpJSDukGcPeriod;
};

/** Nlp env default value */
#define DOgNlpJSDukGcPeriod   100
#define DOgNlpMaxRawTextSize  1024
#define DOgNlpMaxRequestRawTextSize  8092

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

  /** hcontext_ba : String heap for context, read-only after compilation step */
  og_heap hcontext_ba;
  og_heap hcontext_compile;
  og_heap hcontext;

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

  og_heap hnumber_input_part;

  /** Automaton : "<string_word>\1<Iinput_part>" */
  void *ha_word;
  int nb_words;

  /** Automaton : "<nb_words><string_entity\1<ptr_expression>" */
  void *ha_entity;
  int nb_entities;
  int max_nb_words_per_entity;

  /** Automaton : "<interpretation_id>\1<Iinput_part>" */
  void *ha_interpretation_id;

  /** Automatons for ltrac/ltraf */
  struct ltra_dictionaries ltra_dictionaries[1];
  int ltra_min_frequency;
  int ltra_min_frequency_swap;

  /** Automatons for ltrac/ltraf entities */
  struct ltra_dictionaries ltra_entity_dictionaries[1];
  int ltra_entity_min_frequency;
  int ltra_entity_min_frequency_swap;

};

typedef struct package *package_t;

enum nlp_alias_type
{
  nlp_alias_type_Nil = 0, nlp_alias_type_Interpretation, nlp_alias_type_Any, nlp_alias_type_Number, nlp_alias_type_Regex
};

struct alias_compile
{
  enum nlp_alias_type type;
  int alias_start, alias_length;
  int slug_start, slug_length;       // interpretation slug
  int id_start, id_length;           // interpretation id
  int package_id_start, package_id_length;
  int regex_start, regex_length;
};

struct alias
{
  enum nlp_alias_type type;

  /** alias name */
  og_string alias;
  int alias_length;

  /** type interpretation */
  og_string slug;
  og_string id;
  og_string package_id;

  /** type regex */
  og_string regex_string;
  GRegex *regex;

};

struct context_compile
{
  int flag_start;
};

enum nlp_glue_strength
{
  nlp_glue_strength_Nil = 0, nlp_glue_strength_Total, nlp_glue_strength_Punctuation
};

struct expression_compile
{
  int id_start;
  int text_start;
  og_bool keep_order;
  og_bool glued;
  enum nlp_glue_strength glue_strength;
  int glue_distance;
  int alias_start, aliases_nb;
  int locale;
  int input_part_start, input_parts_nb;
  json_t *json_solution;
};

struct context
{
  og_string flag;
};

struct expression
{
  /** Parent */
  struct interpretation *interpretation;

  og_string id;

  og_string text;

  og_bool keep_order;
  og_bool glued;
  enum nlp_glue_strength glue_strength;
  int glue_distance;

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
  int any_input_part_position;

  og_bool is_recursive;
  og_bool is_super_list;

  json_t *json_solution;
};

enum nlp_interpretation_scope_type
{
  nlp_interpretation_scope_type_public = 0,
  nlp_interpretation_scope_type_private = 1,
  nlp_interpretation_scope_type_hidden = 2
};

struct interpretation_compile
{
  package_t package;
  int id_start, id_length;
  int slug_start, slug_length;
  int context_start, contexts_nb;
  int expression_start, expressions_nb;
  json_t *json_solution;
  enum nlp_interpretation_scope_type scope;
};

struct interpretation
{
  /** Parent */
  package_t package;

  og_string id;
  og_string slug;
  json_t *json_solution;

  enum nlp_interpretation_scope_type scope;

  int contexts_nb;
  struct context *contexts;

  int expressions_nb;
  struct expression *expressions;

  og_bool is_recursive;
  og_bool is_super_list;
};

struct interpret_package
{
  int self_index;
  package_t package;
};

enum nlp_input_part_type
{
  nlp_input_part_type_Nil = 0,
  nlp_input_part_type_Word,
  nlp_input_part_type_Interpretation,
  nlp_input_part_type_Number,
  nlp_input_part_type_Regex
};

struct input_part_word
{
  int raw_word_start;
  int word_start;
};

struct input_part
{
  int self_index;

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

struct number_input_part
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
  int self_index;

  /** whole string representing normalized request_word (in ort case it is the corrected one) */
  int start;
  int length;

  /** whole string representing NON normalized request_word */
  int raw_start;
  int raw_length;

  /** Position in orginal request string */
  int start_position;
  int length_position;

  int nb_matched_words;

  og_bool is_number;
  double number_value;
  double spelling_score;
  og_bool is_auto_complete_word;
  og_bool is_punctuation;
  og_bool is_expression_punctuation;
  og_bool is_regex;
  og_bool lang_id;
  struct input_part *regex_input_part;

  /**
   * chain the list in order to ignore merged words
   *
   * It can be used only after calling NlpMatchWordChainRequestWords
   */
  struct request_word *next;
};

struct accept_language
{
  int locale;
  float quality_factor;
};

enum nlp_super_list_status
{
  nlp_super_list_status_Nil = 0, nlp_super_list_status_Part, nlp_super_list_status_Top
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

  int safe_request_position_start;
  int safe_request_positions_nb;

  int Ioriginal_request_input_part;

  og_bool interpret_word_as_number;

  int sparse_mark;

  enum nlp_super_list_status super_list_status;
};

struct request_position
{
  int start;
  int length;
};

/** Cache Ogheap cells access */
struct request_expression_access_cache
{
  struct request_expression * request_expressions;
  int request_expressions_used;

  struct request_position *request_positions;
  int request_positions_used;
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
  /** List of struct request_word covered by any */
  GQueue queue_request_words[1];
  int distance;

  /** used to optimize the attachement any <-> request_expression */
  GQueue queue_request_expression[1];
  og_bool is_attached;
  int consumed;
};

struct request_score
{
  double coverage;
  double locale;
  double spelling;
  double overlap;
  double any;
  double context;
  double scope;
};

#define DOgNlpAnyTopologyNil      0
#define DOgNlpAnyTopologyLeft     1
#define DOgNlpAnyTopologyRight    2
#define DOgNlpAnyTopologyBothSide (DOgNlpAnyTopologyLeft+DOgNlpAnyTopologyRight)

struct request_expression
{
  int self_index;

  /** from zero (only words) to N */
  int level;

  /** matched expression */
  struct expression *expression;

  int request_position_start;
  int request_positions_nb;

  int nb_matched_words;

  int safe_request_position_start;
  int safe_request_positions_nb;

  // overlapping rate of the tree of the request_expression
  int overlap_mark;
  int sparse_mark;

  int orip_start;
  int orips_nb;

  /** all possible any for request expression */
  int request_any_start;
  int request_anys_nb;

  int Isuper_request_expression;
  // can be NULL when it is the root expression
  struct alias *mothers_alias;

  int Irequest_any;
  struct request_word *auto_complete_request_word;

  /** used locally for various scanning */
  int analyzed;

  /** Mark of an expression as not to be use anymore **/
  int deleted;

  og_bool recursive_without_any_chosen;

  og_bool keep_as_result;
  int nb_anys;
  int nb_anys_attached;
  /** 0: invalidated, 1: unknown, 2: validated **/
  int any_validate_status;
  /** nil, left, right, or both size **/
  int any_topology;

  /* sorted flat representation of the recursive list
   * when flat_list->length != 0 use this structure to navigate
   * the flat list is sorted according to word order */
  GQueue sorted_flat_list[1];

  struct request_score score[1];
  double total_score;

  GQueue tmp_solutions[1];

  json_t *json_solution;

  enum nlp_super_list_status super_list_status;
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
  og_bool is_sorted_flat_list;
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
  /** Single unicode char treated as word */
  gunichar punct_char_word[DOgNlpParsePunctCharMaxNb];
  int punct_char_word_used;

  /** Single unicode char for expression treated as word used by glue_strength */
  gunichar punct_char_word_expression[DOgNlpParsePunctCharMaxNb];
  int punct_char_word_expression_used;

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
  // See : http://duktape.org/api.html#duk_get_context.4

  /** Permanant ctx : accross the request (lib, momement setup) */
  duk_context *duk_perm_context;

  /** Request ctx : wipped at the ends of the request */
  duk_context *duk_request_context;

  /** buffer store intermediate js computation */
  og_heap buffer;

  /** variables store variables as string */
  GStringChunk *variables;

  /** variable name: "toto", "tata" */
  GQueue variables_name_list[1];

  /** variable values : "var toto=1;", "var tata = 2;" */
  GQueue variables_values[1];

  size_t reset_counter;

  /** random number used to protect variable internal name (moment lib). */
  guint32 random_number;

};

/** non matching expression that will be search upon the "why-not-matching" object of an interpret request */
struct nm_expression
{
  struct expression *expression;
  int m_input_part_start;
// m_input_parts_nb is always expression->input_parts_nb
};

struct m_input_part
{
  struct input_part *input_part;
  int m_expression_start;
  int m_expressions_nb;
};

struct m_expression
{
  int Irequest_input_part;
};

struct request_context
{
  int flag_start;
  int flag_length;
};

typedef struct number_sep_conf *nlp_match_group_numbers_conf;

struct nlp_match_group_numbers
{

  struct og_ctrl_nlp_threaded *nlpth;

  /* string of all seprators except \" \" space */
  og_string all_separators;

  /* list of (struct number_sep_conf) */
  GQueue sep_conf[1];

  /* list of (struct number_sep_conf_locale) */
  GQueue sep_conf_lang[1];

  /* hash of Glist (struct number_sep_conf_locale) indexed by (lang + DOgLangMax * country) */
  GHashTable *sep_conf_lang_by_lang_country;

};

struct highlight_word
{
  og_bool is_any;
  union
  {
    struct request_word *request_word;
    struct request_any *request_any;
  };
  struct request_expression *request_expression;
};

struct super_list
{
  struct expression *recursive_expression;
  struct alias *alias;
  struct interpretation *interpretation;
  struct expression *single_expression;
  struct interpret_package *interpret_package;
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
  json_t *json_answer_unit;
  json_t *json_warnings;
  int nb_warnings;

  /** Stack of current lock (struct nlp_synchro_lock) owned by the thread */
  struct nlp_synchro_current_lock current_lock[1];

  /** Package beeing created */
  package_t package_in_progress;

  /** Loop level for the matching **/
  int level;

  /** interpret request */

  /** Primary package can be null for backward compatibilities */
  og_string primary_package_id;
  package_t primary_package;
  og_bool show_private;

  og_heap hinterpret_package;
  og_string request_sentence;
  int basic_request_word_used;
  int basic_group_request_word_nb;
  og_heap haccept_language;
  og_bool show_explanation;
  og_bool auto_complete;
  unsigned int regular_trace;
  og_string date_now;
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

  og_heap hhighlight_word;

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

  /* struct used by match_group_numbers */
  struct nlp_match_group_numbers group_numbers_settings[1];

  void *hltrac;
  void *hltras;

  og_heap hnm_expression;
  og_heap hm_input_part;
  og_heap hm_expression;

  og_heap hrequest_context;

  og_heap hre_in_list;
  og_heap hre_to_sort;

  og_bool accept_any_expressions;

  og_heap hsuper_list;

};

#define DOgLemNameSize 256

struct lem_data {
  og_bool active;
  char root[DOgLemNameSize];
  char form[DOgLemNameSize];
  void *ha_root;
  void *ha_form;
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

  /** Environment conf */
  struct og_nlp_env env[1];

  /** Parsing configuration */
  struct og_nlp_parse_conf parse_conf[1];

  struct lem_data ld[DOgLangMax];

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
og_status NlpPackageContextLog(og_nlp_th ctrl_nlp_th, package_t package, struct context *context);
og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);
og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias);
og_status NlpPackageInputPartLog(og_nlp_th ctrl_nlp_th, package_t package, struct input_part *input_part);
og_status NlpPackageInputPartExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part, char *label);
og_status NlpPackageExpressionSolutionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);

og_status NlpPackageCompileLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpPackageCompileInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation);
og_status NlpPackageCompileInterpretationSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation);
og_status NlpPackageCompileContextLog(og_nlp_th ctrl_nlp_th, package_t package, struct context_compile *context);
og_status NlpPackageCompileExpressionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression);
og_status NlpPackageCompileExpressionSolutionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression);
og_status NlpPackageCompileAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias_compile *alias);

og_status NlpLogRequestWords(og_nlp_th ctrl_nlp_th);
og_status NlpLogRequestWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word);
const char *NlpAliasTypeString(enum nlp_alias_type type);
const char *NlpGlueStrengthString(enum nlp_glue_strength glue_strength);

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

/* nlpackagelist.c */
typedef og_status (*nlp_package_list_callback)(og_nlp_th ctrl_nlp_th, og_string package_id);
og_status NlpPackageListInternal(og_nlp_th ctrl_nlp_th, nlp_package_list_callback func);

/* nlpthreaded.c */
og_status OgNlpThreadedResetKeepJsonAnswer(og_nlp_th ctrl_nlp_th);

/* nlpinterpret.c */
og_status NlpInterpretInit(og_nlp_th ctrl_nlp_th, struct og_nlp_threaded_param *param);
og_status NlpInterpretReset(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretFlush(og_nlp_th ctrl_nlp_th);

/* nlpcompile.c */
og_status NlpCompilePackageInterpretation(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretation);
og_status NlpCompilePackage(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_package);

/* nlpconsolidate.c */
og_status NlpConsolidatePackage(og_nlp_th ctrl_nlp_th, package_t package);
int NlpConsolidateExpressionWord(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);


/* nlpipword.c */
og_status NlpInputPartWordInit(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpInputPartWordFlush(package_t package);
og_status NlpInputPartWordAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string string_word, int length_string_word,
    int Iinput_part);
og_status NlpInputPartAliasNumberAdd(og_nlp_th ctrl_nlp_th, package_t package, size_t Iinput_part);
og_status NlpInputPartWordLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpNumberInputPartLog(og_nlp_th ctrl_nlp_th, package_t package);

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

og_status NlpMatchWordChainRequestWords(og_nlp_th ctrl_nlp_th);
og_status NlpMatchWordChainUpdateWordCount(og_nlp_th ctrl_nlp_th);

/* nlpmatch_group_numbers.c */
og_status NlpMatchGroupNumbersInit(og_nlp_th ctrl_nlp_th);
og_status NlpMatchGroupNumbersFlush(og_nlp_th ctrl_nlp_th);
og_status NlpMatchGroupNumbers(og_nlp_th ctrl_nlp_th);

/* nlpmatch_expression.c */
og_status NlpMatchExpressions(og_nlp_th ctrl_nlp_th);

/* nlpmatch_interpretation.c */
og_bool NlpMatchInterpretations(og_nlp_th ctrl_nlp_th);

/* nlpparse.c */
og_status NlpParseConfInit(og_nlp ctrl_nlp);
og_status NlpParseConfFlush(og_nlp ctrl_nlp);
og_status NlpParseRequestSentence(og_nlp_th ctrl_nlp_th);
og_bool NlpParseIsPunctuation(og_nlp_th ctrl_nlp_th, int max_word_size, og_string current_word, og_bool *p_skip,
    og_bool *p_expression, int *p_punct_length_bytes);

/* nlprip.c */
og_status NlpRequestInputPartAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    struct interpret_package *interpret_package, int Iinput_part, og_bool word_as_number);
og_status NlpRequestInputPartAddInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct interpret_package *interpret_package, int Iinput_part);
struct request_input_part *NlpGetRequestInputPart(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int Iorip);
og_bool NlpRequestInputPartsAreOrdered(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2);
og_bool NlpRequestInputPartsAreGlued(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2, og_bool keep_order);
og_bool NlpRequestInputPartsAreExpressionGlued(og_nlp_th ctrl_nlp_th, struct request_input_part *request_input_part1,
    struct request_input_part *request_input_part2);
og_status NlpRequestInputPartsLog(og_nlp_th ctrl_nlp_th, int request_input_part_start, char *title);
og_status NlpRequestInputPartLog(og_nlp_th ctrl_nlp_th, int Irequest_input_part);

/* nlprexpression.c */
og_bool NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression,
    struct match_zone_input_part *match_zone_input_part, struct request_expression **prequest_expression,
    og_bool is_super_list);
og_bool NlpRequestExpressionIsOrdered(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_bool NlpRequestExpressionsAreGlued(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2, og_bool keep_order);
og_status NlpRequestExpressionsCalculate(og_nlp_th ctrl_nlp_th);
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations);
og_status NlpSortedRequestExpressionsLog(og_nlp_th ctrl_nlp_th, char *title);
og_status NlpRequestExpressionsLog(og_nlp_th ctrl_nlp_th, int request_expression_start, char *title);
og_status NlpRequestExpressionLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int offset);
og_status NlpRequestExpressionShowTree(og_nlp_th ctrl_nlp_th, int Irequest_expression, og_string label);

/* nlprposition.c */
og_status NlpRequestPositionAdd(og_nlp_th ctrl_nlp_th, int start, int length, size_t *pIrequest_position);
og_status NlpRequestPositionSort(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb);
og_bool NlpRequestPositionSame(og_nlp_th ctrl_nlp_th, struct request_expression_access_cache *cache,
    int request_position_start1, int request_positions_nb1, int request_position_start2, int request_positions_nb2);
og_bool NlpRequestPositionIncluded(og_nlp_th ctrl_nlp_th, struct request_position *request_positions,
    int request_position_start1, int request_positions_nb1, int request_position_start2, int request_positions_nb2);
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
og_status NlpInterpretTreeLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int offset);
og_status NlpSetSuperExpression(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpany.c */
og_status NlpInterpretAnyFlush(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretAnyReset(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretTreeAttachAny(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpRequestAnysAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpRequestAnyAddRequestExpression(og_nlp_th ctrl_nlp_th, struct request_any *request_any,
    struct request_expression *request_expression);
og_status NlpSetNbAnys(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpSetAnyTopology(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpGetNbAnysAttached(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
int NlpRequestExpressionAnysLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
int NlpRequestExpressionAnyLog(og_nlp_th ctrl_nlp_th, struct request_any *request_any);
og_status NlpRequestAnyAddClosest(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression);
int NlpRequestAnyString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string);
int NlpRequestAnyPositionString(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string);
int NlpRequestAnyStringPretty(og_nlp_th ctrl_nlp_th, struct request_any *request_any, int size, char *string);
og_string NlpAnyTopologyString(int any_topology);

/* nlpanyopt.c */
int NlpRequestAnyOptimizeMatch(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    og_bool must_attach);

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
og_status NlpJsStackRequestSetup(og_nlp_th ctrl_nlp_th);
og_bool NlpJsStackRequestWipe(og_nlp_th ctrl_nlp_th);
og_bool NlpJsStackLocalWipe(og_nlp_th ctrl_nlp_th);
og_status NlpJsFlush(og_nlp_th ctrl_nlp_th);
og_status NlpJsAddVariable(og_nlp_th ctrl_nlp_th, og_string variable_name, og_string variable_eval,
    int variable_eval_length);
og_status NlpJsAddVariableJson(og_nlp_th ctrl_nlp_th, og_string variable_name, json_t *variable_value);
og_status NlpJsSetNow(og_nlp_th ctrl_nlp_th);
og_status NlpJsEval(og_nlp_th ctrl_nlp_th, int js_script_size, og_string js_script, json_t **p_json_anwser);

/* nlpglue.c */
og_status NlpGlueInit(og_nlp_th ctrl_nlp_th);
og_status NlpGlueFlush(og_nlp_th ctrl_nlp_th);
og_status NlpGlueReset(og_nlp_th ctrl_nlp_th);
og_status NlpGlueBuild(og_nlp_th ctrl_nlp_th);
enum nlp_glue_status NlpGluedGetStatusForPositions(og_nlp_th ctrl_nlp_th, int position1, int position2);
og_status NlpGlueLog(og_nlp_th ctrl_nlp_th);

/* nlpcheck.c */
og_status NlpCheckPackages(og_nlp_th ctrl_nlp_th);

/* nlplocale.c */
og_status NlpInterpretRequestBuildAcceptLanguage(og_nlp_th ctrl_nlp_th, json_t *json_accept_language);
int NlpAcceptLanguageString(og_nlp_th ctrl_nlp_th, int size, char *string);
og_status NlpAdjustLocaleScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpscore.c */
og_status NlpCalculateScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpCalculateScoreDuringParsing(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpclean.c */
og_status NlpRequestExpressionsClean(og_nlp_th ctrl_nlp_th);

/* nlpltrac.c */
og_status NlpLtracInit(og_nlp_th ctrl_nlp_th);
og_status NlpLtracFlush(og_nlp_th ctrl_nlp_th);
og_status NlpLtracPackage(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpLtracPackageFlush(package_t package);

/* nlpltrac_entity.c */
og_status NlpLtracEntityPackage(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpLtracEntityPackageFlush(package_t package);

/* nlpltras.c */
og_status NlpLtrasInit(og_nlp_th ctrl_nlp_th);
og_status NlpLtrasFlush(og_nlp_th ctrl_nlp_th);
og_status NlpLtras(og_nlp_th ctrl_nlp_th);

/* nlpltras_entity.c */
og_status NlpLtrasEntityPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package,
    struct request_word **request_word_list, int request_word_list_length, unsigned char *string_entity,
    int string_entity_length);

/* nlpwhy.c */
og_status NlpWhyNotMatchingInit(og_nlp_th ctrl_nlp_th, og_string name);
og_status NlpWhyNotMatchingReset(og_nlp_th ctrl_nlp_th);
og_status NlpWhyNotMatchingFlush(og_nlp_th ctrl_nlp_th);
og_status NlpWhyNotMatchingBuild(og_nlp_th ctrl_nlp_th, json_t *json_why_not_matching);
og_status NlpWhyNotMatchingLog(og_nlp_th ctrl_nlp_th);
og_status NlpWhyNotMatchingExpressionLog(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression);

/* nlpwhycal.c */
og_status NlpWhyCalculate(og_nlp_th ctrl_nlp_th);

/* nlpwhyjson.c */
og_status NlpWhyJson(og_nlp_th ctrl_nlp_th, json_t *json_interpretation);

/* nlpwarn.c */
og_status NlpWarningReset(og_nlp_th ctrl_nlp_th);
og_status NlpWarningAdd(og_nlp_th ctrl_nlp_th, og_string format, ...);

/* nlpac.c */
og_status NlpAutoComplete(og_nlp_th ctrl_nlp_th);
og_status NlpGetAutoCompleteRequestWord(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_bool NlpDifferentAutoCompleteRequestWord(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression1,
    struct request_expression *request_expression2);

/* nlpcontext.c */
og_status NlpContextIsValid(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
og_status NlpContextGetScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlprelist.c */
og_status NlpRequestExpressionListsSortInit(og_nlp_th ctrl_nlp_th, og_string name);
og_status NlpRequestExpressionListsSortFlush(og_nlp_th ctrl_nlp_th);
og_status NlpRequestExpressionListsSort(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);

/* nlpexplainhighlight.c */
og_status NlpExplainHighlightInit(og_nlp_th ctrl_nlp_th, og_string name);
og_status NlpExplainHighlightReset(og_nlp_th ctrl_nlp_th);
og_status NlpExplainHighlightFlush(og_nlp_th ctrl_nlp_th);
og_status NlpExplainHighlightAddWord(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct request_word *request_word, struct request_any *request_any);
og_status NlpExplainHighlight(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_explanation);
og_status NlpExplainHighlightLog(og_nlp_th ctrl_nlp_th);

/* nlpregex.c */
og_status NlpRegexBuildPackage(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpRegexPackageLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpRegexMatch(og_nlp_th ctrl_nlp_th);
og_status NlpRegexLog(og_nlp_th ctrl_nlp_th);

/* nlpsuperlist.c */
og_status NlpSuperListInit(og_nlp_th ctrl_nlp_th, og_string name);
og_status NlpSuperListReset(og_nlp_th ctrl_nlp_th);
og_status NlpSuperListFlush(og_nlp_th ctrl_nlp_th);
og_status NlpConsolidateSuperListPackage(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpSuperListGet(og_nlp_th ctrl_nlp_th);
og_bool NlpSuperListValidate(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part);
og_bool NlpSuperListsCreate(og_nlp_th ctrl_nlp_th);

/* nlprword.c */
og_bool NlpRequestWordGet(og_nlp_th ctrl_nlp_th, int position, int *pIrequest_word);

/* nlplem.c */
og_status NlpLemInit(og_nlp ctrl_nlp);
og_status NlpLemFlush(og_nlp ctrl_nlp);
og_status NlpLem(og_nlp_th ctrl_nlp_th);


/* nlpentity.c */
og_status NlpEntityInit(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpEntityFlush(package_t package);
og_status NlpEntityAdd(og_nlp_th ctrl_nlp_th, package_t package, int nb_words, og_string string_word,
    int length_string_word, struct expression *expression);
og_status NlpReduceEntities(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpEntityLog(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpmatch_entity.c */
og_status NlpMatchEntities(og_nlp_th ctrl_nlp_th);
og_status NlpMatchEntitiesNgramInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package,
    struct request_word **request_word_list, int request_word_list_length, unsigned char *string_entity,
    int string_entity_length, og_bool must_spellcheck);



