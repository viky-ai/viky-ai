/*
 *  The Swap module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January 2010
 *  Version 1.0
*/
#include <logltras.h>

struct og_ctrl_swap {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int max_nb_swaps;
  double swap_cost;
  int start_trf;
  void *hstm;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


static int LtrasModuleSwap1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int nb_swaps);
static int LtrasModuleSwap2(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int nb_swaps, int Itrf);





void *OgLtrasModuleSwapInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_swap *ctrl_swap;
char erreur[DOgErrorSize];

IFn(ctrl_swap=(struct og_ctrl_swap *)malloc(sizeof(struct og_ctrl_swap))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_swap");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_swap,0,sizeof(struct og_ctrl_swap));

ctrl_swap->herr = param->herr;
ctrl_swap->hltras = param->hltras;
ctrl_swap->hmutex = param->hmutex;
ctrl_swap->cloginfo = param->loginfo;
ctrl_swap->loginfo = &ctrl_swap->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_swap->herr;
msg_param->hmutex=ctrl_swap->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_swap->loginfo->where;
msg_param->module_name="ltra_module_swap";
IFn(ctrl_swap->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_swap->hmsg,param->hmsg)) return(0);

ctrl_swap->hstm = OgLtrasHstm(ctrl_swap->hltras);

IF(OgLtrasGetLevenshteinCosts(ctrl_swap->hltras, ctrl_swap->levenshtein_costs)) return(0);

return((void *)ctrl_swap);
}




int OgLtrasModuleSwapFlush(void *handle)
{
struct og_ctrl_swap *ctrl_swap = (struct og_ctrl_swap *)handle;
IFE(OgMsgFlush(ctrl_swap->hmsg));
DPcFree(ctrl_swap);
DONE;
}





int OgLtrasModuleSwap(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_swap *ctrl_swap = (struct og_ctrl_swap *) module_input->handle;
  void *hltras = ctrl_swap->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found;

  ogint64_t micro_clock_start = OgMicroClock();

  ctrl_swap->max_nb_swaps = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_swap->hltras,"swap_max_nb_swaps",DPcPathSize,buffer));
  if (found)
  {
    ctrl_swap->max_nb_swaps = atoi(buffer);
  }
  if (module_input->argc > 1)
  {
    ctrl_swap->max_nb_swaps = atoi(module_input->argv[1]);
  }

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  ctrl_swap->start_trf = 0;
  for (i = 1; i <= ctrl_swap->max_nb_swaps; i++)
  {
    IFE(LtrasModuleSwap1(module_input, trfs, i));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}




static int LtrasModuleSwap1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int nb_swaps)
{
struct og_ctrl_swap *ctrl_swap = (struct og_ctrl_swap *)module_input->handle;
int i,start_trf,TrfUsed;

start_trf = ctrl_swap->start_trf;
ctrl_swap->start_trf = trfs->TrfUsed;
TrfUsed = trfs->TrfUsed;

for (i=start_trf; i<TrfUsed; i++) {
  IFE(LtrasModuleSwap2(module_input,trfs,nb_swaps,i));
  }

DONE;
}






static int LtrasModuleSwap2(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *trfs, int nb_swaps, int Itrf)
{
  struct og_ctrl_swap *ctrl_swap = (struct og_ctrl_swap *) module_input->handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  int words_length;
  unsigned char words[DPcPathSize];
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_ltra_add_trf_word *new_word;
  unsigned char new_words[DPcPathSize];

  //Keep original string of trfs to compare with transmormation string and
  //to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  void *hltras = ctrl_swap->hltras;
  IFE(OgLtrasTrfGetOriginalString(hltras,trfs, Itrf, origin, DPcPathSize, &origin_length));

  words_length = 0;
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  for (int i = 0; (i < trf->nb_words && tinput->nb_words < DOgLtrasAddTrfMaxNbWords); i++)
  {
    // To many word, solution skipped
    if (tinput->nb_words >= DOgLtrasAddTrfMaxNbWords)
    {
      break;
    }

    struct og_ltra_word *word = trfs->Word + trf->start_word + i;
    new_word = tinput->word + tinput->nb_words;
    memcpy(words + words_length, trfs->Ba + word->start, word->length);
    new_word->string = words + words_length;
    new_word->string_length = word->length;
    new_word->frequency = word->frequency;
    new_word->base_frequency = word->base_frequency;
    new_word->start_position = word->start_position;
    new_word->length_position = word->length_position;
    words_length += word->length;
    tinput->nb_words++;
  }

  tinput->start = trf->start;
  tinput->length = trf->length;
  tinput->module_id = module_input->id;
  tinput->from_trf = Itrf;

  for (int i = 0; i < tinput->nb_words; i++)
  {
    int string_length;
    unsigned char *string;
    new_word = tinput->word + i;
    string_length = new_word->string_length;
    string = new_word->string;
    for (int j = 0; j + 2 < string_length; j += 2)
    {
      memcpy(new_words, string, j);
      new_words[j] = string[j + 2];
      new_words[j + 1] = string[j + 3];
      new_words[j + 2] = string[j];
      new_words[j + 3] = string[j + 1];
      memcpy(new_words + j + 4, string + j + 4, string_length - j - 4);
      new_word->string_length = string_length;
      new_word->string = new_words;

      unsigned char tranformed[DPcPathSize];
      int tranformed_length = 0;

      //Create the transformed string to calculate levenshtein distance with original string
      for (int k = 0; k < tinput->nb_words; k++)
      {
        memcpy(tranformed + tranformed_length, tinput->word[k].string, tinput->word[k].string_length);
        tranformed_length += tinput->word[k].string_length;
        if (k < (tinput->nb_words - 1))
        {
          memcpy(tranformed + tranformed_length, "\0 ", 2);
          tranformed_length += 2;
        }
      }

      double dlevenshtein_distance;
      IFE(dlevenshtein_distance = OgStmLevenshteinFast(ctrl_swap->hstm, origin_length, origin, tranformed_length,
          tranformed, ctrl_swap->levenshtein_costs));
      tinput->score = 1.0 - dlevenshtein_distance;

      //We need to make sure that the swap operation creates a modification.
      //For instance pizza can become pizza with a swap operation with a score of 1
      //(always better than other transformations).
      //Since we keep the best transformation, only comparing the output and the scores
      //if we do not discard useless swap transformation we create a bug.
      if (string_length == new_word->string_length)
      {
        if (memcmp(new_word->string, string, string_length) != 0)
        {
          IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));
        }
      }
      else
      {
        IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));
      }
    }
    new_word->string_length = string_length;
    new_word->string = string;
  }

  DONE;
}


