/*
 *  Handling all code pages.
 *  Copyright (c) 2004 Pertimm by Patrick Constant
 *  Dev : September 2004
 *  Version 1.0
*/
#include <loguni.h>


/*
 *  Changes a Unicode string to a string with given codepage.
 *  'unicode' is unicode encoded string of length 'iunicode'
 *  'code' is string where the coded string with 'codepage'
 *  is written. scode is maximun length of code.
 *  'code' is zero terminated and *truncated is 1 (0 otherwise)
 *  if the 'code' string does not contain all encoded data.
*/

PUBLIC(int) OgUniToCp(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code, int codepage,
    int*position, int *truncated)
{
int i,j,c,start,byte_zero,byte_one,retour=0,iout=0;
int (*func)(pr_(int) pr(int *));
char sys_erreur[DOgErrorSize];
char erreur[DOgErrorSize];
int iunicode_iconv;
char *iconv_cp=0;

if (truncated) *truncated=0;

if(codepage==DOgCodePageUnicode) {
  i=iunicode;
  if(i>scode) i = scode;
  memcpy(code,unicode,i);
  *icode = i;
  DONE;
  }

if (codepage==DOgCodePageUTF8) {
  return(OgUniToUTF8(iunicode,unicode,scode,icode,code,position,truncated));
  }
else if (codepage==DOgCodePageBig5) {
  return(OgUniToBig5(iunicode,unicode,scode,icode,code,position,truncated));
  }
else if (codepage==DOgCodePageGb2312 || codepage==DOgCodePageGbk) {
  return(OgUniToCp936(iunicode,unicode,scode,icode,code,position,truncated));
  }

switch(codepage) {
  case DOgCodePageANSI:   func=OgUniToCp1252; break;
  case DOgCodePageASCII:   func=OgUniToCp850; break;
  case DOgCodePage1250:   func=OgUniToCp1250; break;
  case DOgCodePage8859_2: func=OgUniToCp8859_2; break;
  case DOgCodePage8859_5: func=OgUniToCp8859_5; break;
  case DOgCodePage866: func=OgUniToCp866; break;
  //case DOgCodePageBig5: func=0; iconv_cp="BIG5"; break;
  //case DOgCodePageGb2312: func=0; iconv_cp="GB2312"; break;
  //case DOgCodePageGbk: func=0; iconv_cp="GBK"; break;
  default: func=OgUniToCp1252;
  }

if(iconv_cp) {
  iconv_t hiconv; int discard_ilseq=1;
  hiconv=iconv_open(iconv_cp,"unicodebig");
  /** We could also add "//IGNORE" to iconv_cp **/
  iconvctl(hiconv,ICONV_SET_DISCARD_ILSEQ,&discard_ilseq);
  iout=scode; iunicode_iconv=iunicode;
  retour=iconv(hiconv,&unicode,&iunicode_iconv,&code,&iout);
  if (retour==(size_t)-1) {
    strcpy(sys_erreur,strerror(errno));
    sprintf(erreur,"OgUniToCp: iconv error (%d): %s",errno,sys_erreur);
    PcErr(-1,erreur); DPcErr;
    }
  iconv_close(hiconv);
  *icode=scode-iout;

  for (i=0,j=0; i+1<iunicode; i+=2) {
    if (j>scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j++]=i>>1;
    }
  if (position) position[j++]=i>>1;
  }
else {
  if (unicode[0]==0xfe && unicode[1]==0xff) { start=2; byte_zero=0; byte_one=1; } /** big endian **/
  else if (unicode[0]==0xff && unicode[1]==0xfe) { start=2; byte_zero=1; byte_one=0; } /** small endian **/
  else { start=0; byte_zero=0; byte_one=1; } /** defaults to big endian **/

  for (i=start,j=0; i+1<iunicode; i+=2) {
    c = (unicode[i+byte_zero]<<8) + unicode[i+byte_one];
    if (j>scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i>>1;
    code[j++] = (*func)(c,0);
    }
  code[j]=0;
  if (position) position[j]=i>>1;
  if (icode) *icode=j;
  }

DONE;
}


