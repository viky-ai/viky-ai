/*
 *  The add module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January 2010
 *  Version 1.0
*/
#include <logltras.h>
#include <logaut.h>
#include <logstm.h>

struct og_ctrl_add {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  void *ha_swap,*hstm;
  int del_module_id;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


static int LtrasModuleAdd1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf);





void *OgLtrasModuleAddInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_add *ctrl_add;
char erreur[DOgErrorSize];

IFn(ctrl_add=(struct og_ctrl_add *)malloc(sizeof(struct og_ctrl_add))) {
  sprintf(erreur,"OgLtrasModuleAddInit: malloc error on ctrl_add");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_add,0,sizeof(struct og_ctrl_add));

ctrl_add->herr = param->herr;
ctrl_add->hltras = param->hltras;
ctrl_add->hmutex = param->hmutex;
ctrl_add->cloginfo = param->loginfo;
ctrl_add->loginfo = &ctrl_add->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_add->herr;
msg_param->hmutex=ctrl_add->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_add->loginfo->where;
msg_param->module_name="ltra_module_add";
IFn(ctrl_add->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_add->hmsg,param->hmsg)) return(0);

//IFn(ctrl_add->ha_swap=OgLtrasHaSwap(ctrl_add->hltras)) return(0);
ctrl_add->hstm = OgLtrasHstm(ctrl_add->hltras);

IF(OgLtrasGetLevenshteinCosts(ctrl_add->hltras, ctrl_add->levenshtein_costs)) return(0);

return((void *)ctrl_add);
}




int OgLtrasModuleAddFlush(void *handle)
{
struct og_ctrl_add *ctrl_add = (struct og_ctrl_add *)handle;
IFE(OgMsgFlush(ctrl_add->hmsg));
DPcFree(ctrl_add);
DONE;
}





int OgLtrasModuleAdd(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_add *ctrl_add = (struct og_ctrl_add *) module_input->handle;
  void *hltras = ctrl_add->hltras;
  int TrfUsed = input->TrfUsed;
  struct og_ltra_trfs *trfs;
  int i;

  ogint64_t micro_clock_start=OgMicroClock();

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  IFn(ctrl_add->ha_swap=OgLtrasHaSwap(ctrl_add->hltras)) DONE;

  for (i = 0; i < TrfUsed; i++)
  {
    IFE(LtrasModuleAdd1(module_input, trfs, i));
  }

  *elapsed=OgMicroClock()-micro_clock_start;

  DONE;

}




static int LtrasModuleAdd1(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *trfs, int Itrf)
{
  struct og_ctrl_add *ctrl_add = (struct og_ctrl_add *) module_input->handle;

  /** Keep original string of trfs to compare with transmormation string and
   * to calculate a levenchtein score */
  unsigned char orig[DPcPathSize];
  int iorig = 0;
  void *hltras = ctrl_add->hltras;
  IFE(OgLtrasTrfGetOriginalString(hltras,trfs, Itrf, orig, DPcPathSize, &iorig));

  struct og_ltra_add_trf_input tinput[1];
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  unsigned char words[DPcPathSize];
  OgLtrasTrfInitTinput(trfs, Itrf, tinput, module_input->id,words);

  for (int i = 0; i < tinput->nb_words; i++)
  {
    int string_length;
    unsigned char *string;
    struct og_ltra_add_trf_word *new_word = tinput->word + i;

    int language = DOgLangNil;
    if (module_input->language_code != 0)
    {
      language = module_input->language_code;
    }
    else if (new_word->language != 0)
    {
      language = new_word->language;
    }

    string_length = new_word->string_length;
    string = new_word->string;

    int ibuffer;
    unsigned char buffer[DPcPathSize];
    memcpy(buffer, string, string_length);
    ibuffer = string_length;
    buffer[ibuffer++] = 0;
    buffer[ibuffer++] = 1;

    int retour, nstate0, nstate1, iout;
    oindex states[DPcAutMaxBufferSize + 9];
    unsigned char out[DPcAutMaxBufferSize + 9];
    if ((retour = OgAufScanf(ctrl_add->ha_swap, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
    {
      do
      {
        IFE(retour);
        unsigned char *p = out;
        int language_code, position, frequency;
        IFE(DOgPnin4(ctrl_add->herr,&p,&language_code));
        IFE(DOgPnin4(ctrl_add->herr,&p,&position));
        IFE(DOgPnin4(ctrl_add->herr,&p,&frequency));

        // check language frequency
        int language_lang = OgIso639_3166ToLang(language);
        if (language_lang != 0 && language_code != 0 && language_lang != language_code)
        {
          continue;
        }

        new_word->language = language;
        new_word->frequency = frequency;
        struct og_ltra_trf *trf = trfs->Trf + Itrf;
        struct og_ltra_word *word = trfs->Word + trf->start_word + i;
        new_word->string_length = iout - (p - out);
        new_word->string = p;
        if (new_word->string_length <= 0) continue;

        og_bool is_false = OgLtrasIsFalseTransformation(hltras, word->length_position, trfs->Ba + word->start_position,
            new_word->string_length, new_word->string);
        IFE(is_false);
        if (is_false) continue;

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

        double dlevenshtein_distance = OgStmLevenshteinFast(ctrl_add->hstm, iorig, orig, transformed_length,
            transformed, ctrl_add->levenshtein_costs);
        IFE(dlevenshtein_distance);
        tinput->score = 1.0 - dlevenshtein_distance;

        /** if there is only one word, it has already been checked in the dictionary thus final = 1
         * if there are more than one words, only one is checked in the dictionary to permit further
         * transformations in other modules. We need to check all the words in term module thus final = 0 */
        if (tinput->nb_words == 1)
        {
          tinput->final = 1;
        }
        int Intrf;
        int added = OgLtrasTrfAdd(hltras, trfs, tinput, &Intrf);
        IFE(added);
        if (!added) continue;
      }
      while ((retour = OgAufScann(ctrl_add->ha_swap, &iout, out, nstate0, &nstate1, states)));
    }
    /** We have changed tinput->word + i (string and string_length) to create transformation.
     * We have to put it back when we iterate to i+1 */
    new_word->string_length = string_length;
    new_word->string = string;
  }

  DONE;
}
