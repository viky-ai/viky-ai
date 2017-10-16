/*
 * Handling code page 1256 quite far from 8859-6.
 * Copyright (c) 2007 Pertimm by Patrick Constant
 * Dev : October 2007
 * Version 1.0
*/
#include <loguni.h>


static unsigned OgCp1256[256];

/*
 *  Changes a Unicode char to a cp1256 char. This function returns 
 *  a truncated char if the Unicode code does not correspond.
*/

PUBLIC(int) OgUniToCp1256(u,truncated)
int u,*truncated;
{
if (truncated) *truncated=0;
switch(u) {
  case 0x20AC: return(0x80);  /* #EURO SIGN */
  case 0x067E: return(0x81);  /* #ARABIC LETTER PEH */
  case 0x201A: return(0x82);  /* #SINGLE LOW-9 QUOTATION MARK */
  case 0x0192: return(0x83);  /* #LATIN SMALL LETTER F WITH HOOK */
  case 0x201E: return(0x84);  /* #DOUBLE LOW-9 QUOTATION MARK */
  case 0x2026: return(0x85);  /* #HORIZONTAL ELLIPSIS */
  case 0x2020: return(0x86);  /* #DAGGER */
  case 0x2021: return(0x87);  /* #DOUBLE DAGGER */
  case 0x02C6: return(0x88);  /* #MODIFIER LETTER CIRCUMFLEX ACCENT */
  case 0x2030: return(0x89);  /* #PER MILLE SIGN */
  case 0x0679: return(0x8A);  /* #ARABIC LETTER TTEH */
  case 0x2039: return(0x8B);  /* #SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
  case 0x0152: return(0x8C);  /* #LATIN CAPITAL LIGATURE OE */
  case 0x0686: return(0x8D);  /* #ARABIC LETTER TCHEH */
  case 0x0698: return(0x8E);  /* #ARABIC LETTER JEH */
  case 0x0688: return(0x8F);  /* #ARABIC LETTER DDAL */
  case 0x06AF: return(0x90);  /* #ARABIC LETTER GAF */
  case 0x2018: return(0x91);  /* #LEFT SINGLE QUOTATION MARK */
  case 0x2019: return(0x92);  /* #RIGHT SINGLE QUOTATION MARK */
  case 0x201C: return(0x93);  /* #LEFT DOUBLE QUOTATION MARK */
  case 0x201D: return(0x94);  /* #RIGHT DOUBLE QUOTATION MARK */
  case 0x2022: return(0x95);  /* #BULLET */
  case 0x2013: return(0x96);  /* #EN DASH */
  case 0x2014: return(0x97);  /* #EM DASH */
  case 0x06A9: return(0x98);  /* #ARABIC LETTER KEHEH */
  case 0x2122: return(0x99);  /* #TRADE MARK SIGN */
  case 0x0691: return(0x9A);  /* #ARABIC LETTER RREH */
  case 0x203A: return(0x9B);  /* #SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
  case 0x0153: return(0x9C);  /* #LATIN SMALL LIGATURE OE */
  case 0x200C: return(0x9D);  /* #ZERO WIDTH NON-JOINER */
  case 0x200D: return(0x9E);  /* #ZERO WIDTH JOINER */
  case 0x06BA: return(0x9F);  /* #ARABIC LETTER NOON GHUNNA */
  case 0x00A0: return(0xA0);  /* #NO-BREAK SPACE */
  case 0x060C: return(0xA1);  /* #ARABIC COMMA */
  case 0x00A2: return(0xA2);  /* #CENT SIGN */
  case 0x00A3: return(0xA3);  /* #POUND SIGN */
  case 0x00A4: return(0xA4);  /* #CURRENCY SIGN */
  case 0x00A5: return(0xA5);  /* #YEN SIGN */
  case 0x00A6: return(0xA6);  /* #BROKEN BAR */
  case 0x00A7: return(0xA7);  /* #SECTION SIGN */
  case 0x00A8: return(0xA8);  /* #DIAERESIS */
  case 0x00A9: return(0xA9);  /* #COPYRIGHT SIGN */
  case 0x06BE: return(0xAA);  /* #ARABIC LETTER HEH DOACHASHMEE */
  case 0x00AB: return(0xAB);  /* #LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
  case 0x00AC: return(0xAC);  /* #NOT SIGN */
  case 0x00AD: return(0xAD);  /* #SOFT HYPHEN */
  case 0x00AE: return(0xAE);  /* #REGISTERED SIGN */
  case 0x00AF: return(0xAF);  /* #MACRON */
  case 0x00B0: return(0xB0);  /* #DEGREE SIGN */
  case 0x00B1: return(0xB1);  /* #PLUS-MINUS SIGN */
  case 0x00B2: return(0xB2);  /* #SUPERSCRIPT TWO */
  case 0x00B3: return(0xB3);  /* #SUPERSCRIPT THREE */
  case 0x00B4: return(0xB4);  /* #ACUTE ACCENT */
  case 0x00B5: return(0xB5);  /* #MICRO SIGN */
  case 0x00B6: return(0xB6);  /* #PILCROW SIGN */
  case 0x00B7: return(0xB7);  /* #MIDDLE DOT */
  case 0x00B8: return(0xB8);  /* #CEDILLA */
  case 0x00B9: return(0xB9);  /* #SUPERSCRIPT ONE */
  case 0x061B: return(0xBA);  /* #ARABIC SEMICOLON */
  case 0x00BB: return(0xBB);  /* #RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  case 0x00BC: return(0xBC);  /* #VULGAR FRACTION ONE QUARTER */
  case 0x00BD: return(0xBD);  /* #VULGAR FRACTION ONE HALF */
  case 0x00BE: return(0xBE);  /* #VULGAR FRACTION THREE QUARTERS */
  case 0x061F: return(0xBF);  /* #ARABIC QUESTION MARK */
  case 0x06C1: return(0xC0);  /* #ARABIC LETTER HEH GOAL */
  case 0x0621: return(0xC1);  /* #ARABIC LETTER HAMZA */
  case 0x0622: return(0xC2);  /* #ARABIC LETTER ALEF WITH MADDA ABOVE */
  case 0x0623: return(0xC3);  /* #ARABIC LETTER ALEF WITH HAMZA ABOVE */
  case 0x0624: return(0xC4);  /* #ARABIC LETTER WAW WITH HAMZA ABOVE */
  case 0x0625: return(0xC5);  /* #ARABIC LETTER ALEF WITH HAMZA BELOW */
  case 0x0626: return(0xC6);  /* #ARABIC LETTER YEH WITH HAMZA ABOVE */
  case 0x0627: return(0xC7);  /* #ARABIC LETTER ALEF */
  case 0x0628: return(0xC8);  /* #ARABIC LETTER BEH */
  case 0x0629: return(0xC9);  /* #ARABIC LETTER TEH MARBUTA */
  case 0x062A: return(0xCA);  /* #ARABIC LETTER TEH */
  case 0x062B: return(0xCB);  /* #ARABIC LETTER THEH */
  case 0x062C: return(0xCC);  /* #ARABIC LETTER JEEM */
  case 0x062D: return(0xCD);  /* #ARABIC LETTER HAH */
  case 0x062E: return(0xCE);  /* #ARABIC LETTER KHAH */
  case 0x062F: return(0xCF);  /* #ARABIC LETTER DAL */
  case 0x0630: return(0xD0);  /* #ARABIC LETTER THAL */
  case 0x0631: return(0xD1);  /* #ARABIC LETTER REH */
  case 0x0632: return(0xD2);  /* #ARABIC LETTER ZAIN */
  case 0x0633: return(0xD3);  /* #ARABIC LETTER SEEN */
  case 0x0634: return(0xD4);  /* #ARABIC LETTER SHEEN */
  case 0x0635: return(0xD5);  /* #ARABIC LETTER SAD */
  case 0x0636: return(0xD6);  /* #ARABIC LETTER DAD */
  case 0x00D7: return(0xD7);  /* #MULTIPLICATION SIGN */
  case 0x0637: return(0xD8);  /* #ARABIC LETTER TAH */
  case 0x0638: return(0xD9);  /* #ARABIC LETTER ZAH */
  case 0x0639: return(0xDA);  /* #ARABIC LETTER AIN */
  case 0x063A: return(0xDB);  /* #ARABIC LETTER GHAIN */
  case 0x0640: return(0xDC);  /* #ARABIC TATWEEL */
  case 0x0641: return(0xDD);  /* #ARABIC LETTER FEH */
  case 0x0642: return(0xDE);  /* #ARABIC LETTER QAF */
  case 0x0643: return(0xDF);  /* #ARABIC LETTER KAF */
  case 0x00E0: return(0xE0);  /* #LATIN SMALL LETTER A WITH GRAVE */
  case 0x0644: return(0xE1);  /* #ARABIC LETTER LAM */
  case 0x00E2: return(0xE2);  /* #LATIN SMALL LETTER A WITH CIRCUMFLEX */
  case 0x0645: return(0xE3);  /* #ARABIC LETTER MEEM */
  case 0x0646: return(0xE4);  /* #ARABIC LETTER NOON */
  case 0x0647: return(0xE5);  /* #ARABIC LETTER HEH */
  case 0x0648: return(0xE6);  /* #ARABIC LETTER WAW */
  case 0x00E7: return(0xE7);  /* #LATIN SMALL LETTER C WITH CEDILLA */
  case 0x00E8: return(0xE8);  /* #LATIN SMALL LETTER E WITH GRAVE */
  case 0x00E9: return(0xE9);  /* #LATIN SMALL LETTER E WITH ACUTE */
  case 0x00EA: return(0xEA);  /* #LATIN SMALL LETTER E WITH CIRCUMFLEX */
  case 0x00EB: return(0xEB);  /* #LATIN SMALL LETTER E WITH DIAERESIS */
  case 0x0649: return(0xEC);  /* #ARABIC LETTER ALEF MAKSURA */
  case 0x064A: return(0xED);  /* #ARABIC LETTER YEH */
  case 0x00EE: return(0xEE);  /* #LATIN SMALL LETTER I WITH CIRCUMFLEX */
  case 0x00EF: return(0xEF);  /* #LATIN SMALL LETTER I WITH DIAERESIS */
  case 0x064B: return(0xF0);  /* #ARABIC FATHATAN */
  case 0x064C: return(0xF1);  /* #ARABIC DAMMATAN */
  case 0x064D: return(0xF2);  /* #ARABIC KASRATAN */
  case 0x064E: return(0xF3);  /* #ARABIC FATHA */
  case 0x00F4: return(0xF4);  /* #LATIN SMALL LETTER O WITH CIRCUMFLEX */
  case 0x064F: return(0xF5);  /* #ARABIC DAMMA */
  case 0x0650: return(0xF6);  /* #ARABIC KASRA */
  case 0x00F7: return(0xF7);  /* #DIVISION SIGN */
  case 0x0651: return(0xF8);  /* #ARABIC SHADDA */
  case 0x00F9: return(0xF9);  /* #LATIN SMALL LETTER U WITH GRAVE */
  case 0x0652: return(0xFA);  /* #ARABIC SUKUN */
  case 0x00FB: return(0xFB);  /* #LATIN SMALL LETTER U WITH CIRCUMFLEX */
  case 0x00FC: return(0xFC);  /* #LATIN SMALL LETTER U WITH DIAERESIS */
  case 0x200E: return(0xFD);  /* #LEFT-TO-RIGHT MARK */
  case 0x200F: return(0xFE);  /* #RIGHT-TO-LEFT MARK */
  case 0x06D2: return(0xFF);  /* #ARABIC LETTER YEH BARREE */
  }
if (truncated) { if (u>0xff) *truncated=1; }
return(u&0xff);
}





/*
 *  reverse function.
*/

PUBLIC(int) OgCp1256ToUni(c)
int c;
{
if (c<256) return(OgCp1256[c]);
else return(c);
}



static unsigned OgCp1256[256] = {
  0x0000, /* 0x00 #NULL */
  0x0001, /* 0x01 #START OF HEADING */
  0x0002, /* 0x02 #START OF TEXT */
  0x0003, /* 0x03 #END OF TEXT */
  0x0004, /* 0x04 #END OF TRANSMISSION */
  0x0005, /* 0x05 #ENQUIRY */
  0x0006, /* 0x06 #ACKNOWLEDGE */
  0x0007, /* 0x07 #BELL */
  0x0008, /* 0x08 #BACKSPACE */
  0x0009, /* 0x09 #HORIZONTAL TABULATION */
  0x000A, /* 0x0A #LINE FEED */
  0x000B, /* 0x0B #VERTICAL TABULATION */
  0x000C, /* 0x0C #FORM FEED */
  0x000D, /* 0x0D #CARRIAGE RETURN */
  0x000E, /* 0x0E #SHIFT OUT */
  0x000F, /* 0x0F #SHIFT IN */
  0x0010, /* 0x10 #DATA LINK ESCAPE */
  0x0011, /* 0x11 #DEVICE CONTROL ONE */
  0x0012, /* 0x12 #DEVICE CONTROL TWO */
  0x0013, /* 0x13 #DEVICE CONTROL THREE */
  0x0014, /* 0x14 #DEVICE CONTROL FOUR */
  0x0015, /* 0x15 #NEGATIVE ACKNOWLEDGE */
  0x0016, /* 0x16 #SYNCHRONOUS IDLE */
  0x0017, /* 0x17 #END OF TRANSMISSION BLOCK */
  0x0018, /* 0x18 #CANCEL */
  0x0019, /* 0x19 #END OF MEDIUM */
  0x001A, /* 0x1A #SUBSTITUTE */
  0x001B, /* 0x1B #ESCAPE */
  0x001C, /* 0x1C #FILE SEPARATOR */
  0x001D, /* 0x1D #GROUP SEPARATOR */
  0x001E, /* 0x1E #RECORD SEPARATOR */
  0x001F, /* 0x1F #UNIT SEPARATOR */
  0x0020, /* 0x20 #SPACE */
  0x0021, /* 0x21 #EXCLAMATION MARK */
  0x0022, /* 0x22 #QUOTATION MARK */
  0x0023, /* 0x23 #NUMBER SIGN */
  0x0024, /* 0x24 #DOLLAR SIGN */
  0x0025, /* 0x25 #PERCENT SIGN */
  0x0026, /* 0x26 #AMPERSAND */
  0x0027, /* 0x27 #APOSTROPHE */
  0x0028, /* 0x28 #LEFT PARENTHESIS */
  0x0029, /* 0x29 #RIGHT PARENTHESIS */
  0x002A, /* 0x2A #ASTERISK */
  0x002B, /* 0x2B #PLUS SIGN */
  0x002C, /* 0x2C #COMMA */
  0x002D, /* 0x2D #HYPHEN-MINUS */
  0x002E, /* 0x2E #FULL STOP */
  0x002F, /* 0x2F #SOLIDUS */
  0x0030, /* 0x30 #DIGIT ZERO */
  0x0031, /* 0x31 #DIGIT ONE */
  0x0032, /* 0x32 #DIGIT TWO */
  0x0033, /* 0x33 #DIGIT THREE */
  0x0034, /* 0x34 #DIGIT FOUR */
  0x0035, /* 0x35 #DIGIT FIVE */
  0x0036, /* 0x36 #DIGIT SIX */
  0x0037, /* 0x37 #DIGIT SEVEN */
  0x0038, /* 0x38 #DIGIT EIGHT */
  0x0039, /* 0x39 #DIGIT NINE */
  0x003A, /* 0x3A #COLON */
  0x003B, /* 0x3B #SEMICOLON */
  0x003C, /* 0x3C #LESS-THAN SIGN */
  0x003D, /* 0x3D #EQUALS SIGN */
  0x003E, /* 0x3E #GREATER-THAN SIGN */
  0x003F, /* 0x3F #QUESTION MARK */
  0x0040, /* 0x40 #COMMERCIAL AT */
  0x0041, /* 0x41 #LATIN CAPITAL LETTER A */
  0x0042, /* 0x42 #LATIN CAPITAL LETTER B */
  0x0043, /* 0x43 #LATIN CAPITAL LETTER C */
  0x0044, /* 0x44 #LATIN CAPITAL LETTER D */
  0x0045, /* 0x45 #LATIN CAPITAL LETTER E */
  0x0046, /* 0x46 #LATIN CAPITAL LETTER F */
  0x0047, /* 0x47 #LATIN CAPITAL LETTER G */
  0x0048, /* 0x48 #LATIN CAPITAL LETTER H */
  0x0049, /* 0x49 #LATIN CAPITAL LETTER I */
  0x004A, /* 0x4A #LATIN CAPITAL LETTER J */
  0x004B, /* 0x4B #LATIN CAPITAL LETTER K */
  0x004C, /* 0x4C #LATIN CAPITAL LETTER L */
  0x004D, /* 0x4D #LATIN CAPITAL LETTER M */
  0x004E, /* 0x4E #LATIN CAPITAL LETTER N */
  0x004F, /* 0x4F #LATIN CAPITAL LETTER O */
  0x0050, /* 0x50 #LATIN CAPITAL LETTER P */
  0x0051, /* 0x51 #LATIN CAPITAL LETTER Q */
  0x0052, /* 0x52 #LATIN CAPITAL LETTER R */
  0x0053, /* 0x53 #LATIN CAPITAL LETTER S */
  0x0054, /* 0x54 #LATIN CAPITAL LETTER T */
  0x0055, /* 0x55 #LATIN CAPITAL LETTER U */
  0x0056, /* 0x56 #LATIN CAPITAL LETTER V */
  0x0057, /* 0x57 #LATIN CAPITAL LETTER W */
  0x0058, /* 0x58 #LATIN CAPITAL LETTER X */
  0x0059, /* 0x59 #LATIN CAPITAL LETTER Y */
  0x005A, /* 0x5A #LATIN CAPITAL LETTER Z */
  0x005B, /* 0x5B #LEFT SQUARE BRACKET */
  0x005C, /* 0x5C #REVERSE SOLIDUS */
  0x005D, /* 0x5D #RIGHT SQUARE BRACKET */
  0x005E, /* 0x5E #CIRCUMFLEX ACCENT */
  0x005F, /* 0x5F #LOW LINE */
  0x0060, /* 0x60 #GRAVE ACCENT */
  0x0061, /* 0x61 #LATIN SMALL LETTER A */
  0x0062, /* 0x62 #LATIN SMALL LETTER B */
  0x0063, /* 0x63 #LATIN SMALL LETTER C */
  0x0064, /* 0x64 #LATIN SMALL LETTER D */
  0x0065, /* 0x65 #LATIN SMALL LETTER E */
  0x0066, /* 0x66 #LATIN SMALL LETTER F */
  0x0067, /* 0x67 #LATIN SMALL LETTER G */
  0x0068, /* 0x68 #LATIN SMALL LETTER H */
  0x0069, /* 0x69 #LATIN SMALL LETTER I */
  0x006A, /* 0x6A #LATIN SMALL LETTER J */
  0x006B, /* 0x6B #LATIN SMALL LETTER K */
  0x006C, /* 0x6C #LATIN SMALL LETTER L */
  0x006D, /* 0x6D #LATIN SMALL LETTER M */
  0x006E, /* 0x6E #LATIN SMALL LETTER N */
  0x006F, /* 0x6F #LATIN SMALL LETTER O */
  0x0070, /* 0x70 #LATIN SMALL LETTER P */
  0x0071, /* 0x71 #LATIN SMALL LETTER Q */
  0x0072, /* 0x72 #LATIN SMALL LETTER R */
  0x0073, /* 0x73 #LATIN SMALL LETTER S */
  0x0074, /* 0x74 #LATIN SMALL LETTER T */
  0x0075, /* 0x75 #LATIN SMALL LETTER U */
  0x0076, /* 0x76 #LATIN SMALL LETTER V */
  0x0077, /* 0x77 #LATIN SMALL LETTER W */
  0x0078, /* 0x78 #LATIN SMALL LETTER X */
  0x0079, /* 0x79 #LATIN SMALL LETTER Y */
  0x007A, /* 0x7A #LATIN SMALL LETTER Z */
  0x007B, /* 0x7B #LEFT CURLY BRACKET */
  0x007C, /* 0x7C #VERTICAL LINE */
  0x007D, /* 0x7D #RIGHT CURLY BRACKET */
  0x007E, /* 0x7E #TILDE */
  0x007F, /* 0x7F #DELETE */
  0x20AC, /* 0x80 #EURO SIGN */
  0x067E, /* 0x81 #ARABIC LETTER PEH */
  0x201A, /* 0x82 #SINGLE LOW-9 QUOTATION MARK */
  0x0192, /* 0x83 #LATIN SMALL LETTER F WITH HOOK */
  0x201E, /* 0x84 #DOUBLE LOW-9 QUOTATION MARK */
  0x2026, /* 0x85 #HORIZONTAL ELLIPSIS */
  0x2020, /* 0x86 #DAGGER */
  0x2021, /* 0x87 #DOUBLE DAGGER */
  0x02C6, /* 0x88 #MODIFIER LETTER CIRCUMFLEX ACCENT */
  0x2030, /* 0x89 #PER MILLE SIGN */
  0x0679, /* 0x8A #ARABIC LETTER TTEH */
  0x2039, /* 0x8B #SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
  0x0152, /* 0x8C #LATIN CAPITAL LIGATURE OE */
  0x0686, /* 0x8D #ARABIC LETTER TCHEH */
  0x0698, /* 0x8E #ARABIC LETTER JEH */
  0x0688, /* 0x8F #ARABIC LETTER DDAL */
  0x06AF, /* 0x90 #ARABIC LETTER GAF */
  0x2018, /* 0x91 #LEFT SINGLE QUOTATION MARK */
  0x2019, /* 0x92 #RIGHT SINGLE QUOTATION MARK */
  0x201C, /* 0x93 #LEFT DOUBLE QUOTATION MARK */
  0x201D, /* 0x94 #RIGHT DOUBLE QUOTATION MARK */
  0x2022, /* 0x95 #BULLET */
  0x2013, /* 0x96 #EN DASH */
  0x2014, /* 0x97 #EM DASH */
  0x06A9, /* 0x98 #ARABIC LETTER KEHEH */
  0x2122, /* 0x99 #TRADE MARK SIGN */
  0x0691, /* 0x9A #ARABIC LETTER RREH */
  0x203A, /* 0x9B #SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
  0x0153, /* 0x9C #LATIN SMALL LIGATURE OE */
  0x200C, /* 0x9D #ZERO WIDTH NON-JOINER */
  0x200D, /* 0x9E #ZERO WIDTH JOINER */
  0x06BA, /* 0x9F #ARABIC LETTER NOON GHUNNA */
  0x00A0, /* 0xA0 #NO-BREAK SPACE */
  0x060C, /* 0xA1 #ARABIC COMMA */
  0x00A2, /* 0xA2 #CENT SIGN */
  0x00A3, /* 0xA3 #POUND SIGN */
  0x00A4, /* 0xA4 #CURRENCY SIGN */
  0x00A5, /* 0xA5 #YEN SIGN */
  0x00A6, /* 0xA6 #BROKEN BAR */
  0x00A7, /* 0xA7 #SECTION SIGN */
  0x00A8, /* 0xA8 #DIAERESIS */
  0x00A9, /* 0xA9 #COPYRIGHT SIGN */
  0x06BE, /* 0xAA #ARABIC LETTER HEH DOACHASHMEE */
  0x00AB, /* 0xAB #LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
  0x00AC, /* 0xAC #NOT SIGN */
  0x00AD, /* 0xAD #SOFT HYPHEN */
  0x00AE, /* 0xAE #REGISTERED SIGN */
  0x00AF, /* 0xAF #MACRON */
  0x00B0, /* 0xB0 #DEGREE SIGN */
  0x00B1, /* 0xB1 #PLUS-MINUS SIGN */
  0x00B2, /* 0xB2 #SUPERSCRIPT TWO */
  0x00B3, /* 0xB3 #SUPERSCRIPT THREE */
  0x00B4, /* 0xB4 #ACUTE ACCENT */
  0x00B5, /* 0xB5 #MICRO SIGN */
  0x00B6, /* 0xB6 #PILCROW SIGN */
  0x00B7, /* 0xB7 #MIDDLE DOT */
  0x00B8, /* 0xB8 #CEDILLA */
  0x00B9, /* 0xB9 #SUPERSCRIPT ONE */
  0x061B, /* 0xBA #ARABIC SEMICOLON */
  0x00BB, /* 0xBB #RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
  0x00BC, /* 0xBC #VULGAR FRACTION ONE QUARTER */
  0x00BD, /* 0xBD #VULGAR FRACTION ONE HALF */
  0x00BE, /* 0xBE #VULGAR FRACTION THREE QUARTERS */
  0x061F, /* 0xBF #ARABIC QUESTION MARK */
  0x06C1, /* 0xC0 #ARABIC LETTER HEH GOAL */
  0x0621, /* 0xC1 #ARABIC LETTER HAMZA */
  0x0622, /* 0xC2 #ARABIC LETTER ALEF WITH MADDA ABOVE */
  0x0623, /* 0xC3 #ARABIC LETTER ALEF WITH HAMZA ABOVE */
  0x0624, /* 0xC4 #ARABIC LETTER WAW WITH HAMZA ABOVE */
  0x0625, /* 0xC5 #ARABIC LETTER ALEF WITH HAMZA BELOW */
  0x0626, /* 0xC6 #ARABIC LETTER YEH WITH HAMZA ABOVE */
  0x0627, /* 0xC7 #ARABIC LETTER ALEF */
  0x0628, /* 0xC8 #ARABIC LETTER BEH */
  0x0629, /* 0xC9 #ARABIC LETTER TEH MARBUTA */
  0x062A, /* 0xCA #ARABIC LETTER TEH */
  0x062B, /* 0xCB #ARABIC LETTER THEH */
  0x062C, /* 0xCC #ARABIC LETTER JEEM */
  0x062D, /* 0xCD #ARABIC LETTER HAH */
  0x062E, /* 0xCE #ARABIC LETTER KHAH */
  0x062F, /* 0xCF #ARABIC LETTER DAL */
  0x0630, /* 0xD0 #ARABIC LETTER THAL */
  0x0631, /* 0xD1 #ARABIC LETTER REH */
  0x0632, /* 0xD2 #ARABIC LETTER ZAIN */
  0x0633, /* 0xD3 #ARABIC LETTER SEEN */
  0x0634, /* 0xD4 #ARABIC LETTER SHEEN */
  0x0635, /* 0xD5 #ARABIC LETTER SAD */
  0x0636, /* 0xD6 #ARABIC LETTER DAD */
  0x00D7, /* 0xD7 #MULTIPLICATION SIGN */
  0x0637, /* 0xD8 #ARABIC LETTER TAH */
  0x0638, /* 0xD9 #ARABIC LETTER ZAH */
  0x0639, /* 0xDA #ARABIC LETTER AIN */
  0x063A, /* 0xDB #ARABIC LETTER GHAIN */
  0x0640, /* 0xDC #ARABIC TATWEEL */
  0x0641, /* 0xDD #ARABIC LETTER FEH */
  0x0642, /* 0xDE #ARABIC LETTER QAF */
  0x0643, /* 0xDF #ARABIC LETTER KAF */
  0x00E0, /* 0xE0 #LATIN SMALL LETTER A WITH GRAVE */
  0x0644, /* 0xE1 #ARABIC LETTER LAM */
  0x00E2, /* 0xE2 #LATIN SMALL LETTER A WITH CIRCUMFLEX */
  0x0645, /* 0xE3 #ARABIC LETTER MEEM */
  0x0646, /* 0xE4 #ARABIC LETTER NOON */
  0x0647, /* 0xE5 #ARABIC LETTER HEH */
  0x0648, /* 0xE6 #ARABIC LETTER WAW */
  0x00E7, /* 0xE7 #LATIN SMALL LETTER C WITH CEDILLA */
  0x00E8, /* 0xE8 #LATIN SMALL LETTER E WITH GRAVE */
  0x00E9, /* 0xE9 #LATIN SMALL LETTER E WITH ACUTE */
  0x00EA, /* 0xEA #LATIN SMALL LETTER E WITH CIRCUMFLEX */
  0x00EB, /* 0xEB #LATIN SMALL LETTER E WITH DIAERESIS */
  0x0649, /* 0xEC #ARABIC LETTER ALEF MAKSURA */
  0x064A, /* 0xED #ARABIC LETTER YEH */
  0x00EE, /* 0xEE #LATIN SMALL LETTER I WITH CIRCUMFLEX */
  0x00EF, /* 0xEF #LATIN SMALL LETTER I WITH DIAERESIS */
  0x064B, /* 0xF0 #ARABIC FATHATAN */
  0x064C, /* 0xF1 #ARABIC DAMMATAN */
  0x064D, /* 0xF2 #ARABIC KASRATAN */
  0x064E, /* 0xF3 #ARABIC FATHA */
  0x00F4, /* 0xF4 #LATIN SMALL LETTER O WITH CIRCUMFLEX */
  0x064F, /* 0xF5 #ARABIC DAMMA */
  0x0650, /* 0xF6 #ARABIC KASRA */
  0x00F7, /* 0xF7 #DIVISION SIGN */
  0x0651, /* 0xF8 #ARABIC SHADDA */
  0x00F9, /* 0xF9 #LATIN SMALL LETTER U WITH GRAVE */
  0x0652, /* 0xFA #ARABIC SUKUN */
  0x00FB, /* 0xFB #LATIN SMALL LETTER U WITH CIRCUMFLEX */
  0x00FC, /* 0xFC #LATIN SMALL LETTER U WITH DIAERESIS */
  0x200E, /* 0xFD #LEFT-TO-RIGHT MARK */
  0x200F, /* 0xFE #RIGHT-TO-LEFT MARK */
  0x06D2 /* 0xFF #ARABIC LETTER YEH BARREE */
  }; 


