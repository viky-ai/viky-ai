/*
 *  Functions for the ltra_base dictionary
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#include "ogm_ltrac.h"
#include <logis639_3166.h>




int LtracDicBaseAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{

  int ientry;
  unsigned char *p, entry[DPcPathSize];

  /* word | language_code frequency **/
  memcpy(entry, dic_input->value, dic_input->value_length);
  p = entry + dic_input->value_length;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->language_code, &p);
  OggNout(dic_input->frequency, &p);
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_base, ientry, entry));

  DONE;
}




PUBLIC(int) OgLtracDicBaseLog(void *handle)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
struct og_aut_param caut_param,*aut_param=&caut_param;
int ibuffer; unsigned char buffer[DPcPathSize];
int language_code,frequency;
oindex states[DPcAutMaxBufferSize+9];
int iout; unsigned char *p,out[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1;
char erreur[DOgErrorSize];
void *ha_base;
FILE *fd;

IFn(handle) DONE;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_ltrac->herr;
aut_param->hmutex=ctrl_ltrac->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory;
aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
aut_param->state_number = 0;
sprintf(aut_param->name,"ltrac base");
IFn(ha_base=OgAutInit(aut_param)) DPcErr;
IFE(OgAufRead(ha_base,ctrl_ltrac->name_base));

IFn(fd=fopen(ctrl_ltrac->log_base,"w")) {
  sprintf(erreur,"OgLtracDicBaseLog: impossible to open '%s' for writing",ctrl_ltrac->log_base);
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

if ((retour=OgAufScanf(ha_base,0,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    int i,c,sep=(-1);
    IFE(retour);
    for (i=0; i<iout; i+=2) {
      c=(out[i]<<8)+out[i+1];
      if (c==DOgLtracExtStringSeparator) { sep=i; break; }
      }
    if (sep<0) continue;
    p=out+sep+2;
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));
    IFE(OgUniToCp(sep,out,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));


    char slang_country[DPcPathSize];

      fprintf(fd, "%s | %s %d\n", buffer, OgIso639_3166ToCode(language_code, slang_country), frequency);

    }
  while((retour=OgAufScann(ha_base,&iout,out,nstate0,&nstate1,states)));
  }

fclose(fd);
IFE(OgAutFlush(ha_base));

DONE;
}


