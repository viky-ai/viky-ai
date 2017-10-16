/*
 *	D\'efinitions des codes des langues (potentiellement) reconnues par Pertimm
 *	Copyright (c) 1992-2004 Pertimm by Patrick Constant
 *	Dev :	July, August 1992, May 1993, September 1998, August 2004, October 2007
 *	Version 1.4
*/

#ifndef _LOGLANGALIVE_

#include <logis639.h>


#define DOgCodePageStd          0	
#define DOgCodePageANSI         1	/* Windows characters (1252) (very near from ISO_Latin_1) */
#define DOgCodePageASCII        2	/* Extended dos characters (850) */
#define DOgCodePageRuKOI        3	/* Russian KOI */
#define DOgCodePageRuISO        4	/* Russion ISO 8859_5 */
#define DOgCodePage8859_5       4	/* Russion ISO 8859_5 */
#define DOgCodePageJaJIS        5	/* Japanese JIS */
#define DOgCodePageJaSJIS       6	/* Japanese SJIS */
#define DOgCodePageJaEUC        7	/* Japanese EUC */
#define DOgCodePage1250         8	/* Windows characters (1250) (very near from ISO_Latin_2) */
#define DOgCodePageUTF8         9	/* UTF-8 */
#define DOgCodePageUTF16       10	/* UTF-16 */
#define DOgCodePage8859_2      11	/* ISO_Latin_2 */
#define DOgCodePage8859_1      12	/* ISO_Latin_1 */
#define DOgCodePageUnicode     13	/* Unicode */
#define DOgCodePage1256        14	/* Arabic Windows characters (1256) */
#define DOgCodePage1251        15	/* Windows characters (1251) Cyrillic */
#define DOgCodePage866         16	/* Extended dos characters (866) */
#define DOgCodePageBig5        17	/* Chinese BIG5 */
#define DOgCodePageGb2312      18	/* Chinese GB2312 */
#define DOgCodePageGbk         19	/* Chinese GBK */
#define DOgCodePageMax         19


struct og_lang {
  int language;
  int codepage;
  int percent_non_printable;
  int percent_255;
  int percent_0;
  int flag;
};


DEFPUBLIC(char *) OgCodepageToStr(pr(int));
DEFPUBLIC(int) OgStrToCodepage(pr(char *));

#define _LOGLANGALIVE_

#endif


