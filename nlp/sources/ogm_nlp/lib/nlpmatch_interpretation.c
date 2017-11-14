/*
 *  Handling log messages for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : September 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_bool NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression);
static og_bool NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int input_length, unsigned char *input, struct interpret_package *interpret_package);

og_bool NlpMatchInterpretations(og_nlp_th ctrl_nlp_th)
{
  og_bool at_least_one_input_part_added = FALSE;
  int request_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_expression);
  struct request_expression *request_expression_all = OgHeapGetCell(ctrl_nlp_th->hrequest_expression, 0);

  for (int i = ctrl_nlp_th->new_request_expression_start; i < request_expression_used; i++)
  {
    struct request_expression *request_expression = request_expression_all + i;
    IFN(request_expression) DPcErr;
    int at_least_one_input_part_added_here = NlpMatchInterpretation(ctrl_nlp_th, request_expression);
    IFE(at_least_one_input_part_added_here);
    if (at_least_one_input_part_added_here) at_least_one_input_part_added = TRUE;
  }

  if (at_least_one_input_part_added)
  {
    if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceMatch)
    {
      char buffer[DPcPathSize];
      snprintf(buffer, DPcPathSize, "List of new request expression at level %d:", ctrl_nlp_th->level);
      IFE(NlpRequestExpressionsLog(ctrl_nlp_th, ctrl_nlp_th->new_request_expression_start, buffer));

      snprintf(buffer, DPcPathSize, "List of new request input parts deducted at level %d:", ctrl_nlp_th->level);
      IFE(NlpRequestInputPartsLog(ctrl_nlp_th, ctrl_nlp_th->new_request_input_part_start, buffer));
    }

  }
  return at_least_one_input_part_added;
}

static og_bool NlpMatchInterpretation(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  struct interpretation *interpretation = request_expression->expression->interpretation;
  NlpLog(DOgNlpTraceMatch, "Looking for input parts for interpretation '%s' '%s' containing expression '%.*s':",
      interpretation->slug, interpretation->id, DPcPathSize, request_expression->expression->text);

  unsigned char input[DPcAutMaxBufferSize + 9];
  int input_length = strlen(interpretation->id);
  memcpy(input, interpretation->id, input_length);
  input[input_length++] = '\1';
  input[input_length] = 0;

  og_bool at_least_one_input_part_added = FALSE;
  int interpret_package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  for (int i = 0; i < interpret_package_used; i++)
  {
    struct interpret_package *interpret_package = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, i);
    IFN(interpret_package) DPcErr;

    og_bool input_part_added = NlpMatchInterpretationInPackage(ctrl_nlp_th, request_expression, input_length, input,
        interpret_package);
    IFE(input_part_added);
    if (input_part_added) at_least_one_input_part_added = TRUE;
  }

  return at_least_one_input_part_added;
}

static og_bool NlpMatchInterpretationInPackage(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression,
    int input_length, unsigned char *input, struct interpret_package *interpret_package)
{
  package_t package = interpret_package->package;

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  int found_input_part = 0;
  if ((retour = OgAufScanf(package->ha_interpretation_id, input_length, input, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int Iinput_part;
      unsigned char *p = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      NlpLog(DOgNlpTraceMatch, "    found input part %d in interpret package %d", Iinput_part,
          interpret_package->self_index)
      IFE(NlpRequestInputPartAddInterpretation(ctrl_nlp_th, request_expression, interpret_package, Iinput_part));
      found_input_part = 1;

    }
    while ((retour = OgAufScann(package->ha_interpretation_id, &iout, out, nstate0, &nstate1, states)));
  }

  return (found_input_part);
}

