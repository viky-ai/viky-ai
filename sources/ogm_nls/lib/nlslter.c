/*
 *  Handling errors for listening lt function.
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : October 2006
 *  Version 1.0
 */
#include "ogm_nls.h"

static int OgListeningThreadErrorUci(struct og_listening_thread *);

/*
 *  Handling errors for listening threads.
 */

int OgListeningThreadError(struct og_listening_thread *lt)
{
  int is_error = 0;
  IFE(is_error = OgListeningThreadErrorUci(lt));
  return (is_error);
}

static int OgListeningThreadErrorUci(lt)
  struct og_listening_thread *lt;
{
  char erreur[DOgErrorSize], sys_erreur[DOgErrorSize];
  int h, is_error = 0;
  char client_error[DOgErrorSize + 100];
  struct og_ucisw_input winput[1];
  int is;
  unsigned char *s;
  int nb_error = 0;

  char lt_heap_name[DPcPathSize];
  snprintf(lt_heap_name, DPcPathSize, "nls_lt%d_thread_error", lt->ID);
  og_heap hanswer;
  IFn(hanswer = OgHeapInit(lt->hmsg, lt_heap_name, sizeof(unsigned char), DPcPathSize)) DPcErr;

  s = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
      "<search_answers>\n"
      "  <errors>\n";
  is = strlen(s);
  IFE(OgHeapAppend(hanswer, is, s));

  while (OgErrLast(lt->herr, erreur, 0))
  {
    strcpy(sys_erreur, erreur);
    PcStrlwr(sys_erreur);
    if (memcmp(sys_erreur, "warning", 7)) is_error = 1;
    sprintf(client_error, "    <error>%s</error>\n", erreur);
    IFE(OgHeapAppend(hanswer, strlen(client_error), client_error));
    OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr, "%s", erreur);
    nb_error++;
  }
  while (PcErrDiag(&h, erreur))
  {
    strcpy(sys_erreur, erreur);
    PcStrlwr(sys_erreur);
    if (memcmp(sys_erreur, "warning", 7)) is_error = 1;
    sprintf(client_error, "    <error>%s</error>\n", erreur);
    IFE(OgHeapAppend(hanswer, strlen(client_error), client_error));
    OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr, "%s", erreur);
    nb_error++;
  }

  s = "  </errors>\n"
      "</search_answers>\n";
  is = strlen(s);
  IFE(OgHeapAppend(hanswer, is, s));

  memset(winput, 0, sizeof(struct og_ucisw_input));
  winput->http_status = 500;
  winput->http_status_message = "Internal Server Error";
  winput->content_length = OgHeapGetCellsUsed(hanswer);
  winput->content = OgHeapGetCell(hanswer, 0);
  winput->hsocket = lt->hsocket_in;

  IF(OgUciServerWrite(lt->hucis,winput))
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
        "OgUciServerWrite (%d): connexion was prematurely closed by client on an error message, giving up", lt->ID);
  }

  IFE(OgHeapFlush(hanswer));

  return (is_error);
}

