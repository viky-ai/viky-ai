/*
 *  Subtree functions
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : September 2010
 *  Version 1.0
*/
#include "ogm_rqp.h"



static int RqpSubtreeIdToNumber(struct og_ctrl_rqp *ctrl_rqp, unsigned char *subtree_id, int *psubtree_number);


PUBLIC(int) OgRqpSubtreeNumberToId(void *handle, int subtree_number, unsigned char *subtree_id)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
return(RqpSubtreeNumberToId(ctrl_rqp, subtree_number, subtree_id, NULL));
}





PUBLIC(int) OgRqpSubtreeIdToNumber(void *handle , unsigned char *subtree_id, int *psubtree_number)
{
struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *)handle;
return(RqpSubtreeIdToNumber(ctrl_rqp, subtree_id, psubtree_number));
}




PUBLIC(int) OgRqpSubtreeScan(void *handle,
    int (*func)(void *context, int subtree_number, unsigned char *subtree_id, og_rqp_subtree_type type), void *context)
{
  struct og_ctrl_rqp *ctrl_rqp = (struct og_ctrl_rqp *) handle;
  int retour, nstate0, nstate1, iout;
  oindex states[DPcAutMaxBufferSize + 9];
  unsigned char out[DPcAutMaxBufferSize + 9];

  if ((retour = OgAutScanf(ctrl_rqp->ha_subtree_id, 2, "n:", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      int type = DOgRqpSubtreeTypeNormal;
      char subtree_id[DOgRqpSubtreeMaxNameSize];
      int subtree_number = 0;
      sscanf(out, "%d\1%d\1%256s", &subtree_number, &type, subtree_id);
      IFE(func(context, subtree_number, subtree_id, (og_rqp_subtree_type)type));
    }
    while ((retour = OgAutScann(ctrl_rqp->ha_subtree_id, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}




int RqpSubtreeAddId(struct og_ctrl_rqp *ctrl_rqp, int subtree_number, int uni_subtree_id_length,
    unsigned char *uni_subtree_id, og_rqp_subtree_type type)
{
  for (int i = 0; i < uni_subtree_id_length; i += 2)
  {
    int c = (uni_subtree_id[i] << 8) + uni_subtree_id[i + 1];
    if (c == '"' || c == ',')
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "RqpAddSubtreeId: char '%c' not allowed in subtree id", c);
      OgErr(ctrl_rqp->herr, erreur);
      DPcErr;
    }
  }

  unsigned char subtree_id[DOgRqpSubtreeMaxNameSize];
  int subtree_id_length = 0;
  IFE(OgUniToCp(uni_subtree_id_length,uni_subtree_id,DOgRqpSubtreeMaxNameSize,&subtree_id_length,subtree_id,DOgCodePageUTF8,0,0));

  int tmp_subtree_number = 0;
  int found = RqpSubtreeIdToNumber(ctrl_rqp, subtree_id, &tmp_subtree_number);
  IFE(found);
  if (found)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur,
        "RqpAddSubtreeId: duplicate subtree_id '%s', request should have a different tree id for each named tree",
        subtree_id);
    OgErr(ctrl_rqp->herr, erreur);
    DPcErr;
  }

  unsigned char buffer[DPcPathSize];
  int ibuffer = 0;
  sprintf(buffer, "n:%d\1%d\1%s", subtree_number, type, subtree_id);
  ibuffer = strlen(buffer);
  IFE(OgAutAdd(ctrl_rqp->ha_subtree_id, ibuffer, buffer));

  sprintf(buffer, "i:%s\1%d\1%d", subtree_id, subtree_number, type);
  ibuffer = strlen(buffer);
  IFE(OgAutAdd(ctrl_rqp->ha_subtree_id, ibuffer, buffer));

  DONE;
}




int RqpSubtreeNumberToId(struct og_ctrl_rqp *ctrl_rqp, int subtree_number, unsigned char *subtree_id,
    og_rqp_subtree_type *type)
{
  int retour, nstate0, nstate1, iout;
  oindex states[DPcAutMaxBufferSize + 9];
  unsigned char out[DPcAutMaxBufferSize + 9];
  char buffer[DPcPathSize];

  sprintf(buffer, "n:%d\1", subtree_number);

  if ((retour = OgAutScanf(ctrl_rqp->ha_subtree_id, strlen(buffer), buffer, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      int type_temp;
      sscanf(out, "%d\1%256s", &type_temp, subtree_id);
      if(type!=NULL) *type = type_temp;
      return (1);
    }
    while ((retour = OgAutScann(ctrl_rqp->ha_subtree_id, &iout, out, nstate0, &nstate1, states)));
  }

  return (0);
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



