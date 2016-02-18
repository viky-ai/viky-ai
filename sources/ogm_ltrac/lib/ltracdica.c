/*
 *	Functions for the ltra_phon dictionary
 *	Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *	Dev : November 2009
 *	Version 1.0
*/
#include "ogm_ltrac.h"





int LtracDicAspellAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input)
{
int ibuffer; unsigned char buffer[DPcPathSize];
IFE(OgUniToCp(dic_input->value_length,dic_input->value,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
fprintf(ctrl_ltrac->fd_aspell,"%s\n",buffer);
DONE;
}


