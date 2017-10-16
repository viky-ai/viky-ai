/*
 *	Handling code page 8859_5.
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : September 2009
 *	Version 1.0
*/
#include <loguni.h>


static unsigned OgCp8859_5[256];

/*
 *  Changes a Unicode char to a cp8859_5 char. This function returns
 *  a truncated char if the Unicode code does not correspond.
*/

PUBLIC(int) OgUniToCp8859_5(u,truncated)
int u,*truncated;
{
if (truncated) *truncated=0;
switch(u) {
  case 0x0401: return(0xA1);  /* CYRILLIC CAPITAL LETTER IO */                             
  case 0x0402: return(0xA2);  /* CYRILLIC CAPITAL LETTER DJE */                            
  case 0x0403: return(0xA3);  /* CYRILLIC CAPITAL LETTER GJE */                            
  case 0x0404: return(0xA4);  /* CYRILLIC CAPITAL LETTER UKRAINIAN IE */                   
  case 0x0405: return(0xA5);  /* CYRILLIC CAPITAL LETTER DZE */                            
  case 0x0406: return(0xA6);  /* CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */       
  case 0x0407: return(0xA7);  /* CYRILLIC CAPITAL LETTER YI */                             
  case 0x0408: return(0xA8);  /* CYRILLIC CAPITAL LETTER JE */                             
  case 0x0409: return(0xA9);  /* CYRILLIC CAPITAL LETTER LJE */                            
  case 0x040A: return(0xAA);  /* CYRILLIC CAPITAL LETTER NJE */                            
  case 0x040B: return(0xAB);  /* CYRILLIC CAPITAL LETTER TSHE */                           
  case 0x040C: return(0xAC);  /* CYRILLIC CAPITAL LETTER KJE */                            
  case 0x00AD: return(0xAD);  /* SOFT HYPHEN */                                            
  case 0x040E: return(0xAE);  /* CYRILLIC CAPITAL LETTER SHORT U */                        
  case 0x040F: return(0xAF);  /* CYRILLIC CAPITAL LETTER DZHE */                           
  case 0x0410: return(0xB0);  /* CYRILLIC CAPITAL LETTER A */                              
  case 0x0411: return(0xB1);  /* CYRILLIC CAPITAL LETTER BE */                             
  case 0x0412: return(0xB2);  /* CYRILLIC CAPITAL LETTER VE */                             
  case 0x0413: return(0xB3);  /* CYRILLIC CAPITAL LETTER GHE */                            
  case 0x0414: return(0xB4);  /* CYRILLIC CAPITAL LETTER DE */                             
  case 0x0415: return(0xB5);  /* CYRILLIC CAPITAL LETTER IE */                             
  case 0x0416: return(0xB6);  /* CYRILLIC CAPITAL LETTER ZHE */                            
  case 0x0417: return(0xB7);  /* CYRILLIC CAPITAL LETTER ZE */                             
  case 0x0418: return(0xB8);  /* CYRILLIC CAPITAL LETTER I */                              
  case 0x0419: return(0xB9);  /* CYRILLIC CAPITAL LETTER SHORT I */                        
  case 0x041A: return(0xBA);  /* CYRILLIC CAPITAL LETTER KA */                             
  case 0x041B: return(0xBB);  /* CYRILLIC CAPITAL LETTER EL */                             
  case 0x041C: return(0xBC);  /* CYRILLIC CAPITAL LETTER EM */                             
  case 0x041D: return(0xBD);  /* CYRILLIC CAPITAL LETTER EN */                             
  case 0x041E: return(0xBE);  /* CYRILLIC CAPITAL LETTER O */                              
  case 0x041F: return(0xBF);  /* CYRILLIC CAPITAL LETTER PE */                             
  case 0x0420: return(0xC0);  /* CYRILLIC CAPITAL LETTER ER */                             
  case 0x0421: return(0xC1);  /* CYRILLIC CAPITAL LETTER ES */                             
  case 0x0422: return(0xC2);  /* CYRILLIC CAPITAL LETTER TE */                             
  case 0x0423: return(0xC3);  /* CYRILLIC CAPITAL LETTER U */                              
  case 0x0424: return(0xC4);  /* CYRILLIC CAPITAL LETTER EF */                             
  case 0x0425: return(0xC5);  /* CYRILLIC CAPITAL LETTER HA */                             
  case 0x0426: return(0xC6);  /* CYRILLIC CAPITAL LETTER TSE */                            
  case 0x0427: return(0xC7);  /* CYRILLIC CAPITAL LETTER CHE */                            
  case 0x0428: return(0xC8);  /* CYRILLIC CAPITAL LETTER SHA */                            
  case 0x0429: return(0xC9);  /* CYRILLIC CAPITAL LETTER SHCHA */                          
  case 0x042A: return(0xCA);  /* CYRILLIC CAPITAL LETTER HARD SIGN */                      
  case 0x042B: return(0xCB);  /* CYRILLIC CAPITAL LETTER YERU */                           
  case 0x042C: return(0xCC);  /* CYRILLIC CAPITAL LETTER SOFT SIGN */                      
  case 0x042D: return(0xCD);  /* CYRILLIC CAPITAL LETTER E */                              
  case 0x042E: return(0xCE);  /* CYRILLIC CAPITAL LETTER YU */                             
  case 0x042F: return(0xCF);  /* CYRILLIC CAPITAL LETTER YA */                             
  case 0x0430: return(0xD0);  /* CYRILLIC SMALL LETTER A */                                
  case 0x0431: return(0xD1);  /* CYRILLIC SMALL LETTER BE */                               
  case 0x0432: return(0xD2);  /* CYRILLIC SMALL LETTER VE */                               
  case 0x0433: return(0xD3);  /* CYRILLIC SMALL LETTER GHE */                              
  case 0x0434: return(0xD4);  /* CYRILLIC SMALL LETTER DE */                               
  case 0x0435: return(0xD5);  /* CYRILLIC SMALL LETTER IE */                               
  case 0x0436: return(0xD6);  /* CYRILLIC SMALL LETTER ZHE */                              
  case 0x0437: return(0xD7);  /* CYRILLIC SMALL LETTER ZE */                               
  case 0x0438: return(0xD8);  /* CYRILLIC SMALL LETTER I */                                
  case 0x0439: return(0xD9);  /* CYRILLIC SMALL LETTER SHORT I */                          
  case 0x043A: return(0xDA);  /* CYRILLIC SMALL LETTER KA */                               
  case 0x043B: return(0xDB);  /* CYRILLIC SMALL LETTER EL */                               
  case 0x043C: return(0xDC);  /* CYRILLIC SMALL LETTER EM */                               
  case 0x043D: return(0xDD);  /* CYRILLIC SMALL LETTER EN */                               
  case 0x043E: return(0xDE);  /* CYRILLIC SMALL LETTER O */                                
  case 0x043F: return(0xDF);  /* CYRILLIC SMALL LETTER PE */                               
  case 0x0440: return(0xE0);  /* CYRILLIC SMALL LETTER ER */                               
  case 0x0441: return(0xE1);  /* CYRILLIC SMALL LETTER ES */                               
  case 0x0442: return(0xE2);  /* CYRILLIC SMALL LETTER TE */                               
  case 0x0443: return(0xE3);  /* CYRILLIC SMALL LETTER U */                                
  case 0x0444: return(0xE4);  /* CYRILLIC SMALL LETTER EF */                               
  case 0x0445: return(0xE5);  /* CYRILLIC SMALL LETTER HA */                               
  case 0x0446: return(0xE6);  /* CYRILLIC SMALL LETTER TSE */                              
  case 0x0447: return(0xE7);  /* CYRILLIC SMALL LETTER CHE */                              
  case 0x0448: return(0xE8);  /* CYRILLIC SMALL LETTER SHA */                              
  case 0x0449: return(0xE9);  /* CYRILLIC SMALL LETTER SHCHA */                            
  case 0x044A: return(0xEA);  /* CYRILLIC SMALL LETTER HARD SIGN */                        
  case 0x044B: return(0xEB);  /* CYRILLIC SMALL LETTER YERU */                             
  case 0x044C: return(0xEC);  /* CYRILLIC SMALL LETTER SOFT SIGN */                        
  case 0x044D: return(0xED);  /* CYRILLIC SMALL LETTER E */                                
  case 0x044E: return(0xEE);  /* CYRILLIC SMALL LETTER YU */                               
  case 0x044F: return(0xEF);  /* CYRILLIC SMALL LETTER YA */                               
  case 0x2116: return(0xF0);  /* NUMERO SIGN */                                            
  case 0x0451: return(0xF1);  /* CYRILLIC SMALL LETTER IO */                               
  case 0x0452: return(0xF2);  /* CYRILLIC SMALL LETTER DJE */                              
  case 0x0453: return(0xF3);  /* CYRILLIC SMALL LETTER GJE */                              
  case 0x0454: return(0xF4);  /* CYRILLIC SMALL LETTER UKRAINIAN IE */                     
  case 0x0455: return(0xF5);  /* CYRILLIC SMALL LETTER DZE */                              
  case 0x0456: return(0xF6);  /* CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */         
  case 0x0457: return(0xF7);  /* CYRILLIC SMALL LETTER YI */                               
  case 0x0458: return(0xF8);  /* CYRILLIC SMALL LETTER JE */                               
  case 0x0459: return(0xF9);  /* CYRILLIC SMALL LETTER LJE */                              
  case 0x045A: return(0xFA);  /* CYRILLIC SMALL LETTER NJE */                              
  case 0x045B: return(0xFB);  /* CYRILLIC SMALL LETTER TSHE */                             
  case 0x045C: return(0xFC);  /* CYRILLIC SMALL LETTER KJE */                              
  case 0x00A7: return(0xFD);  /* SECTION SIGN */                                           
  case 0x045E: return(0xFE);  /* CYRILLIC SMALL LETTER SHORT U */                          
  case 0x045F: return(0xFF);  /* CYRILLIC SMALL LETTER DZHE */                             
  }
if (truncated) { if (u>0xff) *truncated=1; }
return(u&0xff);
}





/*
 *  reverse function.
*/

PUBLIC(int) OgCp8859_5ToUni(c)
int c;
{
if (c<256) return(OgCp8859_5[c]);
else return(c);
}



static unsigned OgCp8859_5[256] = {
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
  0x000A, /* 0x0A  LINE FEED */
  0x000B, /* 0x0B  VERTICAL TABULATION */
  0x000C, /* 0x0C  FORM FEED */
  0x000D, /* 0x0D  CARRIAGE RETURN */
  0x000E, /* 0x0E  SHIFT OUT */
  0x000F, /* 0x0F  SHIFT IN */
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
  0x001A, /* 0x1A  SUBSTITUTE */
  0x001B, /* 0x1B  ESCAPE */
  0x001C, /* 0x1C  FILE SEPARATOR */
  0x001D, /* 0x1D  GROUP SEPARATOR */
  0x001E, /* 0x1E  RECORD SEPARATOR */
  0x001F, /* 0x1F  UNIT SEPARATOR */
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
  0x002A, /* 0x2A  ASTERISK */
  0x002B, /* 0x2B  PLUS SIGN */
  0x002C, /* 0x2C  COMMA */
  0x002D, /* 0x2D  HYPHEN-MINUS */
  0x002E, /* 0x2E  FULL STOP */
  0x002F, /* 0x2F  SOLIDUS */
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
  0x003A, /* 0x3A  COLON */
  0x003B, /* 0x3B  SEMICOLON */
  0x003C, /* 0x3C  LESS-THAN SIGN */
  0x003D, /* 0x3D  EQUALS SIGN */
  0x003E, /* 0x3E  GREATER-THAN SIGN */
  0x003F, /* 0x3F  QUESTION MARK */
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
  0x004A, /* 0x4A  LATIN CAPITAL LETTER J */
  0x004B, /* 0x4B  LATIN CAPITAL LETTER K */
  0x004C, /* 0x4C  LATIN CAPITAL LETTER L */
  0x004D, /* 0x4D  LATIN CAPITAL LETTER M */
  0x004E, /* 0x4E  LATIN CAPITAL LETTER N */
  0x004F, /* 0x4F  LATIN CAPITAL LETTER O */
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
  0x005A, /* 0x5A  LATIN CAPITAL LETTER Z */
  0x005B, /* 0x5B  LEFT SQUARE BRACKET */
  0x005C, /* 0x5C  REVERSE SOLIDUS */
  0x005D, /* 0x5D  RIGHT SQUARE BRACKET */
  0x005E, /* 0x5E  CIRCUMFLEX ACCENT */
  0x005F, /* 0x5F  LOW LINE */
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
  0x006A, /* 0x6A  LATIN SMALL LETTER J */
  0x006B, /* 0x6B  LATIN SMALL LETTER K */
  0x006C, /* 0x6C  LATIN SMALL LETTER L */
  0x006D, /* 0x6D  LATIN SMALL LETTER M */
  0x006E, /* 0x6E  LATIN SMALL LETTER N */
  0x006F, /* 0x6F  LATIN SMALL LETTER O */
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
  0x007A, /* 0x7A  LATIN SMALL LETTER Z */
  0x007B, /* 0x7B  LEFT CURLY BRACKET */
  0x007C, /* 0x7C  VERTICAL LINE */
  0x007D, /* 0x7D  RIGHT CURLY BRACKET */
  0x007E, /* 0x7E  TILDE */
  0x007F, /* 0x7F  DELETE */
  0x0080, /* 0x80  <control> */ 
  0x0081, /* 0x81  <control> */ 
  0x0082, /* 0x82  <control> */ 
  0x0083, /* 0x83  <control> */ 
  0x0084, /* 0x84  <control> */ 
  0x0085, /* 0x85  <control> */ 
  0x0086, /* 0x86  <control> */ 
  0x0087, /* 0x87  <control> */ 
  0x0088, /* 0x88  <control> */ 
  0x0089, /* 0x89  <control> */ 
  0x008A, /* 0x8A  <control> */ 
  0x008B, /* 0x8B  <control> */ 
  0x008C, /* 0x8C  <control> */ 
  0x008D, /* 0x8D  <control> */ 
  0x008E, /* 0x8E  <control> */ 
  0x008F, /* 0x8F  <control> */ 
  0x0090, /* 0x90  <control> */ 
  0x0091, /* 0x91  <control> */ 
  0x0092, /* 0x92  <control> */ 
  0x0093, /* 0x93  <control> */ 
  0x0094, /* 0x94  <control> */ 
  0x0095, /* 0x95  <control> */ 
  0x0096, /* 0x96  <control> */ 
  0x0097, /* 0x97  <control> */ 
  0x0098, /* 0x98  <control> */ 
  0x0099, /* 0x99  <control> */ 
  0x009A, /* 0x9A  <control> */ 
  0x009B, /* 0x9B  <control> */ 
  0x009C, /* 0x9C  <control> */ 
  0x009D, /* 0x9D  <control> */ 
  0x009E, /* 0x9E  <control> */ 
  0x009F, /* 0x9F  <control> */ 
  0x00A0, /* 0xA0  NO-BREAK SPACE */ 
  0x0401, /* 0xA1  CYRILLIC CAPITAL LETTER IO */    
  0x0402, /* 0xA2  CYRILLIC CAPITAL LETTER DJE */   
  0x0403, /* 0xA3  CYRILLIC CAPITAL LETTER GJE */   
  0x0404, /* 0xA4  CYRILLIC CAPITAL LETTER UKRAINIAN IE */    
  0x0405, /* 0xA5  CYRILLIC CAPITAL LETTER DZE */   
  0x0406, /* 0xA6  CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I */  
  0x0407, /* 0xA7  CYRILLIC CAPITAL LETTER YI */    
  0x0408, /* 0xA8  CYRILLIC CAPITAL LETTER JE */    
  0x0409, /* 0xA9  CYRILLIC CAPITAL LETTER LJE */   
  0x040A, /* 0xAA  CYRILLIC CAPITAL LETTER NJE */   
  0x040B, /* 0xAB  CYRILLIC CAPITAL LETTER TSHE */  
  0x040C, /* 0xAC  CYRILLIC CAPITAL LETTER KJE */   
  0x00AD, /* 0xAD  SOFT HYPHEN */    
  0x040E, /* 0xAE  CYRILLIC CAPITAL LETTER SHORT U */    
  0x040F, /* 0xAF  CYRILLIC CAPITAL LETTER DZHE */  
  0x0410, /* 0xB0  CYRILLIC CAPITAL LETTER A */
  0x0411, /* 0xB1  CYRILLIC CAPITAL LETTER BE */    
  0x0412, /* 0xB2  CYRILLIC CAPITAL LETTER VE */    
  0x0413, /* 0xB3  CYRILLIC CAPITAL LETTER GHE */   
  0x0414, /* 0xB4  CYRILLIC CAPITAL LETTER DE */    
  0x0415, /* 0xB5  CYRILLIC CAPITAL LETTER IE */    
  0x0416, /* 0xB6  CYRILLIC CAPITAL LETTER ZHE */   
  0x0417, /* 0xB7  CYRILLIC CAPITAL LETTER ZE */    
  0x0418, /* 0xB8  CYRILLIC CAPITAL LETTER I */
  0x0419, /* 0xB9  CYRILLIC CAPITAL LETTER SHORT I */    
  0x041A, /* 0xBA  CYRILLIC CAPITAL LETTER KA */    
  0x041B, /* 0xBB  CYRILLIC CAPITAL LETTER EL */    
  0x041C, /* 0xBC  CYRILLIC CAPITAL LETTER EM */    
  0x041D, /* 0xBD  CYRILLIC CAPITAL LETTER EN */    
  0x041E, /* 0xBE  CYRILLIC CAPITAL LETTER O */
  0x041F, /* 0xBF  CYRILLIC CAPITAL LETTER PE */    
  0x0420, /* 0xC0  CYRILLIC CAPITAL LETTER ER */    
  0x0421, /* 0xC1  CYRILLIC CAPITAL LETTER ES */    
  0x0422, /* 0xC2  CYRILLIC CAPITAL LETTER TE */    
  0x0423, /* 0xC3  CYRILLIC CAPITAL LETTER U */
  0x0424, /* 0xC4  CYRILLIC CAPITAL LETTER EF */    
  0x0425, /* 0xC5  CYRILLIC CAPITAL LETTER HA */    
  0x0426, /* 0xC6  CYRILLIC CAPITAL LETTER TSE */   
  0x0427, /* 0xC7  CYRILLIC CAPITAL LETTER CHE */   
  0x0428, /* 0xC8  CYRILLIC CAPITAL LETTER SHA */   
  0x0429, /* 0xC9  CYRILLIC CAPITAL LETTER SHCHA */ 
  0x042A, /* 0xCA  CYRILLIC CAPITAL LETTER HARD SIGN */  
  0x042B, /* 0xCB  CYRILLIC CAPITAL LETTER YERU */  
  0x042C, /* 0xCC  CYRILLIC CAPITAL LETTER SOFT SIGN */  
  0x042D, /* 0xCD  CYRILLIC CAPITAL LETTER E */
  0x042E, /* 0xCE  CYRILLIC CAPITAL LETTER YU */    
  0x042F, /* 0xCF  CYRILLIC CAPITAL LETTER YA */    
  0x0430, /* 0xD0  CYRILLIC SMALL LETTER A */  
  0x0431, /* 0xD1  CYRILLIC SMALL LETTER BE */ 
  0x0432, /* 0xD2  CYRILLIC SMALL LETTER VE */ 
  0x0433, /* 0xD3  CYRILLIC SMALL LETTER GHE */
  0x0434, /* 0xD4  CYRILLIC SMALL LETTER DE */ 
  0x0435, /* 0xD5  CYRILLIC SMALL LETTER IE */ 
  0x0436, /* 0xD6  CYRILLIC SMALL LETTER ZHE */
  0x0437, /* 0xD7  CYRILLIC SMALL LETTER ZE */ 
  0x0438, /* 0xD8  CYRILLIC SMALL LETTER I */  
  0x0439, /* 0xD9  CYRILLIC SMALL LETTER SHORT I */ 
  0x043A, /* 0xDA  CYRILLIC SMALL LETTER KA */ 
  0x043B, /* 0xDB  CYRILLIC SMALL LETTER EL */ 
  0x043C, /* 0xDC  CYRILLIC SMALL LETTER EM */ 
  0x043D, /* 0xDD  CYRILLIC SMALL LETTER EN */ 
  0x043E, /* 0xDE  CYRILLIC SMALL LETTER O */  
  0x043F, /* 0xDF  CYRILLIC SMALL LETTER PE */ 
  0x0440, /* 0xE0  CYRILLIC SMALL LETTER ER */ 
  0x0441, /* 0xE1  CYRILLIC SMALL LETTER ES */ 
  0x0442, /* 0xE2  CYRILLIC SMALL LETTER TE */ 
  0x0443, /* 0xE3  CYRILLIC SMALL LETTER U */  
  0x0444, /* 0xE4  CYRILLIC SMALL LETTER EF */ 
  0x0445, /* 0xE5  CYRILLIC SMALL LETTER HA */ 
  0x0446, /* 0xE6  CYRILLIC SMALL LETTER TSE */
  0x0447, /* 0xE7  CYRILLIC SMALL LETTER CHE */
  0x0448, /* 0xE8  CYRILLIC SMALL LETTER SHA */
  0x0449, /* 0xE9  CYRILLIC SMALL LETTER SHCHA */   
  0x044A, /* 0xEA  CYRILLIC SMALL LETTER HARD SIGN */    
  0x044B, /* 0xEB  CYRILLIC SMALL LETTER YERU */    
  0x044C, /* 0xEC  CYRILLIC SMALL LETTER SOFT SIGN */    
  0x044D, /* 0xED  CYRILLIC SMALL LETTER E */  
  0x044E, /* 0xEE  CYRILLIC SMALL LETTER YU */ 
  0x044F, /* 0xEF  CYRILLIC SMALL LETTER YA */ 
  0x2116, /* 0xF0  NUMERO SIGN */    
  0x0451, /* 0xF1  CYRILLIC SMALL LETTER IO */ 
  0x0452, /* 0xF2  CYRILLIC SMALL LETTER DJE */
  0x0453, /* 0xF3  CYRILLIC SMALL LETTER GJE */
  0x0454, /* 0xF4  CYRILLIC SMALL LETTER UKRAINIAN IE */ 
  0x0455, /* 0xF5  CYRILLIC SMALL LETTER DZE */
  0x0456, /* 0xF6  CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I */    
  0x0457, /* 0xF7  CYRILLIC SMALL LETTER YI */ 
  0x0458, /* 0xF8  CYRILLIC SMALL LETTER JE */ 
  0x0459, /* 0xF9  CYRILLIC SMALL LETTER LJE */
  0x045A, /* 0xFA  CYRILLIC SMALL LETTER NJE */
  0x045B, /* 0xFB  CYRILLIC SMALL LETTER TSHE */    
  0x045C, /* 0xFC  CYRILLIC SMALL LETTER KJE */
  0x00A7, /* 0xFD  SECTION SIGN */   
  0x045E, /* 0xFE  CYRILLIC SMALL LETTER SHORT U */ 
  0x045F  /* 0xFF  CYRILLIC SMALL LETTER DZHE */    
  };

