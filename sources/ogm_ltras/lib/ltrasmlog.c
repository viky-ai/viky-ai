/*
 *  Handling trf structures
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev: November 2009
 *  Version 1.0
*/
#include "ogm_ltras.h"



PUBLIC(og_status) OgLtrasTrfsLog(void *handle, struct og_ltra_trfs *trfs)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
int ibuffer; unsigned char buffer[DPcPathSize];
int i,iuni; unsigned char *uni;

uni=trfs->Ba+trfs->start_text;
iuni=trfs->length_text;

IFE(OgUniToCp(iuni,uni,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog, "text: '%s'", buffer);

for (i=0; i<trfs->TrfUsed; i++) {
  IFE(OgLtrasTrfLog(handle,trfs,i));
  }

DONE;
}




PUBLIC(int) OgLtrasTrfLog(void *handle, struct og_ltra_trfs *trfs, int Itrf)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
struct og_ltra_trf *trf = trfs->Trf + Itrf;
unsigned char modules[DPcPathSize];
struct og_ltra_module *module;
unsigned char words[DPcPathSize];
int isword; unsigned char sword[DPcPathSize];
unsigned char slanguage[DPcPathSize];
unsigned char sfinal[DPcPathSize];
unsigned char stotal[DPcPathSize];
unsigned char sterm[DPcPathSize];
unsigned char stransposition[DPcPathSize*4];
struct og_ltra_word *word;
int i;

for (i=0,words[0]=0; i<trf->nb_words; i++) {
  word = trfs->Word + trf->start_word + i;
  IFE(LtrasWordString(ctrl_ltras,trfs,trf->start_word+i,&isword,sword));
  slanguage[0]=0;
  if (word->language) {
    sprintf(slanguage,"%s,",OgIso639ToCode(word->language));
    }
  sprintf(words+strlen(words),"%s%s:%s%d,%d:%d,%d",(i?" ":""),sword,slanguage
    ,word->frequency, word->base_frequency, word->start_position, word->length_position);
  }

if (trf->nb_modules > 0) {
  sprintf(modules,"[");
  for (i=0; i<trf->nb_modules; i++) {
    module = trfs->Module + trf->start_module + i;
    char *module_name = OgLtrasModuleName(ctrl_ltras,module->module_id);
    sprintf(modules+strlen(modules),"%s%s",(i?",":""),module_name);
    }
  sprintf(modules+strlen(modules),"]");
  }
else modules[0]=0;

sfinal[0]=0; if (trf->final) sprintf(sfinal," final");
stotal[0]=0; if (trf->total) sprintf(stotal," total");
slanguage[0]=0; if (trf->language) sprintf(slanguage,"%s ",OgIso639ToCode(word->language));

sterm[0]=0;
if (trf->span_start_trf >= 0) {
  sprintf(sterm," [%d-%d %.4f %.4f %d]", trf->span_start_trf
    , trf->span_nb_trfs, trf->final_score, trf->global_score, trf->global_frequency);
  }

stransposition[0]=0;
if (trf->length_transposition>0)
{
  sprintf(stransposition," {");
  for (i=0; i<trf->length_transposition; i++)
  {
    int *ptransposition=OgHeapGetCell(trfs->htransposition, trf->start_transposition+i);
    int transposition = *ptransposition;
    sprintf(stransposition+strlen(stransposition),"%s%d:%d",i?" ":"",i,transposition);
  }
  sprintf(stransposition+strlen(stransposition),"}");
}

OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog, "  %2d: %s -> %s%d,%d %s%s%s%s%s"
  , Itrf, words, slanguage, trf->start, trf->length, modules, sfinal, stotal, sterm, stransposition);

DONE;
}


PUBLIC(int) OgLtrasInputTrfString(void *handle, struct og_ltra_add_trf_input *input, int string_size, unsigned char *string)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
unsigned char scores[DPcPathSize];
unsigned char words[DPcPathSize];
int isword; unsigned char sword[DPcPathSize];
struct og_ltra_add_trf_word *new_word;
unsigned char sfrom_trf[DPcPathSize];
unsigned char sfinal[DPcPathSize];
unsigned char stotal[DPcPathSize];
int i;

for (i=0,words[0]=0; i<input->nb_words; i++) {
  new_word = input->word+i;
  IFE(OgUniToCp(new_word->string_length,new_word->string,DPcPathSize,&isword,sword,DOgCodePageUTF8,0,0));
  sprintf(words+strlen(words),"%s%s:%d,%d:%d,%d",(i?" ":""),sword
    ,new_word->frequency, new_word->base_frequency, new_word->start_position, new_word->length_position);
  }

sprintf(scores,"[%s:%.4f]",OgLtrasModuleName(ctrl_ltras,input->module_id),input->score);

sfinal[0]=0; if (input->final) sprintf(sfinal," final");
stotal[0]=0; if (input->total) sprintf(stotal," total");
sprintf(sfrom_trf," from_trf=%d",input->from_trf);

sprintf(string, "%s -> %d,%d %s%s%s%s"
  , words, input->start, input->length, scores, sfinal, stotal, sfrom_trf);

DONE;
}


