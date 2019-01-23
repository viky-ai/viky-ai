/*
 *  Working on super list, which are recursive interpretation that can be very long
 *  there can be only one, and it should be on a top interpretation
 *  Thoses lists are typically used to do semantic analysis
 *  A super list is necessarily a recursive expression
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : January 2019
 *  Version 1.0
 */
#include <lognlp.h>

#include "ogm_nlp.h"

// Super List glued Request Expression
struct slgre
{
  struct request_expression *request_expression;
};

static og_status NlpConsolidateSuperListInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretations,
    int Iinterpretation);
static og_status NlpSuperListGetInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package);
static int NlpSuperListAliasInterpretationGetInPackage(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package, struct super_list *super_list);
static og_bool NlpSuperListCreate(og_nlp_th ctrl_nlp_th, struct super_list *super_list);
static og_bool NlpSuperListCreateOne(og_nlp_th ctrl_nlp_th, struct super_list *super_list,
    GQueue *sorted_request_expressions);
static og_bool NlpSuperListCreateGlued(og_nlp_th ctrl_nlp_th, struct super_list *super_list,
    GQueue *sorted_request_expressions);
static og_bool NlpSuperListCreateGluedRecursive(og_nlp_th ctrl_nlp_th, struct super_list *super_list, GList *iter,
    struct slgre *slgre, int slgre_length);
static og_bool NlpSuperListCreateGluedSingle(og_nlp_th ctrl_nlp_th, struct super_list *super_list, struct slgre *slgre,
    int slgre_length);
static og_bool NlpSuperListBuild(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions,
    struct super_list *super_list, og_bool use_any_request_expressions);
static og_status NlpSuperListInputPartCreate(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct super_list *super_list, int Iinput_part);
static int NlpSuperListRequestExpressionCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data);
static int NlpSuperListRequestExpressionGlueCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data);

og_status NlpSuperListInit(og_nlp_th ctrl_nlp_th, og_string name)
{
  og_char_buffer nlpc_name[DPcPathSize];
  snprintf(nlpc_name, DPcPathSize, "%s_super_list", name);
  ctrl_nlp_th->hsuper_list = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(struct super_list), 1);
  IFN(ctrl_nlp_th->hsuper_list)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpSuperListInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  DONE;
}

og_status NlpSuperListReset(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapReset(ctrl_nlp_th->hsuper_list));
  DONE;
}

og_status NlpSuperListFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(OgHeapFlush(ctrl_nlp_th->hsuper_list));
  DONE;
}

/*
 * A super list is a recursive list called from a public interpretation
 */

og_status NlpConsolidateSuperListPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    IFE(NlpConsolidateSuperListInterpretation(ctrl_nlp_th, interpretations, i));
  }
  DONE;
}

static og_status NlpConsolidateSuperListInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretations,
    int Iinterpretation)
{
  struct interpretation *public_interpretation = interpretations + Iinterpretation;
  if (public_interpretation->scope != nlp_interpretation_scope_type_public) DONE;

  if (public_interpretation->expressions_nb != 1) DONE;
  struct expression *public_expression = public_interpretation->expressions;
  if (public_expression->aliases_nb != 1) DONE;
  struct alias *public_alias = public_expression->aliases;

  int interpretation_used = OgHeapGetCellsUsed(interpretations->package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    if (!interpretation->is_recursive) continue;
    // Super list recursive interpretation have only two expression: recursive and single
    if (interpretation->expressions_nb != 2) continue;
    if (public_alias->type != nlp_alias_type_Interpretation) continue;
    if (strcmp(interpretation->id, public_alias->id)) continue;
    interpretation->is_super_list = TRUE;
    for (int j = 0; j < interpretation->expressions_nb; j++)
    {
      if (interpretation->expressions[j].is_recursive)
      {
        interpretation->expressions[j].is_super_list = TRUE;
        break;
      }
    }
    break;
  }
  DONE;
}

/*
 * Getting a super list, which is typically an interpretation
 * that is created as a public list and which is the only public an a list
 * this is used for text analysis.
 * NOTE: normally there are no other such lists in sub agents. If this is the case we have an error.
 * NOTE: it might be interesting to control that in the interface so that this calculation is not necessary
 */
og_status NlpSuperListGet(og_nlp_th ctrl_nlp_th)
{
  struct interpret_package *interpret_packages = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);

  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    IFE(NlpSuperListGetInPackage(ctrl_nlp_th, interpret_packages + i));
  }

  DONE;
}

static og_status NlpSuperListGetInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;
  struct super_list super_list[1];

  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    if (!interpretation->is_super_list) continue;
    memset(super_list, 0, sizeof(struct super_list));
    for (int j = 0; j < interpretation->expressions_nb; j++)
    {
      if (interpretation->expressions[j].is_super_list)
      {
        super_list->recursive_expression = interpretation->expressions + j;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
        {
          NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list expression:");
          IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, super_list->recursive_expression));
        }
        struct expression *expression = super_list->recursive_expression;
        for (int a = 0; a < expression->aliases_nb; a++)
        {
          struct alias *alias = expression->aliases + a;
          if (strcmp(alias->id, expression->interpretation->id))
          {
            super_list->alias = alias;
            if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
            {
              NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list alias:");
              IFE(NlpPackageAliasLog(ctrl_nlp_th, package, super_list->alias));
            }
            IFE(NlpSuperListAliasInterpretationGetInPackage(ctrl_nlp_th, interpret_package, super_list));
            break;
          }
        }
        super_list->interpret_package = interpret_package;
      }
      else // can only be the single expression, because super_list interpretation have only two expressions
      {
        super_list->single_expression = interpretation->expressions + j;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
        {
          NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list single expression:");
          IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, super_list->single_expression));
        }
      }
    }
    if (super_list->recursive_expression != NULL)
    {
      IFE(OgHeapAppend(ctrl_nlp_th->hsuper_list, 1, super_list));
    }
  }


  DONE;
}

static int NlpSuperListAliasInterpretationGetInPackage(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package, struct super_list *super_list)
{
  package_t package = interpret_package->package;
  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    if (!strcmp(interpretation->id, super_list->alias->id))
    {
      super_list->interpretation = interpretation;
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
      {
        NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list interpretation:");
        IFE(NlpPackageInterpretationLog(ctrl_nlp_th, package, super_list->interpretation));
      }
      break;
    }
  }
  DONE;
}

og_bool NlpSuperListValidate(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part)
{
  struct input_part *input_part = OgHeapGetCell(package->hinput_part, Iinput_part);
  IFN(input_part) DPcErr;

  int super_list_used = OgHeapGetCellsUsed(ctrl_nlp_th->hsuper_list);
  struct super_list *super_lists = OgHeapGetCell(ctrl_nlp_th->hsuper_list, 0);
  IFN(super_lists) DPcErr;

  for (int i = 0; i < super_list_used; i++)
  {
    struct super_list *super_list = super_lists + i;
    if (input_part->expression->interpretation == super_list->recursive_expression->interpretation)
    {
      return FALSE;
    }
  }
  return TRUE;
}

og_bool NlpSuperListsCreate(og_nlp_th ctrl_nlp_th)
{
  int super_list_used = OgHeapGetCellsUsed(ctrl_nlp_th->hsuper_list);
  struct super_list *super_lists = OgHeapGetCell(ctrl_nlp_th->hsuper_list, 0);
  IFN(super_lists) DPcErr;

  int at_least_one_super_list_created = FALSE;
  for (int i = 0; i < super_list_used; i++)
  {
    og_bool super_list_created;
    IFE(super_list_created = NlpSuperListCreate(ctrl_nlp_th, super_lists + i));
    if (super_list_created) at_least_one_super_list_created = TRUE;
  }
  return at_least_one_super_list_created;
}

static og_bool NlpSuperListCreate(og_nlp_th ctrl_nlp_th, struct super_list *super_list)
{
  og_bool super_list_created = FALSE;
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expressions) DPcErr;

  GQueue sorted_request_expressions[1];
  g_queue_init(sorted_request_expressions);

  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    if (request_expression->expression->interpretation != super_list->interpretation) continue;
    if (request_expression->consumed_by_super_list) continue;
    g_queue_push_tail(sorted_request_expressions, request_expressions + i);
  }

  if (super_list->recursive_expression->glued)
  {
    NlpLog(DOgNlpTraceMatch, "NlpSuperListCreate: this super list is glued, we need to calculate all the glued lists");
    IFE(super_list_created = NlpSuperListCreateGlued(ctrl_nlp_th, super_list, sorted_request_expressions));
  }
  else
  {
    super_list_created = NlpSuperListCreateOne(ctrl_nlp_th, super_list, sorted_request_expressions);
    IFE(super_list_created);
  }

  g_queue_clear(sorted_request_expressions);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    int new_request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
    if (new_request_expression_used > ctrl_nlp_th->new_request_expression_start)
    {
      char buffer[DPcPathSize];
      snprintf(buffer, DPcPathSize, "NlpSuperListCreate: list of new request expression at level %d:",
          ctrl_nlp_th->level);
      IFE(NlpRequestExpressionsLog(ctrl_nlp_th, ctrl_nlp_th->new_request_expression_start, buffer));
    }
  }

  return (super_list_created);
}

static og_bool NlpSuperListCreateOne(og_nlp_th ctrl_nlp_th, struct super_list *super_list,
    GQueue *sorted_request_expressions)
{
  og_bool super_list_created = FALSE;
  // sort again to take into account scores
  g_queue_sort(sorted_request_expressions, NlpSuperListRequestExpressionCmp, NULL);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpSuperListCreateOne: list of super list request expressions at level %d:",
        ctrl_nlp_th->level);
    for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    }
  }

  og_bool super_list_built;
  IFE(super_list_built = NlpSuperListBuild(ctrl_nlp_th, sorted_request_expressions, super_list, FALSE));
  if (super_list_built) super_list_created = TRUE;
  IFE(super_list_built = NlpSuperListBuild(ctrl_nlp_th, sorted_request_expressions, super_list, TRUE));
  if (super_list_built) super_list_created = TRUE;

  return (super_list_created);
}

static og_bool NlpSuperListCreateGlued(og_nlp_th ctrl_nlp_th, struct super_list *super_list,
    GQueue *sorted_request_expressions)
{
  og_bool at_least_one_glued_super_list_created = FALSE;
  // sort in position order to facilitate the calculation of possible glued lists
  g_queue_sort(sorted_request_expressions, NlpSuperListRequestExpressionGlueCmp, ctrl_nlp_th);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpSuperListCreateGlued: list of super list request expressions at level %d:",
        ctrl_nlp_th->level);
    for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    }
  }

  int slgre_size = sorted_request_expressions->length;
  struct slgre *slgre = malloc(slgre_size * sizeof(struct slgre));
  IFN(slgre)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpSuperListCreateGlued : malloc error on slgre");
    DPcErr;
  }
  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    slgre[0].request_expression = request_expression;
    og_bool glued_super_list_created = NlpSuperListCreateGluedRecursive(ctrl_nlp_th, super_list, iter->next, slgre, 1);
    IFE(glued_super_list_created);
    if (glued_super_list_created) at_least_one_glued_super_list_created = TRUE;
  }

  DPcFree(slgre);

  return at_least_one_glued_super_list_created;
}

static og_bool NlpSuperListCreateGluedRecursive(og_nlp_th ctrl_nlp_th, struct super_list *super_list, GList *iter_first,
    struct slgre *slgre, int slgre_length)
{
  og_bool at_least_one_glued_super_list_created = FALSE;
  og_bool glued_super_list_created;
  if (!iter_first)
  {
    glued_super_list_created = NlpSuperListCreateGluedSingle(ctrl_nlp_th, super_list, slgre, slgre_length);
    IFE(glued_super_list_created);
    if (glued_super_list_created) at_least_one_glued_super_list_created = TRUE;
  }
  else
  {
    for (GList *iter = iter_first; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      og_bool are_glued = NlpRequestExpressionsAreGlued(ctrl_nlp_th, slgre[slgre_length - 1].request_expression,
          request_expression, FALSE);
      IFE(are_glued);

//      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
//      {
//        NlpLog(DOgNlpTraceMatch, "NlpRequestExpressionsAreGlued: comparing the two expressions at position %d: %s",
//            slgre_length, (are_glued ? "glued" : "not glued"));
//        IFE(NlpRequestExpressionLog(ctrl_nlp_th, slgre[slgre_length - 1].request_expression, 2));
//        IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
//      }

      if (are_glued)
      {
        slgre[slgre_length++].request_expression = request_expression;
        glued_super_list_created = NlpSuperListCreateGluedRecursive(ctrl_nlp_th, super_list, iter->next, slgre,
            slgre_length);
        IFE(glued_super_list_created);
        if (glued_super_list_created) at_least_one_glued_super_list_created = TRUE;
        slgre_length--;
      }
      else
      {
        glued_super_list_created = NlpSuperListCreateGluedSingle(ctrl_nlp_th, super_list, slgre, slgre_length);
        IFE(glued_super_list_created);
        if (glued_super_list_created) at_least_one_glued_super_list_created = TRUE;
      }
    }
  }
  return at_least_one_glued_super_list_created;
}

static og_bool NlpSuperListCreateGluedSingle(og_nlp_th ctrl_nlp_th, struct super_list *super_list, struct slgre *slgre,
    int slgre_length)
{
  if (slgre_length >= 1)
  {
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch, "NlpSuperListCreateGluedSingle: ordered candidate list:", ctrl_nlp_th->level);
      for (int i = 0; i < slgre_length; i++)
      {
        IFE(NlpRequestExpressionLog(ctrl_nlp_th, slgre[i].request_expression, 2));
      }
    }

    GQueue sorted_request_expressions[1];
    g_queue_init(sorted_request_expressions);

    for (int i = 0; i < slgre_length; i++)
    {
      g_queue_push_tail(sorted_request_expressions, slgre[i].request_expression);
    }

    og_bool super_list_created = NlpSuperListCreateOne(ctrl_nlp_th, super_list, sorted_request_expressions);
    IFE(super_list_created);

    g_queue_clear(sorted_request_expressions);

    return super_list_created;
  }
  return FALSE;
}

static og_bool NlpSuperListBuild(og_nlp_th ctrl_nlp_th, GQueue *sorted_request_expressions,
    struct super_list *super_list, og_bool use_any_request_expressions)
{
  og_bool first_expression = TRUE;
  struct request_expression *new_request_expression = NULL;

  if (use_any_request_expressions)
  {
    og_bool found_any = FALSE;
    for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
    {
      struct request_expression *request_expression = iter->data;
      if (request_expression->nb_anys > 0)
      {
        found_any = TRUE;
        break;
      }
    }
    if (!found_any)
    {
      NlpLog(DOgNlpTraceMatch, "NlpSuperListBuild: no any to build a super list.");
      return FALSE;
    }
  }

  og_string string_using_any = "";
  if (use_any_request_expressions)
  {
    string_using_any = " (using any)";
  }

  for (GList *iter = sorted_request_expressions->head; iter; iter = iter->next)
  {
    struct request_expression *request_expression = iter->data;
    if (request_expression->nb_anys > 0 && !use_any_request_expressions) continue;
    int new_request_input_part_start = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
    request_expression->consumed_by_super_list = TRUE;
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch, "NlpSuperListBuild: working on the following expression %s:", string_using_any);
      IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    }
    if (first_expression)
    {
      og_status Irequest_input_part = NlpSuperListInputPartCreate(ctrl_nlp_th, request_expression, super_list,
          super_list->single_expression->input_parts[0].self_index);

      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
      {
        char buffer[DPcPathSize];
        snprintf(buffer, DPcPathSize, "NlpSuperListBuild: list of new request input parts for first expression:");
        IFE(NlpRequestInputPartsLog(ctrl_nlp_th, new_request_input_part_start, buffer));
      }

      // start and end are not used in NlpRequestExpressionAdd
      struct match_zone_input_part match_zone_input_part[1];
      match_zone_input_part->current = Irequest_input_part;
      og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, super_list->single_expression,
          match_zone_input_part, &new_request_expression, TRUE);
      IF(request_expression_added) DPcErr;
      if (request_expression_added)
      {
        new_request_expression->super_list_status = nlp_super_list_status_Part;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          NlpLog(DOgNlpTraceMatch, "NlpSuperListBuild: new request_expression created from first expression:");
          IFE(NlpInterpretTreeLog(ctrl_nlp_th, new_request_expression, 2));
        }
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th,
            "NlpSuperListBuild: could not create new_request_expression from first expression");
        DPcErr;
      }
      first_expression = FALSE;
    }
    else
    {
      og_status Irequest_input_part = NlpSuperListInputPartCreate(ctrl_nlp_th, request_expression, super_list,
          super_list->recursive_expression->input_parts[0].self_index);

      og_status Irequest_input_part_recur = NlpSuperListInputPartCreate(ctrl_nlp_th, new_request_expression, super_list,
          super_list->recursive_expression->input_parts[1].self_index);

      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
      {
        char buffer[DPcPathSize];
        snprintf(buffer, DPcPathSize, "NlpSuperListBuild: list of new request input parts for next expression:");
        IFE(NlpRequestInputPartsLog(ctrl_nlp_th, new_request_input_part_start, buffer));
      }

      struct match_zone_input_part match_zone_input_part[2];
      match_zone_input_part[0].current = Irequest_input_part;
      match_zone_input_part[1].current = Irequest_input_part_recur;
      struct request_expression *current_new_request_expression = NULL;
      og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, super_list->recursive_expression,
          match_zone_input_part, &current_new_request_expression, TRUE);
      IF(request_expression_added) DPcErr;
      if (request_expression_added)
      {
        new_request_expression = current_new_request_expression;
        new_request_expression->super_list_status = nlp_super_list_status_Part;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          NlpLog(DOgNlpTraceMatch, "NlpSuperListBuild: new request_expression created from next expression:");
          IFE(NlpInterpretTreeLog(ctrl_nlp_th, new_request_expression, 2));
        }
      }
    }
  }

// Could not find any input part for the super list
  if (new_request_expression == NULL)
  {
    NlpLog(DOgNlpTraceMatch, "NlpSuperListBuild: new request_expression not created %s:", string_using_any);
    return FALSE;
  }

  new_request_expression->super_list_status = nlp_super_list_status_Top;
  ctrl_nlp_th->new_request_input_part_start = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    NlpLog(DOgNlpTraceMatch, "NlpSuperListBuild: new request_expression created %s:", string_using_any);
    //IFE(NlpRequestExpressionLog(ctrl_nlp_th, new_request_expression, 2));
    IFE(NlpInterpretTreeLog(ctrl_nlp_th, new_request_expression, 2));
  }

  return TRUE;
}

static og_status NlpSuperListInputPartCreate(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    struct super_list *super_list, int Iinput_part)
{
  og_status Irequest_input_part = NlpRequestInputPartAddInterpretation(ctrl_nlp_th, request_expression,
      super_list->interpret_package, Iinput_part);
  IFE(Irequest_input_part);
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;

  request_input_part->super_list_status = nlp_super_list_status_Part;

  size_t Ioriginal_request_input_part;
  struct original_request_input_part *original_request_input_part = OgHeapNewCell(
      ctrl_nlp_th->horiginal_request_input_part, &Ioriginal_request_input_part);
  IFN(original_request_input_part) DPcErr;
  original_request_input_part->Irequest_input_part = Irequest_input_part;

  return (Irequest_input_part);
}

static int NlpSuperListRequestExpressionCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data)
{
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  if (request_expression1->request_positions_nb != request_expression2->request_positions_nb)
  {
    return (request_expression2->request_positions_nb - request_expression1->request_positions_nb);
  }
  if (request_expression1->overlap_mark != request_expression2->overlap_mark)
  {
    return (request_expression1->overlap_mark - request_expression2->overlap_mark);
  }
  if (request_expression1->nb_anys != request_expression2->nb_anys)
  {
    return (request_expression2->nb_anys - request_expression1->nb_anys);
  }
  if (request_expression1->total_score != request_expression2->total_score)
  {
    double cmp = request_expression2->total_score - request_expression1->total_score;
    if (cmp > 0) return 1;
    else return -1;
  }
  if (request_expression1->level != request_expression2->level)
  {
    return (request_expression2->level - request_expression1->level);
  }

// Just to make sure it is different
  return request_expression1 - request_expression2;
}

static int NlpSuperListRequestExpressionGlueCmp(gconstpointer ptr_request_expression1,
    gconstpointer ptr_request_expression2, gpointer user_data)
{
  og_nlp_th ctrl_nlp_th = (og_nlp_th) user_data;
  struct request_expression *request_expression1 = (struct request_expression *) ptr_request_expression1;
  struct request_expression *request_expression2 = (struct request_expression *) ptr_request_expression2;

  struct request_position *request_positions = OgHeapGetCell(ctrl_nlp_th->hrequest_position, 0);
  IFN(request_positions) DPcErr;

  struct request_position *request_position1 = request_positions + request_expression1->request_position_start;
  struct request_position *request_position2 = request_positions + request_expression2->request_position_start;

  int start_position_expression1 = request_position1[0].start;
  int last_request_position1 = request_expression1->request_positions_nb - 1;
  int end_position_expression1 = request_position1[last_request_position1].start
      + request_position1[last_request_position1].length;

  int start_position_expression2 = request_position2[0].start;
  int last_request_position2 = request_expression2->request_positions_nb - 1;
  int end_position_expression2 = request_position2[last_request_position2].start
      + request_position2[last_request_position2].length;

  if (start_position_expression1 != start_position_expression2)
  {
    return (start_position_expression1 - start_position_expression2);
  }
  if (end_position_expression1 != end_position_expression2)
  {
    return (end_position_expression2 - end_position_expression1);
  }
// Just to make sure it is different
  return request_expression1 - request_expression2;
}

