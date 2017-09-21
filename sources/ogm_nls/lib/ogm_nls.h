/*
 *  Internal header for Natural Language Server library
 *  Copyright (c) 2017 Pertimm by Patrick Constant
 *  Dev: August 2017
 *  Version 1.0
 */
#include <lognls.h>

#include <logaddr.h>
#include <loguci.h>
#include <logpath.h>
#include <logthr.h>
#include <logheap.h>
#include <loggen.h>
#include <glib.h>

#include <jansson.h>

#include <uriparser/Uri.h>
#include <uriparser/UriBase.h>

#include <yajl/yajl_gen.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_tree.h>

#include <string.h>

typedef struct og_listening_thread og_listening_thread ;

#define DOgNlsPortNumber  9345

struct timeout_conf_context
{
  char *timeout_name; /** name of the timeout : answer_timeout, socket_read_timeout...*/
  int old_timeout; /** timeout from previous conf */
  int default_timeout; /** default timeout value */
};

struct og_nls_env
{
  char listenning_address[DPcPathSize];
  int listenning_port;
};

struct og_nls_conf
{

  struct og_nls_env env[1];

  int max_listening_threads;
  int max_parallel_threads;
  char data_directory[DPcPathSize];
  int permanent_threads;

  int max_request_size;

  /** Global timeout determining socket_read_timeout and request_processing_timeout*/
  int answer_timeout;

  /** backlog timeout. If only answer_timeout is specified, backlog_timeout is 10 % of answer_timeout*/
  int backlog_timeout;

  /** socket timeout. If only answer_timeout is specified, socket_read_timeout is 10 % of answer_timeout*/
  int socket_read_timeout;

  /** Timeout for the request. If only answer_timeout is specified, request_processing_timeout is 80 % of answer_timeout*/
  int request_processing_timeout;

  /** backlog timeout specified only for indexing requests. If not specified, it has the same value as backlog_timeout*/
  int backlog_indexing_timeout;
  int backlog_max_pending_requests;

};

struct json_object
{
  yajl_gen yajl_gen;

  og_bool error_detected;

  /**  Yajl Json json buffer **/
  og_heap hb_json_buffer;

};

/** data structure for a listening thread **/
struct og_listening_thread
{

  /** Error handler */
  void *herr;

  /** Message handler */
  void *hmsg;

  ogmutex_t *hmutex;
  struct og_ctrl_nls *ctrl_nls;
  struct og_loginfo loginfo[1];
  int ID, running, looping, disabled, state;
  int request_running, request_running_start, request_running_time;
  ogthread_t IT;
  unsigned hsocket_in;
  int connection_closed;
  struct sockaddr_in socket_in;

  int content_start, content_length;

  struct og_ucisr_output output[1];
  int valid_request;

  void *hucis;

  /** for permanent lt threads **/
  ogsem_t hsem[1];
  int must_stop;

  ogint64_t t0, t1, t2, t3, ot3;

  pthread_t current_thread;
  struct json_object json[1];
  yajl_handle parser;

};

/** nlsmt.c **/
#define DOgNlsClockTick        10

/** data structure for the maintenance thread **/
struct og_maintenance_thread
{
  void *herr;
  void *hmsg;
  ogmutex_t *hmutex;

  struct og_ctrl_nls *ctrl_nls;

  ogthread_t IT;
};

struct og_ctrl_nls
{
  void *herr, *hmsg;
  ogmutex_t *hmutex;
  struct og_loginfo loginfo[1];
  char WorkingDirectory[DPcPathSize];
  char configuration_file[DPcPathSize];
  int icwd;
  unsigned char cwd[DPcPathSize];
  struct og_nls_conf conf[1];

  struct og_ucisr_output output[1];
  struct og_ucisw_input winput[1];
  struct og_ucisr_input input[1];

  char sremote_addr[DPcPathSize];
  int must_stop;

  void *haddr, *hucis;

  struct og_listening_thread *Lt;
  int LtNumber;

  ogsem_t hsem_run3[1];
  /** Mutex to choose current lt */
  ogmutex_t hmutex_run_lt[1];

  struct og_maintenance_thread mt;

};

typedef struct
{
    const char * key;
    const char * value;
}nls_request_param;

typedef struct
{
  og_heap hba;
  int length;
  nls_request_param params[50];
} nls_request_paramList;

#define maxArrayLevel 10

/** nlsrun.c **/
int NlsRunSendErrorStatus(void *ptr, struct og_socket_info *info, int error_status, og_string message);
int NlsWaitForListeningThreads(char *label, struct og_ctrl_nls *ctrl_nls);

/** nlsltp.c Permanent threads **/
int OgPermanentLtThread(void *ptr);
int NlsInitPermanentLtThreads(struct og_ctrl_nls *ctrl_nls);
int NlsStopPermanentLtThreads(struct og_ctrl_nls *ctrl_nls);
int NlsFlushPermanentLtThreads(struct og_ctrl_nls *ctrl_nls);

/** nlslt.c **/
int OgListeningThread(void *ptr);
og_status NlsListeningThreadReset(struct og_listening_thread * lt);
og_status OgNlsLtReleaseCurrentRunnning(struct og_listening_thread * lt);


/** nlslog.c **/
og_status NlsRequestLog(struct og_listening_thread *lt, og_string function_name, og_string label,
    int additional_log_flags);
og_status NlsThrowError(struct og_listening_thread *lt, og_string format, ...);

/** nlsler.c **/
og_status OgListeningThreadError(struct og_listening_thread *lt);

/** nlsconf.c **/
og_status NlsConfReadFile(struct og_ctrl_nls *ctrl_nls, int init);
og_status NlsConfReadEnv(struct og_ctrl_nls *ctrl_nl);

/** nlsltu.c **/
og_bool OgListeningThreadAnswerUci(struct og_listening_thread *lt);

/** nlsonem.c **/
og_status NlsOnEmergency(struct og_ctrl_nls *ctrl_nls);
/*

/** nlsmt.c **/
int OgMaintenanceThread(void *ptr);
