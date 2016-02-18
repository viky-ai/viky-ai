/**
 * Handling synchronization for servers and indexers (Multiple reader / One Writer)
 * Copyright (c) 2014 Pertimm by Brice Ruzand
 * Dev : January 2014
 * Version 2.0
 */
#include "ogm_sysi.h"

/**
 * Get the current number of write lock owned by the current thread
 *
 * @param ctrl_sysi ssyi handle
 * @return the number of write lock owned by the current thread
 */
int SysiGetCurrentThreadWriteLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi)
{
  return OG_POINTER_TO_INT(pthread_getspecific(ctrl_sysi->write_lock_number_key));
}

/**
 * Set the current number of write lock owned by the current thread (for multiple lock
 *
 * @param ctrl_sysi ssyi handle
 * @param write_lock_number_owned the number of write lock owned by the current thread
 * @return execution status
 */
og_status SysiSetCurrentThreadWriteLockNumberOwned(struct og_ctrl_sysi *ctrl_sysi, int write_lock_number_owned)
{
  int status = pthread_setspecific(ctrl_sysi->write_lock_number_key, OG_INT_TO_POINTER(write_lock_number_owned));
  if (status != 0)
  {
    OgSysiLogError(ctrl_sysi, "SysiSetCurrentThreadWriteLockNumberOwned: error while setting thread specific value",
        status);
    DPcErr;
  }

  DONE;
}

/**
 * Acquire a write access lock.
 *
 * /!\ Don't forget to release it with OgSysiEndIndex /!\
 * /!\ You can't ask for write access if you already have read access /!\
 *
 * @param handle ssyi handle
 * @return execution status
 */
PUBLIC(og_status) OgSysiWriteLock(ogsysi_rwlock handle)
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

  if (read_lock_number > 0)
  {
    OgSysiLogError(ctrl_sysi, "OgSysiWriteLock: Cannot acquire Write access on rw_lock"
        " with Read access already owned", 0);
    DPcErr;
  }
  else if (write_lock_number == 0)
  {

#ifdef OGM_SYSI_DEBUG
    OgSysiLogDebug(ctrl_sysi, "WAIT    Write", read_lock_number, write_lock_number);
#endif

    int rwl_status = 0;

    int nb_try = 0;
    rwl_status = EAGAIN;

    if (ctrl_sysi->write_access_timeout > 0)
    {
      // get current time
      struct timeval tv[1];
      gettimeofday(tv, NULL);

      struct timespec abstime[1];
      abstime->tv_nsec = 0;
      abstime->tv_sec = tv->tv_sec + ctrl_sysi->write_access_timeout;

      // retry if status is EAGAIN
      while (rwl_status == EAGAIN && nb_try <= ctrl_sysi->max_lock_retry)
      {
        if (nb_try > 0) usleep(ctrl_sysi->lock_retry_wait_microsec);

        // try get the Write lock quickly
        rwl_status = pthread_rwlock_timedwrlock(ctrl_sysi->lock->rw_lock, abstime);

        nb_try++;
      }

    }
    else
    {
      // retry if status is EAGAIN
      while (rwl_status == EAGAIN && nb_try <= ctrl_sysi->max_lock_retry)
      {
        if (nb_try > 0) usleep(ctrl_sysi->lock_retry_wait_microsec);

        rwl_status = pthread_rwlock_wrlock(ctrl_sysi->lock->rw_lock);

        nb_try++;
      }

    }

    if (rwl_status == ETIMEDOUT)
    {
      og_char_buffer msg_error[DPcPathSize];
      snprintf(msg_error, DPcPathSize, "OgSysiWriteLock: acquiring Write access on rw_lock"
          " (lock stolen after %d seconds)", ctrl_sysi->write_access_timeout);
      OgSysiLogError(ctrl_sysi, msg_error, rwl_status);
      DPcErr;
    }
    else if (rwl_status != 0)
    {
      OgSysiLogError(ctrl_sysi, "OgSysiWriteLock: error while acquiring Write access on rw_lock", rwl_status);
      DPcErr;
    }

    if (log_rw_lock_stat_on_flush)
    {
      // compute owned elapsed time
      ctrl_sysi->write_owned_last_write = OgMicroClock();
      ctrl_sysi->write_wait_elapsed += (ctrl_sysi->write_owned_last_write - mc_start);
      ctrl_sysi->write_cmpt++;
    }
    else
    {
      ctrl_sysi->write_owned_last_write = -1;
    }

#ifdef OGM_SYSI_DEBUG
    IFE(OgSysiLogDebug(ctrl_sysi, "ENTER   Write", read_lock_number, write_lock_number));
#endif

  }

#ifdef OGM_SYSI_DEBUG
  else
  {
    IFE(OgSysiLogDebug(ctrl_sysi, "REENTER Write", read_lock_number, write_lock_number));
  }
#endif

  IFE(SysiSetCurrentThreadWriteLockNumberOwned(ctrl_sysi, write_lock_number + 1));

  DONE;
}

/**
 * Release a write access lock.
 *
 * @param handle sysi handle
 * @return execution status
 */
PUBLIC(og_status) OgSysiWriteUnLock(ogsysi_rwlock handle)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;

  int read_lock_number = SysiGetCurrentThreadReadLockNumberOwned(ctrl_sysi);
  int write_lock_number = SysiGetCurrentThreadWriteLockNumberOwned(ctrl_sysi);
  if (write_lock_number == 1)
  {

    if (ctrl_sysi->log_rw_lock_stat_on_flush && ctrl_sysi->write_owned_last_write != -1)
    {
      // compute wait elapsed time
      ctrl_sysi->write_owned_elapsed += (OgMicroClock() - ctrl_sysi->write_owned_last_write);
    }

    int rwl_status = pthread_rwlock_unlock(ctrl_sysi->lock->rw_lock);
    if (rwl_status != 0)
    {
      IFE(OgSysiLogError(ctrl_sysi, "OgSysiWriteUnLock: error while releasing Write access on rw_lock", rwl_status));
      DPcErr;
    }

    if (read_lock_number > 0)
    {
      // auto release read lock when releasing write lock
      IFE(SysiSetCurrentThreadReadLockNumberOwned(ctrl_sysi, 0));

      IFE(
          OgSysiLogError(ctrl_sysi,
              "OgSysiWriteUnLock: auto releasing Read access when releasing Write access on rw_lock,"
                  " check your lock/unlock sequence", 0));
      DPcErr;
    }

#ifdef OGM_SYSI_DEBUG
    IFE(OgSysiLogDebug(ctrl_sysi, "END     Write", read_lock_number, write_lock_number - 1));
#endif

  }
  else if (write_lock_number == 0 && read_lock_number == 0)
  {
    IFE(OgSysiLogError(ctrl_sysi, "OgSysiWriteUnLock: releasing previously stolen Write access on rw_lock", 0));
  }
  else if (write_lock_number == 0 && read_lock_number > 0)
  {
    IFE(
        OgSysiLogError(ctrl_sysi,
            "OgSysiWriteUnLock: you try release Write access on rw_lock, but you own a Read access,"
                " check your lock/unlock sequence", 0));
  }

#ifdef OGM_SYSI_DEBUG
  else
  {
    IFE(OgSysiLogDebug(ctrl_sysi, "REEND   Write", read_lock_number, write_lock_number - 1));
  }
#endif

  if (write_lock_number > 0)
  {
    IFE(SysiSetCurrentThreadWriteLockNumberOwned(ctrl_sysi, write_lock_number - 1));
  }

  DONE;
}

/**
 * Set Write acquiring timeout, to avoid dead lock in emergency case only
 *
 * @param handle ssyi handle
 * @param timeout_sec timeout in seconds
 */
PUBLIC(og_status) OgSysiWriteSetTimeout(ogsysi_rwlock handle, int timeout_sec)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;
  ctrl_sysi->write_access_timeout = timeout_sec;

  DONE;
}

