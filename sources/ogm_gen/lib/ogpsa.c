/*
 *  Parsing server addresses specifications of format "server:port"
 *  Copyright (c) 2005  Pertimm by Patrick Constant
 *  Dev : March 2005
 *  Version 1.0
*/
#include <loggen.h>
#include <logpath.h>



/*
 *  Used to get the first value of addresses, exemple is
 *  ogm_conf.txt with typical section:
 *    [ServerAddresses]
 *    127.0.0.1:8194
 *    indian:8194
 *  This is very interesting for programs such as
 *  ogm_stop, ogm_phox or others that want to reach
 *  programs such as ogm_jsrv, ogm_node through sockets.
*/

PUBLIC(int) OgGetConfServerAddress(char *conf_in, char *section, char *hostname, int *port)
{
int found;
char address[DPcPathSize],conf[DPcPathSize],*DOgMAIN,*DOgINSTANCE;

if(!Ogstricmp(conf_in,DOgFileOgmConf_Txt) && (DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  sprintf(conf,"%s/%s/%s/%s",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE,DOgFileOgmConf_Txt_instance);  }
else {
  strcpy(conf,conf_in); }

IFE(found=OgConfGetVarLine(conf,section,address,DPcPathSize));
if (found) {
  IFE(OgParseServerAddress(address,hostname,port));
  }
else {
  strcpy(hostname,"0.0.0.0");
  }
DONE;
}



/*
 *  addresses has format "[<server>][:<port>]"
 *  if server does not exist value is "0.0.0.0"
 *  if port does not exist, port value is not set.
*/

PUBLIC(int) OgParseServerAddress(const char *address, char *hostname, int *port)
{
int i,dp=(-1);

strcpy(hostname,address);

for (i=0; hostname[i]; i++) {
  if (hostname[i]==':') { dp=i; break; }
  }

if (dp==0) {
  strcpy(hostname,"0.0.0.0");
  }
else if (dp>0) {
  hostname[dp]=0; *port=atoi(hostname+dp+1);
  }

DONE;
}




