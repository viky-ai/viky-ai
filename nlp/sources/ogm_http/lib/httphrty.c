/*
 *	List of http header type.
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : August 2006
 *	Version 1.0
*/
#include "ogm_http.h"



struct og_http_header_type OgHttpHeaderType[] = {
  {  6, "option",   DOgHttpHeaderTypeOption },
  {  3, "get",      DOgHttpHeaderTypeGet },
  {  4, "head",     DOgHttpHeaderTypeHead },
  {  4, "post",     DOgHttpHeaderTypePost },
  {  3, "put",      DOgHttpHeaderTypePut },
  {  6, "delete",   DOgHttpHeaderTypeDelete },
  {  5, "trace",    DOgHttpHeaderTypeTrace },
  {  7, "connect",  DOgHttpHeaderTypeConnect },
  {  4, "http",     DOgHttpHeaderTypeHttp },
  {  0, "", 0 }
  };



