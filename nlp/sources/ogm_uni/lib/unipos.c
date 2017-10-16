/*
 *	Reversing a position vector.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : December 2007
 *	Version 1.0
*/
#include <loguni.h>




PUBLIC(int) OgUniPositionReverse(iin,in,iout,out)
int iin,*in,iout,*out;
{
int i,value,last_value=0;

for (i=0; i<iout; i++) {
  out[i]=(-1);
  }

for (i=0; i<iin; i++) {
  value=in[i];
  if (out[value]<0) out[value]=i;
  }

last_value=0;
for (i=0; i<iout; i++) {
  if (out[i]==(-1)) out[i]=last_value;
  else last_value=out[i];
  }

DONE;
}


