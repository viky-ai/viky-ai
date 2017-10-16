/*
 *	All generic functions for handling unicode characters
 *	Copyright (c) 2001 Pertimm by Patrick Constant
 *	Dev : August 2001
 *	Version 1.0
*/
#include <loggen.h>


/*
 *  Transforms Unicode endian into Unicode binary.
 *  The binary unicode is terminated by zero and its length
 *  is iue/2, but you should provide for one more short
 *  for the zero char.
*/

PUBLIC(int) OgUeToUb(int iue, unsigned char *ue, short unsigned int *ub)
{
int i,j;
for (i=j=0; i<iue; i+=2) {
  ub[j++] = (ue[i]<<8) + ue[i+1];
  }
ub[j]=0; 
DONE;
}



/*
 *  Transforms Unicode binary into Unicode endian.
 *  The endian unicode has a length that is iub*2.
*/

PUBLIC(int) OgUbToUe(int iub, short unsigned int *ub, unsigned char *ue)
{
int i,j;
for (i=j=0; i<iub; i++) {
  ue[j++] = (ub[i]>>8);
  ue[j++] = ub[i] & 0xff;
  }
DONE;
}




PUBLIC(int) OgIsspace(int c)
{
if (c<0x100) return(isspace(c));
else if (c==0x3000) return(1);
else return(0);
}




PUBLIC(int) OgCharToShort(int ischar, unsigned char *schar, short unsigned int *sshort)
{
int i;
for (i=0; i<ischar; i++) {
  sshort[i] = schar[i];
  }
DONE;
}