/*
 *  Language adjustment for some words in the document
 *  Copyright (c) 2012 Pertimm by Patrick Constant
 *  Dev : July 2012
 *  Version 1.0
*/
#include "ogm_lip.h"


static int LipAdjustLang1(struct og_ctrl_lip *ctrl_lip, int document_language_code, int Ipawo);


/* Language adjustement, means that we take into account the language of the document
 * to change some of the word languages with the following rule :
 * - if the word's first language is not the language of the document 
 *   and one of the word's langage is the language of the document 
 *   then swap the two language. Example "hotel in new york city", 
 *   "hotel" and "in" are detected as German, but the document's language
 *   English, we switch both words to English.
*/

int LipAdjustLang(struct og_ctrl_lip *ctrl_lip, int document_language_code)
{
int i;
for (i=0; i<ctrl_lip->PawoUsed; i++) {
  IFE(LipAdjustLang1(ctrl_lip,document_language_code,i));
  }
DONE;
}




static int LipAdjustLang1(struct og_ctrl_lip *ctrl_lip, int document_language_code, int Ipawo)
{
struct pawo *pawo = ctrl_lip->Pawo+Ipawo;
struct og_lip_lang_score lang;
int i, idlc;

if (pawo->lang.nb_languages<=0) DONE;

for (i=0,idlc=(-1); i<pawo->lang.nb_languages; i++) {
  if (pawo->lang.language[i].lang == document_language_code) { idlc=i; break; }
  }
if (idlc<=0) DONE;

lang=pawo->lang.language[idlc];
pawo->lang.language[idlc]=pawo->lang.language[0];
pawo->lang.language[0]=lang;

DONE;
}








