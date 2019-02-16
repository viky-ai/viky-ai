/*
 *  Handling lemmatisation checking for Natural Language Processing library
 *  Copyright (c) 2019 Pertimm, by Patrick Constant
 *  Dev : February 2019
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <logltras.h>

static og_status NlpLemInitLang(og_nlp ctrl_nlp, int langid);
static og_status NlpLemFlushLang(og_nlp ctrl_nlp, int langid);
static og_status NlpLemWord(og_nlp_th ctrl_nlp_th, int Irequest_word);
static og_status NlpLemWordLang(og_nlp_th ctrl_nlp_th, int Irequest_word, int langid);
static og_status NlpLemAddWord(og_nlp_th ctrl_nlp_th, int Irequest_word_basic, int length_corrected_word,
    og_string corrected_word, int lang_id);

og_status NlpLemInit(og_nlp ctrl_nlp)
{
  ctrl_nlp->ld[DOgLangFR].active = 1;
  ctrl_nlp->ld[DOgLangFR].codepage = DOgCodePageANSI;
//  ctrl_nlp->ld[DOgLangEN].active=1; ctrl_nlp->ld[DOgLangEN].codepage=DOgCodePageANSI;
//  ctrl_nlp->ld[DOgLangDE].active=1; ctrl_nlp->ld[DOgLangDE].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangRO].active=1; ctrl_nlp->ld[DOgLangRO].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangAR].active=1; ctrl_nlp->ld[DOgLangAR].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangES].active=1; ctrl_nlp->ld[DOgLangES].codepage=DOgCodePageANSI;
//  ctrl_nlp->ld[DOgLangIT].active=1; ctrl_nlp->ld[DOgLangIT].codepage=DOgCodePageANSI;
//  ctrl_nlp->ld[DOgLangPL].active=1; ctrl_nlp->ld[DOgLangPL].codepage=DOgCodePageANSI;
//  ctrl_nlp->ld[DOgLangPT].active=1; ctrl_nlp->ld[DOgLangPT].codepage=DOgCodePageANSI;
//  ctrl_nlp->ld[DOgLangFI].active=1; ctrl_nlp->ld[DOgLangFI].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangCS].active=1; ctrl_nlp->ld[DOgLangCS].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangNL].active=1; ctrl_nlp->ld[DOgLangNL].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangHU].active=1; ctrl_nlp->ld[DOgLangHU].codepage=DOgCodePageUTF8;
//  ctrl_nlp->ld[DOgLangRU].active=1; ctrl_nlp->ld[DOgLangRU].codepage=DOgCodePageUTF8;

  for (int i = 0; i < DOgLangMax; i++)
  {
    IFE(NlpLemInitLang(ctrl_nlp, i));
  }

  DONE;
}

static og_status NlpLemInitLang(og_nlp ctrl_nlp, int langid)
{
  char ling[DPcPathSize];
  struct lem_data *ld = ctrl_nlp->ld + langid;
  struct og_aut_param aut_param[1];

  if (!ld->active) DONE;

  if (ctrl_nlp->WorkingDirectory[0]) sprintf(ling, "%s/ling", ctrl_nlp->WorkingDirectory);
  else strcpy(ling, "ling");

  sprintf(ld->root, "%s/%sroot.auf", ling, OgIso639ToCode(langid));

  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_nlp->herr;
  aut_param->hmutex = ctrl_nlp->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_nlp->loginfo->where;
  aut_param->state_number = 0;

  if (!access(ld->root, 0))
  {
    sprintf(aut_param->name, "ssi root");
    IFn(ld->ha_root=OgAutInit(aut_param)) return (0);
    IFE(OgAufRead(ld->ha_root, ld->root));
  }
  else
  {
    OgMsg(ctrl_nlp->hmsg, "", DOgMsgDestInLog, "NlpLemInit1: automaton '%s' does not exist", ld->root);
    ld->ha_root = 0;
  }

  if (ld->ha_root == 0) ld->active = 0;

  DONE;
}

og_status NlpLemFlush(og_nlp ctrl_nlp)
{
  for (int i = 0; i < DOgLangMax; i++)
  {
    IFE(NlpLemFlushLang(ctrl_nlp, i));
  }

  DONE;
}

static og_status NlpLemFlushLang(og_nlp ctrl_nlp, int langid)
{
  struct lem_data *ld = ctrl_nlp->ld + langid;
  if (!ld->active) DONE;
  IFE(OgAutFlush(ld->ha_root));
  DONE;
}

/**
 * Do a spell checking on each words and also a spell checking on the whole sentence
 */
og_status NlpLem(og_nlp_th ctrl_nlp_th)
{
  if (ctrl_nlp_th->basic_request_word_used <= 0) DONE;
  int nb_basic_request_word_for_lem = ctrl_nlp_th->basic_request_word_used;
  if (ctrl_nlp_th->auto_complete) nb_basic_request_word_for_lem--;
  for (int i = 0; i < nb_basic_request_word_for_lem; i++)
  {
    IFE(NlpLemWord(ctrl_nlp_th, i));
  }
  DONE;
}

static og_status NlpLemWord(og_nlp_th ctrl_nlp_th, int Irequest_word)
{
  for (int i = 0; i < DOgLangMax; i++)
  {
    IFE(NlpLemWordLang(ctrl_nlp_th, Irequest_word, i));
  }

  DONE;
}

static og_status NlpLemWordLang(og_nlp_th ctrl_nlp_th, int Irequest_word, int langid)
{
  struct lem_data *ld = ctrl_nlp_th->ctrl_nlp->ld + langid;

  if (!ld->active) DONE;
  void *ha = ld->ha_root;
  IFn(ha) DONE;

  int retour, nstate0, nstate1, iout;
  oindex states[DPcAutMaxBufferSize + 9];
  unsigned char out[DPcAutMaxBufferSize + 8];
  unsigned char mot[DPcPathSize], lwmot[DPcPathSize], result[DPcPathSize], utf8[DPcPathSize];
  unsigned char strdecal[100], strtermi[200];
  unsigned char unires[DPcPathSize];
  unsigned char bad_dic_entry[DPcPathSize];
  int imot, i, decal, iresult, iunires, iutf8;

  /** Gets form words (automaton encoded in ld->codepage) **/
  struct request_word *request_word = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word);
  IFN(request_word) DPcErr;
  og_string word = OgHeapGetCell(ctrl_nlp_th->hba, request_word->start);
  IFN(word) DPcErr;
  int word_length = request_word->length;

  NlpLog(DOgNlpTraceMinimal, "NlpLemWordLang: starting with '%s'", word);

  int uni_length;
  unsigned char uni[DPcPathSize];
  IFE(OgCpToUni(word_length, word , DPcPathSize, &uni_length, uni, DOgCodePageUTF8, 0, 0));
  IFE(OgUniToCp(uni_length,uni,DPcPathSize,&imot,lwmot,ld->codepage, NULL, NULL));
  lwmot[imot++] = ':';
  lwmot[imot] = 0;

  if ((retour = OgAufScanf(ha, -1, lwmot, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      for (i = 0; i < iout; i++)
      {
        if (!PcIsdigit(out[i])) break;
      }
      snprintf(result, DPcPathSize, "%.*s", imot, lwmot);
      snprintf(strdecal, 100, "%.*s", i, out);
      decal = atoi(strdecal);
      snprintf(strtermi, 200, "%.*s", iout - i, out + i);

      // check for bad dictionnary input : for instance sa:3casatiune won't make the system crash
      // it will bypass it and check the next dictionary entry
      int result_length = strlen(result);
      int fulldecal = (imot - 1 - decal);
      if (result_length < fulldecal || fulldecal < 0)
      {
        snprintf(bad_dic_entry, DPcPathSize, "%s:%s", mot, out);
        NlpLog(DOgNlpTraceLem, "NlpLemWordLang: bad dictionary entry '%s' with lexicon entry '%s'", bad_dic_entry,
            ld->root);
        continue;
      }

      strcpy(result + fulldecal, strtermi); /** -1 because of ':' **/
      iresult = strlen(result);

      if (iresult == 0) continue;

      IFE(OgCpToUni(iresult,result,DPcPathSize,&iunires,unires,ld->codepage,NULL,NULL));
      IFE(OgUniToCp(iunires,unires,DPcPathSize,&iutf8,utf8,DOgCodePageUTF8, NULL, NULL));

      /** result is the same as the original form **/
      //TODO test si même form
      NlpLog(DOgNlpTraceMinimal, "NlpLemWordLang: found '%s'", utf8);
      og_status status = NlpLemAddWord(ctrl_nlp_th, Irequest_word, iutf8, utf8, langid);
      IFE(status);


    }
    while ((retour = OgAufScann(ha, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static og_status NlpLemAddWord(og_nlp_th ctrl_nlp_th, int Irequest_word_basic, int length_corrected_word,
    og_string corrected_word, int lang_id)
{
  struct request_word *request_word_basic = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word_basic);
  IFN(request_word_basic) DPcErr;

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtras)
  {
    og_string string_request_word = OgHeapGetCell(ctrl_nlp_th->hba, request_word_basic->start);
    IFN(string_request_word) DPcErr;
    NlpLog(DOgNlpTraceLtras, "NlpLemAddWord: adding corrected word '%s' for basic word '%s' at position %d:%d",
        corrected_word, string_request_word, request_word_basic->start_position, request_word_basic->length_position)
  }

  size_t Irequest_word;
  struct request_word *request_word = OgHeapNewCell(ctrl_nlp_th->hrequest_word, &Irequest_word);
  IFn(request_word) DPcErr;
  IF(Irequest_word) DPcErr;

  request_word->self_index = Irequest_word;
  request_word->start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->length = length_corrected_word;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->length, corrected_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

  request_word->raw_start = OgHeapGetCellsUsed(ctrl_nlp_th->hba);
  request_word->raw_length = length_corrected_word;
  IFE(OgHeapAppend(ctrl_nlp_th->hba, request_word->raw_length, corrected_word));
  IFE(OgHeapAppend(ctrl_nlp_th->hba, 1, ""));

// Necessary because of possible reallocation of hrequest_word
  request_word_basic = OgHeapGetCell(ctrl_nlp_th->hrequest_word, Irequest_word_basic);
  IFN(request_word_basic) DPcErr;
  request_word->start_position = request_word_basic->start_position;
  request_word->length_position = request_word_basic->length_position;

  request_word->is_number = FALSE;
  request_word->is_punctuation = FALSE;
  request_word->is_auto_complete_word = FALSE;
  request_word->is_regex = FALSE;
  request_word->regex_input_part = NULL;
  request_word->nb_matched_words = 1;

  request_word->spelling_score = 0.99;
  request_word->lang_id = lang_id;

  DONE;
}
