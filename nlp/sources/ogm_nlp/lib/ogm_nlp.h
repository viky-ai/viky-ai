/*
 *  Internal header for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev: September 2017
 *  Version 1.0
 */
#include <lognlp.h>
#include <logheap.h>
#include <logsysi.h>
#include <logis639_3166.h>
#include <glib-2.0/glib.h>

#define DOgNlpPackageNumber 0x10
#define DOgNlpPackageBaNumber 0x100
#define DOgNlpPackageInterpretationNumber 0x10
#define DOgNlpPackageExpressionNumber (DOgNlpPackageInterpretationNumber*0x10)
#define DOgNlpPackageAliasNumber DOgNlpPackageExpressionNumber
#define DOgNlpInterpretationExpressionMaxLength 0x800

#define DOgNlpiPackageNumber  0x10


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
};

struct interpretation
{
  int id_start, id_length;
  int slug_start, slug_length;
  int expression_start, expressions_nb;
};

struct package
{
  int id_start, id_length;
  int slug_start, slug_length;
  og_heap hba;
  og_heap hinterpretation;
  og_heap hexpression;
  og_heap halias;
};

typedef struct package *package_t;

struct interpret_package
{
  package_t package;
};


enum nlp_synchro_test_timeout_in
{
  nlp_timeout_in_NONE = 0,
  nlp_timeout_in_NlpPackageAddOrReplace,
  nlp_timeout_in_NlpPackageGet
};

enum nlp_synchro_lock_type
{
  nlp_synchro_lock_type_read_lock,
  nlp_synchro_lock_type_write_lock
};

struct nlp_synchro_lock
{
  enum nlp_synchro_lock_type type;

  ogsysi_rwlock rwlock;
};

struct og_ctrl_nlp_threaded
{
  og_nlp ctrl_nlp;
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  /** Use for testing feature : trigger timeout in a specific function */
  enum nlp_synchro_test_timeout_in timeout_in;

  /** common request */
  json_t *json_answer;

  /** Stack of current lock (struct nlp_synchro_lock) owned by the thread */
  GQueue current_rw_lock[1];

  /** interpret request */
  og_heap hinterpret_package;
  og_string request_sentence;

};

struct og_ctrl_nlp
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];

  /** HashTable key: string (package id) , value: package (package_t) */
  GHashTable *packages_hash;
  ogsysi_rwlock rw_lock_packages_hash;

  /** List of deleted package wait for been freed when they are not used any more */
  GQueue deleted_packages[1];
};

/* nlperr.c */
og_status NlpThrowError(og_nlp ctrl_nlp, og_string format, ...);
og_status NlpThrowErrorTh(og_nlp_th ctrl_nlp_th, og_string format, ...);

/* nlplog.c */
og_status NlpJsonToBuffer(const json_t *json, og_char_buffer *buffer, int buffer_size, og_bool *p_truncated);
og_status NlpPackageLog(og_nlp_th ctrl_nlp_th, package_t package);
og_status NlpPackageInterpretationLog(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation);
og_status NlpPackageExpressionLog(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression);
og_status NlpPackageAliasLog(og_nlp_th ctrl_nlp_th, package_t package, int Ialias);

/* nlpsynchro.c */
og_status OgNlpSynchroUnLockAll(og_nlp_th ctrl_nlp_th);
og_status OgNlpSynchroReadLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroReadUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroWriteLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroWriteUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock);
og_status OgNlpSynchroTestSleepIfTimeoutNeeded(og_nlp_th ctrl_nlp_th, enum nlp_synchro_test_timeout_in timeout_in);

/* nlpdump.c */
og_status NlpPackageDump(og_nlp_th ctrl_nlp_th, package_t package, json_t *dump_json);
og_status NlpPackageInterpretationDump(og_nlp_th ctrl_nlp_th, package_t package, int Iinterpretation, json_t *dump_json);
og_status NlpPackageExpressionDump(og_nlp_th ctrl_nlp_th, package_t package, int Iexpression, json_t *dump_json);
og_status NlpPackageAliasDump(og_nlp_th ctrl_nlp_th, package_t package, int Ialias, json_t *json_aliases);

/* nlpackage.c */
package_t NlpPackageCreate(og_nlp_th ctrl_nlp_th, const char *string_id, const char *string_slug);
og_status NlpPackageFlush(package_t package);
og_status NlpPackageAddOrReplace(og_nlp_th ctrl_nlp_th, package_t package);
package_t NlpPackageGet(og_nlp_th ctrl_nlp_th, og_string package_id);
og_status NlpFlushPackageMarkedAsDeletedNosync(og_nlp ctrl_nlp);

/* nlpinterpret.c */
og_status NlpInterpretInit(og_nlp_th ctrl_nlp_th, struct og_nlp_threaded_param *param);
og_status NlpInterpretReset(og_nlp_th ctrl_nlp_th);
og_status NlpInterpretFlush(og_nlp_th ctrl_nlp_th);

/* nlpcompile.c */
og_status NlpCompilePackageInterpretation(og_nlp_th ctrl_nlp_th, package_t package, json_t *json_interpretation);
og_status NlpCompilePackage(og_nlp_th ctrl_nlp_th, struct og_nlp_compile_input *input, json_t *json_package);
