/*
 *	Handling parameters
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev: June 2010
 *	Version 1.0
*/
#include "ogm_ltras.h"




PUBLIC(int) OgLtrasGetParameterValue(void *handle, unsigned char *name, int value_size, unsigned char *value)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
unsigned char out[DPcAutMaxBufferSize+9];
int ibuffer; char buffer[DPcPathSize];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1,iout;

sprintf(buffer,"%s\1",name);
ibuffer=strlen(buffer);

if ((retour=OgAutScanf(ctrl_ltras->ha_param,ibuffer,buffer,&iout,out,&nstate0,&nstate1,states))) {
  do {
    if (iout > value_size) continue;
    /** There should be only one value, but first found value is the one **/
    strcpy(value,out);
    return(1);
    }
  while((retour=OgAutScann(ctrl_ltras->ha_param,&iout,out,nstate0,&nstate1,states)));
  }

return(0);
}





int LtrasAddParameter(struct og_ctrl_ltras *ctrl_ltras,unsigned char *parameter_name,unsigned char *parameter_value)
{
int ibuffer; char buffer[DPcPathSize*2];
sprintf(buffer,"%s\1%s",parameter_name,parameter_value);
ibuffer=strlen(buffer);
IFE(OgAutAdd(ctrl_ltras->ha_param,ibuffer,buffer));
DONE;
}



