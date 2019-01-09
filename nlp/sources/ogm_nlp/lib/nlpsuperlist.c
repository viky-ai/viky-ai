/*
 *  Working on super list, which are recursive interpretation that can be very long
 *  there can be only one, and it should be on a top interpretation
 *  Thoses lists are typically used to do semantic analysis
 *  A super list is necessarily a recursive expression
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : January 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpConsolidateSuperListInterpretation(og_nlp_th ctrl_nlp_th, struct interpretation *interpretations,
    int Iinterpretation);
static og_status NlpSuperListGetInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package);
static int NlpSuperListAliasInterpretationGetInPackage(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package);
static og_bool NlpSuperListMotherPublicGetInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package);

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

  ctrl_nlp_th->super_list = NULL;

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
  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    if (!interpretation->is_super_list) continue;
    for (int j = 0; j < interpretation->expressions_nb; j++)
    {
      if (interpretation->expressions[j].is_super_list)
      {
        if (ctrl_nlp_th->super_list == NULL)
        {
          ctrl_nlp_th->super_list = interpretation->expressions + j;
          if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
          {
            NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list expression:");
            IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, ctrl_nlp_th->super_list));
          }
          struct expression *expression = ctrl_nlp_th->super_list;
          for (int a = 0; a < expression->aliases_nb; a++)
          {
            struct alias *alias = expression->aliases + a;
            if (strcmp(alias->id, expression->interpretation->id))
            {
              ctrl_nlp_th->super_list_alias = alias;
              if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
              {
                NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list alias:");
                IFE(NlpPackageAliasLog(ctrl_nlp_th, package, ctrl_nlp_th->super_list_alias));
              }
              IFE(NlpSuperListAliasInterpretationGetInPackage(ctrl_nlp_th, interpret_package));
              og_bool found = NlpSuperListMotherPublicGetInPackage(ctrl_nlp_th, interpret_package);
              IFE(found);
              if (!found)
              {
                NlpLog(DOgNlpTraceConsolidate,
                    "NlpSuperListGetInPackage: public mother not found, super list not created:");
                ctrl_nlp_th->super_list = NULL;
                ctrl_nlp_th->super_list_alias = NULL;
                ctrl_nlp_th->super_list_interpretation = NULL;
                ctrl_nlp_th->super_list_single = NULL;
                ctrl_nlp_th->super_list_interpret_package = NULL;
                ctrl_nlp_th->super_list_public_mother = NULL;
                DONE;
              }
              break;
            }
          }
          ctrl_nlp_th->super_list_interpret_package = interpret_package;
        }
        else
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpSuperListGetInPackage : cannot have more than one super list expression");
          DPcErr;
        }
      }
      else
      {
        ctrl_nlp_th->super_list_single = interpretation->expressions + j;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
        {
          NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list single expression:");
          IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, ctrl_nlp_th->super_list_single));
        }

      }
    }
  }
  DONE;
}

static int NlpSuperListAliasInterpretationGetInPackage(og_nlp_th ctrl_nlp_th,
    struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;
  struct interpretation *interpretations = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretations) DPcErr;
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretations + i;
    if (!strcmp(interpretation->id, ctrl_nlp_th->super_list_alias->id))
    {
      ctrl_nlp_th->super_list_interpretation = interpretation;
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
      {
        NlpLog(DOgNlpTraceConsolidate, "NlpSuperListGetInPackage: found super list interpretation:");
        IFE(NlpPackageInterpretationLog(ctrl_nlp_th, package, ctrl_nlp_th->super_list_interpretation));
      }
      break;
    }
  }
  DONE;
}

static og_bool NlpSuperListMotherPublicGetInPackage(og_nlp_th ctrl_nlp_th, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;
  struct expression *expressions = OgHeapGetCell(package->hexpression, 0);
  IFN(expressions) DPcErr;
  int expressions_used = OgHeapGetCellsUsed(package->hexpression);
  for (int i = 0; i < expressions_used; i++)
  {
    struct expression *expression = expressions + i;
    if (expression->aliases_nb != 1) continue;
    if (expression->input_parts_nb != 1) continue;
    struct alias *alias = expression->aliases + 0;
    if (alias->type != nlp_alias_type_Interpretation) continue;
    if (!strcmp(alias->id, ctrl_nlp_th->super_list->interpretation->id))
    {
      // Checks that the public mother is the top expression
      // we will see later if it is necessary to expand this limitation
      for (int j = 0; j < expressions_used; j++)
      {
        struct expression *expr = expressions + j;
        for (int a = 0; a < expr->aliases_nb; a++)
        {
          struct alias *ali = expr->aliases + a;
          if (ali->type != nlp_alias_type_Interpretation) continue;
          if (!strcmp(ali->id, expression->interpretation->id))
          {
            return FALSE;
          }
        }
      }
      ctrl_nlp_th->super_list_public_mother = expression;
      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceConsolidate)
      {
        NlpLog(DOgNlpTraceConsolidate, "NlpSuperListMotherPublicGetInPackage: found super list public mother:");
        IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, ctrl_nlp_th->super_list_public_mother));
      }
      return TRUE;
    }
  }
  return FALSE;
}

og_bool NlpSuperListValidate(og_nlp_th ctrl_nlp_th, package_t package, int Iinput_part)
{
  if (ctrl_nlp_th->super_list == NULL) return TRUE;
  struct input_part *input_part = OgHeapGetCell(package->hinput_part, Iinput_part);
  IFN(input_part) DPcErr;
  if (input_part->expression->interpretation == ctrl_nlp_th->super_list->interpretation)
  {
    return FALSE;
  }
  return TRUE;
}

og_status NlpSuperListCreate(og_nlp_th ctrl_nlp_th)
{
  ctrl_nlp_th->level++;
  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
  {
    char buffer[DPcPathSize];
    snprintf(buffer, DPcPathSize, "NlpSuperListCreate: list of all request expression at level %d:",
        ctrl_nlp_th->level);
    IFE(NlpRequestExpressionsLog(ctrl_nlp_th, 0, buffer));
  }

  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expressions = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);
  IFN(request_expressions) DPcErr;

  og_bool first_expression = TRUE;
  struct request_expression *new_request_expression = NULL;
  for (int i = 0; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expressions + i;
    int new_request_input_part_start = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
    if (request_expression->expression->interpretation != ctrl_nlp_th->super_list_interpretation) continue;
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch, "NlpSuperListCreate: working on the following expression:");
      IFE(NlpRequestExpressionLog(ctrl_nlp_th, request_expression, 2));
    }
    if (first_expression)
    {
      og_status Irequest_input_part = NlpRequestInputPartAddInterpretation(ctrl_nlp_th, request_expression,
          ctrl_nlp_th->super_list_interpret_package, ctrl_nlp_th->super_list_single->input_parts[0].self_index);
      IFE(Irequest_input_part);
      struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
          Irequest_input_part);
      IFN(request_input_part) DPcErr;

      size_t Ioriginal_request_input_part;
      struct original_request_input_part *original_request_input_part = OgHeapNewCell(
          ctrl_nlp_th->horiginal_request_input_part, &Ioriginal_request_input_part);
      IFN(original_request_input_part) DPcErr;
      original_request_input_part->Irequest_input_part = Irequest_input_part;

      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
      {
        char buffer[DPcPathSize];
        snprintf(buffer, DPcPathSize, "NlpSuperListValidate: list of new request input parts for first expressions:");
        IFE(NlpRequestInputPartsLog(ctrl_nlp_th, new_request_input_part_start, buffer));
      }

      // start and end are not used in NlpRequestExpressionAdd
      struct match_zone_input_part match_zone_input_part[1];
      match_zone_input_part->current = Irequest_input_part;
      og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, ctrl_nlp_th->super_list_single,
          match_zone_input_part, &new_request_expression, TRUE);
      IF(request_expression_added) DPcErr;
      if (request_expression_added)
      {
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          NlpLog(DOgNlpTraceMatch, "NlpSuperListCreate: new request_expression created from super_list_single:");
          //IFE(NlpRequestExpressionLog(ctrl_nlp_th, new_request_expression, 2));
          IFE(NlpInterpretTreeLog(ctrl_nlp_th, new_request_expression, 2));
        }
      }
      else
      {
        NlpThrowErrorTh(ctrl_nlp_th,
            "NlpSuperListCreate: could not create new_request_expression from super_list_single");
        DPcErr;
      }
      first_expression = FALSE;
    }
    else
    {
      og_status Irequest_input_part = NlpRequestInputPartAddInterpretation(ctrl_nlp_th, request_expression,
          ctrl_nlp_th->super_list_interpret_package, ctrl_nlp_th->super_list->input_parts[0].self_index);
      IFE(Irequest_input_part);
      struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
          Irequest_input_part);
      IFN(request_input_part) DPcErr;

      size_t Ioriginal_request_input_part;
      struct original_request_input_part *original_request_input_part = OgHeapNewCell(
          ctrl_nlp_th->horiginal_request_input_part, &Ioriginal_request_input_part);
      IFN(original_request_input_part) DPcErr;
      original_request_input_part->Irequest_input_part = Irequest_input_part;

      og_status Irequest_input_part_recur = NlpRequestInputPartAddInterpretation(ctrl_nlp_th, new_request_expression,
          ctrl_nlp_th->super_list_interpret_package, ctrl_nlp_th->super_list->input_parts[1].self_index);
      IFE(Irequest_input_part_recur);
      struct request_input_part *request_input_part_recur = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part,
          Irequest_input_part_recur);
      IFN(request_input_part_recur) DPcErr;

      original_request_input_part = OgHeapNewCell(ctrl_nlp_th->horiginal_request_input_part,
          &Ioriginal_request_input_part);
      IFN(original_request_input_part) DPcErr;
      original_request_input_part->Irequest_input_part = Irequest_input_part_recur;

      if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
      {
        char buffer[DPcPathSize];
        snprintf(buffer, DPcPathSize, "NlpSuperListValidate: list of new request input parts for next expressions:");
        IFE(NlpRequestInputPartsLog(ctrl_nlp_th, new_request_input_part_start, buffer));
      }

      struct match_zone_input_part match_zone_input_part[2];
      match_zone_input_part[0].current = Irequest_input_part;
      match_zone_input_part[1].current = Irequest_input_part_recur;
      struct request_expression *current_new_request_expression = NULL;
      og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, ctrl_nlp_th->super_list,
          match_zone_input_part, &current_new_request_expression, TRUE);
      IF(request_expression_added) DPcErr;
      if (request_expression_added)
      {
        new_request_expression = current_new_request_expression;
        if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
        {
          NlpLog(DOgNlpTraceMatch, "NlpSuperListCreate: new request_expression created from super_list:");
          //IFE(NlpRequestExpressionLog(ctrl_nlp_th, new_request_expression, 2));
          IFE(NlpInterpretTreeLog(ctrl_nlp_th, new_request_expression, 2));
        }
      }
    }
  }

  // Could not find any input part for the super list
  if (new_request_expression == NULL) DONE;

  ctrl_nlp_th->level++;
  og_status Irequest_input_part = NlpRequestInputPartAddInterpretation(ctrl_nlp_th, new_request_expression,
      ctrl_nlp_th->super_list_interpret_package, ctrl_nlp_th->super_list_public_mother->input_parts[0].self_index);
  IFE(Irequest_input_part);
  struct request_input_part *request_input_part = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, Irequest_input_part);
  IFN(request_input_part) DPcErr;

  size_t Ioriginal_request_input_part;
  struct original_request_input_part *original_request_input_part = OgHeapNewCell(
      ctrl_nlp_th->horiginal_request_input_part, &Ioriginal_request_input_part);
  IFN(original_request_input_part) DPcErr;
  original_request_input_part->Irequest_input_part = Irequest_input_part;

  struct match_zone_input_part match_zone_input_part[1];
  match_zone_input_part->current = Irequest_input_part;
  og_bool request_expression_added = NlpRequestExpressionAdd(ctrl_nlp_th, ctrl_nlp_th->super_list_public_mother,
      match_zone_input_part, &new_request_expression, TRUE);
  IF(request_expression_added) DPcErr;
  if (request_expression_added)
  {
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      NlpLog(DOgNlpTraceMatch, "NlpSuperListCreate: new request_expression created from super_list_single:");
      //IFE(NlpRequestExpressionLog(ctrl_nlp_th, new_request_expression, 2));
      IFE(NlpInterpretTreeLog(ctrl_nlp_th, new_request_expression, 2));
    }
  }

  DONE;
}

