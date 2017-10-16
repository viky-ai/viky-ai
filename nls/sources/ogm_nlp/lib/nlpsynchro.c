/**
 *  Natural Language Processing library
 *  Safe synchronization feature
 *
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

#define DogNlpSynchroTestTimeoutLongSleep 30000 // 30s

og_status OgNlpSynchroUnLockAll(og_nlp_th ctrl_nlp_th)
{
  struct nlp_synchro_lock *poped_lock = g_queue_pop_tail(ctrl_nlp_th->current_rw_lock);
  while (poped_lock)
  {

    if (poped_lock->type == nlp_synchro_lock_type_read_lock)
    {
      og_status status = OgSysiReadUnLock(poped_lock->rwlock);

      // TODO log status
    }
    else if (poped_lock->type == nlp_synchro_lock_type_write_lock)
    {
      og_status status = OgSysiWriteUnLock(poped_lock->rwlock);
      // TODO log status
    }
    else
    {
      // TODO unsupported
    }

    poped_lock = g_queue_pop_tail(ctrl_nlp_th->current_rw_lock);
  }

  DONE;
}

static og_status push_lock(og_nlp_th ctrl_nlp_th, enum nlp_synchro_lock_type type, ogsysi_rwlock rwlock)
{
  struct nlp_synchro_lock *stack_lock = g_slice_new(struct nlp_synchro_lock);
  stack_lock->type = type;
  stack_lock->rwlock = rwlock;

  g_queue_push_tail(ctrl_nlp_th->current_rw_lock, stack_lock);

  DONE;
}

static og_status pop_lock(og_nlp_th ctrl_nlp_th, enum nlp_synchro_lock_type type, ogsysi_rwlock rwlock)
{
  struct nlp_synchro_lock *poped_lock = g_queue_pop_tail(ctrl_nlp_th->current_rw_lock);

  if (poped_lock->type != type)
  {
    // TODO check lock consistency
  }

  if (poped_lock->rwlock != rwlock)
  {
    // TODO check lock consistency
  }

  DONE;
}

og_status OgNlpSynchroReadLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(OgSysiReadLock(rwlock));

  IFE(push_lock(ctrl_nlp_th, nlp_synchro_lock_type_read_lock, rwlock));

  DONE;
}

og_status OgNlpSynchroReadUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(OgSysiReadUnLock(rwlock));

  IFE(pop_lock(ctrl_nlp_th, nlp_synchro_lock_type_read_lock, rwlock));

  DONE;
}

og_status OgNlpSynchroWriteLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(OgSysiWriteLock(rwlock));

  IFE(push_lock(ctrl_nlp_th, nlp_synchro_lock_type_write_lock, rwlock));

  DONE;
}

og_status OgNlpSynchroWriteUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(OgSysiWriteUnLock(rwlock));

  IFE(pop_lock(ctrl_nlp_th, nlp_synchro_lock_type_write_lock, rwlock));

  DONE;
}


og_status OgNlpSynchroTestSleepIfTimeoutNeeded(og_nlp_th ctrl_nlp_th, enum nlp_synchro_test_timeout_in timeout_in)
{
  if (ctrl_nlp_th->timeout_in != nlp_timeout_in_NONE)
  {
    if (ctrl_nlp_th->timeout_in == timeout_in)
    {
      OgSleep(DogNlpSynchroTestTimeoutLongSleep);

      // We should never reach this code
      NlpThrowErrorTh(ctrl_nlp_th, "OgNlpSynchroTestSleepIfTimeoutNeeded : timeout should have been reach");
      DPcErr;

    }
  }

  DONE;
}

PUBLIC(og_status) OgNlpSynchroTestRegisterTimeout(og_nlp_th ctrl_nlp_th, og_string timeout_in)
{
  if (timeout_in != NULL)
  {

    if (strcmp(timeout_in, "NlpPackageAddOrReplace") == 0)
    {
      ctrl_nlp_th->timeout_in = nlp_timeout_in_NlpPackageAddOrReplace;
    }
    else if (strcmp(timeout_in, "NlpPackageGet") == 0)
    {
      ctrl_nlp_th->timeout_in = nlp_timeout_in_NlpPackageGet;
    }

  }

  DONE;
}
