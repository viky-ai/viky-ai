/*
 *  Handling why an expression or an interpretation has not been found, calculation of reasons
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpWhyCalculateNmExpression(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression);
static og_status NlpWhyCalculateMinputPart(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    struct m_input_part *m_input_part);
static og_status NlpWhyCalculateMinputPartAdd(og_nlp_th ctrl_nlp_th, struct m_input_part *m_input_part,
    int Irequest_input_part);

og_status NlpWhyCalculate(og_nlp_th ctrl_nlp_th)
{
  int nm_expression_used = OgHeapGetCellsUsed(ctrl_nlp_th->hnm_expression);
  IFn(nm_expression_used) DONE;
  struct nm_expression *nm_expressions = OgHeapGetCell(ctrl_nlp_th->hnm_expression, 0);
  IFN(nm_expressions) DPcErr;
  for (int i = 0; i < nm_expression_used; i++)
  {
    IFE(NlpWhyCalculateNmExpression(ctrl_nlp_th, nm_expressions + i));
  }
  IFE(NlpWhyNotMatchingLog(ctrl_nlp_th));
  DONE;
}

static og_status NlpWhyCalculateNmExpression(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression)
{
  struct expression *expression = nm_expression->expression;
  int input_parts_nb = expression->input_parts_nb;
  nm_expression->m_input_part_start = OgHeapGetCellsUsed(ctrl_nlp_th->hm_input_part);
  IFE(OgHeapAddCells(ctrl_nlp_th->hm_input_part, input_parts_nb));

  struct m_input_part *m_input_parts = OgHeapGetCell(ctrl_nlp_th->hm_input_part, nm_expression->m_input_part_start);
  for (int i = 0; i < input_parts_nb; i++)
  {
    struct m_input_part *m_input_part = m_input_parts + i;
    m_input_part->input_part = expression->input_parts + i;
    IFE(NlpWhyCalculateMinputPart(ctrl_nlp_th, nm_expression, m_input_part));
  }

  DONE;
}

static og_status NlpWhyCalculateMinputPart(og_nlp_th ctrl_nlp_th, struct nm_expression *nm_expression,
    struct m_input_part *m_input_part)
{
  int request_input_part_used = OgHeapGetCellsUsed(ctrl_nlp_th->hrequest_input_part);
  struct request_input_part *request_input_parts = OgHeapGetCell(ctrl_nlp_th->hrequest_input_part, 0);
  IFN(request_input_parts) DPcErr;

  for (int i = 0; i < request_input_part_used; i++)
  {
    struct request_input_part *request_input_part = request_input_parts + i;
    if (request_input_part->Iinput_part == m_input_part->input_part->self_index)
    {
      IFE(NlpWhyCalculateMinputPartAdd(ctrl_nlp_th, m_input_part, i));
    }
  }

  DONE;
}

static og_status NlpWhyCalculateMinputPartAdd(og_nlp_th ctrl_nlp_th, struct m_input_part *m_input_part,
    int Irequest_input_part)
{
  size_t Im_expression;
  struct m_expression *m_expression = OgHeapNewCell(ctrl_nlp_th->hm_expression, &Im_expression);
  IFn(m_expression) DPcErr;
  m_expression->Irequest_input_part = Irequest_input_part;
  IFn(m_input_part->m_expressions_nb)
  {
    m_input_part->m_expression_start = Im_expression;
  }
  m_input_part->m_expressions_nb++;
  DONE;
}

