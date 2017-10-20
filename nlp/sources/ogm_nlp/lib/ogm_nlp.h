/*
 *  Internal header for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev: September 2017
 *  Version 1.0
 */
#include <lognlp.h>
#include <logaut.h>
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

#define DOgNlpInterpretationExpressionMaxLength 0x800



#define DOgNlpMaximumOwnedLock      16


struct alias
{
  int alias_start, alias_length;
  int slug_start, slug_length; // interpretation slug
  int id_start, id_length; // interpretation id
  int package_start, package_length;
};

struct expression
{
  int text_start, text_length;
  int alias_start, aliases_nb;
  int locale;
  int input_part_start, input_parts_nb;
};

struct interpretation
{
  int id_start, id_length;
  int slug_start, slug_length;
  int expression_start, expressions_nb;
};

struct package
{
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

  int id_start, id_length;
  int slug_start, slug_length;
  og_heap hba;
  og_heap hinterpretation;
  og_heap hexpression;
  og_heap halias;
  og_heap hinput_part;
  void *ha_word;
  /** HashTable key: string (package id) , value: input_part index */
  GHashTable *interpretation_id_hash;

};

typedef struct package *package_t;

struct interpret_package
{
  package_t package;
};

enum nlp_input_part_type
{
  nlp_input_part_type_Nil = 0,
  nlp_input_part_type_Word,
  nlp_input_part_type_Interpretation
};

struct input_part
{
  enum nlp_input_part_type type;
  int word_start, word_length;
  struct alias *alias;
};


enum nlp_synchro_test_timeout_in
{
  nlp_timeout_in_NONE = 0,
  nlp_timeout_in_NlpPackageAddOrReplace,
  nlp_timeout_in_NlpPackageGet
};

enum nlp_synchro_lock_type
{
  nlp_synchro_lock_type_read,
  nlp_synchro_lock_type_write
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

  /**
   * List of package_t currently used by the og_ctrl_nlp_threaded
   * (reset clean this list at the end of the request), it is better
   * to mark it as used soon as possible
   */
  GQueue package_in_used[1];

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
og_status NlpLogInfo(og_nlp_th ctrl_nlp_th, og_bitfield trace_component, og_string format, ...);
og_status NlpLogDebug(og_nlp_th ctrl_nlp_th, og_bitfield trace_component, og_string format, ...);
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated);
og_status NlpPackageLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpPackageInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation);
og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression);
og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, int Ialias);
og_status NlpPackageInputPartLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part);

/* nlpsynchro.c */
og_status OgNlpSynchroUnLockAll(og_nlp_th ctrl_nlp_th);
og_status OgNlpSynchroReadLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroReadUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroWriteLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroWriteUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroTestSleepIfTimeoutNeeded(og_nlp_th ctrl_nlp_th, enum nlp_synchro_test_timeout_in timeout_in);

/* nlpackage.c */
package_t NlpPackageCreate(og_nlp_th ctrl_nlp_th, const char *string_id, const char *string_slug);
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
og_status NlpInputPartWordFlush(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpInputPartWordAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string string_word, int length_string_word, int Iinput_part);
og_status NlpInputPartWordLog(og_nlp_th ctrl_nlp_th, package_t package);

/* nlpipalias.c */
og_status NlpInputPartAliasInit(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpInputPartAliasAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string interpretation_id, size_t Iinput_part);
og_status NlpInputPartAliasLog(og_nlp_th ctrl_nlp_th, package_t package);





