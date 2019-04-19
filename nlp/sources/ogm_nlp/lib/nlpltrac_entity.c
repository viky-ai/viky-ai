/*
 *  Handling compilation of spell checking for Natural Language Processing library
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"
#include <logltrac.h>

static og_status NlpLtracEntityPackageInit(og_nlp_th ctrl_nlp_th, package_t package);
static og_status NlpLtracEntityPackageWrite(og_nlp_th ctrl_nlp_th, package_t package);
static og_status NlpLtracEntityPrepare(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency);
static og_status NlpLtracEntityPrepareEntry(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency);
static og_bool NlpLtracEntityGetEntry(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length,
    size_t *pIprepare_entity);
static og_status NlpLtracEntityCreate(og_nlp_th ctrl_nlp_th);
static og_status NlpLtracEntityAdd(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency);

static og_status NlpLtracEntityPackageInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  struct og_ltrac_dictionaries dictionaries[1];
  IFE(OgLtracDicInit(ctrl_nlp_th->hltrac, dictionaries));
  package->ltra_entity_dictionaries->ha_base = dictionaries->ha_base;
  package->ltra_entity_dictionaries->ha_swap = dictionaries->ha_swap;
  package->ltra_entity_dictionaries->ha_phon = dictionaries->ha_phon;

  IFE(OgLtracFrequenciesSet(ctrl_nlp_th->hltrac, package->ltra_min_frequency, package->ltra_min_frequency_swap));
  DONE;
}

static og_status NlpLtracEntityPackageWrite(og_nlp_th ctrl_nlp_th, package_t package)
{
  IFE(OgLtracDicWrite(ctrl_nlp_th->hltrac));

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceLtrac)
  {
    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Base ltra entity dictionary for package '%s' '%s':", package->slug,
        package->id);
    IFE(OgLtracDicBaseLog(ctrl_nlp_th->hltrac, package->ltra_entity_dictionaries->ha_base));

    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Swap ltra entity dictionary for package '%s' '%s':", package->slug,
        package->id);
    IFE(OgLtracDicSwapLog(ctrl_nlp_th->hltrac, package->ltra_entity_dictionaries->ha_swap));

    OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Phon ltra entity dictionary for package '%s' '%s':", package->slug,
        package->id);
    IFE(OgLtracDicPhonLog(ctrl_nlp_th->hltrac, package->ltra_entity_dictionaries->ha_phon));
  }

  DONE;
}

og_status NlpLtracEntityPackageFlush(package_t package)
{
  IFE(OgAutFlush(package->ltra_entity_dictionaries->ha_base));
  IFE(OgAutFlush(package->ltra_entity_dictionaries->ha_swap));
  IFE(OgAutFlush(package->ltra_entity_dictionaries->ha_phon));
  DONE;
}

/*
 * Each package has its own small dictionaries that need to be compiled
 */
og_status NlpLtracEntityPackage(og_nlp_th ctrl_nlp_th, package_t package)
{

  if (ctrl_nlp_th->hltrac == NULL) CONT;

  IFE(NlpLtracEntityPrepareReset(ctrl_nlp_th));

  unsigned char out[DPcAutMaxBufferSize + 9];
  unsigned char out_old[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;
  int sep_old = 0, frequency;

  int log2_nb_words = log2(package->nb_words);

  if (log2_nb_words <= 10)
  {
    package->ltra_entity_min_frequency = 1;
    package->ltra_entity_min_frequency_swap = 1;
  }
  else
  {
    package->ltra_entity_min_frequency = (log2_nb_words - 10) / 2;
    if (package->ltra_entity_min_frequency < 1) package->ltra_entity_min_frequency = 1;
    package->ltra_entity_min_frequency_swap = package->ltra_entity_min_frequency * 2;
  }

  NlpLog(DOgNlpTraceLtrac,
      "Creating ltrac entity package '%s' '%s' from %d words log2=%d ltra_entity_min_frequency=%d ltra_entity_min_frequency_swap=%d",
      package->slug, package->id, package->nb_words, log2_nb_words, package->ltra_entity_min_frequency,
      package->ltra_entity_min_frequency_swap);

  IFE(NlpLtracEntityPackageInit(ctrl_nlp_th, package));

  if ((retour = OgAufScanf(package->ha_entity, 0, "", &iout, out, &nstate0, &nstate1, states)))
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
        NlpThrowErrorTh(ctrl_nlp_th, "NlpEntityLog: error in ha_entity");
        DPcErr;
      }

      long expression_ptr;
      struct expression *expression;
      unsigned char *p = out + sep + 1;
      IFE(DOgPnin8(ctrl_nlp_th->herr,&p,&expression_ptr));
      expression = (struct expression *) expression_ptr;
      NlpLog(DOgNlpTraceLtrac, "  %.*s : '%s'", sep, out, expression->text);

      if (sep_old == sep && !memcmp(out_old, out, sep))
      {
        frequency++;
      }
      else
      {
        IFE(NlpLtracEntityPrepare(ctrl_nlp_th, out_old, sep_old, frequency));
        memcpy(out_old, out, sep);
        sep_old = sep;
        frequency = 1;
      }

    }
    while ((retour = OgAufScann(package->ha_entity, &iout, out, nstate0, &nstate1, states)));

    IFE(NlpLtracEntityPrepare(ctrl_nlp_th, out_old, sep_old, frequency));

    IFE(NlpLtracEntityCreate(ctrl_nlp_th));

    IFE(NlpLtracEntityPackageWrite(ctrl_nlp_th, package));

    // Reducing the memory of the automaton to avoid keep temporary memory
    IFE(OgAutResize(ctrl_nlp_th->ha_prepare_entity, 0x10));
  }

  DONE;
}

og_status NlpLtracEntityPrepareInit(og_nlp_th ctrl_nlp_th, og_string name)
{
  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_nlp_th->herr;
  aut_param->hmutex = ctrl_nlp_th->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal;
  aut_param->loginfo.where = ctrl_nlp_th->loginfo->where;
  aut_param->state_number = 0x10;
  sprintf(aut_param->name, "package_ha_prepare_entity");
  ctrl_nlp_th->ha_prepare_entity = OgAutInit(aut_param);
  IFn(ctrl_nlp_th->ha_prepare_entity) DPcErr;

  og_char_buffer nlpc_name[DPcPathSize];
  snprintf(nlpc_name, DPcPathSize, "%s_prepare_entity", name);
  ctrl_nlp_th->hprepare_entity = OgHeapInit(ctrl_nlp_th->hmsg, nlpc_name, sizeof(int), 0x10);
  IFN(ctrl_nlp_th->hprepare_entity)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpLtracEntityPrepareInit : error on OgHeapInit(%s)", nlpc_name);
    DPcErr;
  }
  DONE;
}

og_status NlpLtracEntityPrepareReset(og_nlp_th ctrl_nlp_th)
{
  IFE(OgAutReset(ctrl_nlp_th->ha_prepare_entity));
  IFE(OgHeapReset(ctrl_nlp_th->hprepare_entity));
  DONE;
}

og_status NlpLtracEntityPrepareFlush(og_nlp_th ctrl_nlp_th)
{
  IFE(OgAutFlush(ctrl_nlp_th->ha_prepare_entity));
  IFE(OgHeapFlush(ctrl_nlp_th->hprepare_entity));

  DONE;
}

static og_status NlpLtracEntityPrepare(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency)
{
  NlpLog(DOgNlpTraceLtrac, "  NlpLtracEntityPrepare: preparing '%.*s' with frequency %d", word_length, word, frequency);

  for (int i = 0; i < word_length; i++)
  {
    if (word[i] == ' ')
    {
      IFE(NlpLtracEntityPrepareEntry(ctrl_nlp_th, word, i + 1, frequency));
    }
  }
  DONE;
}

static og_status NlpLtracEntityPrepareEntry(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency)
{
  size_t Iprepare_entity;
  og_bool found = NlpLtracEntityGetEntry(ctrl_nlp_th, word, word_length, &Iprepare_entity);
  IFE(found);
  if (found)
  {
    int *pprepare_entity = OgHeapGetCell(ctrl_nlp_th->hprepare_entity, Iprepare_entity);
    *pprepare_entity += frequency;
    NlpLog(DOgNlpTraceLtrac, "  NlpLtracEntityPrepareEntry: adding '%.*s' with new frequency %d", word_length, word,
        *pprepare_entity);
  }
  else
  {
    NlpLog(DOgNlpTraceLtrac, "  NlpLtracEntityPrepareEntry: adding '%.*s' with frequency %d", word_length, word,
        frequency);
    int *pprepare_entity = OgHeapNewCell(ctrl_nlp_th->hprepare_entity, &Iprepare_entity);
    IFn(pprepare_entity) DPcErr;
    *pprepare_entity = frequency;

    unsigned char buffer[DPcAutMaxBufferSize];
    unsigned char *p;
    int ibuffer = 0;

    memcpy(buffer + ibuffer, word, word_length);
    ibuffer += word_length;
    buffer[ibuffer++] = '\1';

    p = buffer + ibuffer;
    OggNout(Iprepare_entity, &p);

    int length = p - buffer;
    IFE(OgAutAdd(ctrl_nlp_th->ha_prepare_entity, length, buffer));

  }
  DONE;
}

static og_bool NlpLtracEntityGetEntry(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length,
    size_t *pIprepare_entity)
{
  unsigned char buffer[DPcAutMaxBufferSize];
  int ibuffer = 0;

  memcpy(buffer + ibuffer, word, word_length);
  ibuffer += word_length;
  buffer[ibuffer++] = '\1';

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int nstate0, nstate1, iout;

  int retour = OgAutScanf(ctrl_nlp_th->ha_prepare_entity, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states);
  IFE(retour);
  if (retour)
  {
    unsigned char *p = out;
    int Iprepare_entity;
    IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iprepare_entity));
    *pIprepare_entity = Iprepare_entity;
    return TRUE;
  }
  *pIprepare_entity = (size_t) (-1);
  return FALSE;
}

static og_status NlpLtracEntityCreate(og_nlp_th ctrl_nlp_th)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if ((retour = OgAutScanf(ctrl_nlp_th->ha_prepare_entity, 0, "", &iout, out, &nstate0, &nstate1, states)))
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
        NlpThrowErrorTh(ctrl_nlp_th, "NlpLtracEntityCreate: error in ha_entity");
        DPcErr;
      }
      unsigned char *p = out + sep + 1;
      int Iprepare_entity;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iprepare_entity));

      int *pprepare_entity = OgHeapGetCell(ctrl_nlp_th->hprepare_entity, Iprepare_entity);
      IFN(pprepare_entity) DPcErr;
      int frequency = *pprepare_entity;
      NlpLog(DOgNlpTraceLtrac, "  NlpLtracEntityCreate: adding '%.*s' with frequency %d", sep, out, frequency);
      IFE(NlpLtracEntityAdd(ctrl_nlp_th, out, sep, frequency));
    }
    while ((retour = OgAutScann(ctrl_nlp_th->ha_prepare_entity, &iout, out, nstate0, &nstate1, states)));
  }
  DONE;
}

static og_status NlpLtracEntityAdd(og_nlp_th ctrl_nlp_th, unsigned char *word, int word_length, int frequency)
{
  if (word_length <= 0) DONE;

  if (word_length >= DOgNlpMaxLtracWordSize) DONE;

  int uni_length;
  unsigned char uni[DPcPathSize];
  IFE(OgCpToUni(word_length, word , DPcPathSize, &uni_length, uni, DOgCodePageUTF8, 0, 0));

  og_bool keep_word = FALSE;
  for (int i = 0; i < uni_length; i += 2)
  {
    int c = (uni[i] << 8) + uni[i + 1];
    if (OgUniIspunct(c)) continue;
    if (OgUniIsdigit(c)) continue;
    keep_word = TRUE;
  }

  if (!keep_word) DONE;

  for (int i = 0; i < uni_length; i += 2)
  {
    int c = (uni[i] << 8) + uni[i + 1];
    if (OgUniIsspace(c))
    {
      uni[i + 1] = DOgNlpEntitySeparator;
    }
  }

  NlpLog(DOgNlpTraceLtrac, "  NlpLtracEntityAdd: adding '%.*s' with frequency %d", word_length, word, frequency);

  struct og_ltrac_word_input word_input[1];
  word_input->value_length = uni_length;
  word_input->value = uni;

  word_input->frequency = frequency;
  word_input->language_code = DOgLangNil;

  IFE(OgLtracAddWord(ctrl_nlp_th->hltrac, word_input));

  DONE;
}

