/*
 *  Handling parsing tree from a request expression
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpInterpretTreeJsonRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, struct request_expression *request_expression,
    json_t *json_expressions);

/*
 * Adds the request expression in the json_interpretation
 */

og_status NlpInterpretTreeJson(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_interpretation)
{

  json_t *json_expression = json_object();
  IF(json_object_set_new(json_interpretation, "expression", json_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_text");
    DPcErr;
  }

  json_t *json_expression_text = json_string(request_expression->expression->text);
  IF(json_object_set_new(json_expression, "expression", json_expression_text))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expression_text");
    DPcErr;
  }

  IFE(NlpInterpretTreeJsonRecursive(ctrl_nlp_th, request_expression, request_expression, json_expression));
  DONE;
}

static og_status NlpInterpretTreeJsonRecursive(og_nlp_th ctrl_nlp_th,
    struct request_expression *root_request_expression, struct request_expression *request_expression,
    json_t *json_expression)
{
  struct original_request_input_part *original_request_input_part = OgHeapGetCell(
      ctrl_nlp_th->horiginal_request_input_part, 0);
  IFN(original_request_input_part) DPcErr;

  struct orip *orip = OgHeapGetCell(ctrl_nlp_th->horip, 0);
  IFN(orip) DPcErr;

  json_t *json_expressions = json_array();

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    int Ioriginal_request_input_part = orip[request_expression->orip_start + i].Ioriginal_request_input_part;
    int Irequest_input_part = original_request_input_part[Ioriginal_request_input_part].Irequest_input_part;
    struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
        Irequest_input_part);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
      IFN(string_request_word) DPcErr;

      json_t *json_sub_expression = json_object();
      json_t *json_sub_expression_text = json_string(string_request_word);
      IF(json_object_set_new(json_sub_expression, "word", json_sub_expression_text))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_sub_expression_text");
        DPcErr;
      }
      IF(json_array_append_new(json_expressions, json_sub_expression))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson : error appending json_word");
        DPcErr;
      }
    }
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {

      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;

      json_t *json_sub_expression = json_object();
      json_t *json_sub_expression_text = json_string(sub_request_expression->expression->text);
      IF(json_object_set_new(json_sub_expression, "text", json_sub_expression_text))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_sub_expression_text");
        DPcErr;
      }
      IF(json_array_append_new(json_expressions, json_sub_expression))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson : error appending json_word");
        DPcErr;
      }
      og_status status = NlpInterpretTreeJsonRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression,
          json_sub_expression);
      IFE(status);
    }

    if (request_expression->expression->alias_any_input_part_position == i + 1)
    {
      if (request_expression->Irequest_any >= 0)
      {
        struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->Irequest_any);
        IFN(request_any) DPcErr;

        char string_any[DPcPathSize];
        NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any);

        json_t *json_sub_expression = json_object();
        json_t *json_sub_expression_any = json_string(string_any);
        IF(json_object_set_new(json_sub_expression, "any", json_sub_expression_any))
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_sub_expression_any");
          DPcErr;
        }
        IF(json_array_append_new(json_expressions, json_sub_expression))
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson : error appending json_sub_expression");
          DPcErr;
        }


      }
    }

  }

  IF(json_object_set_new(json_expression, "expressions", json_expressions))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretTreeJson: error setting json_expressions");
    DPcErr;
  }

  DONE;
}

