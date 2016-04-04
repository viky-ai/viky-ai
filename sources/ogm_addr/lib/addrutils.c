/*
 *  Function listening to all added addresses.
 *  Copyright (c) 2005-2006 Pertimm by Patrick Constant
 *  Dev : March 2005, December 2006
 *  Version 1.1
*/
#include "ogm_addr.h"

//TODO a voir soucis dans l'envoie : pas de header recu
og_status AddrSendStatusCodeServiceUnavailable(struct og_ctrl_addr *ctrl_addr, int socket)
{
  char header[DPcPathSize];
  sprintf(header, "HTTP/1.1 503 Service Unavailable\r\n"
      "Connection: close\r\n"
      "\r\n");

  int iheader = strlen(header);

  OgMessageLog(DOgMlogInLog, ctrl_addr->loginfo->where, 0, header);

  int error_messages_used = OgHeapGetCellsUsed(ctrl_addr->error_messages);
  IFE(OgHeapAppend(ctrl_addr->error_messages, iheader, header));

  unsigned char *header_to_store = OgHeapGetCell(ctrl_addr->error_messages, error_messages_used);
  if(header_to_store == NULL)
  {
    DPcErr;
  }
  IFE(OgSendSocket(ctrl_addr->herr, socket, header_to_store, iheader));
  DONE;
}



