/*
 *  Internal header for handling http protocol
 *  Copyright (c) 2006 Pertimm, Inc. by Patrick Constant
 *  Dev : August 2006
 *  Version 1.0
*/
#include <loghttp.h>
#include <logaut.h>


struct og_http_header_line {
  int iname; char *name;
  int value;
  };

struct og_http_header_type {
  int iname; char *name;
  int value;
  };


extern struct og_http_header_line OgHttpHeaderLine[];
extern struct og_http_header_type OgHttpHeaderType[];


struct og_ctrl_http {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;

  void *ha_header;

  /** Buffer for all operations **/
  unsigned char *Ba;
  int BaSize,BaUsed;
  };



/** httpba.c **/
int HttpAppendBa(pr_(struct og_ctrl_http *) pr_(int) pr(unsigned char *));
int HttpTestReallocBa(pr_(struct og_ctrl_http *) pr(int));
int HttpReallocBa(pr_(struct og_ctrl_http *) pr(int));



