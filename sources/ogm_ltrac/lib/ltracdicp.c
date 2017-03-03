/*
 *  Functions for the ltra_phon dictionary
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : January 2010
 *  Version 1.0
*/
#include "ogm_ltrac.h"





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

  input->iB = iin;
  input->B = (char *) in;

  if (dic_input->language_code == DOgLangNil)
  {
    //TODO gérer de façon plus propre avec uen variable commune pour tout le monde issue du ogm_ssi.txt
    input->lang = DOgLangFR;
  }
  else
  {
    input->lang = dic_input->language_code;
  }

  IFE(OgPhonet(ctrl_ltrac->hpho, input, output));
  if (output->iB < min_post_phonetisation_char_number * 2) DONE;

  /* phonetic_form | attribute_number language_code frequency **/
  memcpy(entry, output->B, output->iB);
  p = entry + output->iB;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->attribute_number, &p);
  OggNout(dic_input->language_code, &p);
  OggNout(dic_input->frequency, &p);
  memcpy(p, in, iin);
  p += iin;
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_phon, ientry, entry));

  DONE;
}






PUBLIC(int) OgLtracDicPhonLog(void *handle)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
struct og_aut_param caut_param,*aut_param=&caut_param;
int iout; unsigned char *p,out[DPcAutMaxBufferSize+9];
int ibuffer; unsigned char buffer[DPcPathSize];
int attribute_number,language_code,frequency;
int iword; unsigned char word[DPcPathSize];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1;
char erreur[DOgErrorSize];
void *ha_phon;
FILE *fd;

IFn(handle) DONE;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_ltrac->herr;
aut_param->hmutex=ctrl_ltrac->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
aut_param->state_number = 0;
sprintf(aut_param->name,"ltrac phon");
IFn(ha_phon=OgAutInit(aut_param)) DPcErr;
IFE(OgAufRead(ha_phon,ctrl_ltrac->name_phon));

IFn(fd=fopen(ctrl_ltrac->log_phon,"w")) {
  sprintf(erreur,"OgLtracDicPhonLog: impossible to open '%s' for writing",ctrl_ltrac->log_phon);
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

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
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&attribute_number));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));
    word[0]=0;
    if (p-out < iout) {
      IFE(OgUniToCp(iout-(p-out),p,DPcPathSize,&iword,word,DOgCodePageUTF8,0,0));
      }


      fprintf(fd, "%s | %d %d %s\n", buffer, language_code, frequency, word);
    }
  while((retour=OgAufScann(ha_phon,&iout,out,nstate0,&nstate1,states)));
  }

fclose(fd);
IFE(OgAutFlush(ha_phon));

DONE;
}



