/*
 *	Initialization for ogm_lip functions
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev : October 2006
 *	Version 1.0
*/
#include "ogm_lip.h"





PUBLIC(int) OgLipGetWord(void *handle, int Ipawo, struct og_lip_word *w)
{
struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *)handle;
struct pawo *pawo;

if (Ipawo<0 || ctrl_lip->PawoUsed<=Ipawo) return(0);

pawo=ctrl_lip->Pawo+Ipawo;

memset(w,0,sizeof(struct og_lip_word));
w->start=pawo->start;
w->length=pawo->length;
w->real_start=pawo->real_start;
w->real_length=pawo->real_length;
w->punctuation_before_word=pawo->punctuation_before_word;
w->has_quote=pawo->has_quote;
w->has_dot_separation=pawo->has_dot_separation;
w->input=ctrl_lip->input;
if (Ipawo+1==ctrl_lip->PawoUsed) w->last_word=1;
w->nb_words=ctrl_lip->PawoUsed;
w->Iexpression_group=pawo->Iexpression_group;
w->is_indivisible_expression=pawo->is_indivisible_expression;

IFx(ctrl_lip->ha_lang) {
  memcpy(&w->lang,&pawo->lang,sizeof(struct og_lip_lang));
  }
  
return(1);
}




PUBLIC(int) OgLipGetNbWords(void *handle)
{
struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *)handle;
return(ctrl_lip->PawoUsed);
}


