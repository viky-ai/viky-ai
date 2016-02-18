/*
 *	Fonction unicode to ascii.
 *	Copyright (c) 1997	Ogmios par Patrick Constant
 *	Dev : Août 1997, December 2000
 *	Version 1.1
*/
#include <loggen.h>


/*
 *	This function gets the unicode chaine 'u'
 *  whose length is 'length' into something closer to ascii
*/
PUBLIC(int) OgUtoA(unsigned char *u, unsigned char *a, int length)
{
int k,ia=0;
a[0]=0;
for (k=0; k<length; k++) {
  if (k%2==0) {	
    if (u[k]!=0) {
      /* Some unicode codes are translated in cp1252 for better Windows visualization 
       * 0x0152;LATIN CAPITAL LIGATURE OE;Lu;0;L;;;;;N;LATIN CAPITAL LETTER O E;;;0153;
       * 0x0153;LATIN SMALL LIGATURE OE;Ll;0;L;;;;;N;LATIN SMALL LETTER O E;;0152;;0152
       * 0x2026;HORIZONTAL ELLIPSIS;Po;0;ON;<compat> 002E 002E 002E;;;;N;;;;;
       * 0x2020;DAGGER;Po;0;ON;;;;;N;;;;;
       * 0x2021;DOUBLE DAGGER;Po;0;ON;;;;;N;;;;;
       * 0x2022;BULLET;Po;0;ON;;;;;N;;;;; 
       * 0x2122;TRADE MARK SIGN;So;0;ON;<super> 0054 004D;;;;N;TRADEMARK;;;; 
       * 20AC;EURO SIGN;Sc;0;ET;;;;;N;;;;;
      */
      if (k+1<length && u[k] == 0x1 && u[k+1] == 0x52) { k++; a[ia++]=140; a[ia]=0; }
      else if (k+1<length && u[k] == 0x1 && u[k+1] == 0x53) { k++; a[ia++]=156; a[ia]=0; }
      else if (k+1<length && u[k] == 0x20 && u[k+1] == 0x26) { k++; a[ia++]=133; a[ia]=0; }
      else if (k+1<length && u[k] == 0x20 && u[k+1] == 0x20) { k++; a[ia++]=134; a[ia]=0; }
      else if (k+1<length && u[k] == 0x20 && u[k+1] == 0x21) { k++; a[ia++]=135; a[ia]=0; }
      else if (k+1<length && u[k] == 0x20 && u[k+1] == 0x22) { k++; a[ia++]=149; a[ia]=0; }
      else if (k+1<length && u[k] == 0x20 && u[k+1] == 0xac) { k++; a[ia++]=128; a[ia]=0; }
      else if (k+1<length && u[k] == 0x21 && u[k+1] == 0x22) { k++; a[ia++]=153; a[ia]=0; }
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




/*
 *	Same as UtoA, but we garanty that we won't get over length_a
 *  And if the word is too long, we add ... at the end.
*/

PUBLIC(int) OgUtoAsafe(int length_u, unsigned char *u, int length_a, unsigned char *a)
{
int k,len;
a[0]=0;
for (k=len=0; k<length_u; k++) {
  if (k%2==0) {
	if (u[k]!=0)
      if (k+1<length_u && u[k] == 0x1 && u[k+1] == 0x52) { k++; sprintf(a+len,"%c",140); }
      else if (k+1<length_u && u[k] == 0x1 && u[k+1] == 0x53) { k++; sprintf(a+len,"%c",156); }
      else if (k+1<length_u && u[k] == 0x20 && u[k+1] == 0x26) { k++; sprintf(a+len,"%c",133); }
      else if (k+1<length_u && u[k] == 0x20 && u[k+1] == 0x20) { k++; sprintf(a+len,"%c",134); }
      else if (k+1<length_u && u[k] == 0x20 && u[k+1] == 0x21) { k++; sprintf(a+len,"%c",135); }
      else if (k+1<length_u && u[k] == 0x20 && u[k+1] == 0x22) { k++; sprintf(a+len,"%c",149); }
	  else if (PcIsprint(u[k]) && u[k]>=32) sprintf(a+len,"[%c]",u[k]);
  	  else sprintf(a+len,"[\\%.2x]",u[k]);
    else if (k+1 == length_u) sprintf(a+len,"[\\00]");
    }
  else {
	if (PcIsprint(u[k]) && u[k]>=32) sprintf(a+len,"%c",u[k]);
  	else sprintf(a+len,"\\%.2x",u[k]);
    }
  len = strlen(a);
  if (len+10 > length_a) { sprintf(a+len,"..."); break; }
  }
DONE;
}


