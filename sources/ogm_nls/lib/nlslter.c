/*
 *  Handling errors for listening lt function.
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : October 2006
 *  Version 1.0
 */
#include "ogm_nls.h"

/*
 *  Handling errors for listening threads.
 */
og_status OgListeningThreadError(struct og_listening_thread *lt)
{

  IFE(OgNLSJsonReset(lt));

  // on ouvre le fichier json et on ouvre le tableau
  IFE(OgNLSJsonGenMapOpen(lt));

  IFE(OgNLSJsonGenKeyValueArrayOpen(lt, "error"));

  int nb_error = 0;
  char erreur[DOgErrorSize];
  while (OgErrLast(lt->herr, erreur, 0))
  {
    IFE(OgNLSJsonGenString(lt, erreur));
    nb_error++;
  }

  int h = 0;
  while (PcErrDiag(&h, erreur))
  {
    IFE(OgNLSJsonGenString(lt, erreur));
    nb_error++;
  }

  // on ferme le tableau et le fichier json
  IFE(OgNLSJsonGenArrayClose(lt));

  // on logue le nombre d'erreurs
  // IFE(OgNLSJsonGenKeyValueInteger(lt, "error_number", nb_error));
  IFE(OgNLSJsonGenMapClose(lt));

  struct og_ucisw_input winput[1];
  memset(winput, 0, sizeof(struct og_ucisw_input));
  winput->http_status = 500;
  winput->http_status_message = "Internal Server Error";
  winput->hsocket = lt->hsocket_in;
  winput->content_length = OgHeapGetCellsUsed(lt->json->hb_json_buffer);
  winput->content = OgHeapGetCell(lt->json->hb_json_buffer, 0);

  IF(OgUciServerWrite(lt->hucis,winput))
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
        "OgUciServerWrite (%d): connexion was prematurely closed by client on an error message, giving up", lt->ID);
  }

  IFE(OgNLSJsonReset(lt));

  DONE;
}

