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
int i,ibuffer,real_length,language_code;
unsigned char out[DPcAutMaxBufferSize+9];
oindex states[DPcAutMaxBufferSize+9];
unsigned char *p,buffer[DPcPathSize];
int retour,nstate0,nstate1,iout;
int is; unsigned char *s;
double freq,neg_freq;

is = pawo->length; if (is > DPcPathSize/2) DONE;
s = ctrl_lip->input->content+pawo->start;

memcpy(buffer,s,is); ibuffer=is;
buffer[ibuffer++]=0; buffer[ibuffer++]=DOgFreqLanguageSeparator;

i=0;

if ( (retour=OgAufScanf(ctrl_lip->ha_lang,ibuffer,buffer,&iout,out,&nstate0,&nstate1,states)) ) {
  do {
    IFE(retour);
    
    /** Get negative frequency **/
    //IFE(real_length=OgRealLength(out));
    IFE(real_length=OgRealToDouble(out,&neg_freq));
    freq=(-neg_freq);

    p=out+real_length+1;
    IFE(language_code=(int)OggNin4(&p));

    pawo->lang.language[i].score=freq;
    pawo->lang.language[i].lang=language_code;
    i++; if (i>=DOgLipLangMax) break;

    }
  while( (retour=OgAufScann(ctrl_lip->ha_lang,&iout,out,nstate0,&nstate1,states)) );
  }
  
pawo->lang.nb_languages=i;

DONE;
}


