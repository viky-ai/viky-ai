/*
 *	Working on conversion between "old" codepage and "new" codepage
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : November 2007
 *	Version 1.0
*/
#include <loguni.h>





PUBLIC(int) OgOldCpToNew(oldcp)
int oldcp;
{
switch (oldcp) {
  case DOgCodePageANSI: return(DOgCharset_windows_1252);
  case DOgCodePageASCII: return(DOgCharset_cp850);
  case DOgCodePageRuKOI: return(DOgCharset_ECMA_cyrillic);
  case DOgCodePageJaJIS: return(DOgCharset_JIS_Encoding);
  case DOgCodePageJaSJIS: return(DOgCharset_Shift_JIS);
  case DOgCodePageJaEUC: return(DOgCharset_EUC_JP);
  case DOgCodePage1250: return(DOgCharset_windows_1250);  
  case DOgCodePageUTF8: return(DOgCharset_UTF_8);  
  case DOgCodePageUTF16:  return(DOgCharset_UTF_16);
  case DOgCodePage8859_2: return(DOgCharset_ISO_8859_2);
  case DOgCodePage8859_5: return(DOgCharset_ISO_8859_5);
  case DOgCodePage8859_1: return(DOgCharset_ISO_8859_1);
  case DOgCodePageUnicode: return(DOgCharset_csUnicode);
  case DOgCodePage1256: return(DOgCharset_windows_1256);   
  case DOgCodePage1251: return(DOgCharset_windows_1251);   
  case DOgCodePage866: return(DOgCharset_cp866);
  }
return(DOgCharset_windows_1252);
}





PUBLIC(int) OgNewCpToOld(newcp)
int newcp;
{
switch (newcp) {
  case DOgCharset_windows_1252: return(DOgCodePageANSI);
  case DOgCharset_cp850: return(DOgCodePageASCII);
  case DOgCharset_cp866: return(DOgCodePage866);
  case DOgCharset_ECMA_cyrillic: return(DOgCodePageRuKOI);
  case DOgCharset_JIS_Encoding: return(DOgCodePageJaJIS);
  case DOgCharset_Shift_JIS: return(DOgCodePageJaSJIS);
  case DOgCharset_EUC_JP: return(DOgCodePageJaEUC);
  case DOgCharset_windows_1250: return(DOgCodePage1250);  
  case DOgCharset_UTF_8: return(DOgCodePageUTF8);  
  case DOgCharset_UTF_16:  return(DOgCodePageUTF16);
  case DOgCharset_ISO_8859_5: return(DOgCodePage8859_5);
  case DOgCharset_ISO_8859_2: return(DOgCodePage8859_2);
  case DOgCharset_ISO_8859_1: return(DOgCodePage8859_1);
  case DOgCharset_csUnicode: return(DOgCodePageUnicode);
  case DOgCharset_windows_1256: return(DOgCodePage1256);   
  case DOgCharset_windows_1251: return(DOgCodePage1251);   
  }
return(DOgCodePageANSI);
}



