/**
 * Internal header for handling synchronization for servers and indexers (Multiple reader / One Writer)
 * Copyright (c) 2014 Pertimm by Brice Ruzand
 * Dev : January 2014
 * Version 2.0
 */
#include <logsysi.h>
#include <logmsg.h>
#include <pthread.h>
#include <loggen.h>

// Check pthread R/W implementation
#if defined __USE_UNIX98 || defined __USE_XOPEN2K
#define OGM_SYSI_USE_PTHREAD_RW_LOCK 1
#else
#error Sysi needs XOPEN2K or UNIX98 compliant to use R/W lock
#endif

// Advanced R/W lock analysing
// #define OGM_SYSI_DEBUG 1

// Macro to cast void * in int like glib do
#define OG_POINTER_TO_INT(p)  ((int)  (long) (p))
#define OG_INT_TO_POINTER(i)  ((void *) (long) (i))

struct og_ctrl_sysi_lock
{
  og_bool is_initialized;

  /**
   * Multiple reader / One Writer Lock
   *
   * http://www.daemon-systems.org/man/pthread_rwlock.3.html
   */
  pthread_rwlock_t rw_lock[1];

};


struct og_ctrl_sysi
{
  void *herr, *hmsg;
  struct og_loginfo loginfo[1];

  og_char_buffer name[DPcPathSize];

  /** boolean to enable log sysi stat on flush */
  og_bool log_rw_lock_stat_on_flush;

  /** RW lock multi processes */
  og_bool share_between_process;
  og_bool is_lock_owner;
  og_char_buffer shared_key[DPcPathSize];

  /** write access timeout in seconds  (0 = no timeout)*/
  int write_access_timeout;

  /** read access timeout in seconds  (0 = no timeout)*/
  int read_access_timeout;

  /** Maximum number of retry in case of EAGAIN*/
  int max_lock_retry;

  /** Time to wait between retry in micro seconde*/
  int lock_retry_wait_microsec;

  /**
   * TLS
   */
  pthread_key_t read_lock_number_key;
  pthread_key_t write_lock_number_key;

  struct og_ctrl_sysi_lock local_lock[1];
  struct og_ctrl_sysi_lock* lock;

  /** Stat info */
  /** count lock owned ask (non recursive) */
  ogint64_t read_cmpt;

  /** time elapsed waiting for lock access  non recursive) */
  ogint64_t read_wait_elapsed;

  /** count lock owned ask (non recursive) */
  ogint64_t write_cmpt;

  /** time elapsed waiting for lock access (non recursive) */
  ogint64_t write_wait_elapsed;

  /** time elapsed lock is owned for write (non recursive) */
  ogint64_t write_owned_last_write;
  ogint64_t write_owned_elapsed;

};

og_status OgSysiLogError(struct og_ctrl_sysi *ctrl_sysi, unsigned char *msg, int rwl_status);
og_status OgSysiLogDebug(struct og_ctrl_sysi *ctrl_sysi, unsigned char *action, int nb_read, int nb_write);
ogint64_t OgSysiLogMicroTimeDiff(const struct timespec * const start, const struct timespec * const end);

og_status DOG_SYSI_WARN_UNUSED_RESULT SysiSetCurrentThreadReadLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi, int read_lock_number_owned);
int SysiGetCurrentThreadReadLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi);
og_status DOG_SYSI_WARN_UNUSED_RESULT SysiSetCurrentThreadWriteLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi, int write_lock_number_owned);
int SysiGetCurrentThreadWriteLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi);

