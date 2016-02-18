/**
 * Handling synchronization for servers and indexers (Multiple reader / One Writer)
 * Copyright (c) 2014 Pertimm by Brice Ruzand
 * Dev : January 2014
 * Version 2.0
 */
#include "ogm_sysi.h"

/**
 * Get the current number of read lock owned by the current thread
 *
 * @param ctrl_sysi ssyi handle
 * @return the number of read lock owned by the current thread
 */
int SysiGetCurrentThreadReadLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi)
{
  return OG_POINTER_TO_INT(pthread_getspecific(ctrl_sysi->read_lock_number_key));
}

/**
 * Set the current number of read lock owned by the current thread (for multiple lock
 *
 * @param ctrl_sysi ssyi handle
 * @param write_lock_number_owned the number of read lock owned by the current thread
 * @return execution status
 */
og_status SysiSetCurrentThreadReadLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi, int read_lock_number_owned)
{
  int status = pthread_setspecific(ctrl_sysi->read_lock_number_key, OG_INT_TO_POINTER(read_lock_number_owned));
  if (status != 0)
  {
    OgSysiLogError(ctrl_sysi, "SysiSetCurrentThreadReadLockNumberOwned: error while setting thread specific value",
        status);
    DPcErr;
  }

  DONE;
}

/**
 * Acquire a read access lock and goes on error if the time to aquire the lock is longer than a specified timeout.
 * You can acquire several lock recursively, don't forget to release each one.
 *
 * /!\ Don't forget to release it with OgSysiEndSearch /!\
 *
 * @param handle ssyi handle
 * @param timeout_ms timeout in ms
 * @return execution status
 */
PUBLIC(og_status) OgSysiReadLockWithTimeout(ogsysi_rwlock handle, int timeout_ms)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;

  ogint64_t mc_start = 0;
  int log_rw_lock_stat_on_flush = ctrl_sysi->log_rw_lock_stat_on_flush;
  if (log_rw_lock_stat_on_flush)
  {
    mc_start = OgMicroClock();
  }

  int read_lock_number = SysiGetCurrentThreadReadLockNumberOwned(ctrl_sysi);
  int write_lock_number = SysiGetCurrentThreadWriteLockNumberOwned(ctrl_sysi);
  if (read_lock_number == 0 && write_lock_number == 0)
  {

#ifdef OGM_SYSI_DEBUG
    IFE(OgSysiLogDebug(ctrl_sysi, "WAIT    Read ", read_lock_number, write_lock_number));
#endif

    int rwl_status = 0;

    int nb_try = 0;
    rwl_status = EAGAIN;

    if (timeout_ms > 0)
    {
      struct timespec abstime = OgSysiComputeTimeout(timeout_ms);

      // retry if status is EAGAIN
      while (rwl_status == EAGAIN && nb_try <= ctrl_sysi->max_lock_retry)
      {
        if (nb_try > 0) usleep(ctrl_sysi->lock_retry_wait_microsec);

        // try get the Write lock quickly
        rwl_status = pthread_rwlock_timedrdlock(ctrl_sysi->lock->rw_lock, &abstime);

        nb_try++;
      }

    }
    else
    {
      // retry if status is EAGAIN
      while (rwl_status == EAGAIN && nb_try <= ctrl_sysi->max_lock_retry)
      {
        if (nb_try > 0) usleep(ctrl_sysi->lock_retry_wait_microsec);

        rwl_status = pthread_rwlock_rdlock(ctrl_sysi->lock->rw_lock);

        nb_try++;
      }
    }

    if (rwl_status == ETIMEDOUT)
    {
      og_char_buffer msg_error[DPcPathSize];
      snprintf(msg_error, DPcPathSize, "OgSysiReadLock: SysiTimeout: "
          "Read access on rw_lock went on timeout after %d milliseconds", timeout_ms);
      IFE(OgSysiLogError(ctrl_sysi, msg_error, rwl_status));
      DPcErr;

    }
    else if (rwl_status != 0)
    {
      IFE(OgSysiLogError(ctrl_sysi, "OgSysiReadLock: error while acquiring Read access on rw_lock", rwl_status));
      DPcErr;
    }

#ifdef OGM_SYSI_DEBUG
    IFE(OgSysiLogDebug(ctrl_sysi, "ENTER   Read ", read_lock_number, write_lock_number));
#endif

    if (log_rw_lock_stat_on_flush)
    {
      // compute wait elapsed time
      ctrl_sysi->read_wait_elapsed += (OgMicroClock() - mc_start);
      ctrl_sysi->read_cmpt++;
    }

  }

#ifdef OGM_SYSI_DEBUG
  else
  {
    IFE(OgSysiLogDebug(ctrl_sysi, "REENTER Read ", read_lock_number, write_lock_number));
  }
#endif

  IFE(SysiSetCurrentThreadReadLockNumberOwned(ctrl_sysi, read_lock_number + 1));

  DONE;
}

/**
 * Acquire a read access lock.
 * You can acquire several lock recursively, don't forget to release each one.
 *
 * /!\ Don't forget to release it with OgSysiEndSearch /!\
 *
 * @param handle ssyi handle
 * @return execution status
 */
PUBLIC(og_status) OgSysiReadLock(ogsysi_rwlock handle)
{
  return OgSysiReadLockWithTimeout(handle, 0);
}

/**
 * Release a read access lock.
 *
 * @param handle ssyi handle
 * @return execution status
 */
PUBLIC(og_status) OgSysiReadUnLock(ogsysi_rwlock handle)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;

  int read_lock_number = SysiGetCurrentThreadReadLockNumberOwned(ctrl_sysi);
  int write_lock_number = SysiGetCurrentThreadWriteLockNumberOwned(ctrl_sysi);
  if (read_lock_number == 1 && write_lock_number == 0)
  {

    int rwl_status = pthread_rwlock_unlock(ctrl_sysi->lock->rw_lock);
    if (rwl_status != 0)
    {
      IFE(OgSysiLogError(ctrl_sysi, "OgSysiReadUnLock: error while releasing Read access on rw_lock", rwl_status));
      DPcErr;
    }

#ifdef OGM_SYSI_DEBUG
    IFE(OgSysiLogDebug(ctrl_sysi, "END     Read ", read_lock_number - 1, write_lock_number));
#endif

  }
  else if (read_lock_number == 0 && write_lock_number == 0)
  {
    IFE(OgSysiLogError(ctrl_sysi, "OgSysiReadUnLock: releasing previously stolen Read access on rw_lock", 0));
    DPcErr;
  }
  else if (read_lock_number == 0 && write_lock_number > 0)
  {
    IFE(OgSysiLogError(ctrl_sysi, "OgSysiReadUnLock: you try release Read access on rw_lock,"
        " but you own a Write access, check your lock/unlock sequence", 0));
    DPcErr;
  }

#ifdef OGM_SYSI_DEBUG
  else
  {
    IFE(OgSysiLogDebug(ctrl_sysi, "REEND   Read ", read_lock_number - 1, write_lock_number));
  }
#endif

  if (read_lock_number > 0)
  {
    IFE(SysiSetCurrentThreadReadLockNumberOwned(ctrl_sysi, read_lock_number - 1));
  }

  DONE;
}
