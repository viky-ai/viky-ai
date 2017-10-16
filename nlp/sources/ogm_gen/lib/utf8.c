/*
 *	Handling UTF8-Unicode transformations.
 *	Copyright (c) 2004-2007 Pertimm by Patrick Constant
 *	Dev : September 2004, December 2007
 *	Version 1.1
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
PUBLIC(int) OgUnisToUTF8(int iunicode, unsigned short *unicode, int scode, int *icode, unsigned char *code, int *position,int *truncated)
{
int i,j,c,start;

if (unicode[0]==0xfeff) { start=1; } /** big endian **/
else if (unicode[0]==0xfffe) { start=1; } /** small endian **/
else { start=0; } /** defaults to big endian **/

for (i=start,j=0; i<iunicode; i+=1) {
  c = unicode[i];
  if (c <= 0x7f) {
    if (j>=scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i;  
    code[j++]=(unsigned char)c;
    }
  else if (c <= 0x7ff) {
    if (j+1>=scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i;  
    code[j++] = 0xc0 | (c>>6);
    if (position) position[j]=i;  
    code[j++] = 0x80 | (c&0x3f);
    }
  else if (c <= 0xffff) {
    if (j+2>=scode-1) { /** -1 because we want to add zero at end **/
      if (truncated) *truncated=1;
      break;
      }
    if (position) position[j]=i;  
    code[j++] = 0xe0 | (c>>12);
    if (position) position[j]=i>>1;  
    code[j++] = 0x80 | ((c>>6)&0x3f);
    if (position) position[j]=i;  
    code[j++] = 0x80 | (c&0x3f);
    }
  /** else is impossible **/
  }
code[j]=0;
if (position) position[j]=i;  
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

PUBLIC(int) OgUTF8ToUnis(int icode, unsigned char *code, int sunicode, int *iunicode, unsigned short *unicode, int *position, int *truncated)
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
        unicode[j++]=u; 
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
        unicode[j++]=u; 
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
        unicode[j++]=u; 
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
unicode[j]=0;
if (position) position[j>>1]=i;
*iunicode=j;
DONE;
}


