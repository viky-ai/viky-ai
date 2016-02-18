/**
 * Initialization for handling synchronization for servers and indexers (Multiple reader / One Writer)
 * Copyright (c) 2014 Pertimm by Brice Ruzand
 * Dev : January 2014
 * Version 2.0
 */
#include "ogm_sysi.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>

#define lock_size sizeof(struct og_ctrl_sysi_lock)

static int OgSysiInitSharedLock(struct og_ctrl_sysi *ctrl_sysi);

/**
 * Not thread safe !!!!
 */
PUBLIC(og_status) OgSysiChangeHMsg(ogsysi_rwlock handle, void *hMsg)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;

  assert(ctrl_sysi != NULL);
  assert(hMsg != NULL);

  ctrl_sysi->hmsg = hMsg;

  DONE;
}

/**
 * Init sysi handler
 *
 * @param param param
 * @return an sysi handler
 */
PUBLIC(ogsysi_rwlock) OgSysiInit(struct og_sysi_param *param)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) malloc(sizeof(struct og_ctrl_sysi));

  IFn(ctrl_sysi)
  {
    og_char_buffer erreur[DOgErrorSize];
    snprintf(erreur, DOgErrorSize, "OgSysiInit: malloc error on ctrl_sysi");
    OgErr(param->herr, erreur);
    return NULL;
  }
  memset(ctrl_sysi, 0, sizeof(struct og_ctrl_sysi));

  ctrl_sysi->herr = param->herr;
  ctrl_sysi->hmsg = param->hmsg;

  snprintf(ctrl_sysi->name, DPcPathSize, "%s", param->lock_name);
  ctrl_sysi->log_rw_lock_stat_on_flush = !param->disable_log_rw_lock_stat_on_flush;

  og_bool new_lock = FALSE;

  // share_between_process only use with pthread
  ctrl_sysi->share_between_process = param->share_between_process;
  ctrl_sysi->is_lock_owner = param->is_lock_owner;
  ctrl_sysi->max_lock_retry = 3;
  ctrl_sysi->lock_retry_wait_microsec = 1000;
  if (ctrl_sysi->share_between_process)
  {
    snprintf(ctrl_sysi->shared_key, DPcPathSize, "%s", param->shared_key);
    if (ctrl_sysi->shared_key[0] == 0)
    {
      OgSysiLogError(ctrl_sysi, "OgSysiInit: no shared key specified for memory shm_open", 0);
      return NULL;
    }
    else
    {
      int descriptor = -1;

      descriptor = shm_open(ctrl_sysi->shared_key, O_EXCL | O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
      if (descriptor < 0 && errno == EEXIST)
      {
        descriptor = shm_open(ctrl_sysi->shared_key, O_RDWR, S_IRUSR | S_IWUSR);
      }
      else
      {
        new_lock = TRUE;
      }

      if (descriptor < 0)
      {
        OgSysiLogError(ctrl_sysi, "OgSysiInit: error while setting up shared memory shm_open", errno);
        return NULL;
      }

      if (new_lock)
      {
        // resize shm
        int ftruncate_res = ftruncate(descriptor, lock_size);
        if (ftruncate_res != 0)
        {
          OgSysiLogError(ctrl_sysi, "OgSysiInit: error while setting up shared memory ftruncate", errno);
          return NULL;
        }
      }

      // map memory
      ctrl_sysi->lock = (struct og_ctrl_sysi_lock*) mmap(NULL, lock_size, PROT_WRITE | PROT_READ, MAP_SHARED,
          descriptor, 0);

      if (ctrl_sysi->lock == MAP_FAILED)
      {
        OgSysiLogError(ctrl_sysi, "OgSysiInit: mmap", errno);
        return NULL;
      }

      if (new_lock)
      {
        memset(ctrl_sysi->lock, 0, lock_size);
      }
    }
  }
  else
  {
    // init current lock to local lock
    ctrl_sysi->lock = ctrl_sysi->local_lock;
    memset(ctrl_sysi->lock, 0, lock_size);
    new_lock = TRUE;
  }

  {
    // init TLS specific Key
    int key_read_create_status = pthread_key_create(&ctrl_sysi->read_lock_number_key, NULL);
    if (key_read_create_status != 0)
    {
      OgSysiLogError(ctrl_sysi, "OgSysiInit: error while pthread_key_create (ctrl_sysi->read_lock_number_key)",
          key_read_create_status);
      return NULL;
    }
    int key_write_create_status = pthread_key_create(&ctrl_sysi->write_lock_number_key, NULL);
    if (key_write_create_status != 0)
    {
      OgSysiLogError(ctrl_sysi, "OgSysiInit: error while pthread_key_create (ctrl_sysi->write_lock_number_key)",
          key_write_create_status);
      return NULL;
    }

    if (new_lock || ctrl_sysi->lock->is_initialized == FALSE)
    {
      // init lock options
      IF(OgSysiInitSharedLock(ctrl_sysi))
      {
        return NULL;
      }
    }

    // try to get the lock even if the lock is own by an other onwer (it may have crash)
    if (ctrl_sysi->share_between_process && ctrl_sysi->is_lock_owner)
    {
      int rwl_status = 0;

      // get current time
      struct timeval tv[1];
      gettimeofday(tv, NULL);

      struct timespec abstime[1];
      // wait 10s max
      abstime->tv_nsec = 0;
      abstime->tv_sec = tv->tv_sec + 10;

      int nb_try = 0;
      rwl_status = EAGAIN;
      while (rwl_status == EAGAIN && nb_try <= ctrl_sysi->max_lock_retry)
      {
        if (nb_try > 0) usleep(ctrl_sysi->lock_retry_wait_microsec);

        // try get the Write lock quickly
        rwl_status = pthread_rwlock_timedwrlock(ctrl_sysi->lock->rw_lock, abstime);

        nb_try++;
      }

      // try get the Write lock quickly
      if (rwl_status == ETIMEDOUT)
      {
        // if cannot get write lock within 10s, it may be a dead lock cause a crash of a process who own the lock
        // so we reset shared lock
        memset(ctrl_sysi->lock, 0, lock_size);

        // init lock options
        IF(OgSysiInitSharedLock(ctrl_sysi))
        {
          return NULL;
        }

        // it not a real error no return NULL
        OgSysiLogError(ctrl_sysi,
            "OgSysiInit: warn: while pthread_rwlock_timedwrlock timeout after 10s, lock may own by a other process, erase shared lock",
            rwl_status);

      }
      else if (rwl_status == 0)
      {
        // unlock just acquire lock
        rwl_status = pthread_rwlock_unlock(ctrl_sysi->lock->rw_lock);
        if (rwl_status != 0)
        {
          OgSysiLogError(ctrl_sysi, "OgSysiInit: error while pthread_rwlock_unlock (unlock just acquire lock)",
              rwl_status);
          return NULL;
        }
      }
      else
      {
        OgSysiLogError(ctrl_sysi, "OgSysiInit: error while pthread_rwlock_timedwrlock,"
            " trying to acquire rw_lock has owner", rwl_status);
        return NULL;
      }

    }
  }

  return (ogsysi_rwlock) ctrl_sysi;
}

/**
 * Set right option on a process shared lock
 *
 * @param ctrl_sysi ssyi handle
 * @return status
 */
static int OgSysiInitSharedLock(struct og_ctrl_sysi *ctrl_sysi)
{
  int rwl_status = 0;
  pthread_rwlockattr_t attr[1];

  // init attributes
  rwl_status = pthread_rwlockattr_init(attr);
  if (rwl_status != 0)
  {
    OgSysiLogError(ctrl_sysi, "OgSysiInitSharedLock: error while pthread_rwlockattr_init", rwl_status);
    DPcErr;
  }

  // prefer writer to avoid writer's starvation
  rwl_status = pthread_rwlockattr_setkind_np(attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
  if (rwl_status != 0)
  {
    OgSysiLogError(ctrl_sysi, "OgSysiInitSharedLock: error while pthread_rwlockattr_setkind_np", rwl_status);
    goto OgSysiInitSharedLock_destroy;
  }

  // share RW lock between process
  if (ctrl_sysi->share_between_process)
  {
    rwl_status = pthread_rwlockattr_setpshared(attr, PTHREAD_PROCESS_SHARED);
    if (rwl_status != 0)
    {
      OgSysiLogError(ctrl_sysi,
          "OgSysiInitSharedLock: error while pthread_rwlockattr_setpshared(PTHREAD_PROCESS_SHARED)", rwl_status);
      goto OgSysiInitSharedLock_destroy;
    }
  }

  // init rw_lock
  rwl_status = pthread_rwlock_init(ctrl_sysi->lock->rw_lock, attr);
  if (rwl_status != 0)
  {
    OgSysiLogError(ctrl_sysi, "OgSysiInitSharedLock: error while pthread_rwlock_init", rwl_status);
    goto OgSysiInitSharedLock_destroy;
  }

  OgSysiInitSharedLock_destroy:
  {

    // destroy lock options
    int rwl_status_destroy = pthread_rwlockattr_destroy(attr);
    if (rwl_status_destroy != 0)
    {
      OgSysiLogError(ctrl_sysi, "OgSysiInitSharedLock: error while pthread_rwlockattr_destroy", rwl_status_destroy);
      DPcErr;
    }

    if (rwl_status != 0)
    {
      DPcErr;
    }
  }

  ctrl_sysi->lock->is_initialized = TRUE;

  DONE;
}

/**
 * Flush and free sysi handler
 *
 * @param param param
 * @return function status
 */
PUBLIC(int) OgSysiFlush(ogsysi_rwlock handle)
{
  struct og_ctrl_sysi *ctrl_sysi = (struct og_ctrl_sysi *) handle;
  if (ctrl_sysi == NULL) CONT;

  if (ctrl_sysi->share_between_process)
  {
    // release all owned lock, if any
    while (SysiGetCurrentThreadWriteLockNumberOwned(ctrl_sysi) > 0)
    {
      IF(OgSysiWriteUnLock((ogsysi_rwlock) ctrl_sysi))
      {
        continue;
      }
    }

    while (SysiGetCurrentThreadReadLockNumberOwned(ctrl_sysi) > 0)
    {
      IF(OgSysiReadUnLock((ogsysi_rwlock) ctrl_sysi))
      {
        continue;
      }
    }
  }

  if (ctrl_sysi->share_between_process)
  {
    if (ctrl_sysi->is_lock_owner)
    {
      int rwl_status = pthread_rwlock_destroy(ctrl_sysi->lock->rw_lock);
      if (rwl_status != 0)
      {
        OgSysiLogError(ctrl_sysi, "OgSysiFlush: error while destroying rw_lock", rwl_status);
      }
    }

    // detach shared memory
    munmap(ctrl_sysi->lock, sizeof(struct og_ctrl_sysi_lock));

    if (ctrl_sysi->is_lock_owner)
    {
      // remove shared memory for future use, but keep it until one process is mapped on it
      shm_unlink(ctrl_sysi->shared_key);
    }

  }
  else
  {
    int rwl_status = pthread_rwlock_destroy(ctrl_sysi->lock->rw_lock);
    if (rwl_status != 0)
    {
      OgSysiLogError(ctrl_sysi, "OgSysiFlush: error while destroying rw_lock", rwl_status);
    }
  }

  // init TLS specific Key
  pthread_key_delete(ctrl_sysi->read_lock_number_key);
  pthread_key_delete(ctrl_sysi->write_lock_number_key);

  // log stats
  OgSysiLogStat((ogsysi_rwlock) ctrl_sysi);

  DPcFree(ctrl_sysi);

  DONE;
}

/**
 * Compute timeout, add timeout_ms to current time
 *
 * @param timeout_ms
 * @return timespec future timer
 */
struct timespec OgSysiComputeTimeout(long timeout_ms)
{
  // get current time
  struct timeval tv[1];
  gettimeofday(tv, NULL);

  // constant conversion
  long msec_in_a_sec = 1E3;
  long nsec_in_a_usec = 1E3;
  long nsec_in_a_msec = 1E6;
  long nsec_in_a_sec = 1E9;

  // add timeout_ms to current time
  struct timespec abstime[1];
  abstime->tv_sec = tv->tv_sec + timeout_ms / msec_in_a_sec;
  abstime->tv_nsec = tv->tv_usec * nsec_in_a_usec + ((timeout_ms % msec_in_a_sec) * nsec_in_a_msec);

  // check nano sec are over 1 seconds
  if (abstime->tv_nsec >= nsec_in_a_sec)
  {
    abstime->tv_sec += abstime->tv_nsec - nsec_in_a_sec;
    abstime->tv_nsec++;
  }

  return abstime[0];
}
