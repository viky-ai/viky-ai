/*
 *  The Phon module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logpho.h>
#include <logaut.h>
#include <logstm.h>


struct og_ctrl_phon {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  void *ha_phon,*ha_results,*hpho,*hstm;
  int min_nb_letters,max_nb_solutions;
  double max_dlevenshtein_distance;
  int max_ilevenshtein_distance;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };

struct LtrasModulePhonAddTransformationCtx
{
    int ibuffer;
    unsigned char *buffer;
    int origin_length;
    unsigned char *origin;
    /** To know the position of the word we are treating with phonetization*/
    int word_index;
    struct og_ltra_add_trf_input *tinput;
};

static int LtrasModulePhon1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf);

static int LtrasModulePhonAddTransformation(struct og_ctrl_phon *ctrl_phon, struct og_ltra_trfs *trfs, int Itrf,
    struct LtrasModulePhonAddTransformationCtx *ctx);




void *OgLtrasModulePhonInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_aut_param caut_param,*aut_param=&caut_param;
struct og_ctrl_phon *ctrl_phon;
char erreur[DOgErrorSize];

IFn(ctrl_phon=(struct og_ctrl_phon *)malloc(sizeof(struct og_ctrl_phon))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_phon");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_phon,0,sizeof(struct og_ctrl_phon));

ctrl_phon->herr = param->herr;
ctrl_phon->hltras = param->hltras;
ctrl_phon->hmutex = param->hmutex;
ctrl_phon->cloginfo = param->loginfo;
ctrl_phon->loginfo = &ctrl_phon->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_phon->herr;
msg_param->hmutex=ctrl_phon->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_phon->loginfo->where;
msg_param->module_name="ltra_module_phon";
IFn(ctrl_phon->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_phon->hmsg,param->hmsg)) return(0);

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_phon->herr;
aut_param->hmutex=ctrl_phon->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = ctrl_phon->loginfo->where;
aut_param->state_number = 0x4000;
sprintf(aut_param->name,"ltra_module_phon_results");
IFn(ctrl_phon->ha_results=OgAutInit(aut_param)) return(0);

IFn(ctrl_phon->ha_phon=OgLtrasHaPhon(ctrl_phon->hltras)) return(0);
ctrl_phon->hpho = OgLtrasHpho(ctrl_phon->hltras);
ctrl_phon->hstm = OgLtrasHstm(ctrl_phon->hltras);

IF(OgLtrasGetLevenshteinCosts(ctrl_phon->hltras, ctrl_phon->levenshtein_costs)) return(0);

return((void *)ctrl_phon);
}




int OgLtrasModulePhonFlush(void *handle)
{
struct og_ctrl_phon *ctrl_phon = (struct og_ctrl_phon *)handle;
IFE(OgAutFlush(ctrl_phon->ha_results));
IFE(OgMsgFlush(ctrl_phon->hmsg));
DPcFree(ctrl_phon);
DONE;
}





int OgLtrasModulePhon(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_phon *ctrl_phon = (struct og_ctrl_phon *) module_input->handle;
  void *hltras = ctrl_phon->hltras;
  double dlevenshtein_distance;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found, TrfUsed;

  ogint64_t micro_clock_start = OgMicroClock();

  ctrl_phon->min_nb_letters = 4;
  IFE(found=OgLtrasGetParameterValue(ctrl_phon->hltras,"phon_min_nb_letters",DPcPathSize,buffer));
  if (found)
  {
    ctrl_phon->min_nb_letters = atoi(buffer);
  }
  if (module_input->argc > 1)
  {
    ctrl_phon->min_nb_letters = atoi(module_input->argv[1]);
  }

  ctrl_phon->max_nb_solutions = 10;
  IFE(found=OgLtrasGetParameterValue(ctrl_phon->hltras,"phon_max_nb_solutions",DPcPathSize,buffer));
  if (found)
  {
    ctrl_phon->max_nb_solutions = atoi(buffer);
  }
  if (module_input->argc > 2)
  {
    ctrl_phon->max_nb_solutions = atoi(module_input->argv[2]);
  }

  ctrl_phon->max_dlevenshtein_distance = 1.0;
  if (module_input->argc > 3)
  {
    ctrl_phon->max_dlevenshtein_distance = atof(module_input->argv[3]);
    if (ctrl_phon->max_dlevenshtein_distance > 1.0) ctrl_phon->max_dlevenshtein_distance = 1.0;
  }
  dlevenshtein_distance = ctrl_phon->max_dlevenshtein_distance;
  dlevenshtein_distance *= DOgLtrasLevenshteinDistancePrecision;
  ctrl_phon->max_ilevenshtein_distance = (int) dlevenshtein_distance;

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  TrfUsed = trfs->TrfUsed;

  for (i = 0; i < TrfUsed; i++)
  {
    IFE(LtrasModulePhon1(module_input, trfs, i));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}

static int LtrasModulePhon1(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ctrl_phon *ctrl_phon = (struct og_ctrl_phon *) module_input->handle;
  struct og_pho_output coutput, *output = &coutput;
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_pho_input cinput, *input = &cinput;
  int min_post_phonetisation_char_number = 2;
  int min_phonetisation_char_number = 3;


  /** We do not phonetize transformation that is final
   * This can be discussed, but, for the moment
   * that is the choice */
  if (trf->final) DONE;

  /** Keep original string of trfs to compare with transmormation string and
   *to calculate a levenchtein score */
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  void *hltras = ctrl_phon->hltras;
  IFE(OgLtrasTrfGetOriginalString(hltras,trfs, Itrf, origin, DPcPathSize, &origin_length));

  struct og_ltra_add_trf_input tinput[1];
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  unsigned char words[DPcPathSize];
  OgLtrasTrfInitTinput(trfs, Itrf, tinput, module_input->id,words);
  for (int i = 0; i < tinput->nb_words; i++)
  {
    int iin;
    unsigned char *in;
    struct og_ltra_add_trf_word *new_word = tinput->word + i;
    iin = new_word->string_length;
    in = new_word->string;

    /** if the word to phonetize must not be phonetized, we iterate to the next word of the string.*/
    if (iin < ctrl_phon->min_nb_letters * 2) continue;
    if (iin < min_phonetisation_char_number * 2) continue;
    if (iin >= DOgStmMaxWordLength) continue;

    /** We do not phonetize names with digits */
    for (int j = 0; j < iin; j += 2)
    {
      int c = (in[j] << 8) + in[j + 1];
      if (OgUniIsdigit(c)) DONE;
    }

    /** We phonetize the string and we check if the result is not too small otherwise
     * we iterate to the next word of the string. */
    input->iB = iin;
    input->B = in;

    //changer quand on gère les langues
    input->lang = 34;// fr

    IFE(OgPhonet(ctrl_phon->hpho, input, output));
    if (output->iB < min_post_phonetisation_char_number * 2) continue;

    int ibuffer;
    unsigned char buffer[DPcPathSize];
    memcpy(buffer, output->B, output->iB);
    ibuffer = output->iB;
    buffer[ibuffer++] = 0;
    buffer[ibuffer++] = 1;

    IFE(OgAutReset(ctrl_phon->ha_results));

    struct LtrasModulePhonAddTransformationCtx ctx[1];
    memset(ctx,0,sizeof(struct LtrasModulePhonAddTransformationCtx));
    ctx->buffer = buffer;
    ctx->ibuffer = ibuffer;
    ctx->origin = origin;
    ctx->origin_length = origin_length;
    ctx->tinput = tinput;
    ctx->word_index = i;

    struct og_ltra_add_trf_word *previous_word = tinput->word + i;
    unsigned char *string = previous_word->string;
    int string_length = previous_word->string_length;
    LtrasModulePhonAddTransformation(ctrl_phon,trfs,Itrf,ctx);
    /** We have changed tinput->word + i (string and string_length) to create transformation.
     * We have to put it back when we iterate to i+1 */
    previous_word->string = string;
    previous_word->string_length = string_length;
  }
  DONE;
}

static int LtrasModulePhonAddTransformation(struct og_ctrl_phon *ctrl_phon, struct og_ltra_trfs *trfs, int Itrf, struct LtrasModulePhonAddTransformationCtx *ctx)
{

  /** we put results with Levenshtein distance inside the automaton ha_results and the frequency
   * in order to order by results by lev distance and then frequency. */
  int retour, nstate0, nstate1;
  int iout;
  oindex states[DPcAutMaxBufferSize + 9];
  unsigned char out[DPcAutMaxBufferSize + 9];

  if ((retour = OgAufScanf(ctrl_phon->ha_phon, ctx->ibuffer, ctx->buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      unsigned char *p = out;
      int attribute_number, language_code, frequency;
      IFE(DOgPnin4(ctrl_phon->herr,&p,&attribute_number));
      IFE(DOgPnin4(ctrl_phon->herr,&p,&language_code));
      IFE(DOgPnin4(ctrl_phon->herr,&p,&frequency));
      int real_string_length = iout - (p - out);
      unsigned char *real_string = p;
      if (real_string_length >= DOgStmMaxWordLength) continue;

      struct og_ltra_trf *trf = trfs->Trf + Itrf;
      struct og_ltra_word *word = trfs->Word + trf->start_word + ctx->word_index;

      void *hltras = ctrl_phon->hltras;
      int is_false = OgLtrasIsFalseTransformation(hltras, word->length_position, trfs->Ba + word->start_position,
          real_string_length, real_string);
      IFE(is_false);
      if (is_false) continue;

      struct og_ltra_add_trf_input *tinput = ctx->tinput;
      struct og_ltra_add_trf_word *new_word = tinput->word + ctx->word_index;
      new_word->string_length = real_string_length;
      new_word->string = real_string;

      unsigned char transformed[DPcPathSize];
      int transformed_length = 0;

      /** Create the transformed string to calculate levenshtein distance with original string */
      for (int k = 0; k < tinput->nb_words; k++)
      {
        memcpy(transformed + transformed_length, tinput->word[k].string, tinput->word[k].string_length);
        transformed_length += tinput->word[k].string_length;
        if (k < (tinput->nb_words - 1))
        {
          memcpy(transformed + transformed_length, "\0 ", 2);
          transformed_length += 2;
        }
      }

      double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_phon->hstm, ctx->origin_length, ctx->origin, transformed_length,
          transformed, ctrl_phon->levenshtein_costs);
      IFE(dlevenshtein_distance);

      /** Levenshtein distance is a double and automaton can only save int. Thus we have to multiply it to a big
       * number in order to keep precision*/
      dlevenshtein_distance *= DOgLtrasLevenshteinDistancePrecision;
      int ilevenshtein_distance = (int) round(dlevenshtein_distance);
      if (ilevenshtein_distance > ctrl_phon->max_ilevenshtein_distance) continue;

      unsigned char entry[DPcAutMaxBufferSize + 9];
      p = entry;
      OggNout(ilevenshtein_distance, &p);

      /** To save by decreasing order of frequency ????*/
      int negative_frequency = 0x7fffffff - frequency;
      OggNout(negative_frequency, &p);
      memcpy(p, real_string, real_string_length);
      p += real_string_length;
      int ientry = p - entry;
      OgAutAdd(ctrl_phon->ha_results, ientry, entry);
    }
    while ((retour = OgAufScann(ctrl_phon->ha_phon, &iout, out, nstate0, &nstate1, states)));
  }

  /** We get results ordered by the automaton and add new transformations if necessary*/
  int nb_results = 0;
  if ((retour = OgAutScanf(ctrl_phon->ha_results, -1, "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      int added;
      unsigned char *p = out;
      int ilevenshtein_distance, negative_frequency;
      IFE(DOgPnin4(ctrl_phon->herr,&p,&ilevenshtein_distance));
      IFE(DOgPnin4(ctrl_phon->herr,&p,&negative_frequency));
      int frequency = 0x7fffffff - negative_frequency;
      double dlevenshtein_distance = ilevenshtein_distance;
      dlevenshtein_distance /= DOgLtrasLevenshteinDistancePrecision;
      int real_string_length = iout - (p - out);
      unsigned char *real_string = p;

      struct og_ltra_add_trf_input *tinput = ctx->tinput;
      struct og_ltra_add_trf_word *new_word = tinput->word + ctx->word_index;
      new_word->string_length = real_string_length;
      new_word->string = real_string;
      new_word->frequency = frequency;
      tinput->score = 1.0 - dlevenshtein_distance;

      /** if there is only one word, it has already been checked in the dictionary thus final = 1
       * if there are more than one words, only one is checked in the dictionary to permit further
       * transformations in other modules. We need to check all the words in term module thus final = 0 */
      if(tinput->nb_words == 1)
      {
        tinput->final = 1;
      }

      void *hltras = ctrl_phon->hltras;
      IFE(added = OgLtrasTrfAdd(hltras, trfs, tinput, 0));
      if (added) nb_results++;
      if (nb_results >= ctrl_phon->max_nb_solutions) break;
    }
    while ((retour = OgAutScann(ctrl_phon->ha_results, &iout, out, nstate0, &nstate1, states)));
  }
  DONE;
}


