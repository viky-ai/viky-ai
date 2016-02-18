/*
 * Handling code page cp936 (chinese simplified and traditional).
 * Copyright (c) 2009 Pertimm by Patrick Constant
 * Dev : November 2009
 * Version 1.0
*/
#include "cp936.h"


static int  Cp936ToUnicodeInitialized=0;
static unsigned Cp936ToUnicode[0x10000];

static int  UnicodeToCp936Initialized=0;
static unsigned UnicodeToCp936[0x10000];

static int OgCharCp936ToUni(int c, int *plength, int *valid);
static int OgCharUniToCp936(int u, int *plength, int *valid);




PUBLIC(int) OgUniCjkCp936(int c)
{
int length,valid;
OgCharUniToCp936(c,&length,&valid);
if (valid && length==2) return(1);
return(0);
}




PUBLIC(int) OgCp936CjkUni(int c)
{
int length,valid;
OgCharCp936ToUni(c,&length,&valid);
if (valid && length==2) return(1);
return(0);
}




PUBLIC(int) OgCp936ToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int *position, int *truncated)
{
int i,j,c1,c2,u1,u2,length,valid;

for (i=0,j=0; i<icode; i++) {
  c1=code[i]; c2=(code[i]<<8)+code[i+1];
  u2=OgCharCp936ToUni(c2,&length,&valid);
  if (length==2) {
    if (position) position[j>>1]=i;
    if (j+1>=sunicode) {
      if (truncated) *truncated=1;
      goto endOgCp936ToUni;
      }
    unicode[j++]=(u2>>8); unicode[j++]=(u2&0xff);
    i++; /* because length == 2 */
    }
  else {
    u1=OgCharCp936ToUni(c1,&length,&valid);
    /** We keep only valid chars **/
    if (valid) {
      if (position) position[j>>1]=i;
      if (j+1>=sunicode) {
        if (truncated) *truncated=1;
        goto endOgCp936ToUni;
        }
      unicode[j++]=(u1>>8); unicode[j++]=(u1&0xff);
      }
    }

  }
endOgCp936ToUni:
if (position) position[j>>1]=i;
*iunicode=j;
DONE;
}





static int OgCharCp936ToUni(int c, int *plength, int *valid)
{
int u,i;
*valid=0;
if (!Cp936ToUnicodeInitialized) {
  memset(Cp936ToUnicode,0,0x10000*sizeof(unsigned));
  for (i=0; Cp936Unicode[i].code; i++) {
    Cp936ToUnicode[Cp936Unicode[i].code] = Cp936Unicode[i].unicode;
    }
  Cp936ToUnicodeInitialized=1;
  }
if (c >=  0x10000) {
  *valid=0;
  *plength=3;
  return(c);
  }
u = Cp936ToUnicode[c];
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





PUBLIC(int) OgUniToCp936(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code, int *position, int *truncated)
{
int i,j,u,c,length,valid;

for (i=0,j=0; i<iunicode; i+=2) {
  u=(unicode[i]<<8)+unicode[i+1];
  c=OgCharUniToCp936(u,&length,&valid);
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





static int OgCharUniToCp936(int u, int *plength, int *valid)
{
int c,i;
*valid=0;
if (!UnicodeToCp936Initialized) {
  memset(UnicodeToCp936,0,0x10000*sizeof(unsigned));
  for (i=0; Cp936Unicode[i].code; i++) {
    UnicodeToCp936[Cp936Unicode[i].unicode] = Cp936Unicode[i].code;
    }
  UnicodeToCp936Initialized=1;
  }
if (u >= 0x10000) {
  *valid=0;
  *plength=3;
  return(u);
  }
c = UnicodeToCp936[u];
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



