/*
 * stmlevfast.c
 *
 *  Created on: 3 avr. 2015
 *      Author: caroline
 */
#include "ogm_stm.h"

static og_status StmInitInsertionAndDeletion(struct og_ctrl_stm *ctrl_stm, unsigned char *string, int is_an_insertion,
    int length, struct og_stm_levenshtein_input_param *lev_params);
static og_bool StmIsPunctuation(struct og_ctrl_stm *ctrl_stm, int c);
static og_status StmGetSubstitutionCost(struct og_ctrl_stm *ctrl_stm, int c1, int c2, double *substitution_cost);

/**
 *
 * First we prepare Levenshtein distance by initializing the border of M matrix.
 * If we compare chiens to niche, vertical scores are insertion scores and horizontal
 * scores are deletion scores.
 *
 * |   |   | C | H | I | E | N | S |
 * |---|---|---|---|---|---|---|---|
 * |   | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
 * | N | 1 |   |   |   |   |   |   |
 * | I | 2 |   |   |   |   |   |   |
 * | C | 3 |   |   |   |   |   |   |
 * | H | 4 |   |   |   |   |   |   |
 * | E | 5 |   |   |   |   |   |   |
 *
 */
og_status StmInitBorderScores(struct og_ctrl_stm *ctrl_stm, unsigned char *string1, unsigned char *string2,
    struct og_stm_levenshtein_input_param *lev_params)
{
  for (int i = 0; i <= ctrl_stm->length1; i++)
  {
    memset(ctrl_stm->score[i], 0, (ctrl_stm->length2 + 1) * sizeof(double));
    memset(ctrl_stm->from_i[i], 0, (ctrl_stm->length2 + 1) * sizeof(int));
    memset(ctrl_stm->from_j[i], 0, (ctrl_stm->length2 + 1) * sizeof(int));
    memset(ctrl_stm->path[i], 0, (ctrl_stm->length2 + 1) * sizeof(LEV_OPERATION));
  }

  // init deletions (first horizontal line in the matric M)
  IFE(StmInitInsertionAndDeletion(ctrl_stm, string1, FALSE, ctrl_stm->length1,lev_params));

  //init insertions (first vertical line in the matric M)
  IFE(StmInitInsertionAndDeletion(ctrl_stm, string2, TRUE, ctrl_stm->length2,lev_params));

  ctrl_stm->score[0][0] = 0;
  ctrl_stm->from_i[0][0] = ctrl_stm->from_j[0][0] = 0;
  ctrl_stm->path[0][0] = same;

  DONE;

}

//TODO to make optimization, maybe in the future we could make two separate functions to avoid if statements but we will have repetitions of code

/**
 * This fonction fill the first line of the matrix M, either vertical or horizontal depending on the parameter is_an_insertion
 * if TRUE, it is for the horizontal line, if FALSE for the vertical one
 * we need to precise specific cost for space character, punctuation and same letter, since they have specific cost
 * for insertion and deletion.
 */
static og_status StmInitInsertionAndDeletion(struct og_ctrl_stm *ctrl_stm, unsigned char *string, int is_an_insertion,
    int length,
    struct og_stm_levenshtein_input_param *lev_params)
{
  int c, prev;
  c = prev = 0;

  for (int i = 0; i <= length; i++)
  {
    if (i != 0)
    {
      if (c != 0) prev = c;
      c = (string[(i - 1) * 2] << 8) + string[(i - 1) * 2 + 1];

      if (is_an_insertion)
      {
        // Init for insertion
        ctrl_stm->score[0][i] = lev_params->insertion_cost + ctrl_stm->score[0][i - 1];

        // Init for insertion of a same letter
        if (lev_params->same_letter_insertion_cost > 0)
        {
          if (prev != 0 && prev == c)
          {
            ctrl_stm->score[0][i] = lev_params->same_letter_insertion_cost + ctrl_stm->score[0][i - 1];
          }
        }

        // Init for insertion of a space
        if (lev_params->space_cost > 0)
        {
          if (OgUniIsspace(c))
          {
            ctrl_stm->score[0][i] = lev_params->space_cost + ctrl_stm->score[0][i - 1];
          }
        }

        // Init for insertion of a punctuation
        if (lev_params->punctuation_cost > 0)
        {
          if (OgLipIsPunctuation(&ctrl_stm->lip_conf, c))
          {
            ctrl_stm->score[0][i] = lev_params->punctuation_cost + ctrl_stm->score[0][i - 1];
          }
        }

        ctrl_stm->path[0][i] = insertion;

      }
      else
      {
        // Init for deletion
        ctrl_stm->score[i][0] = lev_params->deletion_cost + ctrl_stm->score[i - 1][0];

        // Init for deletion of a same letter
        if (lev_params->same_letter_deletion_cost > 0)
        {
          if (prev != 0 && prev == c)
          {
            ctrl_stm->score[i][0] = lev_params->same_letter_deletion_cost + ctrl_stm->score[i - 1][0];
          }
        }

        // Init for deletion of a space
        if (lev_params->space_cost > 0)
        {
          if (OgUniIsspace(c))
          {
            ctrl_stm->score[i][0] = lev_params->space_cost + ctrl_stm->score[i - 1][0];
          }
        }
      }

      // Init for deletion of a punctuation
      if (lev_params->punctuation_cost > 0)
      {
        if (OgLipIsPunctuation(&ctrl_stm->lip_conf, c))
        {
          ctrl_stm->score[i][0] = lev_params->punctuation_cost + ctrl_stm->score[i - 1][0];
        }
      }

      ctrl_stm->path[i][0] = deletion;
    }

    ctrl_stm->from_i[i][0] = i - 1;
    ctrl_stm->from_j[i][0] = 0;
  }

  DONE;
}

/**
 *  We compute a cost matrix :
 *
 *  Cost(i,j)=0 if A(i)=B(j) and Cost(i,j)=1 if A(i)!=B(j)
 *
 *  |   | C | H | I | E | N | S |
 *  |---|---|---|---|---|---|---|
 *  | N | 1 | 1 | 1 | 1 | 0 | 1 |
 *  | I | 1 | 1 | 0 | 1 | 1 | 1 |
 *  | C | 0 | 1 | 1 | 1 | 1 | 1 |
 *  | H | 1 | 0 | 1 | 1 | 1 | 1 |
 *  | E | 1 | 1 | 1 | 0 | 1 | 1 |
 *
 *  In the following code, costs are more complex, we precised costs for same_letter_insertion,
 *  case_cost, space_costs and so on. So the matrix is filled regarding of these costs.
 *
 *  We fill in M matrix following the rule M[i,j] must be the minimum of :
 *  - M[i-1, j] + deletion_cost (deletion)
 *  - M[i, j-1] + insertion_cost (insertion)
 *  - M[i-1, j-1] + Cost(i-1, j-1) (substitution)
 *
 *  Filling the first line will result :
 *
 * |   |   | C | H | I | E | N | S |
 * |---|---|---|---|---|---|---|---|
 * |   | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
 * | N | 1 | 1 | 2 | 3 | 4 | 4 | 5 |
 * | I | 2 | 0 | 0 | 0 | 0 | 0 | 0 |
 * | C | 3 | 0 | 0 | 0 | 0 | 0 | 0 |
 * | H | 4 | 0 | 0 | 0 | 0 | 0 | 0 |
 * | E | 5 | 0 | 0 | 0 | 0 | 0 | 0 |
 *
 *  At the end of the algorithm we obtain :
 *
 * |     |     |  C  |  H  |  I  |  E  |  N  |  S  |
 * |-----|-----|-----|-----|-----|-----|-----|-----|
 * |     |  0  |  1  |  2  |  3  |  4  |  5  |  6  |
 * |  N  | (1) |  1  |  2  |  3  |  4  |  4  |  5  |
 * |  I  | (2) |  2  |  2  |  2  |  3  |  4  |  5  |
 * |  C  |  3  | (2) |  3  |  3  |  3  |  4  |  5  |
 * |  H  |  4  |  3  | (2) | (3) |  4  |  4  |  5  |
 * |  E  |  5  |  4  |  3  |  3  | (3) | (4) | (5) |
 *
 *  the numbers between parenthesis indicate the path to find the levenshtein distance. Here Levenshtein
 *  distance is given by M[n, m] that is 5.
 *
 *  To make niche from chien, we insert an N (1), and an I (2), then we let C and H untouched (2), we
 *  delete an I (3), we keep the E (3), and we delete N (4) and S (5).
 *
 *  We can see that vertical path indicates insertion, horizontal path deletion and diagonal path substitution.
 *
 */
og_status StmComputeLevenshteinBoard(struct og_ctrl_stm *ctrl_stm, unsigned char *string1,
    unsigned char *string2,
    struct og_stm_levenshtein_input_param *lev_params)
{
  /* Applying Levenshtein iterative rule to determine
   if, at a given step, it is better to insert, delete
   or permute.  */
  int c1, c2, prev1, prev2, next1, next2;
  double insertion_cost, deletion_cost, substitution_cost;
  c1 = c2 = prev1 = prev2 = 0;
  int prof = 0;
  for (int i = 1; i <= ctrl_stm->length1; i++)
  {
    //Get character of first string
    if (c1 != 0) prev1 = c1;
    if (i < ctrl_stm->length1) next1 = (string1[i * 2] << 8) + string1[i * 2 + 1];
    else next1 = 0;
    c1 = (string1[(i - 1) * 2] << 8) + string1[(i - 1) * 2 + 1];

    for (int j = 1; j <= ctrl_stm->length2; j++)
    {
      //Get character of second string
      prof++;
      if (c2 != 0) prev2 = c2;
      if (j < ctrl_stm->length2) next2 = (string2[j * 2] << 8) + string2[j * 2 + 1];
      else next2 = 0;
      c2 = (string2[(j - 1) * 2] << 8) + string2[(j - 1) * 2 + 1];

      // If a same letter has been inserted (we need to check letters before and after)
      insertion_cost = lev_params->insertion_cost;
      if (lev_params->same_letter_insertion_cost > 0)
      {
        if (((prev2 && prev2 == c2) || (next2 && next2 == c2))
            && ((prev1 && prev1 == c2) || (next1 && next1 == c2) || (c1 == c2)))
        insertion_cost = lev_params->same_letter_insertion_cost;
      }

      // If a same letter has been deleted (we need to check letters before and after)
      deletion_cost = lev_params->deletion_cost;
      if (lev_params->same_letter_deletion_cost > 0)
      {
        if (((prev1 && prev1 == c1) || (next1 && next1 == c1))
            && ((prev2 && prev2 == c1) || (next2 && next2 == c1) || (c2 == c1)))
        deletion_cost = lev_params->same_letter_deletion_cost;
      }

      //if an accentuated character has been replaced by the same with no accent
      substitution_cost = lev_params->substitution_cost;

      if(ctrl_stm->has_equivalent_letters)
      {
        IFE(StmGetSubstitutionCost(ctrl_stm, c1, c2, &substitution_cost));
      }

      if (lev_params->accents_substitution_cost > 0)
      {
        int unaccc1 = OgUniUnaccent(c1);
        int unaccc2 = OgUniUnaccent(c2);
        if (unaccc1 == unaccc2)
        {
          if(substitution_cost > lev_params->accents_substitution_cost)
          {
            substitution_cost = lev_params->accents_substitution_cost;
          }
        }
      }
      //check if c1 and c2 are the same characters excepted for the case
      if (lev_params->case_cost > 0)
      {
        int lowerc1 = OgUniTolower(c1);
        int lowerc2 = OgUniTolower(c2);
        if (lowerc1 == lowerc2)
        {
          if (substitution_cost > lev_params->case_cost)
          {
            substitution_cost = lev_params->case_cost;
          }
        }
      }
      if (lev_params->space_cost > 0)
      {
        //if character in string1 is a space to delete
        if (OgUniIsspace(c1))
        {
          deletion_cost = lev_params->space_cost;
        }
        //if character in string2 is a space to insert in string1
        if (OgUniIsspace(c2))
        {
          insertion_cost = lev_params->space_cost;
        }
      }
      if (lev_params->punctuation_cost > 0)
      {
        //if character is a punctuation taken from punctuation conf file
        if (StmIsPunctuation(ctrl_stm, c1) && StmIsPunctuation(ctrl_stm, c2))
        {
          if(lev_params->punctuation_cost < substitution_cost)
          {
            substitution_cost = lev_params->punctuation_cost;
          }
        }
        if (StmIsPunctuation(ctrl_stm, c1))
        {
          deletion_cost = lev_params->punctuation_cost;
        }
        if (StmIsPunctuation(ctrl_stm, c2))
        {
          insertion_cost = lev_params->punctuation_cost;
        }
      }
      if (c1 == c2)
      {
        ctrl_stm->score[i][j] = ctrl_stm->score[i - 1][j - 1];
        ctrl_stm->from_i[i][j] = i - 1;
        ctrl_stm->from_j[i][j] = j - 1;
        ctrl_stm->path[i][j] = same;
      }
      else
      {
        // We keep the minimum between insertion, deletion and substitution
        ctrl_stm->score[i][j] = ctrl_stm->score[i][j - 1] + insertion_cost;
        ctrl_stm->from_i[i][j] = i;
        ctrl_stm->from_j[i][j] = j - 1;
        ctrl_stm->path[i][j] = insertion;

        if (ctrl_stm->score[i][j] > ctrl_stm->score[i - 1][j] + deletion_cost)
        {
          ctrl_stm->score[i][j] = ctrl_stm->score[i - 1][j] + deletion_cost;
          ctrl_stm->from_i[i][j] = i - 1;
          ctrl_stm->from_j[i][j] = j;
          ctrl_stm->path[i][j] = deletion;
        }
        if (ctrl_stm->score[i][j] > ctrl_stm->score[i - 1][j - 1] + substitution_cost)
        {
          ctrl_stm->score[i][j] = ctrl_stm->score[i - 1][j - 1] + substitution_cost;
          ctrl_stm->from_i[i][j] = i - 1;
          ctrl_stm->from_j[i][j] = j - 1;
          ctrl_stm->path[i][j] = substitution;
        }
        // If the letters are just inverted, we keep a swap cost
        if (i >= 2 && j >= 2 && c1 == prev2 && c2 == prev1
            && ctrl_stm->score[i][j] > ctrl_stm->score[i - 2][j - 2] + lev_params->swap_cost)
        {
          ctrl_stm->score[i][j] = ctrl_stm->score[i - 2][j - 2] + lev_params->swap_cost;
          ctrl_stm->from_i[i][j] = i - 2;
          ctrl_stm->from_j[i][j] = j - 2;
          ctrl_stm->path[i][j] = swap;
        }
      }
    }
  }

  if (ctrl_stm->loginfo->trace & DOgStmTraceScore)
  {
    char message[DPcPathSize];
    sprintf(message, "OgStmLevenshteinFast: Levenshtein distance is '%f' %d tours de boucle",
        ctrl_stm->score[ctrl_stm->length1][ctrl_stm->length2], prof);
    OgMsg(ctrl_stm->hmsg, "", DOgMsgDestInLog, message);
  }

  DONE;
}

static og_bool StmIsPunctuation(struct og_ctrl_stm *ctrl_stm, int c)
{
  og_bool is_punctuation = OgLipIsPunctuation(&ctrl_stm->lip_conf, c);
  char buffer[2];
  buffer[0] = (char)(c >> 8);
  buffer[1] = (char)(c & 0x00ff);
  int length;
  og_bool is_punctuation_word = OgLipIsPunctuationWord(&ctrl_stm->lip_conf, 2, buffer, &length);
  return (is_punctuation || is_punctuation_word);
}

// Change the substitution cost only if the letters are equivalents and the new substitution cost is lower
static og_status StmGetSubstitutionCost(struct og_ctrl_stm *ctrl_stm, int c1, int c2, double *substitution_cost)
{

  struct equivalent_letter equivalent_letter[1];
  equivalent_letter->letter1 = c1;
  equivalent_letter->letter2 = c2;

  double *pcost = (double *) g_hash_table_lookup(ctrl_stm->equivalent_letters_hash, equivalent_letter);
  if(pcost != NULL)
  {
    if(*pcost < *substitution_cost)
    {
      *substitution_cost = *pcost;
    }
  }
  DONE;
}
