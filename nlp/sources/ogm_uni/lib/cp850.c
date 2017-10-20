/*
 *	Handling code page 850.
 *	Copyright (c) 2003 Pertimm by Patrick Constant
 *	Dev : December 2003
 *	Version 1.0
*/
#include <loguni.h>


static unsigned OgCp850[256];

/*
 *  Changes a Unicode char to a cp850 char. This function returns
 *  a truncated char if the Unicode code does not correspond.
*/

PUBLIC(int) OgUniToCp850(u,truncated)
int u,*truncated;
{
if (truncated) *truncated=0;
switch(u) {
  case 0x00c7: return(0x80);  /* 0x80  LATIN CAPITAL LETTER C WITH CEDILLA */
  case 0x00fc: return(0x81);  /* 0x81  LATIN SMALL LETTER U WITH DIAERESIS */
  case 0x00e9: return(0x82);  /* 0x82  LATIN SMALL LETTER E WITH ACUTE */
  case 0x00e2: return(0x83);  /* 0x83  LATIN SMALL LETTER A WITH CIRCUMFLEX */
  case 0x00e4: return(0x84);  /* 0x84  LATIN SMALL LETTER A WITH DIAERESIS */
  case 0x00e0: return(0x85);  /* 0x85  LATIN SMALL LETTER A WITH GRAVE */
  case 0x00e5: return(0x86);  /* 0x86  LATIN SMALL LETTER A WITH RING ABOVE */
  case 0x00e7: return(0x87);  /* 0x87  LATIN SMALL LETTER C WITH CEDILLA */
  case 0x00ea: return(0x88);  /* 0x88  LATIN SMALL LETTER E WITH CIRCUMFLEX */
  case 0x00eb: return(0x89);  /* 0x89  LATIN SMALL LETTER E WITH DIAERESIS */
  case 0x00e8: return(0x8A);  /* 0x8A  LATIN SMALL LETTER E WITH GRAVE */
  case 0x00ef: return(0x8B);  /* 0x8B  LATIN SMALL LETTER I WITH DIAERESIS */
  case 0x00ee: return(0x8C);  /* 0x8C  LATIN SMALL LETTER I WITH CIRCUMFLEX */
  case 0x00ec: return(0x8D);  /* 0x8D  LATIN SMALL LETTER I WITH GRAVE */
  case 0x00c4: return(0x8E);  /* 0x8E  LATIN CAPITAL LETTER A WITH DIAERESIS */
  case 0x00c5: return(0x8F);  /* 0x8F  LATIN CAPITAL LETTER A WITH RING ABOVE */
  case 0x00c9: return(0x90);  /* 0x90  LATIN CAPITAL LETTER E WITH ACUTE */
  case 0x00e6: return(0x91);  /* 0x91  LATIN SMALL LIGATURE AE */
  case 0x00c6: return(0x92);  /* 0x92  LATIN CAPITAL LIGATURE AE */
  case 0x00f4: return(0x93);  /* 0x93  LATIN SMALL LETTER O WITH CIRCUMFLEX */
  case 0x00f6: return(0x94);  /* 0x94  LATIN SMALL LETTER O WITH DIAERESIS */
  case 0x00f2: return(0x95);  /* 0x95  LATIN SMALL LETTER O WITH GRAVE */
  case 0x00fb: return(0x96);  /* 0x96  LATIN SMALL LETTER U WITH CIRCUMFLEX */
  case 0x00f9: return(0x97);  /* 0x97  LATIN SMALL LETTER U WITH GRAVE */
  case 0x00ff: return(0x98);  /* 0x98  LATIN SMALL LETTER Y WITH DIAERESIS */
  case 0x00d6: return(0x99);  /* 0x99  LATIN CAPITAL LETTER O WITH DIAERESIS */
  case 0x00dc: return(0x9A);  /* 0x9A  LATIN CAPITAL LETTER U WITH DIAERESIS */
  case 0x00f8: return(0x9B);  /* 0x9B  LATIN SMALL LETTER O WITH STROKE */
  case 0x00a3: return(0x9C);  /* 0x9C  POUND SIGN */
  case 0x00d8: return(0x9D);  /* 0x9D  LATIN CAPITAL LETTER O WITH STROKE */
  case 0x00d7: return(0x9E);  /* 0x9E  MULTIPLICATION SIGN */
  case 0x0192: return(0x9F);  /* 0x9F  LATIN SMALL LETTER F WITH HOOK */
  case 0x00e1: return(0xA0);  /* 0xA0  LATIN SMALL LETTER A WITH ACUTE */
  case 0x00ed: return(0xA1);  /* 0xA1  LATIN SMALL LETTER I WITH ACUTE */
  case 0x00f3: return(0xA2);  /* 0xA2  LATIN SMALL LETTER O WITH ACUTE */
  case 0x00fa: return(0xA3);  /* 0xA3  LATIN SMALL LETTER U WITH ACUTE */
  case 0x00f1: return(0xA4);  /* 0xA4  LATIN SMALL LETTER N WITH TILDE */
  case 0x00d1: return(0xA5);  /* 0xA5  LATIN CAPITAL LETTER N WITH TILDE */
  case 0x00aa: return(0xA6);  /* 0xA6  FEMININE ORDINAL INDICATOR */
  case 0x00ba: return(0xA7);  /* 0xA7  MASCULINE ORDINAL INDICATOR */
  case 0x00bf: return(0xA8);  /* 0xA8  INVERTED QUESTION MARK */
  case 0x00ae: return(0xA9);  /* 0xA9  REGISTERED SIGN */
  case 0x00ac: return(0xAA);  /* 0xAA  NOT SIGN */
  case 0x00bd: return(0xAB);  /* 0xAB  VULGAR FRACTION ONE HALF */
  case 0x00bc: return(0xAC);  /* 0xAC  VULGAR FRACTION ONE QUARTER */
  case 0x00a1: return(0xAD);  /* 0xAD  INVERTED EXCLAMATION MARK */
  case 0x00ab: return(0xAE);  /* 0xAE  LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
  case 0x00bb: return(0xAF);  /* 0xAF  RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  case 0x2591: return(0xB0);  /* 0xB0  LIGHT SHADE */
  case 0x2592: return(0xB1);  /* 0xB1  MEDIUM SHADE */
  case 0x2593: return(0xB2);  /* 0xB2  DARK SHADE */
  case 0x2502: return(0xB3);  /* 0xB3  BOX DRAWINGS LIGHT VERTICAL */
  case 0x2524: return(0xB4);  /* 0xB4  BOX DRAWINGS LIGHT VERTICAL AND LEFT */
  case 0x00c1: return(0xB5);  /* 0xB5  LATIN CAPITAL LETTER A WITH ACUTE */
  case 0x00c2: return(0xB6);  /* 0xB6  LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
  case 0x00c0: return(0xB7);  /* 0xB7  LATIN CAPITAL LETTER A WITH GRAVE */
  case 0x00a9: return(0xB8);  /* 0xB8  COPYRIGHT SIGN */
  case 0x2563: return(0xB9);  /* 0xB9  BOX DRAWINGS DOUBLE VERTICAL AND LEFT */
  case 0x2551: return(0xBA);  /* 0xBA  BOX DRAWINGS DOUBLE VERTICAL */
  case 0x2557: return(0xBB);  /* 0xBB  BOX DRAWINGS DOUBLE DOWN AND LEFT */
  case 0x255d: return(0xBC);  /* 0xBC  BOX DRAWINGS DOUBLE UP AND LEFT */
  case 0x00a2: return(0xBD);  /* 0xBD  CENT SIGN */
  case 0x00a5: return(0xBE);  /* 0xBE  YEN SIGN */
  case 0x2510: return(0xBF);  /* 0xBF  BOX DRAWINGS LIGHT DOWN AND LEFT */
  case 0x2514: return(0xC0);  /* 0xC0  BOX DRAWINGS LIGHT UP AND RIGHT */
  case 0x2534: return(0xC1);  /* 0xC1  BOX DRAWINGS LIGHT UP AND HORIZONTAL */
  case 0x252c: return(0xC2);  /* 0xC2  BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
  case 0x251c: return(0xC3);  /* 0xC3  BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
  case 0x2500: return(0xC4);  /* 0xC4  BOX DRAWINGS LIGHT HORIZONTAL */
  case 0x253c: return(0xC5);  /* 0xC5  BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
  case 0x00e3: return(0xC6);  /* 0xC6  LATIN SMALL LETTER A WITH TILDE */
  case 0x00c3: return(0xC7);  /* 0xC7  LATIN CAPITAL LETTER A WITH TILDE */
  case 0x255a: return(0xC8);  /* 0xC8  BOX DRAWINGS DOUBLE UP AND RIGHT */
  case 0x2554: return(0xC9);  /* 0xC9  BOX DRAWINGS DOUBLE DOWN AND RIGHT */
  case 0x2569: return(0xCA);  /* 0xCA  BOX DRAWINGS DOUBLE UP AND HORIZONTAL */
  case 0x2566: return(0xCB);  /* 0xCB  BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL */
  case 0x2560: return(0xCC);  /* 0xCC  BOX DRAWINGS DOUBLE VERTICAL AND RIGHT */
  case 0x2550: return(0xCD);  /* 0xCD  BOX DRAWINGS DOUBLE HORIZONTAL */
  case 0x256c: return(0xCE);  /* 0xCE  BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL */
  case 0x00a4: return(0xCF);  /* 0xCF  CURRENCY SIGN */
  case 0x00f0: return(0xD0);  /* 0xD0  LATIN SMALL LETTER ETH */
  case 0x00d0: return(0xD1);  /* 0xD1  LATIN CAPITAL LETTER ETH */
  case 0x00ca: return(0xD2);  /* 0xD2  LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
  case 0x00cb: return(0xD3);  /* 0xD3  LATIN CAPITAL LETTER E WITH DIAERESIS */
  case 0x00c8: return(0xD4);  /* 0xD4  LATIN CAPITAL LETTER E WITH GRAVE */
  case 0x0131: return(0xD5);  /* 0xD5  LATIN SMALL LETTER DOTLESS I */
  case 0x00cd: return(0xD6);  /* 0xD6  LATIN CAPITAL LETTER I WITH ACUTE */
  case 0x00ce: return(0xD7);  /* 0xD7  LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
  case 0x00cf: return(0xD8);  /* 0xD8  LATIN CAPITAL LETTER I WITH DIAERESIS */
  case 0x2518: return(0xD9);  /* 0xD9  BOX DRAWINGS LIGHT UP AND LEFT */
  case 0x250c: return(0xDA);  /* 0xDA  BOX DRAWINGS LIGHT DOWN AND RIGHT */
  case 0x2588: return(0xDB);  /* 0xDB  FULL BLOCK */
  case 0x2584: return(0xDC);  /* 0xDC  LOWER HALF BLOCK */
  case 0x00a6: return(0xDD);  /* 0xDD  BROKEN BAR */
  case 0x00cc: return(0xDE);  /* 0xDE  LATIN CAPITAL LETTER I WITH GRAVE */
  case 0x2580: return(0xDF);  /* 0xDF  UPPER HALF BLOCK */
  case 0x00d3: return(0xE0);  /* 0xE0  LATIN CAPITAL LETTER O WITH ACUTE */
  case 0x00df: return(0xE1);  /* 0xE1  LATIN SMALL LETTER SHARP S */
  case 0x00d4: return(0xE2);  /* 0xE2  LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
  case 0x00d2: return(0xE3);  /* 0xE3  LATIN CAPITAL LETTER O WITH GRAVE */
  case 0x00f5: return(0xE4);  /* 0xE4  LATIN SMALL LETTER O WITH TILDE */
  case 0x00d5: return(0xE5);  /* 0xE5  LATIN CAPITAL LETTER O WITH TILDE */
  case 0x00b5: return(0xE6);  /* 0xE6  MICRO SIGN */
  case 0x00fe: return(0xE7);  /* 0xE7  LATIN SMALL LETTER THORN */
  case 0x00de: return(0xE8);  /* 0xE8  LATIN CAPITAL LETTER THORN */
  case 0x00da: return(0xE9);  /* 0xE9  LATIN CAPITAL LETTER U WITH ACUTE */
  case 0x00db: return(0xEA);  /* 0xEA  LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
  case 0x00d9: return(0xEB);  /* 0xEB  LATIN CAPITAL LETTER U WITH GRAVE */
  case 0x00fd: return(0xEC);  /* 0xEC  LATIN SMALL LETTER Y WITH ACUTE */
  case 0x00dd: return(0xED);  /* 0xED  LATIN CAPITAL LETTER Y WITH ACUTE */
  case 0x00af: return(0xEE);  /* 0xEE  MACRON */
  case 0x00b4: return(0xEF);  /* 0xEF  ACUTE ACCENT */
  case 0x00ad: return(0xF0);  /* 0xF0  SOFT HYPHEN */
  case 0x00b1: return(0xF1);  /* 0xF1  PLUS-MINUS SIGN */
  case 0x2017: return(0xF2);  /* 0xF2  DOUBLE LOW LINE */
  case 0x00be: return(0xF3);  /* 0xF3  VULGAR FRACTION THREE QUARTERS */
  case 0x00b6: return(0xF4);  /* 0xF4  PILCROW SIGN */
  case 0x00a7: return(0xF5);  /* 0xF5  SECTION SIGN */
  case 0x00f7: return(0xF6);  /* 0xF6  DIVISION SIGN */
  case 0x00b8: return(0xF7);  /* 0xF7  CEDILLA */
  case 0x00b0: return(0xF8);  /* 0xF8  DEGREE SIGN */
  case 0x00a8: return(0xF9);  /* 0xF9  DIAERESIS */
  case 0x00b7: return(0xFA);  /* 0xFA  MIDDLE DOT */
  case 0x00b9: return(0xFB);  /* 0xFB  SUPERSCRIPT ONE */
  case 0x00b3: return(0xFC);  /* 0xFC  SUPERSCRIPT THREE */
  case 0x00b2: return(0xFD);  /* 0xFD  SUPERSCRIPT TWO */
  case 0x25a0: return(0xFE);  /* 0xFE  BLACK SQUARE */
  case 0x00a0: return(0xFF);  /* 0xFF  NO-BREAK SPACE */
  }
if (truncated) { if (u>0xff) *truncated=1; }
return(u&0xff);
}





/*
 *  reverse function.
*/

PUBLIC(int) OgCp850ToUni(c)
int c;
{
if (c<256) return(OgCp850[c]);
else return(c);
}



static unsigned OgCp850[256] = {
  0x0000, /* 0x00  NULL */
  0x0001, /* 0x01  START OF HEADING */
  0x0002, /* 0x02  START OF TEXT */
  0x0003, /* 0x03  END OF TEXT */
  0x0004, /* 0x04  END OF TRANSMISSION */
  0x0005, /* 0x05  ENQUIRY */
  0x0006, /* 0x06  ACKNOWLEDGE */
  0x0007, /* 0x07  BELL */
  0x0008, /* 0x08  BACKSPACE */
  0x0009, /* 0x09  HORIZONTAL TABULATION */
  0x000a, /* 0x0A  LINE FEED */
  0x000b, /* 0x0B  VERTICAL TABULATION */
  0x000c, /* 0x0C  FORM FEED */
  0x000d, /* 0x0D  CARRIAGE RETURN */
  0x000e, /* 0x0E  SHIFT OUT */
  0x000f, /* 0x0F  SHIFT IN */
  0x0010, /* 0x10  DATA LINK ESCAPE */
  0x0011, /* 0x11  DEVICE CONTROL ONE */
  0x0012, /* 0x12  DEVICE CONTROL TWO */
  0x0013, /* 0x13  DEVICE CONTROL THREE */
  0x0014, /* 0x14  DEVICE CONTROL FOUR */
  0x0015, /* 0x15  NEGATIVE ACKNOWLEDGE */
  0x0016, /* 0x16  SYNCHRONOUS IDLE */
  0x0017, /* 0x17  END OF TRANSMISSION BLOCK */
  0x0018, /* 0x18  CANCEL */
  0x0019, /* 0x19  END OF MEDIUM */
  0x001a, /* 0x1A  SUBSTITUTE */
  0x001b, /* 0x1B  ESCAPE */
  0x001c, /* 0x1C  FILE SEPARATOR */
  0x001d, /* 0x1D  GROUP SEPARATOR */
  0x001e, /* 0x1E  RECORD SEPARATOR */
  0x001f, /* 0x1F  UNIT SEPARATOR */
  0x0020, /* 0x20  SPACE */
  0x0021, /* 0x21  EXCLAMATION MARK */
  0x0022, /* 0x22  QUOTATION MARK */
  0x0023, /* 0x23  NUMBER SIGN */
  0x0024, /* 0x24  DOLLAR SIGN */
  0x0025, /* 0x25  PERCENT SIGN */
  0x0026, /* 0x26  AMPERSAND */
  0x0027, /* 0x27  APOSTROPHE */
  0x0028, /* 0x28  LEFT PARENTHESIS */
  0x0029, /* 0x29  RIGHT PARENTHESIS */
  0x002a, /* 0x2A  ASTERISK */
  0x002b, /* 0x2B  PLUS SIGN */
  0x002c, /* 0x2C  COMMA */
  0x002d, /* 0x2D  HYPHEN-MINUS */
  0x002e, /* 0x2E  FULL STOP */
  0x002f, /* 0x2F  SOLIDUS */
  0x0030, /* 0x30  DIGIT ZERO */
  0x0031, /* 0x31  DIGIT ONE */
  0x0032, /* 0x32  DIGIT TWO */
  0x0033, /* 0x33  DIGIT THREE */
  0x0034, /* 0x34  DIGIT FOUR */
  0x0035, /* 0x35  DIGIT FIVE */
  0x0036, /* 0x36  DIGIT SIX */
  0x0037, /* 0x37  DIGIT SEVEN */
  0x0038, /* 0x38  DIGIT EIGHT */
  0x0039, /* 0x39  DIGIT NINE */
  0x003a, /* 0x3A  COLON */
  0x003b, /* 0x3B  SEMICOLON */
  0x003c, /* 0x3C  LESS-THAN SIGN */
  0x003d, /* 0x3D  EQUALS SIGN */
  0x003e, /* 0x3E  GREATER-THAN SIGN */
  0x003f, /* 0x3F  QUESTION MARK */
  0x0040, /* 0x40  COMMERCIAL AT */
  0x0041, /* 0x41  LATIN CAPITAL LETTER A */
  0x0042, /* 0x42  LATIN CAPITAL LETTER B */
  0x0043, /* 0x43  LATIN CAPITAL LETTER C */
  0x0044, /* 0x44  LATIN CAPITAL LETTER D */
  0x0045, /* 0x45  LATIN CAPITAL LETTER E */
  0x0046, /* 0x46  LATIN CAPITAL LETTER F */
  0x0047, /* 0x47  LATIN CAPITAL LETTER G */
  0x0048, /* 0x48  LATIN CAPITAL LETTER H */
  0x0049, /* 0x49  LATIN CAPITAL LETTER I */
  0x004a, /* 0x4A  LATIN CAPITAL LETTER J */
  0x004b, /* 0x4B  LATIN CAPITAL LETTER K */
  0x004c, /* 0x4C  LATIN CAPITAL LETTER L */
  0x004d, /* 0x4D  LATIN CAPITAL LETTER M */
  0x004e, /* 0x4E  LATIN CAPITAL LETTER N */
  0x004f, /* 0x4F  LATIN CAPITAL LETTER O */
  0x0050, /* 0x50  LATIN CAPITAL LETTER P */
  0x0051, /* 0x51  LATIN CAPITAL LETTER Q */
  0x0052, /* 0x52  LATIN CAPITAL LETTER R */
  0x0053, /* 0x53  LATIN CAPITAL LETTER S */
  0x0054, /* 0x54  LATIN CAPITAL LETTER T */
  0x0055, /* 0x55  LATIN CAPITAL LETTER U */
  0x0056, /* 0x56  LATIN CAPITAL LETTER V */
  0x0057, /* 0x57  LATIN CAPITAL LETTER W */
  0x0058, /* 0x58  LATIN CAPITAL LETTER X */
  0x0059, /* 0x59  LATIN CAPITAL LETTER Y */
  0x005a, /* 0x5A  LATIN CAPITAL LETTER Z */
  0x005b, /* 0x5B  LEFT SQUARE BRACKET */
  0x005c, /* 0x5C  REVERSE SOLIDUS */
  0x005d, /* 0x5D  RIGHT SQUARE BRACKET */
  0x005e, /* 0x5E  CIRCUMFLEX ACCENT */
  0x005f, /* 0x5F  LOW LINE */
  0x0060, /* 0x60  GRAVE ACCENT */
  0x0061, /* 0x61  LATIN SMALL LETTER A */
  0x0062, /* 0x62  LATIN SMALL LETTER B */
  0x0063, /* 0x63  LATIN SMALL LETTER C */
  0x0064, /* 0x64  LATIN SMALL LETTER D */
  0x0065, /* 0x65  LATIN SMALL LETTER E */
  0x0066, /* 0x66  LATIN SMALL LETTER F */
  0x0067, /* 0x67  LATIN SMALL LETTER G */
  0x0068, /* 0x68  LATIN SMALL LETTER H */
  0x0069, /* 0x69  LATIN SMALL LETTER I */
  0x006a, /* 0x6A  LATIN SMALL LETTER J */
  0x006b, /* 0x6B  LATIN SMALL LETTER K */
  0x006c, /* 0x6C  LATIN SMALL LETTER L */
  0x006d, /* 0x6D  LATIN SMALL LETTER M */
  0x006e, /* 0x6E  LATIN SMALL LETTER N */
  0x006f, /* 0x6F  LATIN SMALL LETTER O */
  0x0070, /* 0x70  LATIN SMALL LETTER P */
  0x0071, /* 0x71  LATIN SMALL LETTER Q */
  0x0072, /* 0x72  LATIN SMALL LETTER R */
  0x0073, /* 0x73  LATIN SMALL LETTER S */
  0x0074, /* 0x74  LATIN SMALL LETTER T */
  0x0075, /* 0x75  LATIN SMALL LETTER U */
  0x0076, /* 0x76  LATIN SMALL LETTER V */
  0x0077, /* 0x77  LATIN SMALL LETTER W */
  0x0078, /* 0x78  LATIN SMALL LETTER X */
  0x0079, /* 0x79  LATIN SMALL LETTER Y */
  0x007a, /* 0x7A  LATIN SMALL LETTER Z */
  0x007b, /* 0x7B  LEFT CURLY BRACKET */
  0x007c, /* 0x7C  VERTICAL LINE */
  0x007d, /* 0x7D  RIGHT CURLY BRACKET */
  0x007e, /* 0x7E  TILDE */
  0x007f, /* 0x7F  DELETE */
  0x00c7, /* 0x80  LATIN CAPITAL LETTER C WITH CEDILLA */
  0x00fc, /* 0x81  LATIN SMALL LETTER U WITH DIAERESIS */
  0x00e9, /* 0x82  LATIN SMALL LETTER E WITH ACUTE */
  0x00e2, /* 0x83  LATIN SMALL LETTER A WITH CIRCUMFLEX */
  0x00e4, /* 0x84  LATIN SMALL LETTER A WITH DIAERESIS */
  0x00e0, /* 0x85  LATIN SMALL LETTER A WITH GRAVE */
  0x00e5, /* 0x86  LATIN SMALL LETTER A WITH RING ABOVE */
  0x00e7, /* 0x87  LATIN SMALL LETTER C WITH CEDILLA */
  0x00ea, /* 0x88  LATIN SMALL LETTER E WITH CIRCUMFLEX */
  0x00eb, /* 0x89  LATIN SMALL LETTER E WITH DIAERESIS */
  0x00e8, /* 0x8A  LATIN SMALL LETTER E WITH GRAVE */
  0x00ef, /* 0x8B  LATIN SMALL LETTER I WITH DIAERESIS */
  0x00ee, /* 0x8C  LATIN SMALL LETTER I WITH CIRCUMFLEX */
  0x00ec, /* 0x8D  LATIN SMALL LETTER I WITH GRAVE */
  0x00c4, /* 0x8E  LATIN CAPITAL LETTER A WITH DIAERESIS */
  0x00c5, /* 0x8F  LATIN CAPITAL LETTER A WITH RING ABOVE */
  0x00c9, /* 0x90  LATIN CAPITAL LETTER E WITH ACUTE */
  0x00e6, /* 0x91  LATIN SMALL LIGATURE AE */
  0x00c6, /* 0x92  LATIN CAPITAL LIGATURE AE */
  0x00f4, /* 0x93  LATIN SMALL LETTER O WITH CIRCUMFLEX */
  0x00f6, /* 0x94  LATIN SMALL LETTER O WITH DIAERESIS */
  0x00f2, /* 0x95  LATIN SMALL LETTER O WITH GRAVE */
  0x00fb, /* 0x96  LATIN SMALL LETTER U WITH CIRCUMFLEX */
  0x00f9, /* 0x97  LATIN SMALL LETTER U WITH GRAVE */
  0x00ff, /* 0x98  LATIN SMALL LETTER Y WITH DIAERESIS */
  0x00d6, /* 0x99  LATIN CAPITAL LETTER O WITH DIAERESIS */
  0x00dc, /* 0x9A  LATIN CAPITAL LETTER U WITH DIAERESIS */
  0x00f8, /* 0x9B  LATIN SMALL LETTER O WITH STROKE */
  0x00a3, /* 0x9C  POUND SIGN */
  0x00d8, /* 0x9D  LATIN CAPITAL LETTER O WITH STROKE */
  0x00d7, /* 0x9E  MULTIPLICATION SIGN */
  0x0192, /* 0x9F  LATIN SMALL LETTER F WITH HOOK */
  0x00e1, /* 0xA0  LATIN SMALL LETTER A WITH ACUTE */
  0x00ed, /* 0xA1  LATIN SMALL LETTER I WITH ACUTE */
  0x00f3, /* 0xA2  LATIN SMALL LETTER O WITH ACUTE */
  0x00fa, /* 0xA3  LATIN SMALL LETTER U WITH ACUTE */
  0x00f1, /* 0xA4  LATIN SMALL LETTER N WITH TILDE */
  0x00d1, /* 0xA5  LATIN CAPITAL LETTER N WITH TILDE */
  0x00aa, /* 0xA6  FEMININE ORDINAL INDICATOR */
  0x00ba, /* 0xA7  MASCULINE ORDINAL INDICATOR */
  0x00bf, /* 0xA8  INVERTED QUESTION MARK */
  0x00ae, /* 0xA9  REGISTERED SIGN */
  0x00ac, /* 0xAA  NOT SIGN */
  0x00bd, /* 0xAB  VULGAR FRACTION ONE HALF */
  0x00bc, /* 0xAC  VULGAR FRACTION ONE QUARTER */
  0x00a1, /* 0xAD  INVERTED EXCLAMATION MARK */
  0x00ab, /* 0xAE  LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
  0x00bb, /* 0xAF  RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  0x2591, /* 0xB0  LIGHT SHADE */
  0x2592, /* 0xB1  MEDIUM SHADE */
  0x2593, /* 0xB2  DARK SHADE */
  0x2502, /* 0xB3  BOX DRAWINGS LIGHT VERTICAL */
  0x2524, /* 0xB4  BOX DRAWINGS LIGHT VERTICAL AND LEFT */
  0x00c1, /* 0xB5  LATIN CAPITAL LETTER A WITH ACUTE */
  0x00c2, /* 0xB6  LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
  0x00c0, /* 0xB7  LATIN CAPITAL LETTER A WITH GRAVE */
  0x00a9, /* 0xB8  COPYRIGHT SIGN */
  0x2563, /* 0xB9  BOX DRAWINGS DOUBLE VERTICAL AND LEFT */
  0x2551, /* 0xBA  BOX DRAWINGS DOUBLE VERTICAL */
  0x2557, /* 0xBB  BOX DRAWINGS DOUBLE DOWN AND LEFT */
  0x255d, /* 0xBC  BOX DRAWINGS DOUBLE UP AND LEFT */
  0x00a2, /* 0xBD  CENT SIGN */
  0x00a5, /* 0xBE  YEN SIGN */
  0x2510, /* 0xBF  BOX DRAWINGS LIGHT DOWN AND LEFT */
  0x2514, /* 0xC0  BOX DRAWINGS LIGHT UP AND RIGHT */
  0x2534, /* 0xC1  BOX DRAWINGS LIGHT UP AND HORIZONTAL */
  0x252c, /* 0xC2  BOX DRAWINGS LIGHT DOWN AND HORIZONTAL */
  0x251c, /* 0xC3  BOX DRAWINGS LIGHT VERTICAL AND RIGHT */
  0x2500, /* 0xC4  BOX DRAWINGS LIGHT HORIZONTAL */
  0x253c, /* 0xC5  BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL */
  0x00e3, /* 0xC6  LATIN SMALL LETTER A WITH TILDE */
  0x00c3, /* 0xC7  LATIN CAPITAL LETTER A WITH TILDE */
  0x255a, /* 0xC8  BOX DRAWINGS DOUBLE UP AND RIGHT */
  0x2554, /* 0xC9  BOX DRAWINGS DOUBLE DOWN AND RIGHT */
  0x2569, /* 0xCA  BOX DRAWINGS DOUBLE UP AND HORIZONTAL */
  0x2566, /* 0xCB  BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL */
  0x2560, /* 0xCC  BOX DRAWINGS DOUBLE VERTICAL AND RIGHT */
  0x2550, /* 0xCD  BOX DRAWINGS DOUBLE HORIZONTAL */
  0x256c, /* 0xCE  BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL */
  0x00a4, /* 0xCF  CURRENCY SIGN */
  0x00f0, /* 0xD0  LATIN SMALL LETTER ETH */
  0x00d0, /* 0xD1  LATIN CAPITAL LETTER ETH */
  0x00ca, /* 0xD2  LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
  0x00cb, /* 0xD3  LATIN CAPITAL LETTER E WITH DIAERESIS */
  0x00c8, /* 0xD4  LATIN CAPITAL LETTER E WITH GRAVE */
  0x0131, /* 0xD5  LATIN SMALL LETTER DOTLESS I */
  0x00cd, /* 0xD6  LATIN CAPITAL LETTER I WITH ACUTE */
  0x00ce, /* 0xD7  LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
  0x00cf, /* 0xD8  LATIN CAPITAL LETTER I WITH DIAERESIS */
  0x2518, /* 0xD9  BOX DRAWINGS LIGHT UP AND LEFT */
  0x250c, /* 0xDA  BOX DRAWINGS LIGHT DOWN AND RIGHT */
  0x2588, /* 0xDB  FULL BLOCK */
  0x2584, /* 0xDC  LOWER HALF BLOCK */
  0x00a6, /* 0xDD  BROKEN BAR */
  0x00cc, /* 0xDE  LATIN CAPITAL LETTER I WITH GRAVE */
  0x2580, /* 0xDF  UPPER HALF BLOCK */
  0x00d3, /* 0xE0  LATIN CAPITAL LETTER O WITH ACUTE */
  0x00df, /* 0xE1  LATIN SMALL LETTER SHARP S */
  0x00d4, /* 0xE2  LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
  0x00d2, /* 0xE3  LATIN CAPITAL LETTER O WITH GRAVE */
  0x00f5, /* 0xE4  LATIN SMALL LETTER O WITH TILDE */
  0x00d5, /* 0xE5  LATIN CAPITAL LETTER O WITH TILDE */
  0x00b5, /* 0xE6  MICRO SIGN */
  0x00fe, /* 0xE7  LATIN SMALL LETTER THORN */
  0x00de, /* 0xE8  LATIN CAPITAL LETTER THORN */
  0x00da, /* 0xE9  LATIN CAPITAL LETTER U WITH ACUTE */
  0x00db, /* 0xEA  LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
  0x00d9, /* 0xEB  LATIN CAPITAL LETTER U WITH GRAVE */
  0x00fd, /* 0xEC  LATIN SMALL LETTER Y WITH ACUTE */
  0x00dd, /* 0xED  LATIN CAPITAL LETTER Y WITH ACUTE */
  0x00af, /* 0xEE  MACRON */
  0x00b4, /* 0xEF  ACUTE ACCENT */
  0x00ad, /* 0xF0  SOFT HYPHEN */
  0x00b1, /* 0xF1  PLUS-MINUS SIGN */
  0x2017, /* 0xF2  DOUBLE LOW LINE */
  0x00be, /* 0xF3  VULGAR FRACTION THREE QUARTERS */
  0x00b6, /* 0xF4  PILCROW SIGN */
  0x00a7, /* 0xF5  SECTION SIGN */
  0x00f7, /* 0xF6  DIVISION SIGN */
  0x00b8, /* 0xF7  CEDILLA */
  0x00b0, /* 0xF8  DEGREE SIGN */
  0x00a8, /* 0xF9  DIAERESIS */
  0x00b7, /* 0xFA  MIDDLE DOT */
  0x00b9, /* 0xFB  SUPERSCRIPT ONE */
  0x00b3, /* 0xFC  SUPERSCRIPT THREE */
  0x00b2, /* 0xFD  SUPERSCRIPT TWO */
  0x25a0, /* 0xFE  BLACK SQUARE */
  0x00a0  /* 0xFF  NO-BREAK SPACE */
  };

