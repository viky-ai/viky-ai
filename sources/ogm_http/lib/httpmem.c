/*
 *  Calculating memory used for ogm_http
 *  Copyright (c) 2011 Pertimm by Patrick Constant
 *  Dev: March 2011
 *  Version 1.0
*/
#include "ogm_http.h"


#define DOgShowMem(x,y,z) { if (must_log) { Og64FormatThousand(x,v,1); OgMessageLog(DOgMlogInLog,ctrl_http->loginfo->where,0,"%s%s:%s %16s",header,y,z,v); } }


struct og_http_memory {
  ogint64_t total,total_module;
  ogint64_t ctrl_http;
  ogint64_t ha_header;
  };


/*
 *  Logs all the memory module and calculates all the memory.
*/

PUBLIC(int) OgHttpMem(void *hhttp, int must_log, int module_level, ogint64_t *pmem)
{
struct og_ctrl_http *ctrl_http = (struct og_ctrl_http *)hhttp;
char header[DPcPathSize],v[128];
struct og_http_memory cm,*m=&cm;
int i;

IFn(hhttp) DONE;

if (pmem) *pmem=0;
if (must_log) {
  for (i=0; i<module_level; i++) header[i]=' '; header[i]=0;
  }
memset(m,0,sizeof(struct og_http_memory));

m->ctrl_http=sizeof(struct og_ctrl_http);
IFE(OgAutMem(ctrl_http->ha_header,0,module_level+2,&m->ha_header));

m->total_module = m->ctrl_http
                + m->ha_header
                ;

m->total += m->total_module;

DOgShowMem(m->total_module    ,"total_module http"," ")
DOgShowMem(m->ctrl_http       ,"ctrl_http","         ")
DOgShowMem(m->ha_header       ,"ha_header","         ")

if (pmem) *pmem=m->total;

DONE;
}




