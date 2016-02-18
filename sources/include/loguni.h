/*
 *  Header for library ogm_uni.dll
 *  This library is dedicated to all unicode functions.
 *  Copyright (c) 2003-2007 Pertimm by Patrick Constant
 *  Dev: December 2003, March,August,September 2004, June,September,October 2005
 *  Dev: April,October,December 2007, September 2009
 *  Version 1.9
*/
#ifndef _LOGUNIALIVE_
#include <loggen.h>
#include <logthr.h>
#include <iconv/iconv.h>
#include <errno.h>

#define DOgUniBanner  "ogm_uni V1.38, Copyright (c) 2003-2012 Pertimm"
#define DOgUniVersion 138

#define  DOgUni_U  0x01  /* upper case letter */
#define  DOgUni_L  0x02  /* lower case letter */
#define  DOgUni_N  0x04  /* digit */
#define  DOgUni_S  0x08  /* space, tab, newline, vertical tab, formfeed, carriage return, no-break space, etc. */
#define DOgUni_P  0x10  /* punctuation character */
#define DOgUni_C  0x20  /* control character or delete */
#define DOgUni_X  0x40  /* hexadecimal digit [0-9a-fA-F]*/
#define DOgUni_B  0x80  /* unused */
#define DOgUni_A  0x100  /* accent */

#define DOgUniSize  0x0510 /* 1296 */

struct og_uni {
  int ctype; unsigned short upper,lower,unacc;
  };

extern struct og_uni OgUni[];

#define  DOgUniIsalpha(c)  (OgUni[(unsigned short)(c)].ctype&(DOgUni_U|DOgUni_L))
#define  DOgUniIsupper(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_U)
#define  DOgUniIslower(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_L)
#define  DOgUniIsdigit(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_N)
#define  DOgUniIsxdigit(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_X)
#define  DOgUniIsspace(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_S)
#define DOgUniIspunct(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_P)
#define DOgUniIsalnum(c)  (OgUni[(unsigned short)(c)].ctype&(DOgUni_U|DOgUni_L|DOgUni_N))
#define DOgUniIsprint(c)  (OgUni[(unsigned short)(c)].ctype &(DOgUni_P|DOgUni_U|DOgUni_L|DOgUni_N|DOgUni_S))
#define DOgUniIsgraph(c)  (OgUni[(unsigned short)(c)].ctype &(DOgUni_P|DOgUni_U|DOgUni_L|DOgUni_N))
#define DOgUniIscntrl(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_C)
#define DOgUniIsascii(c)  ((unsigned)(c)<=0x7f)
#define DOgUniIsaccent(c)  (OgUni[(unsigned short)(c)].ctype&DOgUni_A)
#define DOgUniToupper(c)  (OgUni[(unsigned short)(c)].upper)
#define DOgUniTolower(c)  (OgUni[(unsigned short)(c)].lower)
#define DOgUniUnaccent(c)  (OgUni[(unsigned short)(c)].unacc)


#define DOgGetcpTraceMinimal  0x1
#define DOgGetcpTraceMemory   0x2
#define DOgGetcpTraceNormal   0x4

/** If hcharset is null, we must instantiate one **/
struct og_getcp_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  void *hcharset;
  };


#define DOgUniCleanTraceMinimal  0x1
#define DOgUniCleanTraceMemory   0x2

/** If hcharset is null, we must instantiate one **/
struct og_uniclean_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  void *hcharset;
  void *hgetcp;
  };

#define DOgUniCleanFlagRemoveTags       0x1
#define DOgUniCleanFlagRemoveEntities   0x2
#define DOgUniCleanFlagRemoveSpaces     0x4

/** Default value for max_tag_length and max_entity_length **/
#define DOgUniCleanMaxTagLength      512
#define DOgUniCleanMaxEntityLength   256

struct og_uniclean_input {
  int control_flags;
  int codepage, default_codepage;
  int icode; unsigned char *code;
  int max_tag_length,max_entity_length;
  };

struct og_uniclean_output {
  int iunicode; unsigned char *unicode;
  int *position, *rposition;
  };


DEFPUBLIC(int) OgUniIsalpha(pr(int));
DEFPUBLIC(int) OgUniIsupper(pr(int));
DEFPUBLIC(int) OgUniIslower(pr(int));
DEFPUBLIC(int) OgUniIsdigit(pr(int));
DEFPUBLIC(int) OgUniIsxdigit(pr(int));
DEFPUBLIC(int) OgUniIsspace(pr(int));
DEFPUBLIC(int) OgUniIsspaceEx(pr(int));
DEFPUBLIC(int) OgUniIspunct(pr(int));
DEFPUBLIC(int) OgUniIsalnum(pr(int));
DEFPUBLIC(int) OgUniIsprint(pr(int));
DEFPUBLIC(int) OgUniIsgraph(pr(int));
DEFPUBLIC(int) OgUniIscntrl(pr(int));
DEFPUBLIC(int) OgUniIsascii(pr(int));
DEFPUBLIC(int) OgUniIsaccent(pr(int));
DEFPUBLIC(int) OgUniToupper(pr(int));
DEFPUBLIC(int) OgUniTolower(pr(int));
DEFPUBLIC(int) OgUniUnaccent(pr(int));
DEFPUBLIC(unsigned char *) OgUniStrlwr(pr_(int) pr_(unsigned char *) pr(unsigned char *));
DEFPUBLIC(unsigned char *) OgUniStrupr(pr_(int) pr_(unsigned char *) pr(unsigned char *));
DEFPUBLIC(unsigned char *) OgUniStruna(pr_(int) pr_(unsigned char *) pr(unsigned char *));
DEFPUBLIC(unsigned short *) OgUnsStrlwr(pr(unsigned short *));
DEFPUBLIC(unsigned short *) OgUnsStrupr(pr(unsigned short *));
DEFPUBLIC(unsigned short *) OgUnsStruna(pr(unsigned short *));

DEFPUBLIC(int) OgCodePageToUnicode(pr_(struct og_lang *) pr_(unsigned char *) pr_(unsigned long)
            pr_(unsigned char *) pr_(unsigned long) pr_(unsigned long *) pr_(int *) pr(char *));

DEFPUBLIC(int) OgUniToCp850(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp1252(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp1250(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp8859_2(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp8859_5(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp1256(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp1251(pr_(int) pr(int *));
DEFPUBLIC(int) OgUniToCp866(pr_(int) pr(int *));

DEFPUBLIC(int) OgCp850ToUni(pr(int));
DEFPUBLIC(int) OgCp1252ToUni(pr(int));
DEFPUBLIC(int) OgCp1250ToUni(pr(int));
DEFPUBLIC(int) OgCp8859_2ToUni(pr(int));
DEFPUBLIC(int) OgCp8859_5ToUni(pr(int));
DEFPUBLIC(int) OgCp1256ToUni(pr(int));
DEFPUBLIC(int) OgCp1251ToUni(pr(int));
DEFPUBLIC(int) OgCp866ToUni(pr(int));

DEFPUBLIC(int) OgCpToUniValid(pr(int));
DEFPUBLIC(int) OgCpToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int codepage, int *position, int *truncated);
DEFPUBLIC(int) OgUniToCp(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code, int codepage,
    int*position, int *truncated);
DEFPUBLIC(int) OgCpToCp(pr_(int) pr_(unsigned char *) pr_(int) pr_(int *)
                        pr_(unsigned char *) pr_(int) pr_(int) pr_(int *) pr(int *));
DEFPUBLIC(int) OgFilesCpToCp(pr_(unsigned char *) pr_(unsigned char *) pr_(int) pr(int));

DEFPUBLIC(int) OgUniToUTF8(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code, int *position,
    int *truncated);
DEFPUBLIC(int) OgUTF8ToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int *position, int *truncated);
DEFPUBLIC(int) OgCheckUTF8(int icode, const unsigned char *code, int *pnb_ascii_char, int *pnb_utf8_chars);

DEFPUBLIC(int) OgUniCjkIsPunct(int c);

DEFPUBLIC(int) OgUniCjkBig5(int c);
DEFPUBLIC(int) OgBig5CjkUni(int c);
DEFPUBLIC(int) OgUniToBig5(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code,
    int *position, int *truncated);
DEFPUBLIC(int) OgBig5ToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int *position, int *truncated);

DEFPUBLIC(int) OgUniCjkCp936(int c);
DEFPUBLIC(int) OgCp936CjkUni(int c);
DEFPUBLIC(int) OgUniToCp936(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code, int *position, int *truncated);
DEFPUBLIC(int) OgCp936ToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int *position, int *truncated);

DEFPUBLIC(int) OgUniCjkGb2312(int c);
DEFPUBLIC(int) OgGb2312CjkUni(int c);
DEFPUBLIC(int) OgUniToGb2312(pr_(int) pr_(unsigned char *) pr_(int) pr_(int *)
                             pr_(unsigned char *) pr_(int *) pr(int *));
DEFPUBLIC(int) OgGb2312ToUni(pr_(int) pr_(unsigned char *) pr_(int) pr_(int *)
                             pr_(unsigned char *) pr_(int *) pr(int *));

DEFPUBLIC(int) OgUniCjkKanjiHiragana(int c);

DEFPUBLIC(int) OgUtoCp(pr_(unsigned char *) pr_(unsigned char *) pr_(int) pr(int));

DEFPUBLIC(int) OgJaInit(pr(void));
DEFPUBLIC(int) OgJaFlush(pr(void));
DEFPUBLIC(int) OgJaSjisToUnicode(pr(unsigned short));
DEFPUBLIC(int) OgJaUnicodeToSjis(pr(unsigned short));
DEFPUBLIC(int) OgJaJisToUnicode(pr(unsigned short));
DEFPUBLIC(int) OgJaUnicodeToJis(pr(unsigned short));
DEFPUBLIC(int) OgJaEucToUnicode(pr(unsigned short));
DEFPUBLIC(int) OgJaUnicodeToEuc(pr(unsigned short));
DEFPUBLIC(int) OgSjisToUnicode(pr_(unsigned char *) pr_(unsigned long) pr_(unsigned char *)
                               pr_(unsigned long) pr_(unsigned long *) pr(int *));
DEFPUBLIC(int) OgJisToUnicode(pr_(unsigned char *) pr_(unsigned long) pr_(unsigned char *)
                              pr_(unsigned long) pr_(unsigned long *) pr(int *));
DEFPUBLIC(int) OgEucToUnicode(pr_(unsigned char *) pr_(unsigned long) pr_(unsigned char *)
                              pr_(unsigned long) pr_(unsigned long *) pr(int *));

DEFPUBLIC(int) OgNameToCp(pr(char *));
DEFPUBLIC(char *) OgCpToName(pr(int));

DEFPUBLIC(int) OgCpToPercent(pr_(int) pr_(unsigned char *) pr_(int) pr_(int *) pr_(unsigned char *) pr(int *));
DEFPUBLIC(int) OgPercentToCp(pr_(int) pr_(unsigned char *) pr_(int) pr_(int *) pr_(unsigned char *) pr(int *));

DEFPUBLIC(void *) OgGetCpInit(pr(struct og_getcp_param *));
DEFPUBLIC(int) OgGetCpFlush(pr(void *));
DEFPUBLIC(int) OgGetCp(pr_(void *) pr_(int) pr_(unsigned char *) pr(int *));

DEFPUBLIC(int) OgOldCpToNew(pr(int));
DEFPUBLIC(int) OgNewCpToOld(pr(int));

DEFPUBLIC(int) OgUniPositionReverse(pr_(int) pr_(int *) pr_(int) pr(int *));

DEFPUBLIC(int) OgUtf8Strlwr(int is1, unsigned char *s1, int *pis2, unsigned char *s2);
DEFPUBLIC(int) OgUtf8Strupr(int is1, unsigned char *s1, int *pis2, unsigned char *s2);
DEFPUBLIC(int) OgUtf8Struna(int is1, unsigned char *s1, int *pis2, unsigned char *s2);


#define _LOGUNIALIVE_
#endif


