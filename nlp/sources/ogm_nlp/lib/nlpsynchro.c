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
  struct nlp_synchro_current_lock *current_locks = ctrl_nlp_th->current_lock;
  for (int i = 0; i < current_locks->used; i++)
  {
    struct nlp_synchro_lock *poped_lock = current_locks->lock + i;

    if (poped_lock->type == nlp_synchro_lock_type_read)
    {
      og_status status = OgSysiReadUnLock(poped_lock->rwlock);
      IF(status)
      {
        NlpLog(DOgNlpTraceSynchro, "OgNlpSynchroUnLockAll : OgSysiReadUnLock failed","")
      }

    }
    else if (poped_lock->type == nlp_synchro_lock_type_write)
    {
      og_status status = OgSysiWriteUnLock(poped_lock->rwlock);
      IF(status)
      {
        NlpLog(DOgNlpTraceSynchro, "OgNlpSynchroUnLockAll : OgSysiWriteUnLock failed","")
      }
    }
    else
    {
      NlpLog(DOgNlpTraceSynchro, "OgNlpSynchroUnLockAll : unsupported lock type : %d", poped_lock->type)
    }

  }
  current_locks->used = 0;

  DONE;
}

static og_status push_lock(og_nlp_th ctrl_nlp_th, enum nlp_synchro_lock_type type, ogsysi_rwlock rwlock)
{
  struct nlp_synchro_current_lock *current_locks = ctrl_nlp_th->current_lock;
  if (current_locks->used >= DOgNlpMaximumOwnedLock)
  {
    // We should never reach this code
    NlpThrowErrorTh(ctrl_nlp_th, "push_lock : failed to many lock >= %d", current_locks->used);
    DPcErr;
  }

  struct nlp_synchro_lock *pushed_lock = current_locks->lock + current_locks->used++;
  pushed_lock->type = type;
  pushed_lock->rwlock = rwlock;

  DONE;
}

static og_status pop_lock(og_nlp_th ctrl_nlp_th, enum nlp_synchro_lock_type type, ogsysi_rwlock rwlock)
{
  struct nlp_synchro_current_lock *current_locks = ctrl_nlp_th->current_lock;
  if (current_locks->used <= 0)
  {
    // We should never reach this code
    NlpThrowErrorTh(ctrl_nlp_th, "pop_lock : no more lock to pop");
    DPcErr;
  }

  struct nlp_synchro_lock *poped_lock = current_locks->lock + (--current_locks->used);

  if (poped_lock->rwlock != rwlock)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "pop_lock : unlock order mismatch, expected lock '%s' vs '%s'", OgSysiGetName(rwlock),
        OgSysiGetName(poped_lock->rwlock));
    DPcErr;
  }

  if (poped_lock->type != type)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "pop_lock : unlock order mismatch, on lock '%s' expected access type %d vs %d",
        OgSysiGetName(rwlock), type, poped_lock->type);
    DPcErr;
  }

  DONE;
}

og_status OgNlpSynchroReadLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(push_lock(ctrl_nlp_th, nlp_synchro_lock_type_read, rwlock));

  IFE(OgSysiReadLock(rwlock));

  DONE;
}

og_status OgNlpSynchroReadUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(OgSysiReadUnLock(rwlock));

  IFE(pop_lock(ctrl_nlp_th, nlp_synchro_lock_type_read, rwlock));

  DONE;
}

og_status OgNlpSynchroWriteLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{

  IFE(push_lock(ctrl_nlp_th, nlp_synchro_lock_type_write, rwlock));

  IFE(OgSysiWriteLock(rwlock));

  DONE;
}

og_status OgNlpSynchroWriteUnLock(og_nlp_th ctrl_nlp_th, ogsysi_rwlock rwlock)
{
  IFE(OgSysiWriteUnLock(rwlock));

  IFE(pop_lock(ctrl_nlp_th, nlp_synchro_lock_type_write, rwlock));

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
    else if (strcmp(timeout_in, "NlpInterpretRequestParse") == 0)
    {
      ctrl_nlp_th->timeout_in = nlp_timeout_in_NlpInterpretRequestParse;
    }
    else if (strcmp(timeout_in, "NlpMatchExpressions") == 0)
    {
      ctrl_nlp_th->timeout_in = nlp_timeout_in_NlpMatchExpressions;
    }
    else if (strcmp(timeout_in, "NlpRequestInterpretationBuild") == 0)
    {
      ctrl_nlp_th->timeout_in = nlp_timeout_in_NlpRequestInterpretationBuild;
    }
    else if (strcmp(timeout_in, "NlpPackageGet") == 0)
    {
      ctrl_nlp_th->timeout_in = nlp_timeout_in_NlpPackageGet;
    }

  }

  DONE;
}
