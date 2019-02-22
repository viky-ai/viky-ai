/*
 *  Handling spell checking for Natural Language Processing library
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : February 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <logltras.h>

og_status NlpLtrasEntity(struct nlp_match_entities_ctrl *me_ctrl)
{
  og_nlp_th ctrl_nlp_th = me_ctrl->ctrl_nlp_th;
  package_t package = me_ctrl->interpret_package->package;
  IFE(OgLtrasHaBaseSet(ctrl_nlp_th->hltras, package->ltra_entity_dictionaries->ha_base));
  IFE(OgLtrasHaSwapSet(ctrl_nlp_th->hltras, package->ltra_entity_dictionaries->ha_swap));
  IFE(OgLtrasHaPhonSet(ctrl_nlp_th->hltras, package->ltra_entity_dictionaries->ha_phon));

  NlpLog(DOgNlpTraceMatch, "NlpLtrasEntityPackage: entity to search is '%s'", me_ctrl->string_entity);

  int string_entity_length = strlen(me_ctrl->string_entity);

  struct og_ltra_trfs *trfs;
  struct og_ltras_input input[1];
  memset(input, 0, sizeof(struct og_ltras_input));

  int uni_length;
  unsigned char uni[DPcPathSize];
  IFE(OgCpToUni(string_entity_length, me_ctrl->string_entity , DPcPathSize, &uni_length, uni, DOgCodePageUTF8, 0, 0));
  for (int i = 0; i < uni_length; i += 2)
  {
    int c = (uni[i] << 8) + uni[i + 1];
    if (OgUniIsspace(c))
    {
      uni[i + 1] = DOgNlpEntitySeparator;
    }
  }

  input->request_length = uni_length;
  input->request = uni;
  input->language_code = DOgLangNil;
  input->frequency_ratio = 1.0;
  input->score_factor = 1.0;
  input->flow_chart = "(del-add/phon/swap)-exc-term";
  input->max_nb_solutions = 3;
  IFE(OgLtras(ctrl_nlp_th->hltras, input, &trfs));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtras)
  {
    OgMsg(ctrl_nlp_th->hmsg, "ltras", DOgMsgDestInLog, "NlpLtrasEntityPackage: list of linguistic transformations:");
    IFE(OgLtrasTrfsLog(ctrl_nlp_th->hltras, trfs));
  }

  unsigned char words[DPcPathSize];
  int isword;
  unsigned char sword[DPcPathSize];

  for (int Itrf = 0; Itrf < trfs->TrfUsed; Itrf++)
  {
    struct og_ltra_trf *trf = trfs->Trf + Itrf;
    if (trf->basic) continue;
    if (!trf->total) continue;
    int i;
    for (i = 0, words[0] = 0; i < trf->nb_words; i++)
    {
      struct og_ltra_word *ltra_word = trfs->Word + trf->start_word + i;
      unsigned char *s = trfs->Ba + ltra_word->start;
      int is = ltra_word->length;
      unsigned char u[DPcPathSize];
      for (int j = 0; j < is; j += 2)
      {
        int c = (s[j] << 8) + s[j + 1];
        if (c == DOgNlpEntitySeparator)
        {
          u[j] = 0;
          u[j + 1] = ' ';
        }
        else
        {
          u[j] = s[j];
          u[j + 1] = s[j + 1];
        }
      }
      IFE(OgUniToCp(is,u,DPcPathSize,&isword,sword,DOgCodePageUTF8,0,0));
      sprintf(words + strlen(words), "%s%s", (i ? " " : ""), sword);
    }
    // Removing the space at the end.
    int length_words = strlen(words);
    length_words--;
    words[length_words] = 0;
    double score_spelling = pow(trf->final_score, 4);
    NlpLog(DOgNlpTraceMatch, "NlpLtrasEntityPackage: found entity '%s' with score %.2f", words, score_spelling);
    IFE(NlpMatchEntitiesChangeToAlternativeString(me_ctrl, length_words, words));
    IFE(NlpMatchCurrentEntity(me_ctrl));

  }

  IFE(OgLtrasTrfsDestroy(ctrl_nlp_th->hltras, trfs));
  DONE;
}

