/*
 *  Implementation of the Levenshtein distance function
 *  Copyright (c) 2007 Pertimm by Patrick Constant, Lo�s Rigouste
 *  Dev : September 2007
 *  Version 1.0
 */

#include "ogm_stm.h"




PUBLIC(double) OgStmLevenshtein(void *hstm, int ustring1, unsigned char *string1, int ustring2, unsigned char *string2,
    struct og_stm_levenshtein_input_param *lev_input_params, struct og_stm_levenshtein_output_param *lev_output_params)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;
  unsigned char messages[DOgStmMaxWordLength + 9][DPcPathSize];
  LEV_OPERATION temp_path[DOgStmMaxWordLength + 9];
  int i, j, old_i;

  IFE(OgStmLevenshteinFast(hstm, ustring1, string1, ustring2, string2, lev_input_params));

  /* Now we backtrack to find the best ctrl_stm->path */
  lev_output_params->path_length = 0;
  i = ustring1 / 2;
  j = ustring2 / 2;
  while (i != 0 && j != 0)
  {
    temp_path[lev_output_params->path_length] = ctrl_stm->path[i][j];
    lev_output_params->path_length++;
    old_i = i;
    i = ctrl_stm->from_i[i][j];
    j = ctrl_stm->from_j[old_i][j];
  }

  for (i = 0; i < lev_output_params->path_length; i++)
  {
    lev_output_params->levenshtein_path[i] = temp_path[lev_output_params->path_length - 1 - i];
    if (ctrl_stm->loginfo->trace & DOgStmTracePath)
    {
      OgMsg(ctrl_stm->hmsg, "", DOgMsgDestInLog, messages[lev_output_params->path_length - 1 - i]);
    }
  }

// Eventually copy score to output_params
  memcpy(lev_output_params->score, ctrl_stm->score, (ctrl_stm->length1 + 1) * sizeof(double*));
  for (i = 0; i <= ctrl_stm->length1; i++)
  {
    memcpy(lev_output_params->score[i], ctrl_stm->score[i], (ctrl_stm->length2 + 1) * sizeof(double));
  }

  lev_output_params->length1 = ctrl_stm->length1;
  lev_output_params->length2 = ctrl_stm->length2;

  return lev_output_params->score[ctrl_stm->length1][ctrl_stm->length2];

}

PUBLIC(double) OgStmLevenshteinFast(void *hstm, int ustring1, unsigned char *string1, int ustring2,
    unsigned char *string2, struct og_stm_levenshtein_input_param *lev_params)
{
  struct og_ctrl_stm *ctrl_stm = (struct og_ctrl_stm *) hstm;
  char B1[DOgStmMaxWordLength * 2 + 9], B2[DOgStmMaxWordLength * 2 + 9];
  int iB1, iB2;

  // if string are to big, we go out with the maximum score possible (based on maximum of all costs and maximum size
  //of the two strings)
  if (ustring1 > 2 * DOgStmMaxWordLength || ustring2 > 2 * DOgStmMaxWordLength)
  {
    double Levenshtein_score = StmGetMaxLevenshteinDistance(hstm, lev_params);
    if (ctrl_stm->loginfo->trace & DOgStmTraceScore)
    {
      memset(B1, 0, DOgStmMaxWordLength * 2);
      memset(B2, 0, DOgStmMaxWordLength * 2);
      IFE(OgUniToCp(ustring1,string1,DOgStmMaxWordLength*2,&iB1,B1,DOgCodePageUTF8,0,0));
      IFE(OgUniToCp(ustring2,string2,DOgStmMaxWordLength*2,&iB2,B2,DOgCodePageUTF8,0,0));
      OgMsg(ctrl_stm->hmsg, "", DOgMsgDestInLog + DOgMsgSeverityWarning,
          "OgStmLevenshteinFast: String too long, string "
              "input \"%s\" with length %d and input \"%s\" with length %d are not treated, Levenshtein score is set to %d",
          B1, iB1, B2, iB2, Levenshtein_score);
    }
    return Levenshtein_score;
  }

  if (ctrl_stm->loginfo->trace & DOgStmTraceScore)
  {
    memset(B1, 0, DOgStmMaxWordLength * 2);
    memset(B2, 0, DOgStmMaxWordLength * 2);
    IFE(OgUniToCp(ustring1,string1,DOgStmMaxWordLength*2,&iB1,B1,DOgCodePageUTF8,0,0));
    IFE(OgUniToCp(ustring2,string2,DOgStmMaxWordLength*2,&iB2,B2,DOgCodePageUTF8,0,0));
    OgMsg(ctrl_stm->hmsg, "", DOgMsgDestInLog, "OgStmLevenshteinFast: Levenshtein distance between '%.*s' and '%.*s'",
        iB1, B1, iB2, B2);
  }

  ctrl_stm->length1 = ustring1 / 2;
  ctrl_stm->length2 = ustring2 / 2;

  IFE(StmInitBorderScores(ctrl_stm, string1, string2, lev_params));
  IFE(StmComputeLevenshteinBoard(ctrl_stm, string1, string2, lev_params));

  if (ctrl_stm->loginfo->trace & DOgStmTracePath)
  {
    IFE(StmLogLevenshteinPathAndScore(ctrl_stm, ustring1, string1, ustring2, string2));
  }

  return ctrl_stm->score[ctrl_stm->length1][ctrl_stm->length2];
}

PUBLIC(double) OgStmLevenshteinFastDefaultParams(void *hstm, int ustring1, unsigned char *string1, int ustring2,
    unsigned char *string2)
{
  struct og_stm_levenshtein_input_param lev_params[1];
  double return_value;

  IFE(OgStmInitDefaultCosts(hstm, lev_params));
  IFE(StmInitDefaultSpaceCost(hstm, TRUE));
  IFE(StmInitDefaultSpaceCost(hstm, FALSE));
  IFE(return_value = OgStmLevenshteinFast(hstm, ustring1, string1, ustring2, string2, lev_params));

  return return_value;
}


