/*
 *	Internal header for handling of XML
 *	Copyright (c) 2004 Pertimm by Patrick Constant
 *	Dev : March 2004
 *	Version 1.0
*/

#include <logxml.h>
#include <logmsg.h>


#if 0 /** For testing memory leaks, link with include/ogmalloc.c **/
#define malloc(x)   OgMalloc(x,"xml",__FILE__,__LINE__)
#define free(x)     OgFree(x,"xml",__FILE__,__LINE__)
#endif

#define DOgRevarNumber  10

#define DOgMaxRevarSize 1024

/** replacement variable **/
struct revar {
  int iname; unsigned char name[DOgMaxRevarSize];
  int ivalue; unsigned char value[DOgMaxRevarSize];
  };

struct og_ctrl_xml {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo; 
  
  int RevarNumber,RevarUsed;
  struct revar *Revar;
  };


/** xmlrevar.c **/
int AddRevar(pr_(struct og_ctrl_xml *) pr_(unsigned char *) pr(unsigned char *));
int FindRevar(pr_(struct og_ctrl_xml *) pr_(int) pr_(unsigned char *) pr(int *));


