/*
 *  Calculate solutions for the expressions tree.
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_bool NlpSolutionCalculateRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression, struct alias *mothers_alias);
static og_bool NlpSolutionNeedJsComputeRecursive(og_nlp_th ctrl_nlp_th, json_t *json_package_solution);
static og_bool NlpSolutionAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_package_solution);
static og_bool NlpSolutionCombine(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_bool NlpSolutionBuildSolutionsQueue(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpSolutionMergeObjectsRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    og_string solution_key, json_t *sub_solution);
static og_bool NlpSolutionComputeJS(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_package_solution);
static og_status NlpSolutionClean(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_status NlpSolutionCleanRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression);

og_status NlpSolutionCalculate(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpSolutionClean(ctrl_nlp_th, request_expression));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceSolution)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
        "NlpSolutionCalculate calculate solutions for following request expression:");
    IFE(NlpRequestExpressionAnysLog(ctrl_nlp_th, request_expression));
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 0));
  }

  IFE(NlpSolutionCalculateRecursive(ctrl_nlp_th, request_expression, request_expression, NULL));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceSolution)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog,
        "NlpSolutionCalculate solutions calculated following request expression:");
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, request_expression, 0));
  }

  DONE;
}

static og_bool NlpSolutionCalculateRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression, struct alias *mothers_alias)
{
  og_bool solution_built = FALSE;
  og_bool must_combine_solution = FALSE;
  request_expression->mothers_alias = mothers_alias;

  if (request_expression->sorted_flat_list->length > 0)
  {
    // Create List with all sub_request_expression->solution
    if (!json_is_array(request_expression->json_solution))
    {
      json_decrefp(&request_expression->json_solution);
    }
    if (!request_expression->json_solution)
    {
      request_expression->json_solution = json_array();
    }

    for (GList *iter = request_expression->sorted_flat_list->head; iter; iter = iter->next)
    {
      int Irequest_expression = GPOINTER_TO_INT(iter->data);
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          Irequest_expression);
      og_bool sub_solution_built = NlpSolutionCalculateRecursive(ctrl_nlp_th, root_request_expression,
          sub_request_expression, mothers_alias);
      IFE(sub_solution_built);
      if (sub_solution_built)
      {
        must_combine_solution = TRUE;
      }
    }

  }
  else
  {

    for (int i = 0; i < request_expression->orips_nb; i++)
    {
      struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
      IFN(request_input_part) DPcErr;

      if (request_input_part->type == nlp_input_part_type_Word)
      {
        if (request_input_part->input_part && request_input_part->input_part->type == nlp_input_part_type_Number)
        {
          must_combine_solution = TRUE;
        }
        else if (request_input_part->input_part && request_input_part->input_part->type == nlp_input_part_type_Regex)
        {
          must_combine_solution = TRUE;
        }
      }
      else if (request_input_part->type == nlp_input_part_type_Interpretation)
      {
        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;
        og_bool sub_solution_built = NlpSolutionCalculateRecursive(ctrl_nlp_th, root_request_expression,
            sub_request_expression, request_input_part->input_part->alias);
        IFE(sub_solution_built);
        if (sub_solution_built)
        {
          must_combine_solution = TRUE;
        }
      }
    }
  }

  if (request_expression->Irequest_any >= 0)
  {
    must_combine_solution = TRUE;
  }

  json_t *json_package_solution = request_expression->expression->json_solution;
  if (json_package_solution == NULL)
  {
    json_package_solution = request_expression->expression->interpretation->json_solution;
  }

  og_bool need_js_compute = NlpSolutionNeedJsComputeRecursive(ctrl_nlp_th, json_package_solution);

  if (must_combine_solution)
  {
    if (need_js_compute)
    {
      IFE(solution_built = NlpSolutionComputeJS(ctrl_nlp_th, request_expression, json_package_solution));
    }
    else
    {
      IFE(solution_built = NlpSolutionCombine(ctrl_nlp_th, request_expression));
    }
  }
  else
  {
    if (need_js_compute)
    {
      IFE(solution_built = NlpSolutionComputeJS(ctrl_nlp_th, request_expression, json_package_solution));
    }
    else
    {
      IFE(solution_built = NlpSolutionAdd(ctrl_nlp_th, request_expression, json_package_solution));
    }
  }

  return solution_built;
}

og_status NlpRequestSolutionString(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression, int size,
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

og_status NlpSolutionString(og_nlp_th ctrl_nlp_th, json_t *json_solution, int size, char *string)
{
  if (json_solution)
  {
    return NlpJsonToBuffer(json_solution, string, size, NULL, JSON_ENCODE_ANY);
  }
  else
  {
    string[0] = 0;
  }
  DONE;
}

static og_bool NlpSolutionAdd(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_package_solution)
{
  og_bool solution_built = FALSE;
  if (json_package_solution != NULL)
  {
    request_expression->json_solution = json_deep_copy(json_package_solution);
    solution_built = TRUE;
  }
  return solution_built;
}

static og_bool NlpSolutionCombine(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  GQueue *solutions = request_expression->tmp_solutions;
  IFE(NlpSolutionBuildSolutionsQueue(ctrl_nlp_th, request_expression));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceSolution)
  {
    NlpLog(DOgNlpTraceSolution, "NlpSolutionCombine: list of solutions for expression:");
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 0));
    for (GList *iter = solutions->head; iter; iter = iter->next)
    {
      struct alias_solution *alias_solution = iter->data;
      json_t *json_solution = alias_solution->json_solution;
      og_char_buffer json_solution_string[DOgMlogMaxMessageSize / 2];
      IFE(NlpJsonToBuffer(json_solution, json_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
      NlpLog(DOgNlpTraceSolution, "  solution: %s", json_solution_string);
    }
  }

  int nb_solutions = g_queue_get_length(solutions);
  if (nb_solutions <= 0) return FALSE;
  struct alias_solution *first_alias_solution = solutions->head->data;
  if (nb_solutions == 1 && !first_alias_solution->is_sorted_flat_list)
  {
    request_expression->json_solution = json_incref(first_alias_solution->json_solution);
  }
  else
  {
    IFE(NlpSolutionMergeObjects(ctrl_nlp_th, request_expression));
  }

  return TRUE;
}

static og_bool NlpSolutionBuildSolutionsQueue(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  int alias_solutions_nb = 0;

  if (request_expression->sorted_flat_list->length > 0)
  {
    if (!json_is_array(request_expression->json_solution))
    {
      json_decrefp(&request_expression->json_solution);
    }
    if (!request_expression->json_solution)
    {
      request_expression->json_solution = json_array();
    }

    for (GList *iter = request_expression->sorted_flat_list->head; iter; iter = iter->next)
    {
      int Irequest_expression = GPOINTER_TO_INT(iter->data);
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          Irequest_expression);
      IFN(sub_request_expression) DPcErr;

      json_t * sub_solution = sub_request_expression->json_solution;
      if (sub_solution == NULL)
      {
        sub_solution = json_null();
      }

      og_string alias_name = "root";
      struct alias *alias = request_expression->mothers_alias;
      if (alias)
      {
        alias_name = alias->alias;
      }

      char solution[DPcPathSize];
      NlpSolutionString(ctrl_nlp_th, sub_solution, DPcPathSize, solution);
      NlpLog(DOgNlpTraceSolution, "NlpSolutionBuildSolutionsQueue: for alias '%s' (flat list) building solution : %s",
          alias_name, solution);

      if (alias_solutions_nb >= DOgAliasSolutionSize)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionBuildSolutionsQueue: alias_solutions_nb (%d) >= DOgAliasSolutionSize (%d)",
            alias_solutions_nb, DOgAliasSolutionSize);
        DPcErr;

      }
      struct alias_solution *alias_solution = g_slice_new0(struct alias_solution);
      alias_solution->alias = alias;
      alias_solution->json_solution = json_incref(sub_solution);
      alias_solution->is_sorted_flat_list = TRUE;

      g_queue_push_tail(request_expression->tmp_solutions, alias_solution);
      alias_solutions_nb++;
    }

  }
  else
  {

    for (int i = 0; i < request_expression->orips_nb; i++)
    {
      struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
      IFN(request_input_part) DPcErr;

      if (request_input_part->type == nlp_input_part_type_Interpretation)
      {
        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;

        json_t * sub_solution = sub_request_expression->json_solution;
        if (sub_solution == NULL)
        {
          sub_solution = json_null();
        }

        struct alias *alias = request_input_part->input_part->alias;

        char solution[DPcPathSize];
        NlpSolutionString(ctrl_nlp_th, sub_solution, DPcPathSize, solution);
        NlpLog(DOgNlpTraceSolution, "NlpSolutionBuildSolutionsQueue: for alias '%s' building solution (interpretation):"
            " %s", alias->alias, solution);

        if (alias_solutions_nb >= DOgAliasSolutionSize)
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionBuildSolutionsQueue: alias_solutions_nb (%d) >= DOgAliasSolutionSize (%d)",
              alias_solutions_nb, DOgAliasSolutionSize);
          DPcErr;

        }
        struct alias_solution *alias_solution = g_slice_new0(struct alias_solution);
        alias_solution->alias = alias;
        alias_solution->json_solution = json_incref(sub_request_expression->json_solution);

        g_queue_push_tail(request_expression->tmp_solutions, alias_solution);
        alias_solutions_nb++;
      }
      else if (request_input_part->type == nlp_input_part_type_Word)
      {
        struct request_word *request_word = request_input_part->request_word;
        og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
        IFN(string_request_word) DPcErr;
        if (request_word->is_number && request_input_part->interpret_word_as_number)
        {
          double value = request_word->number_value;
          json_t *json_solution_number = NULL;
          if ((json_int_t) ((value * 100) / 100) == value)
          {
            json_solution_number = json_integer(value);
          }
          else
          {
            json_solution_number = json_real(value);
          }

          struct alias *alias = request_input_part->input_part->alias;
          char solution[DPcPathSize];
          NlpSolutionString(ctrl_nlp_th, json_solution_number, DPcPathSize, solution);
          NlpLog(DOgNlpTraceSolution, "NlpSolutionBuildSolutionsQueue: for alias '%s' building solution (word): %s",
              alias->alias, solution);
          if (alias_solutions_nb >= DOgAliasSolutionSize)
          {
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionBuildSolutionsQueue: alias_solutions_nb (%d) >= DOgAliasSolutionSize (%d)",
                alias_solutions_nb, DOgAliasSolutionSize);
            DPcErr;

          }
          struct alias_solution *alias_solution = g_slice_new0(struct alias_solution);
          alias_solution->alias = alias;
          alias_solution->json_solution = json_solution_number;
          g_queue_push_tail(request_expression->tmp_solutions, alias_solution);
          alias_solutions_nb++;

        }
        else if (request_word->is_regex)
        {
          json_t *json_solution_regex = NULL;
          json_solution_regex = json_string(string_request_word);

          struct alias *alias = request_input_part->input_part->alias;
          char solution[DPcPathSize];
          NlpSolutionString(ctrl_nlp_th, json_solution_regex, DPcPathSize, solution);
          NlpLog(DOgNlpTraceSolution, "NlpSolutionBuildSolutionsQueue: for alias '%s' building solution (regex): %s",
              alias->alias, solution);
          if (alias_solutions_nb >= DOgAliasSolutionSize)
          {
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionBuildSolutionsQueue: alias_solutions_nb (%d) >= DOgAliasSolutionSize (%d)",
                alias_solutions_nb, DOgAliasSolutionSize);
            DPcErr;

          }
          struct alias_solution *alias_solution = g_slice_new0(struct alias_solution);
          alias_solution->alias = alias;
          alias_solution->json_solution = json_solution_regex;
          g_queue_push_tail(request_expression->tmp_solutions, alias_solution);
          alias_solutions_nb++;

        }
      }

    }
  }

  // Any might not exist or be attached to this request_expression, in that case Irequest_any = -1
  // This means that all the words have been used or that the positions of unused words are incompatible
  if (request_expression->Irequest_any >= 0)
  {
    struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->Irequest_any);
    IFN(request_any) DPcErr;

    char string_any[DPcPathSize];
    IFE(NlpRequestAnyString(ctrl_nlp_th, request_any, DPcPathSize, string_any));

    json_t *json_solution_any = json_string(string_any);

    if (request_expression->expression->alias_any_input_part_position >= request_expression->expression->aliases_nb)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionBuildSolutionsQueue: "
          "request_expression->expression->alias_any_input_part_position (%d) >="
          " request_expression->expression->aliases_nb (%d)",
          request_expression->expression->alias_any_input_part_position, request_expression->expression->aliases_nb);
      DPcErr;
    }

    struct alias *alias = request_expression->expression->aliases
        + request_expression->expression->alias_any_input_part_position;

    NlpLog(DOgNlpTraceSolution, "NlpSolutionBuildSolutionsQueue: for alias '%s' building solution (any): %s",
        alias->alias, string_any);

    if (alias_solutions_nb >= DOgAliasSolutionSize)
    {
      NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionBuildSolutionsQueue: alias_solutions_nb (%d) >= DOgAliasSolutionSize (%d)",
          alias_solutions_nb, DOgAliasSolutionSize);
      DPcErr;

    }

    struct alias_solution *alias_solution = g_slice_new0(struct alias_solution);
    alias_solution->alias = alias;
    alias_solution->json_solution = json_solution_any;
    g_queue_push_tail(request_expression->tmp_solutions, alias_solution);
    alias_solutions_nb++;
  }

  DONE;
}

og_status NlpSolutionMergeObjects(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  // Combining the objects into a single object
  if (!json_is_object(request_expression->json_solution))
  {
    json_decrefp(&request_expression->json_solution);
  }
  if (!request_expression->json_solution)
  {
    request_expression->json_solution = json_object();
  }

  for (GList *iter = request_expression->tmp_solutions->head; iter; iter = iter->next)
  {
    struct alias_solution *alias_solution = iter->data;
    json_t *sub_solution = alias_solution->json_solution;

    og_string solution_key_str = "root_expression";
    og_string solution_key = NULL;
    if (alias_solution->alias)
    {
      solution_key = alias_solution->alias->alias;
      solution_key_str = solution_key;
    }

    // On est dans une interpretation récursive (une liste : alias_solution->is_sorted_flat_list)
    if (alias_solution->is_sorted_flat_list)
    {
      // a list must an array
      json_t *array = NULL;
      json_t *expression_solution = NULL;
      if (json_is_object(request_expression->json_solution))
      {
        expression_solution = json_object_get(request_expression->json_solution, solution_key);
        if (json_is_array(expression_solution))
        {
          array = expression_solution;
        }
      }
      else if (json_is_array(request_expression->json_solution))
      {
        array = request_expression->json_solution;
      }

      // build a new array
      if (array == NULL)
      {
        array = json_array();
        if (solution_key)
        {
          IF(json_object_set_new(request_expression->json_solution, solution_key, array))
          {
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjects: list must be an array, new array failed :"
                " '%s' => []", solution_key_str);
            DPcErr;
          }
        }
        else
        {
          json_decrefp(&request_expression->json_solution);
          request_expression->json_solution = array;
        }
      }

      if (expression_solution != NULL && !json_is_null(expression_solution) && array != expression_solution)
      {
        // append preview value to array
        IF(json_array_append_new(array, expression_solution))
        {
          og_char_buffer buffer[DPcPathSize];
          IFE(NlpJsonToBuffer(expression_solution, buffer, DPcPathSize, NULL, 0));

          NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjects: list must be an array, append preview solution"
              " failed : '%s' => '%s'", solution_key_str, buffer);
          DPcErr;
        }
      }

      if (sub_solution != NULL && !json_is_null(sub_solution) && array != sub_solution)
      {
        // merge both array
        if (json_is_array(sub_solution))
        {
          size_t index = 0;
          json_t *value = NULL;

          json_array_foreach(sub_solution, index, value)
          {
            if (value != NULL && !json_is_null(value))
            {
              IF(json_array_append(array, value))
              {
                og_char_buffer buffer[DPcPathSize];
                IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
                NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjects: append sub_solution merged each simple value"
                    " in existing array failed : '%s' => '%s'", solution_key_str, buffer);
                DPcErr;
              }
            }
          }
        }

        // append sub_solution in array
        else
        {
          IF(json_array_append(array, sub_solution))
          {
            og_char_buffer buffer[DPcPathSize];
            IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: append sub_solution in array"
                " failed : '%s' => '%s'", solution_key_str, buffer);
            DPcErr;
          }
        }

      }

    }
    else
    {
      IFE(NlpSolutionMergeObjectsRecursive(ctrl_nlp_th, request_expression, solution_key, sub_solution));
    }
  }

  DONE;
}

static og_status NlpSolutionMergeObjectsRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    og_string solution_key, json_t *sub_solution)
{
  IFN(sub_solution) CONT;
  if (json_is_null(sub_solution)) CONT;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceSolution)
  {
    NlpLog(DOgNlpTraceSolution, "NlpSolutionMergeObjectsRecursive: working on sub_solution:");
    og_char_buffer json_solution_string[DOgMlogMaxMessageSize / 2];
    IFE(NlpJsonToBuffer(sub_solution, json_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
    NlpLog(DOgNlpTraceSolution, "  sub_solution: %s", json_solution_string);
  }

  // recursive part of list
  if (solution_key == NULL && json_is_object(sub_solution))
  {
    const char *key = NULL;
    json_t *value = NULL;
    json_object_foreach(sub_solution, key, value)
    {
      IFE(NlpSolutionMergeObjectsRecursive(ctrl_nlp_th, request_expression, key, value));
    }
  }

  // object value
  else if (json_is_object(sub_solution))
  {
    const char *key = NULL;
    json_t *value = NULL;
    json_object_foreach(sub_solution, key, value)
    {
      IFE(NlpSolutionMergeObjectsRecursive(ctrl_nlp_th, request_expression, key, value));
    }
  }

  // other value : array, string, number, boolean
  else
  {
    json_t *expression_solution = json_object_get(request_expression->json_solution, solution_key);

    if (expression_solution == NULL || json_is_null(expression_solution))
    {
      IF(json_object_set(request_expression->json_solution, solution_key, sub_solution))
      {
        og_char_buffer buffer[DPcPathSize];
        IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: set simple value failed : '%s' => '%s'",
            solution_key, buffer);
        DPcErr;
      }
    }
    else if (json_is_array(expression_solution))
    {
      // merge both array
      if (json_is_array(sub_solution))
      {
        size_t index = 0;
        json_t *value = NULL;

        json_array_foreach(sub_solution, index, value)
        {
          IF(json_array_append(expression_solution, value))
          {
            og_char_buffer buffer[DPcPathSize];
            IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: append each simple value"
                " in existing array failed : '%s' => '%s'", solution_key, buffer);
            DPcErr;
          }
        }
      }

      // append sub_solution in array
      else
      {
        IF(json_array_append(expression_solution, sub_solution))
        {
          og_char_buffer buffer[DPcPathSize];
          IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
          NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: append simple value in existing array"
              " failed : '%s' => '%s'", solution_key, buffer);
          DPcErr;
        }
      }
    }
    else
    {
      // merge in new array
      json_t *json_new_array_values = json_array();
      IF(json_object_set_new(request_expression->json_solution, solution_key, json_new_array_values))
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: build new array"
            " with simple solution failed : '%s'", solution_key);
        DPcErr;
      }

      IF(json_array_append(json_new_array_values, expression_solution))
      {
        og_char_buffer buffer[DPcPathSize];
        IFE(NlpJsonToBuffer(expression_solution, buffer, DPcPathSize, NULL, 0));
        NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive : merge in new array append expression_solution"
            " in new array failed : '%s' => '%s'", solution_key, buffer);
        DPcErr;
      }

      if (json_is_array(sub_solution))
      {
        size_t index = 0;
        json_t *value = NULL;

        json_array_foreach(sub_solution, index, value)
        {
          IF(json_array_append(json_new_array_values, value))
          {
            og_char_buffer buffer[DPcPathSize];
            IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive: merge in new array append "
                "simple value failed : '%s' => '%s'", solution_key, buffer);
            DPcErr;
          }
        }
      }
      else
      {
        IF(json_array_append(json_new_array_values, sub_solution))
        {
          og_char_buffer buffer[DPcPathSize];
          IFE(NlpJsonToBuffer(sub_solution, buffer, DPcPathSize, NULL, 0));
          NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionMergeObjectsRecursive : merge in new array append sub_solution"
              " in new array failed : '%s' => '%s'", solution_key, buffer);
          DPcErr;
        }
      }

    }

  }

  DONE;

}

static og_status NlpSolutionComputeJSRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_new_solution, json_t **json_solution_computed_value)
{
  og_bool solution_built = FALSE;

  if (json_solution_computed_value == NULL)
  {
    return FALSE;
  }

  if (json_is_object(json_new_solution))
  {
    for (void *iter = json_object_iter(json_new_solution); iter; iter = json_object_iter_next(json_new_solution, iter))
    {
      json_t *value = json_object_iter_value(iter);

      json_t *local_json_solution_computed_value = NULL;
      IFE(NlpSolutionComputeJSRecursive(ctrl_nlp_th, request_expression, value, &local_json_solution_computed_value));

      if (local_json_solution_computed_value != NULL)
      {

        // avoid loop in final json
        if (local_json_solution_computed_value != value && local_json_solution_computed_value != json_new_solution)
        {

          IF(json_object_iter_set_new(json_new_solution, iter, local_json_solution_computed_value))
          {
            NlpThrowErrorTh(ctrl_nlp_th, "NlpSolutionComputeJSRecursive : json_object_iter_set_new failed");
            DPcErr;
          }
        }

      }

    }

    *json_solution_computed_value = json_new_solution;

  }
  else if (json_is_string(json_new_solution))
  {

    og_string string_value = json_string_value(json_new_solution);
    int string_value_length = strlen(string_value);

    // check if we need to evaluate javascript
    if (string_value[0] == '`' && string_value[string_value_length - 1] == '`')
    {
      og_string string_value_js = string_value + 1;
      int string_value_js_size = string_value_length - 2;

      // evaluate javascript
      IF(NlpJsEval(ctrl_nlp_th, string_value_js_size, string_value_js, json_solution_computed_value))
      {
        struct expression *ex = request_expression->expression;
        NlpThrowErrorTh(ctrl_nlp_th,
            "NlpSolutionComputeJSRecursive : Error creating json_solution_computed_value for expression '%s' in '%s' '%s'",
            ex->text, ex->interpretation->slug, ex->interpretation->id);
        DPcErr;
      }

      solution_built = TRUE;

    }
  }

  return solution_built;
}

static og_status NlpSolutionBuildRawText(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  // Adding the "raw_text" variable for use in javascript
  unsigned char *raw_text_name = "raw_text";
  struct request_position *request_position = OgHeapGetCell(ctrl_nlp_th->hrequest_position,
      request_expression->request_position_start);
  IFN(request_position) DPcErr;

  int start_position_expression = request_position[0].start;
  int last_request_position = request_expression->request_positions_nb - 1;
  int end_position_expression = request_position[last_request_position].start + request_position[last_request_position].length;

  int start_position_any = start_position_expression;
  int end_position_any = end_position_expression;

  if (request_expression->Irequest_any >= 0)
  {
    struct request_any *request_any = OgHeapGetCell(ctrl_nlp_th->hrequest_any, request_expression->Irequest_any);
    IFN(request_any) DPcErr;

    struct request_word *first_request_word_any = request_any->queue_request_words->head->data;
    start_position_any = first_request_word_any->start_position;

    struct request_word *last_request_word_any = request_any->queue_request_words->tail->data;
    end_position_any = last_request_word_any->start_position + last_request_word_any->length_position;
  }

  int start_position = start_position_expression;
  if (start_position_any < start_position) start_position = start_position_any;

  int end_position = end_position_expression;
  if (end_position < end_position_any) end_position = end_position_any;

  int length_position = end_position - start_position;

  const unsigned char *raw_text_string = ctrl_nlp_th->request_sentence + start_position;
  if (length_position > DOgNlpMaxRawTextSize)
  {
    unsigned char *p = (unsigned char *) (raw_text_string + DOgNlpMaxRawTextSize);
    p = g_utf8_find_prev_char(raw_text_string,p);
    length_position = p - raw_text_string;
  }
  char raw_text_string_value[DOgNlpMaxRawTextSize*2+9];
  int j=0;
  raw_text_string_value[j++]='"';
  for (int i=0; i<length_position; i++)
  {
    if (raw_text_string[i]== '"')
    {
      raw_text_string_value[j++]='\\';
    }
    raw_text_string_value[j++]=raw_text_string[i];
  }
  raw_text_string_value[j++]='"';
  raw_text_string_value[j]=0;
  int raw_text_string_value_length=j;
  IFE(NlpJsAddVariable(ctrl_nlp_th, raw_text_name, raw_text_string_value, raw_text_string_value_length));

  NlpLog(DOgNlpTraceSolution, "NlpSolutionBuildRawText: raw_text is '%s'",raw_text_string);

  DONE;
}



static og_bool NlpSolutionComputeJS(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    json_t *json_package_solution)
{

  // reset local variable
  IFE(NlpJsStackLocalWipe(ctrl_nlp_th));

  IFE(NlpSolutionBuildSolutionsQueue(ctrl_nlp_th, request_expression));

  IFE(NlpSolutionBuildRawText(ctrl_nlp_th, request_expression));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceSolution)
  {
    og_char_buffer solution[DPcPathSize];
    NlpSolutionString(ctrl_nlp_th, json_package_solution, DPcPathSize, solution);
    NlpLog(DOgNlpTraceSolution, "NlpSolutionComputeJS: working on solution %s:", solution)
    NlpLog(DOgNlpTraceSolution, "NlpSolutionComputeJS: list of solutions for expression:");
    IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 0));

    for (GList *iter = request_expression->tmp_solutions->head; iter; iter = iter->next)
    {
      struct alias_solution *alias_solution = iter->data;
      json_t *json_solution = alias_solution->json_solution;
      og_char_buffer json_solution_string[DOgMlogMaxMessageSize / 2];
      IFE(NlpJsonToBuffer(json_solution, json_solution_string, DOgMlogMaxMessageSize / 2, NULL, 0));
      og_string alias_name = "root";
      if (alias_solution->alias) alias_name = alias_solution->alias->alias;
      NlpLog(DOgNlpTraceSolution, "  alias '%s' solution: %s", alias_name, json_solution_string);
    }
  }

  // We want to add the alias as a variable whose name is the alias and whose value is its associated solution
  for (GList *iter = request_expression->tmp_solutions->head; iter; iter = iter->next)
  {
    struct alias_solution *alias_solution = iter->data;
    if (alias_solution->alias != NULL)
    {
      IFE(NlpJsAddVariableJson(ctrl_nlp_th, alias_solution->alias->alias, alias_solution->json_solution));
    }
  }

  NlpLog(DOgNlpTraceSolution, "Transforming solution by executing JS code");

  og_bool solution_built = FALSE;

  // Now we scan the json_package_solution to get some executable code
  // If there are, we create a copy of the json structure and amend it with the result of the execution of the code
  json_t *json_new_solution = json_deep_copy(json_package_solution);

  json_t *json_solution_computed_value = NULL;
  IFE(NlpSolutionComputeJSRecursive(ctrl_nlp_th, request_expression, json_new_solution, &json_solution_computed_value));

  if (json_solution_computed_value != NULL)
  {
    if (!json_is_null(json_solution_computed_value))
    {
      request_expression->json_solution = json_solution_computed_value;
      solution_built = TRUE;
    }
    else
    {
      request_expression->json_solution = NULL;
      json_decrefp(&json_solution_computed_value);
    }

    if (json_new_solution != json_solution_computed_value)
    {
      json_decrefp(&json_new_solution);
    }
  }
  else
  {
    request_expression->json_solution = json_new_solution;
    solution_built = TRUE;
  }

  // reset local variable
  IFE(NlpJsStackLocalWipe(ctrl_nlp_th));

  return solution_built;
}

static og_bool NlpSolutionNeedJsComputeRecursive(og_nlp_th ctrl_nlp_th, json_t *json_package_solution)
{
  if (json_package_solution == NULL || json_is_null(json_package_solution))
  {
    return FALSE;
  }

  if (json_is_object(json_package_solution))
  {
    for (void *iter = json_object_iter(json_package_solution); iter;
        iter = json_object_iter_next(json_package_solution, iter))
    {
      json_t *value = json_object_iter_value(iter);
      og_bool need_js_compute = NlpSolutionNeedJsComputeRecursive(ctrl_nlp_th, value);
      IFE(need_js_compute);
      if (need_js_compute) return TRUE;
    }

  }
  else if (json_is_string(json_package_solution))
  {

    og_string string_value = json_string_value(json_package_solution);
    int string_value_length = strlen(string_value);

    // check if we need to evaluate javascript
    if (string_value[0] == '`' && string_value[string_value_length - 1] == '`')
    {
      return TRUE;
    }
  }

  return FALSE;
}

static og_status NlpSolutionClean(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpSolutionCleanRecursive(ctrl_nlp_th, request_expression, request_expression));
  DONE;
}

static og_status NlpSolutionCleanRecursive(og_nlp_th ctrl_nlp_th, struct request_expression *root_request_expression,
    struct request_expression *request_expression)
{
  if (request_expression->json_solution)
  {
    json_decrefp(&request_expression->json_solution);
  }

  for (GList *iter = request_expression->tmp_solutions->head; iter; iter = iter->next)
  {
    struct alias_solution *alias_solution = iter->data;

    json_decrefp(&alias_solution->json_solution);

    g_slice_free(struct alias_solution, alias_solution);
    iter->data = NULL;
  }
  g_queue_clear(request_expression->tmp_solutions);

  if (request_expression->sorted_flat_list->length > 0)
  {
    // Create List with all sub_request_expression->solution
    // request_expression->json_solution = json_array();

    for (GList *iter = request_expression->sorted_flat_list->head; iter; iter = iter->next)
    {
      int Irequest_expression = GPOINTER_TO_INT(iter->data);
      struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
          Irequest_expression);
      IFN(sub_request_expression) DPcErr;
      IFE(NlpSolutionCleanRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression));
    }

  }
  else
  {
    for (int i = 0; i < request_expression->orips_nb; i++)
    {
      struct request_input_part *request_input_part = NlpGetRequestInputPart(ctrl_nlp_th, request_expression, i);
      IFN(request_input_part) DPcErr;

      if (request_input_part->type == nlp_input_part_type_Interpretation)
      {
        struct request_expression *sub_request_expression = OgHeapGetCell(ctrl_nlp_th->hrequest_expression,
            request_input_part->Irequest_expression);
        IFN(sub_request_expression) DPcErr;
        IFE(NlpSolutionCleanRecursive(ctrl_nlp_th, root_request_expression, sub_request_expression));
      }
    }
  }

  DONE;
}

