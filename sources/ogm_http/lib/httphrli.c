/*
 *	Working on http header.
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : August 2006
 *	Version 1.0
*/
#include "ogm_http.h"



struct og_http_header_line OgHttpHeaderLine[] = {
  {  6, "Accept",              DOgHttpHeaderLineAccept },
  { 14, "Accept-Charset",      DOgHttpHeaderLineAcceptCharset },
  { 15, "Accept-Encoding",     DOgHttpHeaderLineAcceptEncoding },
  { 15, "Accept-Language",     DOgHttpHeaderLineAcceptLanguage },
  { 13, "Accept-Ranges",       DOgHttpHeaderLineAcceptRanges },
  {  3, "Age",                 DOgHttpHeaderLineAge },
  {  5, "Allow",               DOgHttpHeaderLineAllow },
  { 13, "Authorization",       DOgHttpHeaderLineAuthorization },
  { 13, "Cache-Control",       DOgHttpHeaderLineCacheControl },
  { 10, "Connection",          DOgHttpHeaderLineConnection },
  { 16, "Content-Encoding",    DOgHttpHeaderLineContentEncoding },
  { 16, "Content-Language",    DOgHttpHeaderLineContentLanguage },
  { 14, "Content-Length",      DOgHttpHeaderLineContentLength },
  { 16, "Content-Location",    DOgHttpHeaderLineContentLocation },
  { 11, "Content-MD5",         DOgHttpHeaderLineContentMD5 },
  { 13, "Content-Range",       DOgHttpHeaderLineContentRange },
  { 12, "Content-Type",        DOgHttpHeaderLineContentType },
  {  6, "Cookie",              DOgHttpHeaderLineCookie },  /* RFC 2965  */
  {  7, "Cookie2",             DOgHttpHeaderLineCookie2 }, /* RFC 2965  */
  {  4, "Date",                DOgHttpHeaderLineDate },
  {  4, "ETag",                DOgHttpHeaderLineETag },
  {  6, "Expect",              DOgHttpHeaderLineExpect },
  {  7, "Expires",             DOgHttpHeaderLineExpires },
  {  4, "From",                DOgHttpHeaderLineFrom },
  {  4, "Host",                DOgHttpHeaderLineHost },
  {  8, "If-Match",            DOgHttpHeaderLineIfMatch },
  { 17, "If-Modified-Since",   DOgHttpHeaderLineIfModifiedSince },
  { 13, "If-None-Match",       DOgHttpHeaderLineIfNoneMatch },
  {  8, "If-Range",            DOgHttpHeaderLineIfRange },
  { 19, "If-Unmodified-Since", DOgHttpHeaderLineIfUnmodifiedSince },
  { 13, "Last-Modified",       DOgHttpHeaderLineLastModified },
  {  8, "Location",            DOgHttpHeaderLineLocation },
  { 12, "Max-Forwards",        DOgHttpHeaderLineMaxForwards },
  {  6, "Pragma",              DOgHttpHeaderLinePragma },
  { 18, "Proxy-Authenticate",  DOgHttpHeaderLineProxyAuthenticate },
  { 19, "Proxy-Authorization", DOgHttpHeaderLineProxyAuthorization },
  { 16, "Proxy-Connection",    DOgHttpHeaderLineProxyConnection }, /* used but not in RFC */
  {  5, "Range",               DOgHttpHeaderLineRange },
  {  7, "Referer",             DOgHttpHeaderLineReferer },
  { 11, "Retry-After",         DOgHttpHeaderLineRetryAfter },
  {  6, "Server",              DOgHttpHeaderLineServer },
  { 10, "Set-Cookie",          DOgHttpHeaderLineSetCookie },  /* RFC 2965 */
  { 11, "Set-Cookie2",         DOgHttpHeaderLineSetCookie2 }, /* RFC 2965 */
  {  2, "TE",                  DOgHttpHeaderLineTE },
  {  7, "Trailer",             DOgHttpHeaderLineTrailer },
  { 17, "Transfer-Encoding",   DOgHttpHeaderLineTransferEncoding },
  {  7, "Upgrade",             DOgHttpHeaderLineUpgrade },
  { 10, "User-Agent",          DOgHttpHeaderLineUserAgent },
  {  4, "Vary",                DOgHttpHeaderLineVary },
  {  3, "Via",                 DOgHttpHeaderLineVia },
  {  7, "Warning",             DOgHttpHeaderLineWarning },
  { 16, "WWW-Authenticate",    DOgHttpHeaderLineWWWAuthenticate },
  {  0, "", 0 }
  };



