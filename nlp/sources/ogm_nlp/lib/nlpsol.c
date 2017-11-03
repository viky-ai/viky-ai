/*
 *  Calculate solutions for the expressions tree.
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_bool NlpSolutionCalculateRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression);
static og_bool NlpSolutionAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_bool NlpSolutionCombine(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpSolutionMergeObjects(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_solution, GQueue *solutions);
static og_status NlpSolutionMergeObjectsRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_solution, GList *iter_solutions);

og_status NlpSolutionCalculate(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpSolutionCalculateRecursive(ctrl_nlp_th, request_expression, request_expression));
  DONE;
}

static og_bool NlpSolutionCalculateRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression)
{
  og_bool must_combine_solution = FALSE;

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Word)
    {
      struct request_word *request_word = request_input_part->request_word;
      og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
      IFN(string_request_word) DPcErr;
    }
    else if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      og_bool sub_solution_built = NlpSolutionCalculateRecursive(ctrl_nlp_th, root_request_expression,
          sub_request_expression);
      IFE(sub_solution_built);
      if (sub_solution_built) must_combine_solution = TRUE;
    }

    if (request_expression->expression->alias_any_input_part_position == i + 1)
    {
      if (request_expression->Irequest_any >= 0)
      {
        must_combine_solution = TRUE;
      }
    }
  }

  og_bool solution_built = FALSE;
  if (must_combine_solution)
  {
    IFE(solution_built = NlpSolutionCombine(ctrl_nlp_th, request_expression));
  }
  else
  {
    IFE(solution_built = NlpSolutionAdd(ctrl_nlp_th, request_expression));
  }

  return solution_built;
}

og_status NlpSolutionString(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int size,
    char *string)
{
  if (request_expression->json_solution)
  {
    return NlpJsonToBuffer(request_expression->json_solution, string, size, NULL, 0);
  }
  else
  {
    string[0] = 0;
  }
  DONE;
}

static og_bool NlpSolutionAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  og_bool solution_built = FALSE;
  if (request_expression->expression->json_solution != NULL)
  {
    request_expression->json_solution = request_expression->expression->json_solution;
    json_incref(request_expression->json_solution);
    solution_built = TRUE;
  }
  else if (request_expression->expression->interpretation->json_solution != NULL)
  {
    request_expression->json_solution = request_expression->expression->interpretation->json_solution;
    json_incref(request_expression->json_solution);
    solution_built = TRUE;
  }
  return solution_built;
}

static og_bool NlpSolutionCombine(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  GQueue solutions[1];
  g_queue_init(solutions);

  for (int i = 0; i < request_expression->orips_nb; i++)
  {
    struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
    IFN(request_input_part) DPcErr;

    if (request_input_part->type == nlp_input_part_type_Interpretation)
    {
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          request_input_part->Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      IFX(sub_request_expression->json_solution)
      {
        g_queue_push_tail(solutions, sub_request_expression->json_solution);
      }
    }
    if (request_expression->expression->alias_any_input_part_position == i + 1)
    {
      struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->Irequest_any);
      IFN(request_any) DPcErr;

      char string_any[DPcPathSize];
      NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any);

      json_t *json_solution_any = json_object();
      json_t *json_solution_string_any = json_string(string_any);
      IF(json_object_set_new(json_solution_any, "any", json_solution_string_any))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionCombine: error setting json_sub_expression_any");
        DPcErr;
      }
      g_queue_push_tail(solutions, json_solution_any);
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceSolution)
  {
    NlpLog(DOgNlpTraceSolution, "NlpSolutionCombine: list of solutions for expression:");
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 0));
    for (GList *iter = solutions->head; iter; iter = iter->next)
    {
      json_t *json_solution = iter->data;
      og_char_buffer json_solution_string[DOgMlogMaxMessageSize / 2];
      IFE(NlpJsonToBuffer(json_solution, json_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
      NlpLog(DOgNlpTraceSolution, "  solution: %s", json_solution_string);
    }
  }

  int nb_solutions = g_queue_get_length(solutions);
  if (nb_solutions <= 0) return FALSE;
  else if (nb_solutions == 1)
  {
    request_expression->json_solution = solutions->head->data;
  }
  else
  {
    int nb_objects = 0;
    int nb_arrays = 0;
    for (GList *iter = solutions->head; iter; iter = iter->next)
    {
      json_t *json_sub_solution = iter->data;
      if (json_is_object(json_sub_solution)) nb_objects++;
      else if (json_is_array(json_sub_solution)) nb_arrays++;
      else
      {
        og_char_buffer json_sub_solution_string[DOgMlogMaxMessageSize / 2];
        IFE(NlpJsonToBuffer(json_sub_solution, json_sub_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionCombine: json_sub_solution should be array or object : %s",
            json_sub_solution_string);
        DPcErr;
      }
    }
    if (nb_arrays <= 0)
    {
      // Combining the objects into a single object
      json_t *json_solution = json_object();
      IFE(NlpSolutionMergeObjects(ctrl_nlp_th, request_expression, json_solution, solutions));
      request_expression->json_solution = json_solution;
    }
    else
    {
      //TODO: append array and add objects into newly formed array
    }
  }
  g_queue_clear(solutions);

  return TRUE;
}

static og_status NlpSolutionMergeObjects(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_solution, GQueue *solutions)
{
  GList *iter_solutions = solutions->head;
  return NlpSolutionMergeObjectsRecursive(ctrl_nlp_th, request_expression, json_solution, iter_solutions);
}

static og_status NlpSolutionMergeObjectsRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_solution, GList *iter_solutions)
{
  IFN(iter_solutions) DONE;
  json_t *json_solution_first = iter_solutions->data;
  const char *key;
  json_t *value;
  json_object_foreach(json_solution_first, key, value)
  {
    if (json_object_get(json_solution, key)) continue;
    og_bool several_values = FALSE;
    for (GList *iter = iter_solutions->next; iter; iter = iter->next)
    {
      json_t *json_sub_solution = iter->data;
      json_t *sub_value = json_object_get(json_sub_solution, key);
      if (sub_value) {
        several_values = TRUE;
        break;
      }
    }
    if (several_values)
    {
      json_t *json_array_values = json_array();
      for (GList *iter = iter_solutions; iter; iter = iter->next)
      {
        json_t *json_sub_solution = iter->data;
        json_t *sub_value = json_object_get(json_sub_solution, key);
        IFN(sub_value) continue;
        IF(json_array_append_new(json_array_values, sub_value))
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive : Error while adding json_array_values");
          DPcErr;
        }

      }
      IF(json_object_set(json_solution, key, json_array_values))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: error setting key '%s'", key);
        DPcErr;
      }
    }
    else
    {
      IF(json_object_set(json_solution, key, value))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: error setting key '%s'", key);
        DPcErr;
      }
    }
  }
  return NlpSolutionMergeObjectsRecursive(ctrl_nlp_th, request_expression, json_solution, iter_solutions->next);
}

