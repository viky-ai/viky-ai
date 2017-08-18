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
#include <logxml.h>

typedef struct og_listening_thread og_listening_thread;

#define DOgNlsPortNumber  9345

struct timeout_conf_context
{
  char *timeout_name; /** name of the timeout : answer_timeout, socket_read_timeout...*/
  int old_timeout; /** timeout from previous conf */
  int default_timeout; /** default timeout value */
};

struct og_nls_conf
{
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
  int loop_answer_timeout;

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
  ogsem_t csem, *hsem;
  int must_stop;

  ogint64_t t0,t1,t2,t3,ot3;

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
  struct og_conf_lines nls_addresses;
  char hostname[DPcPathSize];
  int port_number;

  struct og_listening_thread *Lt;
  int LtNumber;

  ogsem_t hsem_run3[1];
  /** Mutex to choose current lt */
  ogmutex_t hmutex_run_lt[1];

};

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

/** nlslog.c **/
int NlsRequestLog(struct og_listening_thread *lt, og_string function_name, og_string label, int additional_log_flags);
int NlsThrowError(struct og_listening_thread *lt, og_string format, ...);

/** nlsler.c **/
int OgListeningThreadError(struct og_listening_thread *lt);

/** nlsconf.c **/
int NlsReadConfigurationFile(struct og_ctrl_nls *ctrl_nls, int init);

/** nlsltu.c **/
og_bool OgListeningThreadAnswerUci(struct og_listening_thread *lt);

/** nlsonem.c **/
int NlsOnEmergency(struct og_ctrl_nls *ctrl_nls);






