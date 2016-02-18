/*
 *  Miscellanous function for socket handling. 
 *  Copyright (c) 2003 Pertimm
 *  Dev : February 2003
 *  Version 1.0
*/
#include <logsock.h>



#if (DPcSystem == DPcSystemUnix)


PUBLIC(int) OgGetRemoteAddrSocket(addr,sra,ira)
struct in_addr *addr;
char *sra; int *ira;
{
int b1,b2,b3,b4;
b1 = addr->s_addr & 0xff;
b2 = (addr->s_addr>>8) & 0xff;
b3 = (addr->s_addr>>16) & 0xff;
b4 = (addr->s_addr>>24) & 0xff;
if (sra) {
  sprintf(sra,"%d.%d.%d.%d",b1,b2,b3,b4);
  }
if (ira) {
  ira[0]=b1; ira[1]=b2; ira[2]=b3; ira[3]=b4; 
  }
DONE;
}


#else
#if (DPcSystem == DPcSystemWin32)


PUBLIC(int) OgGetRemoteAddrSocket(addr,sra,ira)
struct in_addr *addr;
char *sra; int *ira;
{
int b1,b2,b3,b4;
b1 = addr->S_un.S_un_b.s_b1;
b2 = addr->S_un.S_un_b.s_b2;
b3 = addr->S_un.S_un_b.s_b3;
b4 = addr->S_un.S_un_b.s_b4;
if (sra) {
  sprintf(sra,"%d.%d.%d.%d",b1,b2,b3,b4);
  }
if (ira) {
  ira[0]=b1; ira[1]=b2; ira[2]=b3; ira[3]=b4; 
  }
DONE;
}


#endif
#endif








