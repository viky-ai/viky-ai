/*
 *  Handling why an expression or an interpretation has not been found, create the json structure
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpWhyJsonNmExpression(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    json_t *json_nm_expressions);
static og_status NlpWhyJsonMInputPart(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    struct m_input_part *m_input_part, json_t *json_input_parts);
static og_status NlpWhyJsonMExpression(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    struct m_input_part *m_input_part, struct m_expression *m_expression, json_t *json_matches);

og_status NlpWhyJson(og_nlp_th ctrl_nlp_th, json_t *json_interpretation)
{
  json_t *json_nm_expressions = json_array();
  IF(json_object_set_new(json_interpretation, "because-not-matching", json_nm_expressions))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJson: error setting json_nm_expressions");
    DPcErr;
  }

  int nm_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hnm_expression);
  struct nm_expression *nm_expressions = OgHeapGetCell(ctrl_nlp_th->hnm_expression, 0);
  IFN(nm_expressions) DPcErr;

  for (int i = 0; i < nm_expression_used; i++)
  {
    IFE(NlpWhyJsonNmExpression(ctrl_nlp_th, nm_expressions + i, json_nm_expressions));
  }

  DONE;
}

static og_status NlpWhyJsonNmExpression(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    json_t *json_nm_expressions)
{
  json_t *json_nm_expression = json_object();
  IF(json_array_append_new(json_nm_expressions, json_nm_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonNmExpression : error appending json_nm_expression");
    DPcErr;
  }

  struct expression *expression = nm_expression->expression;

  json_t *json_package = json_string(expression->interpretation->package->id);
  IF(json_object_set_new(json_nm_expression, "package", json_package))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonNmExpression: error setting json_package");
    DPcErr;
  }
  json_t *json_interpretation = json_string(expression->interpretation->id);
  IF(json_object_set_new(json_nm_expression, "interpretation", json_interpretation))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonNmExpression: error setting json_interpretation");
    DPcErr;
  }
  json_t *json_expression = json_string(expression->text);
  IF(json_object_set_new(json_nm_expression, "expression", json_expression))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonNmExpression: error setting json_expression");
    DPcErr;
  }

  json_t *json_input_parts = json_array();
  IF(json_object_set_new(json_nm_expression, "input-parts", json_input_parts))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJson: error setting json_input_parts");
    DPcErr;
  }

  int input_parts_nb = expression->input_parts_nb;
  struct m_input_part *m_input_parts = OgHeapGetCell(ctrl_nlp_th->hm_input_part, 0);
  for (int i = 0; i < input_parts_nb; i++)
  {
    struct m_input_part *m_input_part = m_input_parts + nm_expression->m_input_part_start + i;
    IFE(NlpWhyJsonMInputPart(ctrl_nlp_th, nm_expression, m_input_part, json_input_parts));
  }

  DONE;
}

static og_status NlpWhyJsonMInputPart(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    struct m_input_part *m_input_part, json_t *json_input_parts)
{
  json_t *json_input_part = json_object();
  IF(json_array_append_new(json_input_parts, json_input_part))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonNmExpression : error appending json_input_part");
    DPcErr;
  }
  package_t package = nm_expression->expression->interpretation->package;
  struct input_part *input_part = m_input_part->input_part;
  switch (input_part->type)
  {
    case nlp_input_part_type_Nil:
    {
      break;
    }
    case nlp_input_part_type_Word:
    {
      og_string string_word = OgHeapGetCell(package->hinput_part_ba, input_part->word->word_start);
      IFN(string_word) DPcErr;
      json_t *json_word = json_string(string_word);

      IF(json_object_set_new(json_input_part, "word", json_word))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMInputPart: error setting json_word");
        DPcErr;
      }
    }
    case nlp_input_part_type_Interpretation:
    case nlp_input_part_type_Digit:
    {
      json_t *json_alias = json_string(m_input_part->input_part->alias->alias);
      IF(json_object_set_new(json_input_part, "alias", json_alias))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMInputPart: error setting json_alias");
        DPcErr;
      }

      break;
    }
  }

  json_t *json_matches = json_array();
  IF(json_object_set_new(json_input_part, "input-parts", json_matches))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMInputPart: error setting json_matches");
    DPcErr;
  }

  struct m_expression *m_expressions = OgHeapGetCell(ctrl_nlp_th->hm_expression, 0);
  for (int i = 0; i < m_input_part->m_expressions_nb; i++)
  {
    struct m_expression *m_expression = m_expressions + m_input_part->m_expression_start + i;
    IFE(NlpWhyJsonMExpression(ctrl_nlp_th, nm_expression, m_input_part, m_expression, json_matches));
  }

  DONE;
}

static og_status NlpWhyJsonMExpression(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    struct m_input_part *m_input_part, struct m_expression *m_expression, json_t *json_matches)
{
  json_t *json_match = json_object();
  IF(json_array_append_new(json_matches, json_match))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression : error appending json_match");
    DPcErr;
  }
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
      m_expression->Irequest_input_part);
  IFN(request_input_part) DPcErr;

  char highlight[DPcPathSize];
  NlpRequestPositionStringHighlight(ctrl_nlp_th, request_input_part->request_position_start,
      request_input_part->request_positions_nb, DPcPathSize, highlight);

  switch (request_input_part->type)
  {
    case nlp_input_part_type_Nil:
    {
      // should not be used
      break;
    }
    case nlp_input_part_type_Word:
    {
      struct request_word *request_word = request_input_part->request_word;
      og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
      IFN(string_request_word) DPcErr;
      json_t *json_word = json_string(string_request_word);
      IF(json_object_set_new(json_match, "word", json_word))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression: error setting json_word");
        DPcErr;
      }
      break;
    }
    case nlp_input_part_type_Interpretation:
    {
      struct request_expression *request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(request_expression) DPcErr;
      struct expression *expression = request_expression->expression;

      json_t *json_package = json_string(expression->interpretation->package->id);
      IF(json_object_set_new(json_match, "package", json_package))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression: error setting json_package");
        DPcErr;
      }
      json_t *json_interpretation = json_string(expression->interpretation->id);
      IF(json_object_set_new(json_match, "interpretation", json_interpretation))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression: error setting json_interpretation");
        DPcErr;
      }
      json_t *json_expression = json_string(expression->text);
      IF(json_object_set_new(json_match, "expression", json_expression))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression: error setting json_expression");
        DPcErr;
      }
      json_t *json_highlight = json_string(highlight);
      IF(json_object_set_new(json_match, "highlight", json_highlight))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression: error setting json_highlight");
        DPcErr;
      }
      break;
    }
    case nlp_input_part_type_Digit:
    {
      // should not be used
      break;
    }

  }

  json_t *json_level = json_integer(request_input_part->level);
  IF(json_object_set_new(json_match, "level", json_level))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpWhyJsonMExpression: error setting json_level");
    DPcErr;
  }

  DONE;
}

