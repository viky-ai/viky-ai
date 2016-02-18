/*
 *	Subtree functions
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev : September 2010
 *	Version 1.0
*/
#include "ogm_rqp.h"



static int RqpSubtreeIdToNumber(struct og_ctrl_rqp *ctrl_rqp, unsigned char *subtree_id, int *psubtree_number);



PUBLIC(int) OgRqpSubtreeSetFunctionName(void *handle, unsigned char *subtree_function_name)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
IFE(OgCpToUni(strlen(subtree_function_name), subtree_function_name, DPcPathSize
  , &ctrl_rqp->subtree_function_name_length, ctrl_rqp->subtree_function_name, DOgCodePageUTF8, 0, 0));
DONE;
}




PUBLIC(int) OgRqpSubtreeNumberToId(void *handle, int subtree_number, unsigned char *subtree_id)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
return(RqpSubtreeNumberToId(ctrl_rqp, subtree_number, subtree_id));
}





PUBLIC(int) OgRqpSubtreeIdToNumber(void *handle , unsigned char *subtree_id, int *psubtree_number)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
return(RqpSubtreeIdToNumber(ctrl_rqp, subtree_id, psubtree_number));
}




PUBLIC(int) OgRqpSubtreeScan(void *handle, int (*func)(void *context, int subtree_number, unsigned char *subtree_id), void *context)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
unsigned char buffer[DPcPathSize];
int i, subtree_number;

if ((retour=OgAutScanf(ctrl_rqp->ha_subtree_id,2,"n:",&iout,out,&nstate0,&nstate1,states))) {
  do { 
    int sep=(-1);
    IFE(retour);
    for (i=0; i<iout;i++) {
      if (out[i]=='\1') { sep=i; break; }
      }
    if (sep<0) continue;  
    memcpy(buffer,out,sep); buffer[sep]=0; 
    subtree_number=atoi(buffer);
    IFE(func(context,subtree_number,out+sep+1));
    }
  while((retour=OgAutScann(ctrl_rqp->ha_subtree_id,&iout,out,nstate0,&nstate1,states)));
  }

DONE;
}




int RqpSubtreeAddId(struct og_ctrl_rqp *ctrl_rqp, int subtree_number, int uni_subtree_id_length, unsigned char *uni_subtree_id)
{
int subtree_id_length; unsigned char subtree_id[DPcPathSize];
int ibuffer; unsigned char buffer[DPcPathSize];
int tmp_subtree_number,found;
char erreur[DOgErrorSize];
int i,c;

for (i=0; i<uni_subtree_id_length; i+=2) {
  c=(uni_subtree_id[i]<<8)+uni_subtree_id[i+1];
  if (c=='"' || c==',') {
    sprintf(erreur,"RqpAddSubtreeId: char '%c' not allowed in subtree id",c);
    OgErr(ctrl_rqp->herr,erreur); DPcErr;    
    }
  }

IFE(OgUniToCp(uni_subtree_id_length,uni_subtree_id,DPcPathSize,&subtree_id_length,subtree_id,DOgCodePageUTF8,0,0));

IFE(found=RqpSubtreeIdToNumber(ctrl_rqp,subtree_id,&tmp_subtree_number));
if (found) {
  sprintf(erreur,"RqpAddSubtreeId: duplicate subtree_id '%s', request should have a different tree id for each named tree",subtree_id);
  OgErr(ctrl_rqp->herr,erreur); DPcErr;
  }
  
sprintf(buffer,"n:%d\1%s",subtree_number,subtree_id); ibuffer=strlen(buffer);
IFE(OgAutAdd(ctrl_rqp->ha_subtree_id,ibuffer,buffer));

sprintf(buffer,"i:%s\1%d",subtree_id,subtree_number); ibuffer=strlen(buffer);
IFE(OgAutAdd(ctrl_rqp->ha_subtree_id,ibuffer,buffer));

DONE;
}




int RqpSubtreeNumberToId(struct og_ctrl_rqp *ctrl_rqp, int subtree_number, unsigned char *subtree_id)
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
char buffer[DPcPathSize];

sprintf(buffer,"n:%d\1",subtree_number);

if ((retour=OgAutScanf(ctrl_rqp->ha_subtree_id,strlen(buffer),buffer,&iout,out,&nstate0,&nstate1,states))) {
  do { 
    IFE(retour);
    strcpy(subtree_id,out);
    return(1);
    }
  while((retour=OgAutScann(ctrl_rqp->ha_subtree_id,&iout,out,nstate0,&nstate1,states)));
  }

return(0);
}




static int RqpSubtreeIdToNumber(struct og_ctrl_rqp *ctrl_rqp, unsigned char *subtree_id, int *psubtree_number)
{
int retour,nstate0,nstate1,iout;
oindex states[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
char buffer[DPcPathSize];

sprintf(buffer,"i:%s\1",subtree_id);

if ((retour=OgAutScanf(ctrl_rqp->ha_subtree_id,strlen(buffer),buffer,&iout,out,&nstate0,&nstate1,states))) {
  do { 
    IFE(retour);
    *psubtree_number=atoi(out);
    return(1);
    }
  while((retour=OgAutScann(ctrl_rqp->ha_subtree_id,&iout,out,nstate0,&nstate1,states)));
  }

return(0);
}



