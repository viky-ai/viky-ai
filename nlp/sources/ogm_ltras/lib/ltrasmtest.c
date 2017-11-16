/*
 *  A simple test module for linguistic trfs
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include <logltras.h>


struct og_ctrl_test {
  void *herr,*hmsg,*hltras; ogmutex_t *hmutex;
  struct og_loginfo cloginfo,*loginfo;
  };



void *OgLtrasModuleTestInit(struct og_ltra_module_param *param)
{
struct og_msg_param cmsg_param,*msg_param=&cmsg_param;
struct og_ctrl_test *ctrl_test;
char erreur[DOgErrorSize];

IFn(ctrl_test=(struct og_ctrl_test *)malloc(sizeof(struct og_ctrl_test))) {
  sprintf(erreur,"OgLtrasInit: malloc error on ctrl_test");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_test,0,sizeof(struct og_ctrl_test));

ctrl_test->herr = param->herr;
ctrl_test->hltras = param->hltras;
ctrl_test->hmutex = param->hmutex;
ctrl_test->cloginfo = param->loginfo;
ctrl_test->loginfo = &ctrl_test->cloginfo;

memset(msg_param,0,sizeof(struct og_msg_param));
msg_param->herr=ctrl_test->herr;
msg_param->hmutex=ctrl_test->hmutex;
msg_param->loginfo.trace = DOgMsgTraceMinimal+DOgMsgTraceMemory;
msg_param->loginfo.where = ctrl_test->loginfo->where;
msg_param->module_name="ogm_ltras";
IFn(ctrl_test->hmsg=OgMsgInit(msg_param)) return(0);
IF(OgMsgTuneInherit(ctrl_test->hmsg,param->hmsg)) return(0);

return((void *)ctrl_test);
}



int OgLtrasModuleTestFlush(void *handle)
{
struct og_ctrl_test *ctrl_test = (struct og_ctrl_test *)handle;
IFE(OgMsgFlush(ctrl_test->hmsg));
DPcFree(ctrl_test);
DONE;
}



int OgLtrasModuleTest(struct og_ltra_module_input *module_input, struct og_ltra_trfs *input,
    struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_test *ctrl_test = (struct og_ctrl_test *) module_input->handle;
  struct og_ltra_add_trf_input ctinput, *tinput = &ctinput;
  struct og_ltra_add_trf_word *new_word;
  void *hltras = ctrl_test->hltras;
  struct og_ltra_word *word;
  struct og_ltra_trfs *trfs;
  int is;
  unsigned char *s;
  int TrfUsed;
  int i;

  ogint64_t micro_clock_start = OgMicroClock();

  /** On commence par dupliquer la trf **/
  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  *output = trfs;

  /* On ajoute les nouvelles trfs
   * Dans cet exemple, on n'utilise pas de dictionnaire, mais simplement
   * quelques trfs banales */
  TrfUsed = trfs->TrfUsed;
  for (i = 0; i < TrfUsed; i++)
  {
    struct og_ltra_trf *trf = trfs->Trf + i;
    if (!trf->basic) continue;
    s = trfs->Ba + trf->start;
    is = trf->length;
    if (is == 10 && !Ogmemicmp(s, "\0m\0a\0i\0s\0n", is))
    {
      word = trfs->Word + trf->start_word;
      memset(tinput, 0, sizeof(struct og_ltra_add_trf_input));
      tinput->start = trf->start;
      tinput->length = trf->length;
      new_word = tinput->word + tinput->nb_words;
      new_word->string = "\0m\0a\0i\0s\0o\0n";
      new_word->string_length = 12;
      new_word->start_position = trf->start;
      new_word->length_position = trf->length;
      new_word->language = trf->language;
      new_word->frequency = 0;
      new_word->base_frequency = word->base_frequency;
      tinput->nb_words++;
      tinput->module_id = module_input->id;
      tinput->score = 0.5;
      IFE(OgLtrasTrfAdd(hltras, trfs, tinput, 0));
    }
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;
}





