/*
 * Handling code page big5 (chinese traditional only).
 * Copyright (c) 2009 Pertimm by Patrick Constant
 * Dev : November 2009
 * Version 1.0
*/
#include "cpbig5.h"


static int  Big5ToUnicodeInitialized=0;
static unsigned Big5ToUnicode[0x10000];

static int  UnicodeToBig5Initialized=0;
static unsigned UnicodeToBig5[0x10000];

static int OgCharBig5ToUni(int c, int *plength, int *valid);
static int OgCharUniToBig5(int u, int *plength, int *valid);




PUBLIC(int) OgUniCjkBig5(int c)
{
int length,valid;
OgCharUniToBig5(c,&length,&valid);
if (valid && length==2) return(1);
return(0);
}



PUBLIC(int) OgBig5CjkUni(int c)
{
int length,valid;
OgCharBig5ToUni(c,&length,&valid);
if (valid && length==2) return(1);
return(0);
}




PUBLIC(int) OgBig5ToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int *position, int *truncated)
{
int i,j,c1,c2,u1,u2,length,valid;

for (i=0,j=0; i<icode; i++) {
  c1=code[i]; c2=(code[i]<<8)+code[i+1];
  u2=OgCharBig5ToUni(c2,&length,&valid);
  if (length==2) {
    if (position) position[j>>1]=i;
    if (j+1>=sunicode) {
      if (truncated) *truncated=1;
      goto endOgBig5ToUni;
      }
    unicode[j++]=(u2>>8); unicode[j++]=(u2&0xff);
    i++; /* because length == 2 */
    }
  else {
    u1=OgCharBig5ToUni(c1,&length,&valid);
    /** We keep only valid chars **/
    if (valid) {
      if (position) position[j>>1]=i;
      if (j+1>=sunicode) {
        if (truncated) *truncated=1;
        goto endOgBig5ToUni;
        }
      unicode[j++]=(u1>>8); unicode[j++]=(u1&0xff);
      }
    }

  }
endOgBig5ToUni:
if (position) position[j>>1]=i;
*iunicode=j;
DONE;
}





static int OgCharBig5ToUni(int c, int *plength, int *valid)
{
int u,i;
*valid=0;
if (!Big5ToUnicodeInitialized) {
  memset(Big5ToUnicode,0,0x10000*sizeof(unsigned));
  for (i=0; Big5Unicode[i].code; i++) {
    Big5ToUnicode[Big5Unicode[i].code] = Big5Unicode[i].unicode;
    }
  Big5ToUnicodeInitialized=1;
  }
if (c >=  0x10000) {
  *valid=0;
  *plength=3;
  return(c);
  }
u = Big5ToUnicode[c];
IFn(u) {
  if (0x0 <= c && c <= 0x7f) *valid=1;
  else *valid=0;
  *plength=1;
  return(c);
  }
*valid=1;
*plength=2;
return(u);
}





PUBLIC(int) OgUniToBig5(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code,
    int *position, int *truncated)
{
int i,j,u,c,length,valid;

for (i=0,j=0; i<iunicode; i+=2) {
  u=(unicode[i]<<8)+unicode[i+1];
  c=OgCharUniToBig5(u,&length,&valid);
  if (valid) {
    if (length==1) {
      if (position) position[j]=i>>1;
      code[j++]=(unsigned char)c;
      }
    else if (length==2) {
      if (position) position[j]=i>>1;
      code[j++]=(unsigned char)(c>>8);
      if (position) position[j]=i>>1;
      code[j++]=(unsigned char)(c&0xff);
      }
    }
  }
code[j]=0;
if (position) position[j]=i>>1;
if (icode) *icode=j;
DONE;
}





static int OgCharUniToBig5(int u, int *plength, int *valid)
{
int c,i;
*valid=0;
if (!UnicodeToBig5Initialized) {
  memset(UnicodeToBig5,0,0x10000*sizeof(unsigned));
  for (i=0; Big5Unicode[i].code; i++) {
    UnicodeToBig5[Big5Unicode[i].unicode] = Big5Unicode[i].code;
    }
  UnicodeToBig5Initialized=1;
  }
if (u >= 0x10000) {
  *valid=0;
  *plength=3;
  return(u);
  }
c = UnicodeToBig5[u];
IFn(c) {
  if (0x0 <= u && u <= 0x7f) *valid=1;
  else *valid=0;
  *plength=1;
  return(u);
  }
*valid=1;
*plength=2;
return(c);
}



