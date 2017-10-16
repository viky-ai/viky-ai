/*
 *	All generic function for Ogmios numbers
 *	Copyright (c) 1998	Ogmios by Patrick Constant
 *	Dev : August 1998
 *	Version 1.0
*/
#include <loggen.h>



/*
 *	Gets Ogmios number length of 'v'
*/

PUBLIC(int) OggNlenLong(long unsigned int v)
{
char erreur[DPcSzErr];
if (sizeof(long) == 4) return(OggNlen((unsigned)v));
else if (sizeof(long) == 8) return(OggNlen8(v));
sprintf(erreur,"OggNlenLong: unknown long size = %d",sizeof(long));
DPcErr;
}



PUBLIC(int) OggNlen(unsigned int v)
{
if ( v < 0x80) return(1);
else if (v < 0x4000) return(2);
else if (v < 0x200000) return(3);
else if (v < 0x10000000) return(4);
else return(5);
}



PUBLIC(int) OggNlen8(long unsigned int v)
{
if ( v < 0x80) return(1);
else if (v < 0x4000) return(2);
else if (v < 0x200000) return(3);
else if (v < 0x10000000) return(4);
else if (v < 0x800000000) return(5);
else if (v < 0x40000000000) return(6);
else if (v < 0x2000000000000) return(7);
else if (v < 0x100000000000000) return(8);
else if (v < 0x8000000000000000) return(9);
else return(10);
}




PUBLIC(int) OggNinlen(unsigned char *p)
{
if ( *p < 0x80) return(1);
else if (*p < 0xC0) return(2);
else if (*p < 0xE0) return(3);
else if (*p < 0xF0) return(4);
else if (*p < 0xF8) return(5);
else if (*p < 0xFC) return(6);
else if (*p < 0xFE) return(7);
else if (*p < 0xFF) return(8);
else if (*( p + 1) < 0x80) return(9);
else if (*( p + 1) < 0xC0) return(10);
PcErr(-1,"OggNinlen overflow"); DPcErr;
}




PUBLIC(int) OggNoutLong(long unsigned int v, unsigned char **pp)
{
if (sizeof(long) == 4) OggNout((unsigned)v,pp);
else if (sizeof(long) == 8) OggNout8(v,pp);
else {
  char erreur[DPcSzErr];
  sprintf(erreur,"OggNinLong: unknown long size = %d",sizeof(long));
  DPcErr;
  }
DONE;
}


/*
 *	Writes 'v' into buffer pointed to by 'p'
*/

PUBLIC(void) OggNout(unsigned int v, unsigned char **pp)
{
unsigned x;
unsigned char *p = (*pp);

if ( v < 0x80) {
  *p=v; p++;
  }
else if (v < 0x4000) {
  x = v >> 8; *(p++)=x+0x80; *(p++)=(v & 0xFF);
  }
else if (v < 0x200000) {
  x = v >> 16; *(p++)=x+0xC0;
  x = v >> 8; *(p++)=(x & 0xFF);
  *(p++)=(v & 0xFF);
  }
else if (v < 0x10000000) {
  x = v >> 24; *(p++)=x+0xE0;
  x = v >> 16; *(p++)=(x & 0xFF);
  x = v >> 8; *(p++)=(x & 0xFF);
  *(p++)=(v & 0xFF);
  }
else {
  *(p++)=0xF0;
  x =  v >> 24; *(p++)=(x & 0xFF);
  x = v >> 16; *(p++)=(x & 0xFF);
  x = v >> 8; *(p++)=(x & 0xFF);
  *(p++)=(v & 0xFF);
  }
*pp = p;
}




/*
 *	Writes 'v' into buffer pointed to by 'p'
*/

PUBLIC(void) OggNout8(long unsigned int v, unsigned char **pp)
{
unsigned long x;
unsigned char *p = (*pp);

if ( v < 0x80) {
  *p=(unsigned char)(v); p++;
  }
else if (v < 0x4000) {
  x = v >> 8; *(p++)=(unsigned char)(x+0x80); *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x200000) {
  x = v >> 16; *(p++)=(unsigned char)(x+0xC0);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x10000000) {
  x = v >> 24; *(p++)=(unsigned char)(x+0xE0);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x800000000) {
  x = v >> 32; *(p++)=(unsigned char)(x+0xF0);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x40000000000) {
  x = v >> 40; *(p++)=(unsigned char)(x+0xF8);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x2000000000000) {
  x = v >> 48; *(p++)=(unsigned char)(x+0xFC);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x100000000000000) {
  x = v >> 56; *(p++)=(unsigned char)(x+0xFE);
  x = v >> 48; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x8000000000000000) {
  *(p++)=0xFF;
  x = v >> 56; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 48; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else {
  *(p++)=0xFF; *(p++)=0x80;
  x = v >> 56; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 48; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
*pp = p;
}





/*
 *	Writes 'v' into buffer pointed to by 'p'
*/

PUBLIC(void) OggNout64(ogint64_t v, unsigned char **pp)
{
ogint64_t x;
unsigned char *p = (*pp);

if ( v < 0x80) {
  *p=(unsigned char)(v); p++;
  }
else if (v < 0x4000) {
  x = v >> 8; *(p++)=(unsigned char)(x+0x80); *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x200000) {
  x = v >> 16; *(p++)=(unsigned char)(x+0xC0);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x10000000) {
  x = v >> 24; *(p++)=(unsigned char)(x+0xE0);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x800000000) {
  x = v >> 32; *(p++)=(unsigned char)(x+0xF0);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x40000000000) {
  x = v >> 40; *(p++)=(unsigned char)(x+0xF8);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x2000000000000) {
  x = v >> 48; *(p++)=(unsigned char)(x+0xFC);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x100000000000000) {
  x = v >> 56; *(p++)=(unsigned char)(x+0xFE);
  x = v >> 48; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else if (v < 0x8000000000000000) {
  *(p++)=0xFF;
  x = v >> 56; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 48; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
else {
  *(p++)=0xFF; *(p++)=0x80;
  x = v >> 56; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 48; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 40; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 32; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 24; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 16; *(p++)=(unsigned char)(x & 0xFF);
  x = v >> 8; *(p++)=(unsigned char)(x & 0xFF);
  *(p++)=(unsigned char)(v & 0xFF);
  }
*pp = p;
}




/*
 *  Used specifically when one wants to work on unsigned long number
 *  the type ogrefnum_t is a good example of this use.
*/

PUBLIC(unsigned long) OggNinLong(unsigned char **pp)
{
if (sizeof(long) == 4) return(OggNin4(pp));
else if (sizeof(long) == 8) return(OggNin8(pp));
else {
  char erreur[DPcSzErr];
  sprintf(erreur,"OggNinLong: unknown long size = %d",sizeof(long));
  return((unsigned long)-1);
  }
}



/*
 *	Reads an Ogmios number from buffer pointed to by 'p'
 *	Returns this Ogmios number as unsigned
 *	Number read must be less than 4 bytes
 *	Exits program on error.
*/

PUBLIC(unsigned) OggNin4(unsigned char **pp)
{
unsigned x;

if ( *(*pp) < 0x80) return *((*pp)++);
if ( *(*pp) < 0xC0) {
  x = *((*pp)++) & 0x3F ; x = x << 8;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xE0) {
  x = *((*pp)++) & 0x1F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xF0) {
  x = *((*pp)++) & 0x0F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xF1) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ; /* x=0 after that line */
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }

PcErr(-1,"OggNin4 overflow"); return((unsigned)-1);
}





/*
 *	Reads an Ogmios number from buffer pointed to by 'p'
 *	Returns this Ogmios number as unsigned
 *	Number read must be less than 8 bytes
 *	Exits program on error.
*/

PUBLIC(unsigned long) OggNin8(unsigned char **pp)
{
unsigned long x;

if ( *(*pp) < 0x80) return *((*pp)++);
if ( *(*pp) < 0xC0) {
  x = *((*pp)++) & 0x3F ; x = x << 8;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xE0) {
  x = *((*pp)++) & 0x1F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xF0) {
  x = *((*pp)++) & 0x0F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xF8) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xFC) {
  x = *((*pp)++) & 0x03 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xFE) {
  x = *((*pp)++) & 0x01 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xFF) {
  (*pp)++;
  x = *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
(*pp)++;
if ( *(*pp) < 0x80) {
  x = *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0x81) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ; /* x=0 after that line */
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }

PcErr(-1,"OggNin8 overflow"); return((unsigned long)-1);
}




/*
 *	Same as OggNin8, but using type ogint64_t
*/

PUBLIC(ogint64_t) OggNin64(unsigned char **pp)
{
ogint64_t x;

if ( *(*pp) < 0x80) return *((*pp)++);
if ( *(*pp) < 0xC0) {
  x = *((*pp)++) & 0x3F ; x = x << 8;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xE0) {
  x = *((*pp)++) & 0x1F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xF0) {
  x = *((*pp)++) & 0x0F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xF8) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xFC) {
  x = *((*pp)++) & 0x03 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xFE) {
  x = *((*pp)++) & 0x01 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0xFF) {
  (*pp)++;
  x = *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
(*pp)++;
if ( *(*pp) < 0x80) {
  x = *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }
if ( *(*pp) < 0x81) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ; /* x=0 after that line */
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  return x + *((*pp)++);
  }

PcErr(-1,"OggNin64 overflow"); return((ogint64_t)-1);
}




/*
 *	Writes 'v' into buffer pointed to by 'p' on fb bytes
 *  Writes 'v' in small endian, i.e. small order byte
 *  at the end (reads like a written number)
 *  fb is 2, 3 or 4 exclusively. Returns 0.
 *  Returns -1 on error.
*/

PUBLIC(int) OggNormNout(unsigned int v, unsigned int fb, unsigned char **pp)
{
unsigned x;
int is_erreur = 0;
char erreur[DPcSzErr];
unsigned char *p = (*pp);

switch(fb) {
  case 1:
    if (v > 0xff) {
      sprintf(erreur,"OggNormNout impossible value %x too big\n",v);
      is_erreur = 1; break;
      }
    *(p++)=v;
    break;
  case 2:
    if (v > 0xffff) {
      sprintf(erreur,"OggNormNout impossible value %x too big\n",v);
      is_erreur = 1; break;
      }
    x = v >> 8; *(p++) = x; *(p++)=(v & 0xFF);
    break;
  case 3:
    if (v > 0xffffff) {
      sprintf(erreur,"OggNormNout impossible value %x too big\n",v);
      is_erreur = 1; break;
      }
    x = v >> 16; *(p++)=x;
    x = v >> 8; *(p++)=(x & 0xFF);
    *(p++)=(v & 0xFF);
    break;
  case 4:
    if (v > 0xffffffff) {
      sprintf(erreur,"OggNormNout impossible value %x too big\n",v);
      is_erreur = 1; break;
      }
    x = v >> 24; *(p++)=x;
    x = v >> 16; *(p++)=(x & 0xFF);
    x = v >> 8; *(p++)=(x & 0xFF);
    *(p++)=(v & 0xFF);
    break;
  default:
    sprintf(erreur,"OggNormNout impossible value %x too big\n",v);
    is_erreur = 1;
  }

if (is_erreur) { PcErr(-1,erreur); DPcErr; }
*pp = p;
DONE;
}






PUBLIC(int) OggNormNin(unsigned int fb, unsigned char **pp)
{
unsigned i, x = 0;
unsigned char *p = (*pp);

x = *(p++);
for (i=1; i<fb; i++) {
  x = x << 8;
  x = x + *(p++);
  }
*pp = p;
return(x);
}



