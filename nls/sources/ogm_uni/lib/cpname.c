/*
 *	Handling all code pages names, mostly used with human interfaces
 *	Copyright (c) 2004-2007 Pertimm by Patrick Constant
 *	Dev : September 2004, October 2007
 *	Version 1.0
*/
#include <loguni.h>



struct og_codepage {
  char *name;
  int value;
  };

/*  
 *  List of all codepages which are handled by the pair
 *  OgCpToUni/OgUniToCp.
*/
struct og_codepage OgCodepage[] = {
  { "uni", DOgCodePageUnicode },
  { "850", DOgCodePageASCII },
  { "866", DOgCodePage866 },
  { "1252", DOgCodePageANSI },
  { "1250", DOgCodePage1250 },
  { "1256", DOgCodePage1256 },
  { "1251", DOgCodePage1251 },
  { "utf-8", DOgCodePageUTF8 },
  { "utf8", DOgCodePageUTF8 },
  //{ "il1", DOgCodePage8859_1 },
  { "il2", DOgCodePage8859_2 },
  { "il5", DOgCodePage8859_5 },
  { "big5", DOgCodePageBig5 },
  { "gb2312", DOgCodePageGb2312 },
  { "gbk", DOgCodePageGbk },
  { "", 0 }
  };



/*
 *  Gets the codepage from the name, return zero if not found.
*/

PUBLIC(int) OgNameToCp(s)
char *s;
{
int i=0;
while(1) {
  if (!strcmp(s,OgCodepage[i].name)) return(OgCodepage[i].value);
  if (OgCodepage[i].name[0]==0) break;
  i++;
  }
return(0);
}



/*
 *  Gets the name from the codepage.
*/
PUBLIC(char *) OgCpToName(cp)
int cp;
{
int i=0;
while(1) {
  if (cp==OgCodepage[i].value) return(OgCodepage[i].name);
  if (OgCodepage[i].name[0]==0) break;
  i++;
  }
return("unknown");
}


