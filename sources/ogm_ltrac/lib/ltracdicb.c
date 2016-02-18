/*
 *	Functions for the ltra_base dictionary
 *	Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *	Dev : November 2009
 *	Version 1.0
*/
#include "ogm_ltrac.h"





int LtracDicBaseAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input)
{
int ientry; unsigned char *p,entry[DPcPathSize];

/* word | attribute_number language_code frequency **/
memcpy(entry,dic_input->value,dic_input->value_length);
p=entry+dic_input->value_length; *p++=0; *p++=DOgLtracExtStringSeparator;
OggNout(dic_input->attribute_number,&p);
OggNout(dic_input->language_code,&p);
OggNout(dic_input->frequency,&p);
ientry=p-entry;
IFE(OgAutAdd(ctrl_ltrac->ha_base,ientry,entry));

DONE;
}




PUBLIC(int) OgLtracDicBaseLog(void *handle)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
struct og_sidx_param csidx_param,*sidx_param=&csidx_param; 
struct og_aut_param caut_param,*aut_param=&caut_param;
int ibuffer; unsigned char buffer[DPcPathSize];
int attribute_number,language_code,frequency;
oindex states[DPcAutMaxBufferSize+9];
int iout; unsigned char *p,out[DPcAutMaxBufferSize+9];
char attribute_string[DPcPathSize];
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

IFE(LtracAttributesPlugInit(ctrl_ltrac));

memset(sidx_param,0,sizeof(struct og_sidx_param));
sidx_param->herr=ctrl_ltrac->herr; 
sidx_param->hmsg=ctrl_ltrac->hmsg;
sidx_param->hmutex=ctrl_ltrac->hmutex;
sidx_param->loginfo.trace = DOgSidxTraceMinimal+DOgSidxTraceMemory; 
sidx_param->loginfo.where = ctrl_ltrac->loginfo->where;
strcpy(sidx_param->WorkingDirectory,ctrl_ltrac->WorkingDirectory);
strcpy(sidx_param->configuration_file,ctrl_ltrac->configuration_file);
strcpy(sidx_param->data_directory,ctrl_ltrac->data_directory);
strcpy(sidx_param->import_directory,"");
IFn(ctrl_ltrac->OgSidxInit(sidx_param,&ctrl_ltrac->hsidx,&ctrl_ltrac->authorized)) DPcErr;

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
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&attribute_number));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));
    IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));
    IFE(OgUniToCp(sep,out,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
    IFE(ctrl_ltrac->OgSidxAttributeNumberToString(ctrl_ltrac->hsidx,attribute_number,attribute_string));
    fprintf(fd, "%s | %d=%s %d %d\n",buffer,attribute_number,attribute_string,language_code,frequency);
    
    }
  while((retour=OgAufScann(ha_base,&iout,out,nstate0,&nstate1,states)));
  }

fclose(fd);
IFE(OgAutFlush(ha_base));
IFE(ctrl_ltrac->OgSidxFlush(ctrl_ltrac->hsidx));

DONE;
}


