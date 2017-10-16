/*
 *  Handling UTF8-Unicode transformations.
 *  Copyright (c) 2004-2007 Pertimm by Patrick Constant
 *  Dev : September 2004, December 2007
 *  Version 1.1
*/
#include <loguni.h>



/*
 *  Changes a Unicode string to a string in UTF8.
 *  'unicode' is unicode encoded string of length 'iunicode'
 *  'code' is string where the coded string with 'codepage'
 *  is written. scode is maximun length of code.
 *  'code' is zero terminated and *truncated is 1 (0 otherwise)
 *  if the 'code' string does not contain all encoded data.
 *  According to RFC 2279 rules are:
 *   UCS-4 range (hex.)           UTF-8 octet sequence (binary)
 *   0000 0000-0000 007F   0xxxxxxx
 *   0000 0080-0000 07FF   110xxxxx 10xxxxxx
 *   0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
*/
int OgUniToUTF8(int iunicode, const unsigned char *unicode, int scode, int *icode, unsigned char *code, int *position,
    int *truncated)
{
int i,j,c,start,byte_zero,byte_one;

if (unicode[0]==0xfe && unicode[1]==0xff) { start=2; byte_zero=0; byte_one=1; } /** big endian **/
else if (unicode[0]==0xff && unicode[1]==0xfe) { start=2; byte_zero=1; byte_one=0; } /** small endian **/
else { start=0; byte_zero=0; byte_one=1; } /** defaults to big endian **/

for (i=start,j=0; i+1<iunicode; i+=2) {
  c = (unicode[i+byte_zero]<<8) + unicode[i+byte_one];
  if (c <= 0x7f) {
    if (j>=scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i>>1;
    code[j++]=(unsigned char)c;
    }
  else if (c <= 0x7ff) {
    if (j+1>=scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i>>1;
    code[j++] = 0xc0 | (c>>6);
    if (position) position[j]=i>>1;
    code[j++] = 0x80 | (c&0x3f);
    }
  else if (c <= 0xffff) {
    if (j+2>=scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i>>1;
    code[j++] = 0xe0 | (c>>12);
    if (position) position[j]=i>>1;
    code[j++] = 0x80 | ((c>>6)&0x3f);
    if (position) position[j]=i>>1;
    code[j++] = 0x80 | (c&0x3f);
    }
  /** else is impossible **/
  }
code[j]=0;
if (position) position[j]=i>>1;
if (icode) *icode=j;
DONE;
}



/*
 *  Changes a UTF8 string to a Unicode string.
 *  'code' is UTF8 encoded string of length 'icode'
 *  'unicode' is Unicode string, 'sunicode' is maximun length of unicode.
 *  'unicode' is not zero terminated and *truncated is 1 (0 otherwise)
 *  if the 'unicode' string does not contain all encoded data.
 *  According to RFC 2279 rules are:
 *   UCS-4 range (hex.)           UTF-8 octet sequence (binary)
 *   0000 0000-0000 007F   0xxxxxxx
 *   0000 0080-0000 07FF   110xxxxx 10xxxxxx
 *   0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
*/

PUBLIC(int) OgUTF8ToUni(int icode, const unsigned char *code, int sunicode, int *iunicode, unsigned char *unicode,
    int *position, int *truncated)
{
int i,j,c,u=0,state=1;

for (i=j=0; i<icode; i++) {
  c = code[i];
  switch (state) {
    /** first UTF8 char **/
    case 1:
      if (c <= 0x7f) {
        u = c; if (position) position[j>>1]=i;
        if (j+1>=sunicode) {
          if (truncated) *truncated=1;
          goto endOgUTF8ToUni;
          }
        unicode[j++]=(u>>8); unicode[j++]=(u&0xff);
        }
      else if ((c>>5)==0x6) { u = (c&0x1f); u<<=6; state=2; }
      else if ((c>>4)==0xe) { u = (c&0xf); u<<=12; state=3; }
      /** else we forget the character **/
      break;
    /** two UTF8 char, second char **/
    case 2:
      if ((c>>6)==0x2) {
        u |= (c&0x3f);  if (position) position[j>>1]=i-1;
        if (j+1>=sunicode) {
          if (truncated) *truncated=1;
          goto endOgUTF8ToUni;
          }
        unicode[j++]=(u>>8); unicode[j++]=(u&0xff);
        }
      else {
        /* This is not a correct UTF-8 encoding,
         * we forget the first char and keep the
         * second if it is possible */
        i--; state=1;
        }
      break;
    /** three UTF8 char, second char **/
    case 3:
      if ((c>>6)==0x2) { u |= ((c&0x3f)<<6); state=4; }
      else {
        /* This is not a correct UTF-8 encoding,
         * we forget the first char and keep the
         * second if it is possible */
        i--; state=1;
        }
      break;
    /** three UTF8 char, third char **/
    case 4:
      if ((c>>6)==0x2) {
        u |= (c&0x3f);  if (position) position[j>>1]=i-2;
        if (j+1>=sunicode) {
          if (truncated) *truncated=1;
          goto endOgUTF8ToUni;
          }
        unicode[j++]=(u>>8); unicode[j++]=(u&0xff);
        state=1;
        }
      else {
        /* This is not a correct UTF-8 encoding,
         * we forget the first char and keep the
         * second and third if it is possible */
        i-=2; state=1;
        }
      break;
    }
  }
endOgUTF8ToUni:
if (position) position[j>>1]=i;
*iunicode=j;
DONE;
}





PUBLIC(int) OgCheckUTF8(int icode, const unsigned char *code, int *pnb_ascii_char, int *pnb_utf8_chars)
{
int i,j,c,state=1;
int nb_ascii_char=0,nb_utf8_chars=0;

for (i=j=0; i<icode; i++) {
  c = code[i];
  switch (state) {
    /** first UTF8 char **/
    case 1:
      if (c <= 0x7f) nb_ascii_char++;
      else if ((c>>5)==0x6) { state=2; }
      else if ((c>>4)==0xe) { state=3; }
      /** else we forget the character **/
      break;
    /** two UTF8 char, second char **/
    case 2:
      if ((c>>6)==0x2) {
        nb_utf8_chars+=2;
        state=1;
        }
      else {
        /* This is not a correct UTF-8 encoding,
         * we forget the first char and keep the
         * second if it is possible */
        i--; state=1;
        }
      break;
    /** three UTF8 char, second char **/
    case 3:
      if ((c>>6)==0x2) state=4;
      else {
        /* This is not a correct UTF-8 encoding,
         * we forget the first char and keep the
         * second if it is possible */
        i--; state=1;
        }
      break;
    /** three UTF8 char, third char **/
    case 4:
      if ((c>>6)==0x2) {
        nb_utf8_chars+=3;
        state=1;
        }
      else {
        /* This is not a correct UTF-8 encoding,
         * we forget the first char and keep the
         * second and third if it is possible */
        i-=2; state=1;
        }
      break;
    }
  }

if (pnb_ascii_char) *pnb_ascii_char = nb_ascii_char;
if (pnb_utf8_chars) *pnb_utf8_chars = nb_utf8_chars;

if (nb_ascii_char+nb_utf8_chars==icode) return(2);
else if (nb_utf8_chars > 0) return(1);

return(0);
}


