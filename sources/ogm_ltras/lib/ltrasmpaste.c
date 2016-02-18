/*
 *  The Paste module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: June 2010
 *  Version 1.0
*/
#include <logltras.h>

#define DOgMaxBuildedItrf 100



struct og_ctrl_paste {
  void *herr,*hmsg,*hltras,*hstm; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  int nb_buildedItrf; int buildedItrf[DOgMaxBuildedItrf];
  int max_nb_pasted_words;
  int TrfUsed;
  struct og_stm_levenshtein_input_param levenshtein_costs[1];
  };


static int LtrasModulePaste1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs
  , int nb_pasted_words);
static int LtrasModulePaste2(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs
  , int nb_pasted_words, int current_nb_pasted_words, int Itrf);
static int LtrasModulePaste3(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs
  , int nb_pasted_words);
static int LtrasModulePasteTrfInBuildedItrf(struct og_ctrl_paste *ctrl_paste, int Itrf);
static int LtrasModulePasteTrfIsNext(struct og_ctrl_paste *ctrl_paste, struct og_ltra_trfs *trfs, int Itrf);
static int LtrasModulePasteIsGoodSpace(struct og_ctrl_paste *ctrl_paste, struct og_ltra_trfs *trfs, int space_start, int space_end);




void *OgLtrasModulePasteInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_paste *ctrl_paste;
char erreur[DOgErrorSize];

IFn(ctrl_paste=(struct og_ctrl_paste *)malloc(sizeof(struct og_ctrl_paste))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_paste");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_paste,0,sizeof(struct og_ctrl_paste));

ctrl_paste->herr = param->herr;
ctrl_paste->hltras = param->hltras;
ctrl_paste->hmutex = param->hmutex;
ctrl_paste->cloginfo = param->loginfo;
ctrl_paste->loginfo = &ctrl_paste->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_paste->herr;
msg_param->hmutex=ctrl_paste->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_paste->loginfo->where;
msg_param->module_name="ltra_module_paste";
IFn(ctrl_paste->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_paste->hmsg,param->hmsg)) return(0);

ctrl_paste->hstm = OgLtrasHstm(ctrl_paste->hltras);
IF(OgLtrasGetLevenshteinCosts(ctrl_paste->hltras, ctrl_paste->levenshtein_costs)) return(0);

return ctrl_paste;
}



int OgLtrasModulePasteFlush(void *handle)
{
struct og_ctrl_paste *ctrl_paste = (struct og_ctrl_paste *)handle;
IFE(OgMsgFlush(ctrl_paste->hmsg));
DPcFree(ctrl_paste);
DONE;
}



int OgLtrasModulePaste(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_paste *ctrl_paste = (struct og_ctrl_paste *) module_input->handle;
  void *hltras = ctrl_paste->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found;

  ogint64_t micro_clock_start = OgMicroClock();

  ctrl_paste->max_nb_pasted_words = 2;
  IFE(found=OgLtrasGetParameterValue(ctrl_paste->hltras,"paste_max_nb_pasted_words",DPcPathSize,buffer));
  if (found)
  {
    ctrl_paste->max_nb_pasted_words = atoi(buffer);
  }
  if (module_input->argc > 1)
  {
    ctrl_paste->max_nb_pasted_words = atoi(module_input->argv[1]);
  }

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  ctrl_paste->TrfUsed = trfs->TrfUsed;

  for (i = 2; i <= ctrl_paste->max_nb_pasted_words; i++)
  {
    IFE(LtrasModulePaste1(module_input, input, trfs, i));
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}




static int LtrasModulePaste1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs
  , int nb_pasted_words)
{
struct og_ctrl_paste *ctrl_paste = (struct og_ctrl_paste *)module_input->handle;
int i;

for (i=0; i<ctrl_paste->TrfUsed; i++) {
  IFE(LtrasModulePaste2(module_input,input,trfs,nb_pasted_words,0,i));
  }

DONE;
}




static int LtrasModulePaste2(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *input,  struct og_ltra_trfs *trfs
  , int nb_pasted_words, int current_nb_pasted_words, int Itrf)
{
struct og_ctrl_paste *ctrl_paste = (struct og_ctrl_paste *)module_input->handle;
struct og_ltra_trf *tmp_trf;
int i;

if (current_nb_pasted_words == nb_pasted_words) {
  /** we have a solution **/
  IFE(LtrasModulePaste3(module_input,input,trfs,nb_pasted_words));
  DONE;
  }

if (current_nb_pasted_words==0) {
  ctrl_paste->nb_buildedItrf=0;
  ctrl_paste->buildedItrf[ctrl_paste->nb_buildedItrf++]=Itrf;
  current_nb_pasted_words++;
  }

for (i=0; i<ctrl_paste->TrfUsed; i++) {
  tmp_trf = trfs->Trf + i;
  /** We don't paste a trf that is cut/split **/
  if (tmp_trf->nb_words > 1) continue;
  if (LtrasModulePasteTrfInBuildedItrf(ctrl_paste,i)) continue;
  if (!LtrasModulePasteTrfIsNext(ctrl_paste,trfs,i)) continue;
  ctrl_paste->buildedItrf[ctrl_paste->nb_buildedItrf++]=i;
  IFE(LtrasModulePaste2(module_input,input,trfs,nb_pasted_words,current_nb_pasted_words+1,i));
  break;
  }

DONE;
}




static int LtrasModulePaste3(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs *trfs
    , int nb_pasted_words)
{
  struct og_ctrl_paste *ctrl_paste = (struct og_ctrl_paste *) module_input->handle;
  struct og_ltra_add_trf_input tinput[1];
  int words_length;
  unsigned char words[DPcPathSize];
  int start, length, base_frequency, nb_words;
  struct og_ltra_add_trf_word *new_word;
  void *hltras = ctrl_paste->hltras;
  struct og_ltra_word *word;
  struct og_ltra_trf *trf;

  trf = input->Trf + ctrl_paste->buildedItrf[0];
  start = trf->start;
  trf = input->Trf + ctrl_paste->buildedItrf[ctrl_paste->nb_buildedItrf - 1];
  length = trf->start + trf->length - start;

  words_length = 0;
  base_frequency = 0;
  nb_words = 0;
  int origin_start_position = 0;
  int origin_end_position = 0;

  memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
  tinput->start_transposition=OgHeapGetCellsUsed(trfs->htransposition);

  for (int i = 0; i < ctrl_paste->nb_buildedItrf; i++)
  {
    trf = input->Trf + ctrl_paste->buildedItrf[i];
    /** We don't paste a trf that is cut/split, thus there is necessarily only one word per trf **/

    word = trfs->Word + trf->start_word + 0;
    memcpy(words + words_length, trfs->Ba + word->start, word->length);
    words_length += word->length;
    base_frequency += word->base_frequency;
    nb_words++;
    for (int j=0; j<word->length; j+=2)
    {
      int transposition=(word->start_position+j)/2;
      IFE(OgHeapAppend(trfs->htransposition,1,&transposition));
    }

    // Create original string for levenshtein distance
    if (i == 0)
    {
      origin_start_position = word->start_position;
    }
    if (i == (ctrl_paste->nb_buildedItrf - 1))
    {
      origin_end_position = word->start_position + word->length_position;
    }
  }

  int end_transposition = OgHeapGetCellsUsed(trfs->htransposition);
  tinput->length_transposition = end_transposition - tinput->start_transposition;

  //Keep original string of trfs to compare with transformation string and
  //to calculate a levenshtein score
  unsigned char *origin = trfs->Ba + origin_start_position;
  int origin_length = origin_end_position - origin_start_position;

  base_frequency /= nb_words;

  tinput->start = start;
  tinput->length = length;
  new_word = tinput->word + tinput->nb_words;
  new_word->string_length = words_length;
  new_word->string = words;
  new_word->frequency = 0;
  new_word->base_frequency = base_frequency;
  new_word->start_position = start;
  new_word->length_position = length;
  tinput->nb_words++;

  tinput->module_id = module_input->id;
  /* The cost is paste_cost * number of pastes and
   * number of pastes = number of pasted words - 1 */

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
  IFE(dlevenshtein_distance = OgStmLevenshteinFast(ctrl_paste->hstm, origin_length, origin, tranformed_length,
      tranformed, ctrl_paste->levenshtein_costs));

  tinput->score = 1.0 - dlevenshtein_distance;

  /** we use the first word as from_trf **/
  tinput->from_trf = ctrl_paste->buildedItrf[0];
  IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));

  DONE;
}




static int LtrasModulePasteTrfInBuildedItrf(struct og_ctrl_paste *ctrl_paste, int Itrf)
{
int i;
for (i=0; i<ctrl_paste->nb_buildedItrf; i++) {
  if (Itrf == ctrl_paste->buildedItrf[i]) return(1);
  }
return(0);
}




static int LtrasModulePasteTrfIsNext(struct og_ctrl_paste *ctrl_paste, struct og_ltra_trfs *trfs, int Itrf)
{
int Ibf_trf=ctrl_paste->buildedItrf[ctrl_paste->nb_buildedItrf-1];
struct og_ltra_trf *bf_trf = trfs->Trf + Ibf_trf;
struct og_ltra_trf *trf = trfs->Trf + Itrf;
int space_start, space_end;


space_start = bf_trf->start + bf_trf->length;
space_end = trf->start;

if (space_start < space_end) {
  if (LtrasModulePasteIsGoodSpace(ctrl_paste, trfs, space_start, space_end)) {
    return(1);
    }
  }

return(0);
}




static int LtrasModulePasteIsGoodSpace(struct og_ctrl_paste *ctrl_paste, struct og_ltra_trfs *trfs, int space_start, int space_end)
{
int iuni; unsigned char *uni;
int i,c;

uni=trfs->Ba+trfs->start_text;
iuni=trfs->length_text;

if (space_end > iuni) return(0);
if (space_start > iuni) return(0);
if (space_start >= space_end) return(0);

for (i=space_start; i<space_end; i+=2) {
  c = (uni[i]<<8) + uni[i+1];
  if (OgUniIsspace(c)) continue;
  if (c=='-' || c=='_') continue;
  return(0);
  }
return(1);
}


