/*
 *  Handling false expressions
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: March 2010
 *  Version 1.0
*/
#include "ogm_ltras.h"


/**
 * ha_false contains:
 *
 * nfrom:nto
 * nfrom:
 * :nto
 *
 * afrom:ato
 * afrom:
 * :ato
 *
 * nfrom:ato
 * afrom:nfrom
 *
 * we must test the 8 possibilities
*/

PUBLIC(og_bool) OgLtrasIsFalseTransformation(void *handle, int from_length, unsigned char *from, int to_length, unsigned char *to)
{
int ibuffer,sep,retour; unsigned char buffer[DPcAutMaxBufferSize+9];
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
IFn(ctrl_ltras->ha_false) return(0);

ibuffer=2+from_length+2+2+to_length;
if (ibuffer > DPcAutMaxBufferSize) return(0);

ibuffer=0;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,from,from_length);
OgUniStrlwr(from_length,buffer+ibuffer,buffer+ibuffer);
OgUniStruna(from_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=from_length; sep=ibuffer;
memcpy(buffer+ibuffer,"\0\1",2); ibuffer+=2;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,to,to_length);
OgUniStrlwr(to_length,buffer+ibuffer,buffer+ibuffer);
OgUniStruna(to_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=to_length;

/** nfrom:nto **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,sep+2,buffer));
if (retour==2) return(1);

/** :nto **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,ibuffer-sep,buffer+sep));
if (retour==2) return(1);

/** nfrom:nto **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,ibuffer,buffer));
if (retour==2) return(1);

ibuffer=0;
memcpy(buffer+ibuffer,"\0a",2); ibuffer+=2;
memcpy(buffer+ibuffer,from,from_length);
OgUniStrlwr(from_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=from_length; sep=ibuffer;
memcpy(buffer+ibuffer,"\0\1",2); ibuffer+=2;
memcpy(buffer+ibuffer,"\0a",2); ibuffer+=2;
memcpy(buffer+ibuffer,to,to_length);
OgUniStrlwr(to_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=to_length;

/** afrom:ato **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,sep+2,buffer));
if (retour==2) return(1);

/** :ato **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,ibuffer-sep,buffer+sep));
if (retour==2) return(1);

/** afrom:ato **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,ibuffer,buffer));
if (retour==2) return(1);

ibuffer=0;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,from,from_length);
OgUniStrlwr(from_length,buffer+ibuffer,buffer+ibuffer);
OgUniStruna(from_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=from_length; sep=ibuffer;
memcpy(buffer+ibuffer,"\0\1",2); ibuffer+=2;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,to,to_length);
OgUniStrlwr(to_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=to_length;

/** nfrom:ato **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,sep+2,buffer));
if (retour==2) return(1);

ibuffer=0;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,from,from_length);
OgUniStrlwr(from_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=from_length; sep=ibuffer;
memcpy(buffer+ibuffer,"\0\1",2); ibuffer+=2;
memcpy(buffer+ibuffer,"\0n",2); ibuffer+=2;
memcpy(buffer+ibuffer,to,to_length);
OgUniStrlwr(to_length,buffer+ibuffer,buffer+ibuffer);
OgUniStruna(to_length,buffer+ibuffer,buffer+ibuffer);
ibuffer+=to_length;

/** afrom:nto **/
IFE(retour=OgAutTry(ctrl_ltras->ha_false,sep+2,buffer));
if (retour==2) return(1);

return(0);
}

