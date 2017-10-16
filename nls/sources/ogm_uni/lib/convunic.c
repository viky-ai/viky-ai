/*
 *	This function handles Unicode encoding.
 *	Copyright (c) 1997-2004 Pertimm by Patrick Constant
 *	Dev : September,October 1998, August 2004
 *	Version 1.3
*/
#include <logpath.h>
#include <loguni.h>


#define DOgCodePageAnsi1252	 1252
#define DOgCodePageAscii850	 850
#define DOgCodePageAnsi1250 1250


/*
 *	This function just handles Unicode encoding from ANSI and ASCII
 *	codepage, the two codepages that are currently recognized by 
 *	the OgGetLang function (along with the language).
*/

PUBLIC(int) OgCodePageToUnicode(struct og_lang  *lang
   , unsigned char *BuffIn, unsigned long SizeBuffIn
   , unsigned char *BuffOut, unsigned long SizeBuffOut
   , unsigned long *LengthBuffOut, int *position, char *where)
{
//char chaincp[1024];
char erreur[DPcSzErr];
//static unsigned short	CodePageBuff[256];
//static int cpval=(-1);
//unsigned i=0,j=0;
//int code_page=0;
//FILE *cpFd=0;

int ShortByte0,ShortByte1;
if (OgBigEndian()) { ShortByte0=1; ShortByte1=0; }
else { ShortByte0=0; ShortByte1=1; }


if(SizeBuffOut<2*SizeBuffIn) {
  DPcSprintf(erreur, "OgCodePageToUnicode: SizeBuffOut (%ld)<2*SizeBuffIn (%ld)"
            , SizeBuffOut, 2*SizeBuffIn);	
  PcErr(-1,erreur); DPcErr;
  }

if(lang->codepage == DOgCodePageJaJIS) {
  IFE(OgJisToUnicode(BuffIn,SizeBuffIn,BuffOut,SizeBuffOut,LengthBuffOut,position));
  return(0);
  }
else if(lang->codepage == DOgCodePageJaSJIS) {
  IFE(OgSjisToUnicode(BuffIn,SizeBuffIn,BuffOut,SizeBuffOut,LengthBuffOut,position));
  return(0);
  }
else if(lang->codepage == DOgCodePageJaEUC) {
  IFE(OgEucToUnicode(BuffIn,SizeBuffIn,BuffOut,SizeBuffOut,LengthBuffOut,position));
  return(0);
  }

IFE(OgCpToUni(SizeBuffIn,BuffIn,SizeBuffOut,LengthBuffOut,BuffOut,lang->codepage,position,0));

DONE;
}



/* 
 * The rules for putting the text into Unicode with SJIS is crazy:
 * SJIS is encoded the following way (see http://web.lfw.org/text/jp.html):
 * first byte from 0x81 to 0x9f or from 0xe0 to 0xef
 * second byte from 0x40 to 0x7e, or from 0x80 to 0xfc
 * exclusion first byte 0xef second byte from 0x9f to 0xfc
 * We then assume that all other characters are coded in only one byte.
*/

PUBLIC(int) OgSjisToUnicode(unsigned char *BuffIn,unsigned long SizeBuffIn
                           ,unsigned char *BuffOut,unsigned long SizeBuffOut
                           ,unsigned long *LengthBuffOut, int *position)
{
int i,j,c,oldc=0,state=1,japanese=0;
int ShortByte0,ShortByte1;
unsigned short sh,jash;

IFE(OgJaInit());

if (OgBigEndian()) { ShortByte0=1; ShortByte1=0; }
else { ShortByte0=0; ShortByte1=1; }

for(i=j=0; i<(int)SizeBuffIn; i++) {
  c = BuffIn[i];
  switch(state) {
    case 1:
      if ((0x81<=c && c<=0x9f) || (0xe0<=c && c<=0xef)) state=2;
      else {
        if (position) position[j>>1]=i;  
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=c;
        j+=2;
        }
      break;
    case 2:
      sh = (oldc<<8) + c;
      jash=OgJaSjisToUnicode(sh);
      if (jash!=0) japanese=1;
      if (japanese) {
        if (position) position[j>>1]=i-1;  
        BuffOut[j+ShortByte1]=(jash>>8);
        BuffOut[j+ShortByte0]=(jash&0xff);
        j+=2;
        }
      else {
        if (position) position[j>>1]=i;
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=oldc;
        j+=2;
        if (position) position[j>>1]=i;
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=c;
        j+=2;
        }
      state=1;
      break;
    }
  oldc = c;
  }
if (position) position[j>>1]=i;
*LengthBuffOut=j;
DONE;
}




PUBLIC(int) OgJisToUnicode(unsigned char *BuffIn,unsigned long SizeBuffIn
                           ,unsigned char *BuffOut,unsigned long SizeBuffOut
                           ,unsigned long *LengthBuffOut, int *position)
{
int i,j,c,oldc=0,state=1,japanese=0;
int ShortByte0,ShortByte1;
unsigned short sh,jash;

IFE(OgJaInit());

if (OgBigEndian()) { ShortByte0=1; ShortByte1=0; }
else { ShortByte0=0; ShortByte1=1; }

for(i=j=0; i<(int)SizeBuffIn; i++) {
  c = BuffIn[i];
  switch(state) {
    case 1:
      if (0x21<=c && c<=0x7E) state=2;
      else {
        if (position) position[j>>1]=i;  
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=c;
        j+=2;
        }
      break;
    case 2:
      sh = (oldc<<8) + c;
      jash=OgJaJisToUnicode(sh);
      if (jash!=0) japanese=1;
      if (japanese) {
        if (position) position[j>>1]=i-1;  
        BuffOut[j+ShortByte1]=(jash>>8);
        BuffOut[j+ShortByte0]=(jash&0xff);
        j+=2;
        }
      else {
        if (position) position[j>>1]=i;
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=oldc;
        j+=2;
        if (position) position[j>>1]=i;
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=c;
        j+=2;
        }
      state=1;
      break;
    }
  oldc = c;
  }
if (position) position[j>>1]=i;
*LengthBuffOut=j;
DONE;
}






PUBLIC(int) OgEucToUnicode(unsigned char *BuffIn,unsigned long SizeBuffIn
                          ,unsigned char *BuffOut,unsigned long SizeBuffOut
                          ,unsigned long *LengthBuffOut, int *position)
{
int i,j,c,oldc=0,state=1,japanese=0;
int ShortByte0,ShortByte1;
unsigned short sh,jash;

IFE(OgJaInit());

if (OgBigEndian()) { ShortByte0=1; ShortByte1=0; }
else { ShortByte0=0; ShortByte1=1; }

for(i=j=0; i<(int)SizeBuffIn; i++) {
  c = BuffIn[i];
  switch(state) {
    case 1:
      if (0xA1<=c && c<=0xFE) state=2;
      else {
        if (position) position[j>>1]=i;  
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=c;
        j+=2;
        }
      break;
    case 2:
      sh = (oldc<<8) + c;
      jash=OgJaEucToUnicode(sh);
      if (jash!=0) japanese=1;
      if (japanese) {
        if (position) position[j>>1]=i-1;  
        BuffOut[j+ShortByte1]=(jash>>8);
        BuffOut[j+ShortByte0]=(jash&0xff);
        j+=2;
        }
      else {
        if (position) position[j>>1]=i;
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=oldc;
        j+=2;
        if (position) position[j>>1]=i;
        BuffOut[j+ShortByte1]=0;
        BuffOut[j+ShortByte0]=c;
        j+=2;
        }
      state=1;
      break;
    }
  oldc = c;
  }
if (position) position[j>>1]=i;
*LengthBuffOut=j;
DONE;
}


