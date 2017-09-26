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

  json_t * root = json_object();
  json_t * errors = json_array();

  int nb_error = 0;
  char erreur[DOgErrorSize];
  while (OgErrLast(lt->herr, erreur, 0))
  {
    json_array_append(errors, json_string(erreur) );
    nb_error++;
  }

  int h = 0;
  while (PcErrDiag(&h, erreur))
  {
    json_array_append(errors, json_string(erreur) );
    nb_error++;
  }

  json_object_set(root, "errors", errors);

  char * response = json_dumps(root,0);

  json_decref(errors);
  json_decref(root);


  struct og_ucisw_input winput[1];
  memset(winput, 0, sizeof(struct og_ucisw_input));
  winput->http_status = 500;
  winput->http_status_message = "Internal Server Error";
  winput->hsocket = lt->hsocket_in;
  winput->content_type = "application/json";
  winput->content_length = strlen(response);
  winput->content = response;

  IF(OgUciServerWrite(lt->hucis,winput))
  {
    OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestInErr,
        "OgUciServerWrite (%d): connexion was prematurely closed by client on an error message, giving up", lt->ID);
  }


  DONE;
}

