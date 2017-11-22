/*
 *  Handling why an expression or an interpretation has not been found
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

og_status NlpCalculateScore(og_nlp_th ctrl_nlp_th, struct request_expression *request_expression)
{
  IFE(NlpCalculateScoreRecursive(ctrl_nlp_th, request_expression, request_expression));

  DONE;
}

