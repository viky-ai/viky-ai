/*
 *  Levenshtein distance costs module function
 *  Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *  Dev: November 2009, January,August 2010
 *  Version 1.0
 */
#include "ogm_ltras.h"

PUBLIC(int) OgLtrasGetLevenshteinCosts(void *handle, struct og_stm_levenshtein_input_param *levenshtein_costs)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;
  char buffer[DPcPathSize];
  int found;

  if (!ctrl_ltras->levenshtein_costs_calculated)
  {
    struct og_stm_levenshtein_input_param *lc = ctrl_ltras->levenshtein_costs;

    /** Default value for insertion cost is 0.1 **/
    lc->insertion_cost = 0.1;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"insertion_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->insertion_cost = atof(buffer);
      if (lc->insertion_cost > 1.0) lc->insertion_cost = 1.0;
      else if (lc->insertion_cost < 0.0) lc->insertion_cost = 0.0;
    }

    /** Default value for insertion cost with same adjacent letter is 0.02 **/
    lc->same_letter_insertion_cost = 0.02;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"same_letter_insertion_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->same_letter_insertion_cost = atof(buffer);
      if (lc->same_letter_insertion_cost > 1.0) lc->same_letter_insertion_cost = 1.0;
      else if (lc->same_letter_insertion_cost < 0.0) lc->same_letter_insertion_cost = 0.0;
    }

    /** Default value for deletion cost is 0.1 **/
    lc->deletion_cost = 0.1;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"deletion_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->deletion_cost = atof(buffer);
      if (lc->deletion_cost > 1.0) lc->deletion_cost = 1.0;
      else if (lc->deletion_cost < 0.0) lc->deletion_cost = 0.0;
    }

    /** Default value for deletion cost with same adjacent letter is 0.02 **/
    lc->same_letter_deletion_cost = 0.02;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"same_letter_deletion_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->same_letter_deletion_cost = atof(buffer);
      if (lc->same_letter_deletion_cost > 1.0) lc->same_letter_deletion_cost = 1.0;
      else if (lc->same_letter_deletion_cost < 0.0) lc->same_letter_deletion_cost = 0.0;
    }

    /** Default value for substitution cost is 0.1 **/
    lc->substitution_cost = 0.1;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"substitution_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->substitution_cost = atof(buffer);
      if (lc->substitution_cost > 1.0) lc->substitution_cost = 1.0;
      else if (lc->substitution_cost < 0.0) lc->substitution_cost = 0.0;
    }

    /** Default value for substitution cost for a difference of only an accent is 0.01 **/
    lc->accents_substitution_cost = 0.01;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"accents_substitution_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->accents_substitution_cost = atof(buffer);
      if (lc->accents_substitution_cost > 1.0) lc->accents_substitution_cost = 1.0;
      else if (lc->accents_substitution_cost < 0.0) lc->accents_substitution_cost = 0.0;
    }

    /** Default value for swap cost is 0.1 **/
    lc->swap_cost = 0.1;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"swap_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->swap_cost = atof(buffer);
      if (lc->swap_cost > 1.0) lc->swap_cost = 1.0;
      else if (lc->swap_cost < 0.0) lc->swap_cost = 0.0;
    }

    /** Default value for space cost is 0.005 **/
    lc->space_cost = 0.005;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"space_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->space_cost = atof(buffer);
      if (lc->space_cost > 1.0) lc->space_cost = 1.0;
      else if (lc->space_cost < 0.0) lc->space_cost = 0.0;
    }

    /** Default value for case cost is 0.001 **/
    lc->case_cost = 0.001;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"case_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->case_cost = atof(buffer);
      if (lc->case_cost > 1.0) lc->case_cost = 1.0;
      else if (lc->case_cost < 0.0) lc->case_cost = 0.0;
    }

    /** Default value for punctuation cost is 0.000 **/
    lc->punctuation_cost = 0.000;
    IF(found=OgLtrasGetParameterValue(ctrl_ltras,"punctuation_cost",DPcPathSize,buffer)) return (0);
    if (found)
    {
      lc->punctuation_cost = atof(buffer);
      if (lc->punctuation_cost > 1.0) lc->punctuation_cost = 1.0;
      else if (lc->punctuation_cost < 0.0) lc->punctuation_cost = 0.0;
    }

  }

  memcpy(levenshtein_costs, ctrl_ltras->levenshtein_costs, sizeof(struct og_stm_levenshtein_input_param));
  ctrl_ltras->levenshtein_costs_calculated = 1;

  DONE;
}

