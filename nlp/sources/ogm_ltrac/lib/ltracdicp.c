/*
 *  Functions for the ltra_phon dictionary
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : January 2010
 *  Version 1.0
*/
#include "ogm_ltrac.h"
#include <logis639_3166.h>




int LtracDicPhonAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{

  if (dic_input->is_expression) DONE;

  int ientry;
  unsigned char *p, entry[DPcPathSize];
  struct og_pho_output coutput, *output = &coutput;
  struct og_pho_input cinput, *input = &cinput;
  int min_post_phonetisation_char_number = 2;
  int min_phonetisation_char_number = 3;
  int i, c, iin = dic_input->value_length;
  og_string in = dic_input->value;

  IFn(ctrl_ltrac->hpho) DONE;
  if (iin < min_phonetisation_char_number * 2) DONE;

  /* We do not phonetize names with digits */
  for (i = 0; i < iin; i += 2)
  {
    c = (in[i] << 8) + in[i + 1];
    if (OgUniIsdigit(c)) DONE;
  }

  input->lang = dic_input->language_code;
  input->iB = iin;
  input->B = (char *) in;

  IFE(OgPhonet(ctrl_ltrac->hpho, input, output));
  if (output->iB < min_post_phonetisation_char_number * 2) DONE;

  /* phonetic_form | language_code frequency **/
  memcpy(entry, output->B, output->iB);
  p = entry + output->iB;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->language_code, &p);
  OggNout(dic_input->frequency, &p);
  memcpy(p, in, iin);
  p += iin;
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_phon, ientry, entry));

  DONE;
}






PUBLIC(int) OgLtracDicPhonLog(void *handle, void *ha_phon)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
int iout; unsigned char *p,out[DPcAutMaxBufferSize+9];
int ibuffer; unsigned char buffer[DPcPathSize];
int language_code,frequency;
int iword; unsigned char word[DPcPathSize];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1;

IFn(handle) DONE;

if ((retour=OgAufScanf(ha_phon,0,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    int i,c,sep=(-1);
    IFE(retour);
    for (i=0; i<iout; i+=2) {
      c=(out[i]<<8)+out[i+1];
      if (c==DOgLtracExtStringSeparator) { sep=i; break; }
      }
    if (sep<0) continue;
    p=out+sep+2;
    IFE(OgUniToCp(sep,out,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));
    word[0]=0;
    if (p-out < iout) {
      IFE(OgUniToCp(iout-(p-out),p,DPcPathSize,&iword,word,DOgCodePageUTF8,0,0));
      }


    char slang_country[DPcPathSize];

    OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "%s | %s %d %s", buffer, OgIso639_3166ToCode(language_code, slang_country), frequency, word);
    }
  while((retour=OgAufScann(ha_phon,&iout,out,nstate0,&nstate1,states)));
  }

DONE;
}



