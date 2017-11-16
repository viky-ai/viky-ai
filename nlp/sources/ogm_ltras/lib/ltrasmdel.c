/*
 *  The Delete module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January 2010
 *  Version 1.0
*/
#include <logltras.h>


struct og_ctrl_delete {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int max_nb_deleted_letters;
  int min_word_length;
  int start_trf;
  void *hstm;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


static int LtrasModuleDelete1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int nb_deleted_letters);
static int LtrasModuleDelete2(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int nb_deleted_letters, int Itrf);





void *OgLtrasModuleDeleteInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_delete *ctrl_delete;
char erreur[DOgErrorSize];

IFn(ctrl_delete=(struct og_ctrl_delete *)malloc(sizeof(struct og_ctrl_delete))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_delete");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_delete,0,sizeof(struct og_ctrl_delete));

ctrl_delete->herr = param->herr;
ctrl_delete->hltras = param->hltras;
ctrl_delete->hmutex = param->hmutex;
ctrl_delete->cloginfo = param->loginfo;
ctrl_delete->loginfo = &ctrl_delete->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_delete->herr;
msg_param->hmutex=ctrl_delete->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_delete->loginfo->where;
msg_param->module_name="ltra_module_delete";
IFn(ctrl_delete->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_delete->hmsg,param->hmsg)) return(0);

ctrl_delete->hstm = OgLtrasHstm(ctrl_delete->hltras);

IF(OgLtrasGetLevenshteinCosts(ctrl_delete->hltras, ctrl_delete->levenshtein_costs)) return(0);

return((void *)ctrl_delete);
}




int OgLtrasModuleDeleteFlush(void *handle)
{
struct og_ctrl_delete *ctrl_delete = (struct og_ctrl_delete *)handle;
IFE(OgMsgFlush(ctrl_delete->hmsg));
DPcFree(ctrl_delete);
DONE;
}





int OgLtrasModuleDelete(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_delete *ctrl_delete = (struct og_ctrl_delete *) module_input->handle;
  void *hltras = ctrl_delete->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found;

  ogint64_t micro_clock_start=OgMicroClock();

  ctrl_delete->max_nb_deleted_letters = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_delete->hltras,"del_max_nb_deleted_letters",DPcPathSize,buffer));
  if (found)
  {
    ctrl_delete->max_nb_deleted_letters = atoi(buffer);
  }
  if (module_input->argc > 1)
  {
    ctrl_delete->max_nb_deleted_letters = atoi(module_input->argv[1]);
  }
  ctrl_delete->min_word_length = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_delete->hltras,"del_min_word_length",DPcPathSize,buffer));
  if (found)
  {
    ctrl_delete->min_word_length = atoi(buffer);
  }
  if (module_input->argc > 2)
  {
    ctrl_delete->min_word_length = atoi(module_input->argv[2]);
  }

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  ctrl_delete->start_trf = 0;
  for (i = 1; i <= ctrl_delete->max_nb_deleted_letters; i++)
  {
    IFE(LtrasModuleDelete1(module_input, trfs, i));
  }

  *elapsed=OgMicroClock()-micro_clock_start;

  DONE;
}




static int LtrasModuleDelete1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int nb_deleted_letters)
{
struct og_ctrl_delete *ctrl_delete = (struct og_ctrl_delete *)module_input->handle;
int i,start_trf,TrfUsed;

start_trf = ctrl_delete->start_trf;
ctrl_delete->start_trf = trfs->TrfUsed;
TrfUsed = trfs->TrfUsed;

for (i=start_trf; i<TrfUsed; i++) {
  IFE(LtrasModuleDelete2(module_input,trfs,nb_deleted_letters,i));
  }

DONE;
}






static int LtrasModuleDelete2(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *trfs, int nb_deleted_letters, int Itrf)
{
  struct og_ctrl_delete *ctrl_delete = (struct og_ctrl_delete *) module_input->handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  int words_length;
  unsigned char words[DPcPathSize];
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  struct og_ltra_add_trf_word *new_word;
  unsigned char new_words[DPcPathSize];
  void *hltras = ctrl_delete->hltras;

  //Keep original string of trfs to compare with transmormation string and
  //to calculate a levenchtein score
  unsigned char origin[DPcPathSize];
  int origin_length = 0;
  IFE(OgLtrasTrfGetOriginalString(hltras, trfs, Itrf, origin, DPcPathSize, &origin_length));

  words_length = 0;
  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  for (int i = 0; i < trf->nb_words; i++)
  {
    // To many word, soluton skipped
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
    new_word->language = word->language;
    words_length += word->length;
    tinput->nb_words++;
  }

  if (words_length < ctrl_delete->min_word_length * 2) DONE;

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
    for (int j = 0; j < string_length; j += 2)
    {
      memcpy(new_words, string, j);
      memcpy(new_words + j, string + j + 2, string_length - j - 2);
      new_word->string_length = string_length - 2;
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
      IFE(dlevenshtein_distance = OgStmLevenshteinFast(ctrl_delete->hstm, origin_length, origin, tranformed_length,
          tranformed, ctrl_delete->levenshtein_costs));
      tinput->score = 1.0 - dlevenshtein_distance;
      IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));
    }
    new_word->string_length = string_length;
    new_word->string = string;
  }

  DONE;
}


