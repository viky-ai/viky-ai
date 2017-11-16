/*
 *  Language recognition for the document
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : May 2009
 *  Version 1.0
*/
#include "ogm_lip.h"


static int LipGetLang1(struct og_ctrl_lip *,int);
static int language_cmp(const void *, const void *);




PUBLIC(int) OgLipGetLanguages(void *handle, struct og_lip_lang *lang)
{
struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *)handle;
int i;

IFn(ctrl_lip->ha_lang) {
  lang->nb_languages=0;
  DONE;
  }

IFE(LipGetLang(ctrl_lip));

for (i=0; i<DOgLipLangMax; i++) {
  IFn(ctrl_lip->language[i].score) break;
  lang->language[i].score=ctrl_lip->language[i].score;
  lang->language[i].lang=ctrl_lip->language[i].lang;
  }
lang->nb_languages=i;

IFE(LipAdjustLang(ctrl_lip,lang->language[0].lang));

DONE;
}





int LipGetLang(struct og_ctrl_lip *ctrl_lip)
{
int i;

for (i=0; i<DOgLangMax; i++) {
  ctrl_lip->language[i].score=0;
  ctrl_lip->language[i].lang=i;
  }

for (i=0; i<ctrl_lip->PawoUsed; i++) {
  IFE(LipGetLang1(ctrl_lip,i));
  }

qsort(ctrl_lip->language,DOgLangMax,sizeof(struct language),language_cmp);

DONE;
}



static int LipGetLang1(struct og_ctrl_lip *ctrl_lip, int Ipawo)
{
struct pawo *pawo = ctrl_lip->Pawo+Ipawo;
int language_code;

if (pawo->lang.nb_languages<=0) DONE;

#if 1
/* We use the first language and not the score because the score
 * is too different. For example : 'a' (en:0.0128948500)
 * compared to 'petit' (fr:0.0000063566) make 'a' 2028
 * times more efficient than 'petit' while 'petit'
 * is intuitively much more efficient than 'a'
 * The efficiency is linked to the size of the word */
language_code = pawo->lang.language[0].lang;
if (language_code <= 0) DONE;
if (DOgLangMax <= language_code) DONE;
ctrl_lip->language[language_code].score += pawo->length;
#else
for (i=0; i<pawo->lang.nb_languages; i++) {
  language_code = pawo->lang.language[i].lang;
  if (language_code <= 0) continue; 
  if (DOgLangMax <= language_code) continue; 
  ctrl_lip->language[language_code].score += pawo->lang.language[i].score*pawo->length;
  }  
#endif

DONE;
}




static int language_cmp(const void *ptr1, const void *ptr2)
{
struct language *language1 = (struct language *)ptr1;
struct language *language2 = (struct language *)ptr2;
double cmp;

cmp = language2->score - language1->score;
if (cmp < 0) return(-1);
else if (cmp > 0) return(1);
return(language1->lang - language2->lang);
}





