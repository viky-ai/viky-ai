/*
 *  Header for library ogm_http.dll
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : August, December 2006, September 2007
 *  Version 1.3
*/
#ifndef _LOGHTTPALIVE_
#include <loggen.h>
#include <logthr.h>
#include <logmsg.h>


#define DOgHttpBanner  "ogm_http V1.09, Copyright (c) 2006-2007 Pertimm, Inc."
#define DOgHttpVersion 109


/** Trace levels **/
#define DOgHttpTraceMinimal          0x1
#define DOgHttpTraceMemory           0x2

#define DOgHttpHeaderLineSize  2048
#define DOgHttpHeaderMaxLines   256


struct og_http_param {
  void *herr,*hmsg; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  };

struct og_http_header2 {
  void *ha_header;
  int request_method;
  char request_uri[DOgHttpHeaderLineSize];
  char first_line[DOgHttpHeaderLineSize];
  int major,minor,status;
  size_t content_length;
  time_t last_modified;
  int header_length;
  };

/** Type of the http header (request) per RFC 2616 **/
#define DOgHttpHeaderTypeOption    1
#define DOgHttpHeaderTypeGet       2
#define DOgHttpHeaderTypeHead      3
#define DOgHttpHeaderTypePost      4
#define DOgHttpHeaderTypePut       5
#define DOgHttpHeaderTypeDelete    6
#define DOgHttpHeaderTypeTrace     7
#define DOgHttpHeaderTypeConnect   8
/** Type of the http header (response) **/
#define DOgHttpHeaderTypeHttp    100


/** All header lines per RFC 2616 **/
#define DOgHttpHeaderLineAccept                 1
#define DOgHttpHeaderLineAcceptCharset          2
#define DOgHttpHeaderLineAcceptEncoding         3
#define DOgHttpHeaderLineAcceptLanguage         4
#define DOgHttpHeaderLineAcceptRanges           5
#define DOgHttpHeaderLineAge                    6
#define DOgHttpHeaderLineAllow                  7
#define DOgHttpHeaderLineAuthorization          8
#define DOgHttpHeaderLineCacheControl           9
#define DOgHttpHeaderLineConnection            10
#define DOgHttpHeaderLineContentEncoding       11
#define DOgHttpHeaderLineContentLanguage       12
#define DOgHttpHeaderLineContentLength         13
#define DOgHttpHeaderLineContentLocation       14
#define DOgHttpHeaderLineContentMD5            15
#define DOgHttpHeaderLineContentRange          16
#define DOgHttpHeaderLineContentType           17
#define DOgHttpHeaderLineCookie                18
#define DOgHttpHeaderLineCookie2               19
#define DOgHttpHeaderLineDate                  20
#define DOgHttpHeaderLineETag                  21
#define DOgHttpHeaderLineExpect                22
#define DOgHttpHeaderLineExpires               23
#define DOgHttpHeaderLineFrom                  24
#define DOgHttpHeaderLineHost                  25
#define DOgHttpHeaderLineIfMatch               26
#define DOgHttpHeaderLineIfModifiedSince       27
#define DOgHttpHeaderLineIfNoneMatch           28
#define DOgHttpHeaderLineIfRange               29
#define DOgHttpHeaderLineIfUnmodifiedSince     30
#define DOgHttpHeaderLineLastModified          31
#define DOgHttpHeaderLineLocation              32
#define DOgHttpHeaderLineMaxForwards           33
#define DOgHttpHeaderLinePragma                34
#define DOgHttpHeaderLineProxyAuthenticate     35
#define DOgHttpHeaderLineProxyAuthorization    36
#define DOgHttpHeaderLineProxyConnection       37
#define DOgHttpHeaderLineRange                 38
#define DOgHttpHeaderLineReferer               39
#define DOgHttpHeaderLineRetryAfter            40
#define DOgHttpHeaderLineServer                41
#define DOgHttpHeaderLineSetCookie             42
#define DOgHttpHeaderLineSetCookie2            43
#define DOgHttpHeaderLineTE                    44
#define DOgHttpHeaderLineTrailer               45
#define DOgHttpHeaderLineTransferEncoding      46
#define DOgHttpHeaderLineUpgrade               47
#define DOgHttpHeaderLineUserAgent             48
#define DOgHttpHeaderLineVary                  49
#define DOgHttpHeaderLineVia                   50
#define DOgHttpHeaderLineWarning               51
#define DOgHttpHeaderLineWWWAuthenticate       52


DEFPUBLIC(void *) OgHttpInit(pr(struct og_http_param *));
DEFPUBLIC(int) OgHttpHeaderRead(pr_(void *) pr_(int) pr_(unsigned char *) pr(struct og_http_header2 *));
DEFPUBLIC(int) OgHttpHeaderLength(pr_(void *) pr_(int) pr_(unsigned char *) pr(int *));
DEFPUBLIC(int) OgHttpHeader2Log(pr_(void *) pr(struct og_http_header2 *));
DEFPUBLIC(int) OgHttpHeaderScanValues(pr_(void *) pr_(struct og_http_header2 *) pr_(int)
                                      pr_(int (*)(pr_(void *) pr_(int) pr(unsigned char *))) pr(void *));
DEFPUBLIC(int) OgHttpMem(void *hhttp, int must_log, int module_level, ogint64_t *pmem);
DEFPUBLIC(int) OgHttpFlush(pr(void *));

#define _LOGHTTPALIVE_
#endif



