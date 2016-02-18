/**
 *  Header for library ogm_sysi.dll
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : February 2010
 *  Version 1.0
 */
#ifndef _LOGSYSIALIVE_
#include <lpcgentype.h>
#include <loggen.h>
#include <logthr.h>

#define DOgSysiBanner  "ogm_sysi V1.00, Copyright (c) 2010 Pertimm"
#define DOgSysiVersion 100

/** Trace levels **/
#define DOgSysiTraceMinimal          0x1
#define DOgSysiTraceMemory           0x2

// warn if function returned value is not checked
#define DOG_SYSI_WARN_UNUSED_RESULT __attribute__((warn_unused_result))


struct og_sysi_param
{
  void *herr, *hmsg;

  og_char_buffer lock_name[DPcPathSize];

  /** boolean to enable log sysi stat on flush : default is enable*/
  og_bool disable_log_rw_lock_stat_on_flush;

  // share lock between process
  og_bool share_between_process;
  og_bool is_lock_owner;
  og_char_buffer shared_key[DPcPathSize];

};


//  RW-lock (inter Process)
typedef struct _ogsysi_rwlock *ogsysi_rwlock;
DEFPUBLIC(ogsysi_rwlock) DOG_SYSI_WARN_UNUSED_RESULT OgSysiInit(struct og_sysi_param *param);
DEFPUBLIC(og_status) OgSysiChangeHMsg(ogsysi_rwlock rwlock, void *hMsg);

DEFPUBLIC(og_status) DOG_SYSI_WARN_UNUSED_RESULT OgSysiWriteLock(ogsysi_rwlock rwlock);
DEFPUBLIC(og_status) DOG_SYSI_WARN_UNUSED_RESULT OgSysiWriteUnLock(ogsysi_rwlock rwlock);
DEFPUBLIC(og_status) DOG_SYSI_WARN_UNUSED_RESULT OgSysiReadLock(ogsysi_rwlock rwlock);
DEFPUBLIC(og_status) DOG_SYSI_WARN_UNUSED_RESULT OgSysiReadUnLock(ogsysi_rwlock rwlock);

/**
 * OgSysiWriteSetTimeout and OgSysiReadSetTimeout : should be use only to avoid lock in emergency case (crash recovery)
 */
DEFPUBLIC(og_status) OgSysiWriteSetTimeout(ogsysi_rwlock rwlock, int timeout_sec);
DEFPUBLIC(og_status) OgSysiReadSetTimeout(ogsysi_rwlock rwlock, int timeout_sec);

DEFPUBLIC(og_status) OgSysiFlush(ogsysi_rwlock rwlock);
DEFPUBLIC(og_status) OgSysiLogStatEnable(ogsysi_rwlock rwlock, int enable);
DEFPUBLIC(og_status) OgSysiLogStat(ogsysi_rwlock rwlock);

#define _LOGSYSIALIVE_
#endif

