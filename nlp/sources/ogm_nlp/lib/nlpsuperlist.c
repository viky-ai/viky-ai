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
      }
    }
    break;
  }
  DONE;
}

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
          if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
          {
            NlpLog(DOgNlpTraceMatch, "NlpSuperListGetInPackage: found super list expression:");
            IFE(NlpPackageExpressionLog(ctrl_nlp_th, package, ctrl_nlp_th->super_list));
          }
        }
        else
        {
          NlpThrowErrorTh(ctrl_nlp_th, "NlpSuperListGetInPackage : cannot have more than one super list expression");
          DPcErr;
        }
      }
    }
  }
  DONE;
}

