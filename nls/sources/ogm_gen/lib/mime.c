/*
 *  Encoding and decoding mime-like (we do not parse headers)
 *  Copyright (c) 2000	Ogmios by Patrick Constant
 *  Dev : December 2000
 *  Version 1.0
*/
#include <loggen.h>

/*
 * Basic encoding is very simple : 
 *    0-25 = A-Z
 *   26-51 = a-z
 *   52-61 = 0-9
 *      62 = +
 *      63 = /
 *     pad = =
 * We do not use a table (which would be faster)
 * to keep the encoding and decoding code small
 * See RFC1521.txt for more details on the encoding.
*/ 


static int OgMimeEncode64(unsigned char *, unsigned char *);
static int OgMimeDecode64(unsigned char *, unsigned char *);




/*
 *  Mime encoding of 'base' of length 'ibase' into 'encoded', 
 *  total length of 'encoded' is given in 'iencoded'.
 *  Maximum length of 'encoded' is (ibase*4 + 1)/3 
*/

PUBLIC(int) OgMimeEncode(int ibase, unsigned char *base, int *iencoded, unsigned char *encoded)
{
int i,j;
unsigned char b[3];
for (i=0,j=0; i<ibase; i+=3,j+=4) {
  if (i+3<=ibase) OgMimeEncode64(base+i,encoded+j);
  else if (i+2<=ibase) {
    b[0]=base[ibase-2]; b[1]=base[ibase-1]; b[2]=0;
    OgMimeEncode64(b,encoded+j);
    encoded[j+3]='=';
    }
  else {
    b[0]=base[ibase-1]; b[1]=0; b[2]=0;
    OgMimeEncode64(b,encoded+j);
    encoded[j+2]='=';
    encoded[j+3]='=';
    }
  }
encoded[j] = 0;
*iencoded = j;
DONE;
}




static int OgMimeEncode64(unsigned char *base, unsigned char *encoded)
{
int i; unsigned u, e[4];
u  = base[0]; u <<= 8;
u += base[1]; u <<= 8;
u += base[2];
e[3] = (u & 0x3f); u >>= 6;
e[2] = (u & 0x3f); u >>= 6;
e[1] = (u & 0x3f); u >>= 6;
e[0] = (u & 0x3f);
for (i=0; i<4; i++) {
  if (e[i] < 26) encoded[i] = 'A' + e[i];  
  else if (e[i] < 52) encoded[i] = 'a' + e[i] - 26;  
  else if (e[i] < 62) encoded[i] = '0' + e[i] - 52;   
  else if (e[i] == 62) encoded[i] = '+';  
  else if (e[i] == 63) encoded[i] = '/';  
  /** else is impossible **/
  } 
DONE;
}





/*
 *  Mime encoding of 'encoded' of length 'iencoded' into 'base', 
 *  total length of 'base' is given in 'ibase'.
 *  Maximum length of 'base' is (iencoded*3 + 1)/4 
 *  Does not take into account any char that is not 
 *  in the encoding set of characters.
*/

PUBLIC(int) OgMimeDecode(int iencoded, unsigned char *encoded, int *ibase, unsigned char *base)
{
int i,j,k,end=0;
unsigned char e[4];
for (i=0,j=0,k=0; i<iencoded; i++) {
  int mime_code = 64;
  if ('A' <= encoded[i] && encoded[i] <= 'Z') mime_code = encoded[i] - 'A';  
  else if ('a' <= encoded[i] && encoded[i] <= 'z') mime_code = encoded[i] - 'a' + 26;  
  else if ('0' <= encoded[i] && encoded[i] <= '9') mime_code = encoded[i] - '0' + 52;
  else if (encoded[i]=='+') mime_code=62;
  else if (encoded[i]=='/') mime_code=63;
  else if (encoded[i]=='=') { mime_code=0; end=1; }
  if (mime_code == 64) continue;
  e[j++] = mime_code;
  if (j==4 || end) {
    if (j<4) { end=2; e[j++] = 0; }
    OgMimeDecode64(e,base+k);
    if (end) { k+=3-end; break; }
    else k+=3; 
    j=0;
    }
  }
base[k] = 0;
*ibase = k;
DONE;
}




static int OgMimeDecode64(unsigned char *encoded, unsigned char *base)
{
unsigned u;
u  = encoded[0]; u <<= 6;
u += encoded[1]; u <<= 6;
u += encoded[2]; u <<= 6;
u += encoded[3];
base[2] = (u & 0xff); u >>= 8;
base[1] = (u & 0xff); u >>= 8;
base[0] = (u & 0xff);
DONE;
}


