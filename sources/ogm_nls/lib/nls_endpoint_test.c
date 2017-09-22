/*
 * endpoint-test.c
 *
 *  Created on: Sep 21, 2017
 *      Author: mouadh
 */

#include "ogm_nls.h"

char * default_response();

void endpoint_test(struct og_listening_thread *lt, struct og_ucisw_input *winput, struct og_ucisr_output *output,nls_request_paramList *parametersList)
{
  char *response = NULL;
  char *body = NULL ;

  if(output->hh.request_method == DOgHttpHeaderTypePost)
  {
    int headerSize = output->header_length;
    int contentSize = output->content_length - headerSize;
    if(contentSize > 0)
      body = output->content + headerSize;
  }

  if (parametersList->length > 0)
  {
    for (int i = 0; i < parametersList->length; i++)
    {

      OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestMBox, "%s = %s",parametersList->params[i].key,parametersList->params[i].value);
      if (strcmp(parametersList->params[i].key, "name") == 0)
      {
        json_t * response_json = json_object();
        json_object_set(response_json, "hello", json_string(parametersList->params[i].value));
        response = json_dumps(response_json, 0);
        json_decref(response_json);
      }
      else if (strcmp(parametersList->params[i].key, "wait") == 0)
      {
        if(strcmp(parametersList->params[i].value, "infinity") == 0 || strcmp(parametersList->params[i].value, "infinite") == 0 )
        {
          IFE(OgHeapFlush(parametersList->hba));
          while(1){}
        }
        else
        {
          int sleep_for = atoi(parametersList->params[i].value) ;
          IFE(OgHeapFlush(parametersList->hba));
          OgSleep(sleep_for);
        }
      }
    }
  }

  if (response == NULL) response = default_response();

  OgMsg(lt->hmsg, "", DOgMsgDestInLog + DOgMsgDestMBox, " response = %s", response);

  winput->content = response;
  winput->content_length = strlen(response);

}






char * default_response()
{
  json_t * response_json = json_object();
  json_object_set(response_json, "hello", json_string("world"));
  char * response_string = json_dumps(response_json, 0);
  json_decref(response_json);
  return response_string;
}
