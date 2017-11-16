/*
 *  Language recognition
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : May 2009
 *  Version 1.0
*/
#include "ogm_lip.h"


static int LipLang1(struct og_ctrl_lip *,int);



int LipLang(struct og_ctrl_lip *ctrl_lip)
{
int i;

IFn(ctrl_lip->ha_lang) DONE;
if (!ctrl_lip->input->language_recognition) DONE;

for (i=0; i<ctrl_lip->PawoUsed; i++) {
  IFE(LipLang1(ctrl_lip,i));
  }

DONE;
}



static int LipLang1(struct og_ctrl_lip *ctrl_lip, int Ipawo)
{
struct pawo *pawo = ctrl_lip->Pawo+Ipawo;

// BRU disable language_recognition to avoid ogm_freq deps in viky.ai
pawo->lang.nb_languages=0;


DONE;
}


