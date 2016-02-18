/*
 *	Internal header for handling signals
 *	Copyright (c) 2007 Pertimm, Inc. by Patrick Constant
 *	Dev : August 2007
 *	Version 1.0
*/

#define DPcInDll
#include <logsig.h>


#define DOgSsrvSignaNumber  64

struct signa {
  int signal_type;
  void (*func)(pr_(void *) pr(int));
  void *context;
  };


struct og_ctrl_sig {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo; 

  int SignaNumber;
  int SignaUsed;
  struct signa *Signa;  
  };

extern struct og_ctrl_sig *OgCtrlSig;


/** signa.c **/
int SigAddSigna(pr_(struct og_ctrl_sig *) pr_(int) pr_(void (*func)(pr_(void *) pr(int))) pr(void *));
int SigGetSigna(pr_(struct og_ctrl_sig *) pr_(int) pr(int *));


