/*
 *	Handling small words
 *	Copyright (c) 2010 Pertimm by Patrick Constant
 *	Dev : March 2010
 *	Version 1.0
*/
#include "ogm_ltrap.h"



int LtrapIsValidAsSmallWord(struct og_ctrl_ltrap *ctrl_ltrap,int is, unsigned char *s)
{
int retour;

/** No existing small words **/
IFn(ctrl_ltrap->max_small_word_length) return(1);
IFn(ctrl_ltrap->ha_small_words) return(1);

/** word is a big word **/
if (is > (ctrl_ltrap->max_small_word_length+ctrl_ltrap->offset_small_word)*2) return(1);

/** word is a small word we check if it is existing **/
IFE(retour=OgAutTry(ctrl_ltrap->ha_small_words,is,s));
if (retour==2) return(1);

return(0);
}





int LtrapIsFalseWord(struct og_ctrl_ltrap *ctrl_ltrap,int to_length, unsigned char *to)
{
int ibuffer,retour; unsigned char buffer[DPcAutMaxBufferSize+9];

/** No existing false words **/
IFn(ctrl_ltrap->ha_false) return(0);

ibuffer=0;
memcpy(buffer+ibuffer,"\0\1",2); ibuffer+=2;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,to,to_length);
OgUniStrlwr(to_length,buffer+ibuffer,buffer+ibuffer);
OgUniStruna(to_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=to_length;

IFE(retour=OgAutTry(ctrl_ltrap->ha_false,ibuffer,buffer));
if (retour==2) return(1);

ibuffer=0;
memcpy(buffer+ibuffer,"\0\1",2); ibuffer+=2;
memcpy(buffer+ibuffer,"\0a",2); ibuffer+=2;
memcpy(buffer+ibuffer,to,to_length);
OgUniStrlwr(to_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=to_length;

IFE(retour=OgAutTry(ctrl_ltrap->ha_false,ibuffer,buffer));
if (retour==2) return(1);


return(0);
}

