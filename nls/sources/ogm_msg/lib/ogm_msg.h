/*
 *	Internal header for Message Management
 *	Copyright (c) 2007 Pertimm, Inc. by Patrick Constant and Guillaume Logerot
 *	Dev : March 2007
 *	Version 1.0
*/

#include <assert.h>
#include <logmsg.h>

struct og_ctrl_msg {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  char module_name[DPcPathSize];
  char instance_name[DPcPathSize];
  char prog_name[DPcPathSize];

  int default_mask;
  int show_timestamp;
  int show_id;
  
  struct og_loginfo cprog_loginfo,*prog_loginfo;
  NATIVE_FILE pipe;
  FILE *consoleStream;
  };



/* msgmlog.c */
int OgMsgMessageLogMask(int);

/* msgtpl.c */
int OgMsgTemplateBuild(struct og_ctrl_msg *, char *, char *, int);

/* msgmask.c */
int MsgMaskMix(struct og_ctrl_msg *, int, int *);



