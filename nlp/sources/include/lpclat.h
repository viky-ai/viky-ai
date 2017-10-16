/*
 *	Fichier comparable \`a ctype.h pour le code ISO-Latin-1
 *	On ne rencontre malheuresement pas de fichier standard
 *	de ce type, ni sous Unix, ni sous Windows.
 *	Copyright (c) 1997	Patrick Constant
 *	Dev : F\'evrier,Novembre 1997
 *	From systal.dll
 *	Version 1.6
*/

#ifndef _LPCLATALIVE_

#include <lpcosys.h>


#define	DPc_U	0x01	/* upper case letter */
#define	DPc_L	0x02	/* lower case letter */
#define	DPc_N	0x04	/* digit */
#define	DPc_S	0x08	/* space, tab, newline, vertical tab, formfeed, or
				carriage return */
#define DPc_P	0x10	/* punctuation character */
#define DPc_C	0x20	/* control character or delete */
#define DPc_X	0x40	/* hexadecimal digit [0-9a-fA-F]*/
#define DPc_B	0x80	/* unused */
#define DPc_A	0x100	/* accent */


struct pc_latin_1 {
  int ctype; unsigned char upper,lower,unacc;
  };

extern struct pc_latin_1 PcLatin1[];

#define	DPcIsalpha(c)	(PcLatin1[(unsigned char)(c)].ctype&(DPc_U|DPc_L))
#define	DPcIsupper(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_U)
#define	DPcIslower(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_L)
#define	DPcIsdigit(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_N)
#define	DPcIsxdigit(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_X)
#define	DPcIsspace(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_S)
#define DPcIspunct(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_P)
#define DPcIsalnum(c)	(PcLatin1[(unsigned char)(c)].ctype&(DPc_U|DPc_L|DPc_N))
#define DPcIsprint(c)	(PcLatin1[(unsigned char)(c)].ctype &(DPc_P|DPc_U|DPc_L|DPc_N|DPc_S))
#define DPcIsgraph(c)	(PcLatin1[(unsigned char)(c)].ctype &(DPc_P|DPc_U|DPc_L|DPc_N))
#define DPcIscntrl(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_C)
#define DPcIsascii(c)	((unsigned)(c)<=0x7f)
#define DPcIsaccent(c)	(PcLatin1[(unsigned char)(c)].ctype&DPc_A)
#define DPcToupper(c)	(PcLatin1[(unsigned char)(c)].upper)
#define DPcTolower(c)	(PcLatin1[(unsigned char)(c)].lower)
#define DPcUnaccent(c)	(PcLatin1[(unsigned char)(c)].unacc)

DEFPUBLIC(int) PcIsalpha(pr(int));
DEFPUBLIC(int) PcIsupper(pr(int));
DEFPUBLIC(int) PcIslower(pr(int));
DEFPUBLIC(int) PcIsdigit(pr(int));
DEFPUBLIC(int) PcIsxdigit(pr(int));
DEFPUBLIC(int) PcIsspace(pr(int));
DEFPUBLIC(int) PcIspunct(pr(int));
DEFPUBLIC(int) PcIsalnum(pr(int));
DEFPUBLIC(int) PcIsprint(pr(int));
DEFPUBLIC(int) PcIsgraph(pr(int));
DEFPUBLIC(int) PcIscntrl(pr(int));
DEFPUBLIC(int) PcIsascii(pr(int));
DEFPUBLIC(int) PcIsaccent(pr(int));
DEFPUBLIC(int) PcToupper(pr(int));
DEFPUBLIC(int) PcTolower(pr(int));
DEFPUBLIC(int) PcUnaccent(pr(int));
DEFPUBLIC(char *) PcStrlwr(pr(char *));
DEFPUBLIC(char *) PcStrupr(pr(char *));

DEFPUBLIC(int) OgUtoA(pr_(unsigned char *) pr_(unsigned char *)
					  pr(int));


#define _LPCLATALIVE_
#endif

