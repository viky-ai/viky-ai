#include <lpcosys.h>

#if DPcSystem==DPcSystemWin32
#include <winsock2.h>
#include <windows.h>
#include <winbase.h>
#include <process.h>
#include <errno.h>
#else
#include <pthread.h> // MUST be first 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <loggen.h>


/** @ingroup libpsyncAPI
 * create/initialise a process mutex or retrieve existing mutex 
 *
 * \param mutex_name 
 * \return -1: error, >0: mutex id
 **/
PUBLIC(PROCESS_MUTEX) OgInitProcMutex(char *mutex_name) {
  PROCESS_MUTEX	id; 
#if DPcSystem==DPcSystemWin32
  id = CreateMutex(NULL, FALSE, mutex_name);          // Creates or opens a named mutex object without locking 
  if (id == NULL) return (PROCESS_MUTEX)-1;
#else
  key_t 	key;
  unsigned int  i;
  union semun {
    int val;
    struct semid_ds *buf;
    ushort * array;
  } semopts;
  
  // don't use ftok func to generate key to be undependent of a file
  key=0;
  for (i=0; i<strlen(mutex_name); i++) key+=mutex_name[i];
  // check if semaphore exists
  if((id = semget(key, 0, 0666))>=0) return id; 
  // create semaphore and give permissions to the world
  if((id = semget(key, 1, IPC_CREAT|IPC_EXCL|0666))  == -1)  return -1;
  // Initialise semaphore value to 1 (eq 0) 
  semopts.val = 1; 
  semctl(id, 0, SETVAL, semopts);                     
#endif
 return id;
}

/** @ingroup libpsyncAPI
 * Locks the specified mutex. If the mutex is already locked, the
 * calling process blocks until the mutex becomes available. 
 *
 * \partam mutex_id
 * \return -1: error, 0: success
 **/
PUBLIC(int) OgLockProcMutex(PROCESS_MUTEX mutex_id) {
#if DPcSystem==DPcSystemWin32
  DWORD dwWaitResult = WaitForSingleObject (mutex_id, INFINITE);   
  switch (dwWaitResult) {
    case WAIT_OBJECT_0:                    // Got ownership
    case WAIT_ABANDONED: return 0;  break; // Got ownership of an abandoned mutex object	
    case WAIT_TIMEOUT:   return 1;  break; // Cannot get mutex object ownership due to time-out
  }
  return -1;
#else
  struct sembuf sem_lock={ 0, -1, SEM_UNDO};
  sem_lock.sem_num = 0;
  if((semop(mutex_id, &sem_lock, 1)) == -1) return -1;
  DONE;
#endif
}

/** @ingroup libpsyncAPI
 * Try to lock the specified mutex. If the mutex is already locked,
 * an error is returned. Otherwise, this operation returns with the
 * mutex in the locked state with the calling thread as its owner.
 *
 * \param mutex_id
 * \return -1: error, 0: lock success, 1: lock failed
 **/
PUBLIC(int) OgTryLockProcMutex(PROCESS_MUTEX mutex_id) {
#if DPcSystem==DPcSystemWin32
  DWORD dwWaitResult = WaitForSingleObject (mutex_id,0);   
  switch (dwWaitResult) {
  case WAIT_OBJECT_0:                    // Got ownership
  case WAIT_ABANDONED: return 0;  break; // Got ownership of an abandoned mutex object	
  case WAIT_TIMEOUT:   return 1;  break; // Cannot get mutex object ownership due to time-out
  }
  return -1;
#else
struct sembuf operations[1];
  int retval; 

  operations[0].sem_flg = IPC_NOWAIT;   // no wait if mutex is not available
  operations[0].sem_num = 0;            // Set up the lockbuf structure.
  operations[0].sem_op = -1;            // Which operation? Subtract 1 from semaphore value :
  operations[0].sem_flg |= SEM_UNDO;    // set SEM_UNDO flag to release lock at exit 
  retval = semop(mutex_id, operations, 1);
  if (retval == -1 && errno == EAGAIN) return 1;
  return retval;
#endif
}

/** @ingroup libpsyncAPI
 * Process mutex unlock
 *
 * \return -1: error, 0 : success
 **/
PUBLIC(int) OgUnlockProcMutex(PROCESS_MUTEX mutex_id) {
#if DPcSystem==DPcSystemWin32
  if (ReleaseMutex(mutex_id) != 0) return 0;
  return -1;
#else
  struct sembuf sem_unlock={ 0, 1, SEM_UNDO};
  int semval;

  // Is the semaphore set locked? 
  if ((semval =  semctl(mutex_id, 0, GETVAL, 0)==1)) return 1;

  sem_unlock.sem_num = 0;
  // Attempt to lock the semaphore set 
  if((semop(mutex_id, &sem_unlock, 1)) == -1) return -1;
  DONE;
#endif
}




