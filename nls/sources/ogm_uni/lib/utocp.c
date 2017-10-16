/*
 *	Fonction unicode to codepage, more general than OgUtoA function
 *	Copyright (c) 1997-2004 Pertimm par Patrick Constant
 *	Dev : Août 1997, December 2000, August 2004
 *	Version 1.2
*/
#include <loguni.h>


/*
 *	This function gets the unicode chaine 'u' whose length is 'length' 
 *  into something closer to given codepage. Default codepage is 1252.
*/
PUBLIC(int) OgUtoCp(u,a,length,codepage)
unsigned char *u, *a;
int length,codepage;
{
int k,ia=0,c,truncated;
int (*func)(pr_(int) pr(int *));

switch(codepage) {
  case DOgCodePageANSI:   func=OgUniToCp1252; break;
  case DOgCodePageASCII:   func=OgUniToCp850; break;
  case DOgCodePage1250:   func=OgUniToCp1250; break;
  case DOgCodePage8859_2: func=OgUniToCp8859_2; break;
  case DOgCodePage8859_5: func=OgUniToCp8859_5; break;
  case DOgCodePage1256:   func=OgUniToCp1256; break;
  case DOgCodePage1251:   func=OgUniToCp1251; break;
  case DOgCodePage866: func=OgUniToCp866; break;
  default: func=OgUniToCp1252;
  }

a[0]=0;
for (k=0; k<length; k++) {
  if (k%2==0) {	
    if (u[k]!=0) {
      int good=0;
      if (k+1<length) {
        c = (u[k]<<8) + u[k+1];
        a[ia] = (*func)(c,&truncated);
        if (!truncated) { ia++; a[ia]=0; good=1; }
        }
      if (good) k++;      
      else if (PcIsprint(u[k]) && u[k]>=32) {
        a[ia++]='['; a[ia++]=u[k];
        a[ia++]=']'; a[ia]=0;
        }
      else {
        sprintf(a+ia,"[\\%.2x]",u[k]);
        ia+=5;
        }
      }
    else if (k+1 == length) {
      sprintf(a+ia,"[\\00]");
      ia+=5;
      }
    }
  else {
    if (PcIsprint(u[k]) && u[k]>=32) {
      a[ia++]=u[k]; a[ia]=0;
      }
    else {
      sprintf(a+ia,"\\%.2x",u[k]);
      ia+=3;
      }
    }
  }
DONE;
}



