/*
 *	All protected generic function for Pertimm numbers
 *  also called Ogmios numbers
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev: June 2009
 *	Version 1.0
*/
#include <loggen.h>





PUBLIC(int) OgPninlen(void *herr, unsigned char *p, char *file, int line)
{
char erreur[DOgErrorSize];
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
sprintf(erreur,"OgPninlen overflow at file %s:%d", file, line);
OgErr(herr,erreur); DPcErr;
}



/*
 *  Used specifically when one wants to work on unsigned long number
 *  the type ogrefnum_t is a good example of this use.
*/

PUBLIC(int) OgPninLong(void *herr, unsigned char **pp, long unsigned int *pvalue, char *file, int line)
{
char erreur[DOgErrorSize];
if (sizeof(long) == 4) return(OgPnin4(herr,pp,pvalue,file,line));
else if (sizeof(long) == 8) return(OgPnin8(herr,pp,pvalue,file,line));
sprintf(erreur,"OggNinLong: unknown long size = %d at file %s:%d",sizeof(long), file, line);
OgErr(herr,erreur); DPcErr;
}



/*
 *	Reads a Pertimm number from buffer pointed to by 'p'
 *	Returns this Ogmios number as unsigned in *pvalue
 *	Number read must be less than 4 bytes
 *	returns -1 on error and 0 otherwise
*/

PUBLIC(int) OgPnin4(void *herr, unsigned char **pp, unsigned int *pvalue, char *file, int line)
{
char erreur[DOgErrorSize];
unsigned x,value;

if ( *(*pp) < 0x80) value = *((*pp)++);
else if ( *(*pp) < 0xC0) {
  x = *((*pp)++) & 0x3F ; x = x << 8;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xE0) {
  x = *((*pp)++) & 0x1F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xF0) {
  x = *((*pp)++) & 0x0F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xF1) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ; /* x=0 after that line */
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else {
  sprintf(erreur,"OgPnin4 overflow at file %s:%d", file, line);
  *pvalue=0; OgErr(herr,erreur); DPcErr;
  }
*pvalue = value;
DONE;
}





/*
 *	Reads an Ogmios number from buffer pointed to by 'p'
 *	Returns this Ogmios number as unsigned
 *	Number read must be less than 8 bytes
 *	Exits program on error.
*/

PUBLIC(int) OgPnin8(void *herr, unsigned char **pp, long unsigned int *pvalue, char *file, int line)
{
char erreur[DOgErrorSize];
unsigned long x,value;

if ( *(*pp) < 0x80) value = *((*pp)++);
else if ( *(*pp) < 0xC0) {
  x = *((*pp)++) & 0x3F ; x = x << 8;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xE0) {
  x = *((*pp)++) & 0x1F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xF0) {
  x = *((*pp)++) & 0x0F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xF8) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xFC) {
  x = *((*pp)++) & 0x03 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xFE) {
  x = *((*pp)++) & 0x01 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xFF) {
  (*pp)++;
  x = *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else {  
  (*pp)++;
  if ( *(*pp) < 0x80) {
    x = *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    value = x + *((*pp)++);
    }
  else if ( *(*pp) < 0x81) {
    x = *((*pp)++) & 0x07 ; x = x << 8 ; /* x=0 after that line */
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    x = x + *((*pp)++) ; x = x << 8 ;
    value = x + *((*pp)++);
    }
  else {
    sprintf(erreur,"OgPnin8 overflow at file %s:%d", file, line);
    *pvalue=0; OgErr(herr,erreur); DPcErr;
    }
  }
*pvalue = value;
DONE;
}




/*
 *	Same as OggNin8, but using type ogint64_t
*/

PUBLIC(int) OgPnin64(void *herr, unsigned char **pp, ogint64_t *pvalue, char *file, int line)
{
char erreur[DOgErrorSize];
ogint64_t x,value;

if ( *(*pp) < 0x80) value = *((*pp)++);
else if ( *(*pp) < 0xC0) {
  x = *((*pp)++) & 0x3F ; x = x << 8;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xE0) {
  x = *((*pp)++) & 0x1F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xF0) {
  x = *((*pp)++) & 0x0F ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xF8) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xFC) {
  x = *((*pp)++) & 0x03 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xFE) {
  x = *((*pp)++) & 0x01 ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0xFF) {
  (*pp)++;
  x = *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
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
  value = x + *((*pp)++);
  }
else if ( *(*pp) < 0x81) {
  x = *((*pp)++) & 0x07 ; x = x << 8 ; /* x=0 after that line */
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  x = x + *((*pp)++) ; x = x << 8 ;
  value = x + *((*pp)++);
  }
else {
  sprintf(erreur,"OgPnin64 overflow at file %s:%d", file, line);
  *pvalue=0; OgErr(herr,erreur); DPcErr;
  }
*pvalue = value;
DONE;  
}



