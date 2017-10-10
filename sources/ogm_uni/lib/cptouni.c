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

PUBLIC(int) OgCpToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
int codepage, int *position, int *truncated)
{
char sys_erreur[DOgErrorSize],erreur[DOgErrorSize],*iconv_cp=0;
int i,j,u;
int (*func)(pr(int));

if (truncated) *truncated=0;

if(codepage==DOgCodePageUnicode) {
  i=icode;
  if(i>sunicode) i = sunicode;
  memcpy(unicode,code,i);
  *iunicode = i;
  DONE;
  }

if (codepage==DOgCodePageUTF8) {
  return(OgUTF8ToUni(icode,code,sunicode,iunicode,unicode,position,truncated));
  }
else if (codepage==DOgCodePageBig5) {
  return(OgBig5ToUni(icode,code,sunicode,iunicode,unicode,position,truncated));
  }
else if (codepage==DOgCodePageGb2312 || codepage==DOgCodePageGbk) {
  return(OgCp936ToUni(icode,code,sunicode,iunicode,unicode,position,truncated));
  }

switch(codepage) {
  case DOgCodePageANSI:   func=OgCp1252ToUni; break;
  case DOgCodePageASCII:  func=OgCp850ToUni; break;
  case DOgCodePage1250:   func=OgCp1250ToUni; break;
  case DOgCodePage8859_2: func=OgCp8859_2ToUni; break;
  case DOgCodePage8859_5: func=OgCp8859_5ToUni; break;
  case DOgCodePage1256:   func=OgCp1256ToUni; break;
  case DOgCodePage1251:   func=OgCp1251ToUni; break;
  case DOgCodePage866:   func=OgCp866ToUni; break;
  //case DOgCodePageBig5: func=0; iconv_cp="BIG5"; break;
  //case DOgCodePageGb2312: func=0; iconv_cp="gb2312"; break;
  //case DOgCodePageGbk: func=0; iconv_cp="gbk"; break;
  default: func=OgCp1252ToUni;
  }

if(iconv_cp) {
  iconv_t hiconv = iconv_open("unicodebig", iconv_cp);
  size_t iout = sunicode;
  size_t icode_iconv = icode;
  size_t retour = iconv(hiconv, &code, &icode_iconv, &unicode, &iout);
  if (retour==(size_t)-1) {
    iconv_close(hiconv);
    strcpy(sys_erreur,strerror(errno));
    sprintf(erreur,"OgCpToUni: iconv error (%d): %s",errno,sys_erreur);
    PcErr(-1,erreur); DPcErr;
    }
  iconv_close(hiconv);
  *iunicode=sunicode-iout;

  for (i=j=0; i<icode; i++) {
    if (position) position[(j++)>>1]=i;
    if (j+1>=sunicode) {
      if (truncated) *truncated=1;
      break;
      }
    }
  if (position) position[(j++)>>1]=i;

  }
else {
  for (i=j=0; i<icode; i++) {
    u = (*func)(code[i]);
    if (position) position[j>>1]=i;
    if (j+1>=sunicode) {
      if (truncated) *truncated=1;
      break;
      }
    unicode[j++]=(u>>8); unicode[j++]=(u&0xff);
    }
  if (position) position[j>>1]=i;
  *iunicode=j;
  }

DONE;
}






PUBLIC(int) OgCpToUniValid(codepage)
int codepage;
{
switch(codepage) {
  case DOgCodePageANSI:
  case DOgCodePageASCII:
  case DOgCodePageUTF8:
  case DOgCodePage1250:
  case DOgCodePage8859_2:
  case DOgCodePage8859_5:
  case DOgCodePage866:
    return(1);
  }
return(0);
}


