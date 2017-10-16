/*
 *  Encapsulation of gethostbyaddr
 *  Copyright (c) 2003 Pertimm
 *  Dev : November 2003
 *  Version 1.0
*/
#include <logsock.h>



/*
 *  This function is less general that the original 'gethostbyaddr'
 *  but serves well the simple purpose of getting hostname from ip address.
 *  To work well, the OgStartupSockets must have been called first.
 *  Returns 1 if the hostname has been found and 0 otherwise.
*/

PUBLIC(int) OgGetHostByAddr(address,hostname)
char *address, *hostname;
{
struct hostent *hp;
int i,c,start=0,end=0,iaddr=0; unsigned char addr[4];
char *dot,buffer[1024];
int found=0;


for (i=0; !end; i++) {
  if (address[i]==0) { end=1; c='.'; }
  else c=address[i];
  if (c=='.') {
    addr[iaddr++]=atoi(address+start);
    if (iaddr>=4) break;
    start=i+1;
    }
  }

buffer[0]=0;
for (i=0; i<iaddr; i++) {
  if (i==0) dot=""; else dot=".";
  sprintf(buffer+strlen(buffer),"%s%d",dot,addr[i]);
  }

IFn ((hp = gethostbyaddr(addr, 4, PF_INET))) {
  strcpy(hostname,address);
  }
else {
  strcpy(hostname,hp->h_name);
  found=1;
  }
return(found);
}


