/*
 *  Header for general socket handling. 
 *  Copyright (c) 2004 Pertimm by Patrick Constant
 *  Dev : February 2004
 *  Version 1.0
*/
#include <logsock.h>


struct og_url_header_line {
  int iname; char *name;
  int value;
  };

extern struct og_url_header_line OgUrlHeaderLine[];


/** sockconn **/
int php_connect_nonb(pr_(int) pr_(const struct sockaddr *) pr_(int) 
                     pr_(struct timeval *) pr_(int *) pr(struct og_call_socket_stat *st));


