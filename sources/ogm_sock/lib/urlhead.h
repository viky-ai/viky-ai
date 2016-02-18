/*
 *  Header for parsing of an HTTP header
 *  Copyright (c) 2004 Pertimm by Patrick Constant
 *  Dev : February 2004
 *  Version 1.0
*/


struct og_url_header_line OgUrlHeaderLine[] = {
  {  4, "HTTP",                DOgHnHttp }, /* can be 1.0 or 1.1 */
  {  6, "Accept",              DOgHnAccept },
  { 14, "Accept-Charset",      DOgHnAcceptCharset },
  { 15, "Accept-Encoding",     DOgHnAcceptEncoding },
  { 15, "Accept-Language",     DOgHnAcceptLanguage },
  { 13, "Accept-Ranges",       DOgHnAcceptRanges },
  {  3, "Age",                 DOgHnAge },
  {  5, "Allow",               DOgHnAllow },
  { 13, "Authorization",       DOgHnAuthorization },
  { 13, "Cache-Control",       DOgHnCacheControl },
  { 10, "Connection",          DOgHnConnection },
  { 16, "Content-Encoding",    DOgHnContentEncoding },
  { 16, "Content-Language",    DOgHnContentLanguage },
  { 14, "Content-Length",      DOgHnContentLength },
  { 16, "Content-Location",    DOgHnContentLocation },
  { 11, "Content-MD5",         DOgHnContentMD5 },
  { 13, "Content-Range",       DOgHnContentRange },
  { 12, "Content-Type",        DOgHnContentType },
  {  6, "Cookie",              DOgHnCookie },  /* RFC 2965  */
  {  7, "Cookie2",             DOgHnCookie2 }, /* RFC 2965  */
  {  4, "Date",                DOgHnDate },
  {  4, "ETag",                DOgHnETag },
  {  6, "Expect",              DOgHnExpect },
  {  7, "Expires",             DOgHnExpires },
  {  4, "From",                DOgHnFrom },
  {  4, "Host",                DOgHnHost },
  {  8, "If-Match",            DOgHnIfMatch },
  { 17, "If-Modified-Since",   DOgHnIfModifiedSince },
  { 13, "If-None-Match",       DOgHnIfNoneMatch },
  {  8, "If-Range",            DOgHnIfRange },
  { 19, "If-Unmodified-Since", DOgHnIfUnmodifiedSince },
  { 13, "Last-Modified",       DOgHnLastModified },
  {  8, "Location",            DOgHnLocation },
  { 12, "Max-Forwards",        DOgHnMaxForwards },
  {  6, "Pragma",              DOgHnPragma },
  { 18, "Proxy-Authenticate",  DOgHnProxyAuthenticate },
  { 19, "Proxy-Authorization", DOgHnProxyAuthorization },
  { 16, "Proxy-Connection",    DOgHnProxyConnection }, /* used but not in RFC */
  {  5, "Range",               DOgHnRange },
  {  7, "Referer",             DOgHnReferer },
  { 11, "Retry-After",         DOgHnRetryAfter },
  {  6, "Server",              DOgHnServer },
  { 10, "Set-Cookie",          DOgHnStartSetCookie },  /* RFC 2965 */
  { 11, "Set-Cookie2",         DOgHnStartSetCookie2 }, /* RFC 2965 */
  {  2, "TE",                  DOgHnTE },
  {  7, "Trailer",             DOgHnTrailer },
  { 17, "Transfer-Encoding",   DOgHnTransferEncoding },
  {  7, "Upgrade",             DOgHnUpgrade },
  { 10, "User-Agent",          DOgHnUserAgent },
  {  4, "Vary",                DOgHnVary },
  {  3, "Via",                 DOgHnVia },
  {  7, "Warning",             DOgHnWarning },
  { 16, "WWW-Authenticate",    DOgHnWWWAuthenticate },
  {  0, "", 0 }
  };
