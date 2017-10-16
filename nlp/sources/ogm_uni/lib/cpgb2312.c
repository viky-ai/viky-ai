/*
 * Handling code page Gb2312 (chinese simplified only).
 * Copyright (c) 2009 Pertimm by Patrick Constant
 * Dev : November 2009
 * Version 1.0
*/
#include "cpgb2312.h"


static int  Gb2312ToUnicodeInitialized=0;
static unsigned Gb2312ToUnicode[0x10000]; 

static int  UnicodeToGb2312Initialized=0;
static unsigned UnicodeToGb2312[0x10000];

static int OgCharGb2312ToUni(int c, int *plength, int *valid);
static int OgCharUniToGb2312(int u, int *plength, int *valid);




PUBLIC(int) OgUniCjkGb2312(int c)
{
int length,valid;
OgCharUniToGb2312(c,&length,&valid);
if (valid && length==2) return(1);
return(0);
}




PUBLIC(int) OgGb2312CjkUni(int c)
{
int length,valid;
OgCharGb2312ToUni(c,&length,&valid);
if (valid && length==2) return(1);
return(0);
}




PUBLIC(int) OgGb2312ToUni(icode,code,sunicode,iunicode,unicode,position,truncated)
int icode; unsigned char *code;
int sunicode,*iunicode; unsigned char *unicode;
int *position,*truncated;
{
int i,j,c1,c2,u1,u2,length,valid;

for (i=0,j=0; i<icode; i++) {
  c1=code[i]; c2=(code[i]<<8)+code[i+1];
  u2=OgCharGb2312ToUni(c2,&length,&valid);
  if (length==2) {
    if (position) position[j>>1]=i; 
    if (j+1>=sunicode) {
      if (truncated) *truncated=1;
      goto endOgGb2312ToUni;
      }
    unicode[j++]=(u2>>8); unicode[j++]=(u2&0xff);
    i++; /* because length == 2 */
    }
  else {  
    u1=OgCharGb2312ToUni(c1,&length,&valid);
    /** We keep only valid chars **/
    if (valid) {
      if (position) position[j>>1]=i; 
      if (j+1>=sunicode) {
        if (truncated) *truncated=1;
        goto endOgGb2312ToUni;
        }
      unicode[j++]=(u1>>8); unicode[j++]=(u1&0xff);       
      }
    }
  
  }
endOgGb2312ToUni:
if (position) position[j>>1]=i;
*iunicode=j;
DONE;
}





static int OgCharGb2312ToUni(int c, int *plength, int *valid)
{
int u,i;
*valid=0;
if (!Gb2312ToUnicodeInitialized) {
  memset(Gb2312ToUnicode,0,0x10000*sizeof(unsigned));
  for (i=0; Gb2312Unicode[i].code; i++) {
    Gb2312ToUnicode[Gb2312Unicode[i].code] = Gb2312Unicode[i].unicode;
    }
  Gb2312ToUnicodeInitialized=1;
  }
if (c >=  0x10000) {
  *valid=0;
  *plength=3;
  return(c);
  }
u = Gb2312ToUnicode[c];
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





PUBLIC(int) OgUniToGb2312(iunicode,unicode,scode,icode,code,position,truncated)
int iunicode; unsigned char *unicode;
int scode,*icode; unsigned char *code;
int *position,*truncated;
{
int i,j,u,c,length,valid;

for (i=0,j=0; i<iunicode; i+=2) {
  u=(unicode[i]<<8)+unicode[i+1];
  c=OgCharUniToGb2312(u,&length,&valid);
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





static int OgCharUniToGb2312(int u, int *plength, int *valid)
{
int c,i;
*valid=0;
if (!UnicodeToGb2312Initialized) {
  memset(UnicodeToGb2312,0,0x10000*sizeof(unsigned));
  for (i=0; Gb2312Unicode[i].code; i++) {
    UnicodeToGb2312[Gb2312Unicode[i].unicode] = Gb2312Unicode[i].code;
    }
  UnicodeToGb2312Initialized=1;
  }
if (u >= 0x10000) {
  *valid=0;
  *plength=3;
  return(u);
  }
c = UnicodeToGb2312[u];
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



