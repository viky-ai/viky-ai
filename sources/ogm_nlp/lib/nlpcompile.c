/*
 *  Main function for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

PUBLIC(int) OgNlpCompile(og_nlp ctrl_nlp, struct og_nlp_compile_input *input, struct og_nlp_compile_output *output)
{

  json_t *json = json_object();
  output->json_output = json;

  json_t *value = json_string("titi");
  IFE(json_object_set_new(json, "toto", value));

  DONE;
}
