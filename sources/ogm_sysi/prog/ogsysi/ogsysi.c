/*
 *  This is main procedure for ogsysi.exe
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : February 2010
 *  Version 1.0
 */
#include <logsysi.h>
#include <logmsg.h>
#include <getopt.h>
#include <logmisc.h>
#include <logpsync.h>
#include <pthread.h>

/* definition de valeurs par d�fauts non configurables */
#define OG_SYSI_MAX_THREAD   1001

/* structure de controle du programme */
struct og_info
{
  void *hmsg, *herr;
  ogmutex_t *hmutex;
  struct og_sysi_param param[1];
  ogsysi_rwlock hsysi;
  int nb_search_threads;
  int nb_index_threads;

  int shared_lock_process;
  int shared_lock_process_owner;

  THREAD_MUTEX mutex;
};

/* default function to define */
static int OgUse(struct og_info *);
static void OgExit(struct og_info *, int);

/* functions for using main api */
static int Sysi(struct og_info *);
static int indexThread(void *hsysi);
static int searchThread(void *hsysi);

/* MAIN */
int main(int argc, char * argv[])
{
  char carlu;
  struct og_info cinfo, *info = &cinfo;
  struct og_sysi_param *param = NULL;
  int optionIndex = 0;
  int result = 1;
  time_t ltime;

  /* definition of program options */
  // @formatter:off
  struct option longOptions[] = { { "nb-index-threads", required_argument, NULL, 'i' }, { "nb-search-threads",
  required_argument, NULL, 's' }, { "shared-lock-process", no_argument, NULL, 'x' }, { "shared-lock-process_owner",
  no_argument, NULL, 'o' }, { "help", no_argument, NULL, 'h' }, { 0, 0, 0, 0 } };
  // @formatter:on

  /* initialization of program info structure */
  memset(info, 0, sizeof(struct og_info));
  IFn(info->hmsg=OgLogInit("ogsysi", "ogsysi", DOgMsgTraceMinimal+DOgMsgTraceMemory, DOgMsgLogFile)) goto main_end;

  info->herr = OgLogGetErr(info->hmsg);
  info->hmutex = OgLogGetMutex(info->hmsg);
  info->nb_search_threads = 1;
  info->nb_index_threads = 1;

  /* initialization of main api init parameters structure */
  param = info->param;
  memset(param, 0, sizeof(struct og_sysi_param));
  param->hmsg = info->hmsg;
  param->herr = info->herr;
  sprintf(param->lock_name, "%s", "sysi_prog");

  /* parsing options */
  while ((carlu = getopt_long(argc, argv, "i:s:xoh?", longOptions, &optionIndex)) != EOF)
  {
    switch (carlu)
    {
      case 0:
      break;
      case 'i':
        info->nb_index_threads = atoi(optarg);
        if (info->nb_index_threads > OG_SYSI_MAX_THREAD)
        {
          OgUse(info);
          goto main_end;
        }
      break;
      case 's':
        info->nb_search_threads = atoi(optarg);
        if (info->nb_search_threads > OG_SYSI_MAX_THREAD)
        {
          OgUse(info);
          goto main_end;
        }
      break;
      case 'x':
        info->shared_lock_process = TRUE;
      break;
      case 'o':
        info->shared_lock_process_owner = TRUE;
      break;
      case 'h':
      case '?':
        OgUse(info);
        goto main_end;
      break;
    }
  }

  // test multiprocess lock
  if (info->shared_lock_process)
  {
    param->share_between_process = TRUE;
    if (info->shared_lock_process_owner)
    {
      param->is_lock_owner = TRUE;
    }
    sprintf(param->shared_key, "%s", "sysi_prog");
  }

  /* starting log */
  time(&ltime);
  fprintf(stdout, "\nProgram %s starting with pid %d at %.24s (%s%s)\n", OgLogGetModuleName(param->hmsg), getpid(),
      OgGmtime(&ltime), info->shared_lock_process ? "shared lock" : "private lock",
      info->shared_lock_process_owner ? ", lock owner" : "");
  fflush(stdout);

  IF(Sysi(info))
  {
    goto main_end;
  }

  /* ending log */
  time(&ltime);
  fprintf(stdout, "\nProgram %s exiting at %.24s\n", OgLogGetModuleName(param->hmsg), OgGmtime(&ltime));
  fflush(stdout);

  result = 0;
  main_end:
  /* flushing program */
  OgExit(info, result);

  return (1);
}

STATICF(int) OgUse(struct og_info *info)
{
  char buffer[8192];
  int ibuffer = 0;

  ibuffer += sprintf(buffer, "Usage : ogsysi [options]\n");
  ibuffer += sprintf(buffer + ibuffer, "options are:\n");
  ibuffer += sprintf(buffer + ibuffer,
      "   -i,  --nb-index-threads=n: sets number of indexing threads (default 1, max 1000)\n");
  ibuffer += sprintf(buffer + ibuffer,
      "   -s,  --nb-search-threads=n: sets number of search threads (default 1, max 1000)\n");
  ibuffer += sprintf(buffer + ibuffer,
      "   -x,  --shared-lock-process: share the lock between process (default FALSE)\n");
  ibuffer += sprintf(buffer + ibuffer,
      "   -o,  --shared-lock-process-owner: set this process as owner (can resolv DeadLock) (default FALSE)\n");
  ibuffer += sprintf(buffer + ibuffer, "   -h,  --help prints this message\n");

  OgLogConsole(info->hmsg, "%.*s", ibuffer, buffer);

  DONE;
}

static void OgExit(struct og_info *info, int result)
{
  char *error;

  if (info->hmsg)
  {
    error = OgLogLastErr(info->hmsg);
    if (error[0])
    {
      IF(OgMsg(info->hmsg,"",DOgMsgDestInLog+DOgMsgSeverityError,"LAST ERROR: %s",OgLogLastErr(info->hmsg)))
      {
        exit(1);
      }
    }
    IF(OgLogFlush(info->hmsg))
    {
      exit(1);
    }
    else
    {
      exit(result);
    }
  }
  else
  {
    exit(1);
  }

  exit(result);
}

static int Sysi(struct og_info *info)
{
  THREAD_ID index_thread[OG_SYSI_MAX_THREAD];
  THREAD_ID search_thread[OG_SYSI_MAX_THREAD];
  int i;

  IFn(info->hsysi=OgSysiInit(info->param)) DPcErr;

  for (i = 0; i < info->nb_search_threads; i++)
  {
    OgNewThread(searchThread, (void *) info->hsysi, search_thread + i);  // start search
  }
  for (i = 0; i < info->nb_index_threads; i++)
  {
    OgNewThread(indexThread, (void *) info->hsysi, index_thread + i);  // start search(NULL);
  }

  if (info->nb_search_threads > 0)
  {
    OgJoinThread(info->nb_search_threads, search_thread);
  }

  if (info->nb_index_threads > 0)
  {
    OgJoinThread(info->nb_index_threads, index_thread);
  }

  fprintf(stdout, "STOP ALL\n");
  fflush(stdout);

  IFE(OgSysiFlush(info->hsysi));
  DONE;
}

static int indexThread(void *hsysi)
{
  ogsysi_rwlock sysi = (ogsysi_rwlock) hsysi;
  unsigned int TID = (unsigned int) pthread_self();
  int sleepTime;

  int i = 20;
  while (i > 0)
  {
    i--;
    sleepTime = rand() % 100;
    OgUsleep(1000 * (1000 + sleepTime));
    fprintf(stdout, "%d %X Request Indexing\n", getpid(), TID);
    fflush(stdout);

    IFE(OgSysiWriteLock(sysi));

    fprintf(stdout, "%d %X Indexing\n", getpid(), TID);
    fflush(stdout);

    // try recursive lock
    IFE(OgSysiWriteLock(sysi));

    fprintf(stdout, "%d %X Indexing 2 ...\n", getpid(), TID);
    fflush(stdout);

    OgUsleep(1000 * 40);  // wait 0,4 sec

    fprintf(stdout, "%d %X Indexing 2 END\n", getpid(), TID);
    fflush(stdout);

    IFE(OgSysiWriteUnLock(sysi));

    fprintf(stdout, "%d %X Indexing END\n", getpid(), TID);
    fflush(stdout);

    IFE(OgSysiWriteUnLock(sysi));

  }

  fprintf(stdout, "%d %X Indexing Completed\n", getpid(), TID);
  fflush(stdout);

  DONE;
}

static int searchThread(void *hsysi)
{
  ogsysi_rwlock sysi = (ogsysi_rwlock) hsysi;
  unsigned int TID = (unsigned int) pthread_self();
  int sleepTime;

  int i = 1000;
  while (i > 0)
  {
    i--;

    fprintf(stdout, "%d %X Request Searching\n", getpid(), TID);
    fflush(stdout);

    IFE(OgSysiReadLock(sysi));

    fprintf(stdout, "%d %X Searching\n", getpid(), TID);
    fflush(stdout);

    OgUsleep(1000 * 20);  // wait 0,2 sec

    fprintf(stdout, "%d %X Searching END\n", getpid(), TID);
    fflush(stdout);

    IFE(OgSysiReadUnLock(sysi));
    sleepTime = rand() % 50;
    OgUsleep(1000 * sleepTime);
  }

  fprintf(stdout, "%d %X Searching Completed\n", getpid(), TID);
  fflush(stdout);

  DONE;
}
