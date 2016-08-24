/*
 * stmlevutils.c
 *
 *  Created on: 3 avr. 2015
 *      Author: caroline
 */

#include "ogm_stm.h"

double StmGetMaxLevenshteinDistance(void *hstm, struct og_stm_levenshtein_input_param *lev_input_params)
{

  // Get the maximum cost of all the costs
  double cost_max = lev_input_params->accents_substitution_cost;
  if (lev_input_params->case_cost > cost_max)
  {
    cost_max = lev_input_params->case_cost;
  }
  if (lev_input_params->deletion_cost > cost_max)
  {
    cost_max = lev_input_params->deletion_cost;
  }
  if (lev_input_params->insertion_cost > cost_max)
  {
    cost_max = lev_input_params->insertion_cost;
  }
  if (lev_input_params->same_letter_deletion_cost > cost_max)
  {
    cost_max = lev_input_params->same_letter_deletion_cost;
  }
  if (lev_input_params->same_letter_insertion_cost > cost_max)
  {
    cost_max = lev_input_params->same_letter_insertion_cost;
  }
  if (lev_input_params->substitution_cost > cost_max)
  {
    cost_max = lev_input_params->substitution_cost;
  }
  if (lev_input_params->swap_cost > cost_max)
  {
    cost_max = lev_input_params->swap_cost;
  }

  // Return the maximum levenshtein distance : the maximum size + 1 * maximum cost
  return (DOgStmMaxWordLength + 1) * cost_max;
}

og_status StmLogLevenshteinPathAndScore(struct og_ctrl_stm *ctrl_stm, int ustring1, unsigned char *string1,
    int ustring2, unsigned char* string2)
{
  char string1_utf8[DOgStmMaxWordLength];
  int istring1;
  char string2_utf8[DOgStmMaxWordLength];
  int istring2;

  IFE(OgUniToCp(ustring1, string1, DPcPathSize, &istring1, string1_utf8, DOgCodePageUTF8, 0, 0));
  IFE(OgUniToCp(ustring2, string2, DPcPathSize, &istring2, string2_utf8, DOgCodePageUTF8, 0, 0));
  OgMsg(ctrl_stm->hmsg, "", DOgMsgDestInLog,
      "StmLogLevenshteinPathAndScore : Levenshtein distance between \"%s\" and "
          "\"%s\"",
      string1_utf8, string2_utf8);

  char buffer[DOgStmMaxWordLength * 2 + 9][100];
  int ibuffer = 0;
  int i = ustring1 / 2;
  int j = ustring2 / 2;

  unsigned char letter1[64], letter2[64];
  while (!(i == 0 && j == 0))
  {
    switch (ctrl_stm->path[i][j])
    {
      case same:
        snprintf(buffer[ibuffer], DPcPathSize, "Chars %s and %s are identical, score=%f",
            StmUtf8Letter(string1, i, letter1), StmUtf8Letter(string2, j, letter2), ctrl_stm->score[i][j]);
        break;
      case insertion:
        snprintf(buffer[ibuffer], DPcPathSize, "Insertion of %s, score=%f",
            StmUtf8Letter(string2, j, letter2), ctrl_stm->score[i][j]);
        break;
      case deletion:
        snprintf(buffer[ibuffer], DPcPathSize, "Deletion of %s, score=%f",
            StmUtf8Letter(string1, i, letter1), ctrl_stm->score[i][j]);
        break;
      case substitution:
        snprintf(buffer[ibuffer], DPcPathSize, "Substitution of %s to %s, score=%f",
            StmUtf8Letter(string1, i, letter1), StmUtf8Letter(string2, j, letter2), ctrl_stm->score[i][j]);
        break;
      case swap:
        snprintf(buffer[ibuffer], DPcPathSize, "Swap %c%c becomes %c%c, score=%f",
            string1[(i - 2) * 2 + 1], string1[(i - 1) * 2 + 1], string2[(j - 2) * 2 + 1], string2[(j - 1) * 2 + 1],
            ctrl_stm->score[i][j]);
        break;
    }
    int old_i = i;
    i = ctrl_stm->from_i[i][j];
    j = ctrl_stm->from_j[old_i][j];
    ibuffer++;
  }

  for (int k = ibuffer - 1; k >= 0; k--)
  {
    OgMsg(ctrl_stm->hmsg, "", DOgMsgDestInLog, buffer[k]);
  }

  DONE;
}

unsigned char *StmUtf8Letter(unsigned char *string, int i, unsigned char *buffer)
{
  int ibuffer;
  unsigned char uni[2];
  uni[0] = string[(i - 1) * 2];
  uni[1] = string[(i - 1) * 2 + 1];
  IF(OgUniToCp(2,uni,10,&ibuffer,buffer,DOgCodePageUTF8,0,0))
  {
    buffer[0] = 0;
  }
  return (buffer);
}
