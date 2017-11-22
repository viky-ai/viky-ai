/*
 *  Parsing function for Linguistic trf search
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#include "ogm_ltras.h"


struct og_ltras_parse_info {
  struct og_ctrl_ltras *ctrl_ltras;
  struct og_ltra_trfs *trfs;
  };


static int LtrasParseWord(void *context, int Iw);



/*
 * This function parses the unicode text contained in input->string
 * and creates a list of trfs in output.
*/

PUBLIC(og_status) OgLtrasParse(void *handle, struct og_ltras_input *input
  , struct og_ltra_trfs **ptrfs)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct og_lip_input clip_input,*lip_input=&clip_input;
struct og_ltras_parse_info cinfo,*info=&cinfo;
struct og_ltra_trfs *trfs;

IFE(OgLtrasTrfsCreate(handle,&trfs));
IFE(OgLtrasTrfsAddText(handle, trfs, input->request_length, input->request));

memset(info,0,sizeof(struct og_ltras_parse_info));
info->trfs=trfs;
info->ctrl_ltras=ctrl_ltras;

memset(lip_input,0,sizeof(struct og_lip_input));
lip_input->content_length = input->request_length;
lip_input->content = input->request;
lip_input->conf = &ctrl_ltras->lip_conf;
lip_input->word_func = LtrasParseWord;
lip_input->context = info;

IFE(OgLip(ctrl_ltras->hlip,lip_input));

trfs->TrfBasicUsed = trfs->TrfUsed;
*ptrfs=trfs;

DONE;
}




static int LtrasParseWord(void *context, int Iw)
{
struct og_ltras_parse_info *info=(struct og_ltras_parse_info *)context;
struct og_ltra_add_trf_input ctinput,*tinput=&ctinput;
struct og_ctrl_ltras *ctrl_ltras = info->ctrl_ltras;
struct og_ltra_add_trf_word *new_word;
struct og_lip_word cw,*w=&cw;

IFE(OgLipGetWord(ctrl_ltras->hlip,Iw,w));
memset(tinput,0,sizeof(struct og_ltra_add_trf_input));
tinput->basic = 1;
tinput->start = w->start;
tinput->length = w->length;
new_word = tinput->word+tinput->nb_words;
new_word->string = w->input->content + w->start;
new_word->string_length = w->length;

// choose best scored lang by lip
double best_lang_score = 0.0;
int lang = info->ctrl_ltras->input->language_code;
if(lang == 0)
{
  for (int i = 0; i < w->lang.nb_languages; i++)
  {
    struct og_lip_lang_score *lip_lang_score = w->lang.language + i;
    if (lip_lang_score->score > best_lang_score)
    {
      best_lang_score = lip_lang_score->score;
      lang = lip_lang_score->lang;
    }
  }
}
new_word->language = lang;

/** A zero frequency ratio means we want all solutions, thus we do not calculate base frequency **/
if (ctrl_ltras->input->frequency_ratio > 0) {
  IFE(OgLtrasTrfCalculateFrequency(ctrl_ltras, new_word->string_length, new_word->string, new_word->language, &new_word->base_frequency));
  }
new_word->frequency = 0;
new_word->start_position = w->start;
new_word->length_position = w->length;
tinput->nb_words++;
tinput->score = 1;
tinput->from_trf = (-1);
IFE(OgLtrasTrfAdd(ctrl_ltras,info->trfs,tinput,0));

DONE;
}

