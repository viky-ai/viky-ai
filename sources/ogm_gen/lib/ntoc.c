/*
 *	Conversion of OGMI0S numbers to C numbers (32 bits)
 *	Copyright (c) 1997,98,99 Ogmios by Patrick Constant
 *	Dev : August 1997, September 1998, August 1999
 *	Version 1.2
*/
#include <lpcerr.h>
#include <loggen.h>


PUBLIC(long) OgNtoC(unsigned char *var, unsigned char **retvar)
{
long retour;
long l,k,m,t;

if(*var < 0x80)
	retour = (long)*var++;
else if(*var < 0xC0) {
	l = 0x100 * (long)(*var++ - 0x80) ;
	retour = l + ((long)*var++);
	}
else if(*var < 0xE0) {
	l = 0x10000 * (long)(*var++ - 0xC0) ;
	k = (long)(0x100 * *var++) ;
	m = ((long)*var++);
	retour = l + k + m;
	}	
else if(*var < 0xF0) {
	l = 0x1000000 * (long)(*var++ - 0xE0) ;
	t = (long)(0x10000 * *var++);
	k = (long)(0x100 * *var++) ;
	m = ((long)*var++);
	retour = l +	t + k + m;
	}
else if(*var < 0xF1) {
	var++;
	l = (long)(0x1000000 * *var++);
	t = (long)(0x10000 * *var++);
	k = (long)(0x100 * *var++) ;
	m = ((long)*var++);
	retour = l + t + k + m;
    if (retour<0) {
      char erreur[DPcSzErr]; *retvar=0;
	  DPcSprintf(erreur,"OgNtoC : negative OgNumber %ld for 32 bits C numbers",retour);
      PcErr(-1,erreur); return(-1);
      }
	}
else {
	*retvar=0;
    PcErr(-1,"OgNtoC : OgNumber too big for 32 bits C numbers"); return(-1);
    }
*retvar = var;
return(retour);
}

