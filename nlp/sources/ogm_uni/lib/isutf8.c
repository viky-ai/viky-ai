/*
 * lowercase uppercase and unaccentuation for UTF-8 strings
 * Copyright (c) 2009 Pertimm by Patrick Constant
 * Dev : August 2004, June 2005, December 2007
 * Version 1.2
 */
#include <loguni.h>


/*
 *  Lowers the utf-8 chaine s1 of length is into s2
 *  s2 can be equal to s1. Size is limited to DPcPathSize.
*/

PUBLIC(int) OgUtf8Strlwr(int is1, unsigned char *s1, int *pis2, unsigned char *s2)
{
int iuni; unsigned char uni[DPcPathSize*2];
if (is1 > DPcPathSize-1) is1=DPcPathSize-1;
IFE(OgCpToUni(is1,s1,DPcPathSize*2,&iuni,uni,DOgCodePageUTF8,0,0));
OgUniStrlwr(iuni,uni,uni);
IFE(OgUniToCp(iuni,uni,DPcPathSize,pis2,s2,DOgCodePageUTF8,0,0));
DONE;
}



/*
 *  Uppers the utf-8 chaine s1 of length is into s2
 *  s2 can be equal to s1.
*/

PUBLIC(int) OgUtf8Strupr(int is1, unsigned char *s1, int *pis2, unsigned char *s2)
{
DONE; 
}



/*
 *  Unaccentuate the utf-8 chaine s1 of length is into s2
 *  s2 can be equal to s1.
*/

PUBLIC(int) OgUtf8Struna(int is1, unsigned char *s1, int *pis2, unsigned char *s2)
{
DONE; 
}



