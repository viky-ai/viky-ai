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

#define DOgNlpPackageNumber 0x10
#define DOgNlpPackageBaNumber 0x100
#define DOgNlpPackageInterpretationNumber 0x10
#define DOgNlpPackageExpressionNumber (DOgNlpPackageInterpretationNumber*0x10)
#define DOgNlpPackageAliasNumber DOgNlpPackageExpressionNumber
#define DOgNlpPackageInputPartNumber DOgNlpPackageInterpretationNumber

#define DOgNlpRequestWordNumber 0x10
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

  /** Automaton : "<string_word>\1<Iinput_part>" */
  void *ha_word;

  /** Automaton : "<interpretation_id>\1<Iinput_part>" */
  void *ha_interpretation_id;

};

typedef struct package *package_t;

struct alias_compile
{
  int alias_start, alias_length;
  int slug_start, slug_length;       // interpretation slug
  int id_start, id_length;           // interpretation id
  int package_id_start, package_id_length;
};

struct alias
{
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
};

struct interpret_package
{
  int self_index;
  package_t package;
};

enum nlp_input_part_type
{
  nlp_input_part_type_Nil = 0, nlp_input_part_type_Word, nlp_input_part_type_Interpretation
};

struct input_part
{
  /** Parent */
  struct expression *expression;

  enum nlp_input_part_type type;
  union
  {

    /** nlp_input_part_type_Word */
    int word_start;

    /** nlp_input_part_type_Interpretation */
    struct alias *alias;

  };

};

enum nlp_synchro_test_timeout_in
{
  nlp_timeout_in_NONE = 0, nlp_timeout_in_NlpPackageAddOrReplace, nlp_timeout_in_NlpPackageGet
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
  int start_position;
  int length_position;
};

struct request_input_part
{
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
};

struct request_position
{
  int start;
  int length;
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
};

#define DOgMatchZoneInputPartSize 0x100
struct match_zone_input_part
{
  int start;
  int length;
  int current;
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

  /** interpret request */
  og_heap hinterpret_package;
  og_string request_sentence;
  og_heap hrequest_word;
  og_heap hba;

  /** Heap of struct request_input_part */
  og_heap hrequest_input_part;

  /** Heap of struct request_expression */
  og_heap hrequest_expression;

  /** Heap of struct request_position */
  og_heap hrequest_position;

  /**
   * List of package_t currently used by the og_ctrl_nlp_threaded
   * (reset clean this list at the end of the request), it is better
   * to mark it as used as soon as possible
   */
  GQueue package_in_used[1];

  int new_request_expression_start;
  int new_request_input_part_start;

};

struct og_ctrl_nlp
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  /** HashTable key: string (package id) , value: package (package_t) */
  GHashTable *packages_hash;
  ogsysi_rwlock rw_lock_packages_hash;

};

/* nlperr.c */
og_status NlpThrowError(og_nlp ctrl_nlp, og_string format, ...);
og_status NlpThrowErrorTh(og_nlp_th ctrl_nlp_th, og_string format, ...);

/* nlplog.c */
og_status NlpLogImplementation(og_nlp_th ctrl_nlp_th, og_string format, ...);
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated);
og_status NlpPackageLog(og_nlp_th ctrl_nlp_th, og_string label, package_t package);
og_status NlpPackageInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package, struct interpretation *interpretation);
og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, struct expression *expression);
og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias *alias);
og_status NlpPackageInputPartLog(og_nlp_th ctrl_nlp_th, package_t package, struct input_part *input_part);

og_status NlpPackageCompileLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpPackageCompileInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct interpretation_compile *interpretation);
og_status NlpPackageCompileExpressionLog(og_nlp_th ctrl_nlp_th, package_t package,
    struct expression_compile *expression);
og_status NlpPackageCompileAliasLog(og_nlp_th ctrl_nlp_th, package_t package, struct alias_compile *alias);

og_status NlpLogRequestWords(og_nlp_th ctrl_nlp_th);
og_status NlpLogRequestWord(og_nlp_th ctrl_nlp_th, int Irequest_word);

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
og_status NlpInputPartWordLog(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpipalias.c */
og_status NlpInputPartAliasInit(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpInputPartAliasFlush(package_t package);
og_status NlpInputPartAliasAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string interpretation_id,
    size_t Iinput_part);
og_status NlpInputPartAliasLog(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpmatch.c */
og_status NlpMatch(og_nlp_th ctrl_nlp_th);

/* nlpparse.c */
og_status NlpParse(og_nlp_th ctrl_nlp_th);

/* nlprip.c */
og_status NlpRequestInputPartAddWord(og_nlp_th ctrl_nlp_th, struct request_word *request_word,
    struct interpret_package *interpret_package, int Iinput_part);
og_status NlpRequestInputPartAddInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct interpret_package *interpret_package, int Iinput_part);
og_status NlpRequestInputPartsLog(og_nlp_th ctrl_nlp_th, int request_input_part_start, char *title);
og_status NlpRequestInputPartLog(og_nlp_th ctrl_nlp_th, int Irequest_input_part);

/* nlprexpression.c */
og_bool NlpRequestExpressionAdd(og_nlp_th ctrl_nlp_th, struct expression *expression, int level,
    struct match_zone_input_part *match_zone_input_part, struct request_expression **prequest_expression);
og_status NlpRequestExpressionsExplicit(og_nlp_th ctrl_nlp_th);
og_status NlpRequestInterpretationsBuild(og_nlp_th ctrl_nlp_th, json_t *json_interpretations);
og_status NlpRequestExpressionsLog(og_nlp_th ctrl_nlp_th, int request_expression_start, char *title);
og_status NlpRequestExpressionLog(og_nlp_th ctrl_nlp_th, struct request_expression *request_interpretation);

/* nlprposition.c */
og_status NlpRequestPositionAdd(og_nlp_th ctrl_nlp_th, int start, int length, size_t *pIrequest_position);
og_status NlpRequestPositionSort(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb);
og_bool NlpRequestPositionSame(og_nlp_th ctrl_nlp_th, int request_position_start1, int request_positions_nb1,
    int request_position_start2, int request_positions_nb2);
og_bool NlpRequestPositionOverlap(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb);
int NlpRequestPositionString(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb, int size,
    char *string);
int NlpRequestPositionStringPretty(og_nlp_th ctrl_nlp_th, int request_position_start, int request_positions_nb, int size,
    char *string);

