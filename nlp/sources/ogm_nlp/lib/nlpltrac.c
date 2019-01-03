/*
 *  Handling compilation of spell checking for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <logltrac.h>

static og_status NlpLtracAdd(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency);
static og_status NlpLtracPackageInit(og_nlp_th ctrl_nlp_th, package_t package);

og_status NlpLtracInit(og_nlp_th ctrl_nlp_th)
{
  struct og_ltrac_param param[1];
  memset(param, 0, sizeof(struct og_ltrac_param));
  param->herr = ctrl_nlp_th->herr;
  param->hmsg = ctrl_nlp_th->hmsg;
  param->hmutex = ctrl_nlp_th->hmutex;
  param->loginfo.trace = DOgLtracTraceMinimal + DOgLtracTraceMemory;
  param->loginfo.where = ctrl_nlp_th->loginfo->where;
  strcpy(param->WorkingDirectory, ctrl_nlp_th->ctrl_nlp->WorkingDirectory);
  IFN(ctrl_nlp_th->hltrac = OgLtracInit(param)) DPcErr;
  DONE;
}

og_status NlpLtracFlush(og_nlp_th ctrl_nlp_th)
{
  if (ctrl_nlp_th->hltrac == NULL) CONT;

  IFE(OgLtracFlush(ctrl_nlp_th->hltrac));
  ctrl_nlp_th->hltrac = NULL;

  DONE;
}

static og_status NlpLtracPackageInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  struct og_ltrac_dictionaries dictionaries[1];
  IFE(OgLtracDicInit(ctrl_nlp_th->hltrac, dictionaries));
  package->ltra_dictionaries->ha_base = dictionaries->ha_base;
  package->ltra_dictionaries->ha_swap = dictionaries->ha_swap;
  package->ltra_dictionaries->ha_phon = dictionaries->ha_phon;

  IFE(OgLtracFrequenciesSet(ctrl_nlp_th->hltrac,package->ltra_min_frequency,package->ltra_min_frequency_swap));
  DONE;
}

static og_status NlpLtracPackageWrite(og_nlp_th ctrl_nlp_th, package_t package)
{
  IFE(OgLtracDicWrite(ctrl_nlp_th->hltrac));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtrac)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Base ltra dictionary for package '%s' '%s':", package->slug,
        package->id);
    IFE(OgLtracDicBaseLog(ctrl_nlp_th->hltrac, package->ltra_dictionaries->ha_base));

    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Swap ltra dictionary for package '%s' '%s':", package->slug,
        package->id);
    IFE(OgLtracDicSwapLog(ctrl_nlp_th->hltrac, package->ltra_dictionaries->ha_swap));

    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Phon ltra dictionary for package '%s' '%s':", package->slug,
        package->id);
    IFE(OgLtracDicPhonLog(ctrl_nlp_th->hltrac, package->ltra_dictionaries->ha_phon));
  }

  DONE;
}

og_status NlpLtracPackageFlush(package_t package)
{
  IFE(OgAutFlush(package->ltra_dictionaries->ha_base));
  IFE(OgAutFlush(package->ltra_dictionaries->ha_swap));
  IFE(OgAutFlush(package->ltra_dictionaries->ha_phon));
  DONE;
}

/*
 * Each package has its own small dictionaries that need to be compiled
 */
og_status NlpLtracPackage(og_nlp_th ctrl_nlp_th, package_t package)
{

  if (ctrl_nlp_th->hltrac == NULL) CONT;

  unsigned char out[DPcAutMaxBufferSize + 9];
  unsigned char out_old[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;
  int sep_old = 0, frequency;

  int log2_nb_words = log2(package->nb_words);

  if (log2_nb_words <= 10)
  {
    package->ltra_min_frequency = 1;
    package->ltra_min_frequency_swap = 1;
  }
  else
  {
    package->ltra_min_frequency = (log2_nb_words - 10) / 2;
    if (package->ltra_min_frequency < 1) package->ltra_min_frequency = 1;
    package->ltra_min_frequency_swap = package->ltra_min_frequency * 2;
  }

  NlpLog(DOgNlpTraceLtrac,
      "Creating ltrac package '%s' '%s' from %d words log2=%d ltra_min_frequency=%d ltra_min_frequency_swap=%d",
      package->slug, package->id, package->nb_words, log2_nb_words, package->ltra_min_frequency,
      package->ltra_min_frequency_swap);

  IFE(NlpLtracPackageInit(ctrl_nlp_th, package));

  if ((retour = OgAufScanf(package->ha_word, 0, "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int sep = -1;
      for (int i = 0; i < iout; i++)
      {
        if (out[i] == '\1')
        {
          sep = i;
          break;
        }
      }
      if (sep < 0)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpLtracPackage: error in ha_word");
        DPcErr;
      }

      int Iinput_part;
      unsigned char *p = out + sep + 1;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      NlpLog(DOgNlpTraceLtrac, "  %.*s : %d", sep, out, Iinput_part);
      //struct input_part *input_part = OgHeapGetCell(package->hinput_part, Iinput_part);
      //IFN(input_part) DPcErr;
      //int locale = input_part->expression->locale;

      if (sep_old == sep && !memcmp(out_old, out, sep))
      {
        frequency++;
      }
      else
      {
        IFE(NlpLtracAdd(ctrl_nlp_th, out_old, sep_old, frequency));
        memcpy(out_old, out, sep);
        sep_old = sep;
        frequency = 1;
      }

    }
    while ((retour = OgAufScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));

    IFE(NlpLtracAdd(ctrl_nlp_th, out_old, sep_old, frequency));

    IFE(NlpLtracPackageWrite(ctrl_nlp_th, package));
  }

  DONE;
}

static og_status NlpLtracAdd(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency)
{
  if (word_length <= 0) DONE;

  int uni_length;
  unsigned char uni[DPcPathSize];
  IFE(OgCpToUni(word_length, word , DPcPathSize, &uni_length, uni, DOgCodePageUTF8, 0, 0));

  og_bool keep_word = FALSE;
  for (int i=0; i<uni_length; i+=2)
  {
    int c = (uni[i]<<8)+uni[i+1];
    if (OgUniIspunct(c)) continue;
    if (OgUniIsdigit(c)) continue;
    keep_word = TRUE;
  }

  if (!keep_word) DONE;

  NlpLog(DOgNlpTraceLtrac, "  NlpLtracAdd: adding %.*s with frequency %d", word_length, word, frequency);

  struct og_ltrac_word_input word_input[1];
  word_input->value_length = uni_length;
  word_input->value = uni;

  word_input->frequency = frequency;
  word_input->language_code = DOgLangNil;

  IFE(OgLtracAddWord(ctrl_nlp_th->hltrac, word_input));

  DONE;
}

