/*
 *	Handling UTF8-Percent transformations.
 *	Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *	Dev : September 2005, November 2007
 *	Version 1.1
*/
#include <loguni.h>
#include <loggen.h>



/*
 *  Encoding an UTF8 string into %xx string
 *  example: été -> %C3%A9t%C3%A9
 *  According to RFC 2279 rules are:
 *   UCS-4 range (hex.)           UTF-8 octet sequence (binary)
 *   0000 0000-0000 007F   0xxxxxxx
 *   0000 0080-0000 07FF   110xxxxx 10xxxxxx
 *   0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx 
*/

PUBLIC(int) OgCpToPercent(icode,code,spercent,ipercent,percent,truncated)
int icode; unsigned char *code;
int spercent,*ipercent; unsigned char *percent;
int *truncated;
{
int i,j,c;

if (truncated) *truncated=0;

for (i=j=0; i<icode; i++) {
  c = code[i];
  if (c <= 0x7f) {
    percent[j++]=c;
    }
  else {
    sprintf(percent+j,"%%%2X",c); j += strlen(percent+j);
    }
  if (j+3>=spercent) {
    if (i+1<icode) { if (truncated) *truncated=1; }
    break;
    }
  }

percent[j]=0;
*ipercent=j;

DONE;
}




PUBLIC(int) OgPercentToCp(ipercent,percent,scode,icode,code,truncated)
int ipercent; unsigned char *percent;
int scode,*icode; unsigned char *code;
int *truncated;
{
unsigned char *nil,sx[3];
int i,j,c,x;

if (truncated) *truncated=0;

for (i=j=0; i<ipercent; i++) {
  c = percent[i];
  if (i+2<ipercent && c=='%' && isxdigit(percent[i+1]) && isxdigit(percent[i+2])) {
    memcpy(sx,percent+i+1,2); sx[2]=0;
    x=strtol(sx,&nil,16);
    code[j++]=x;
    i+=2;
    } 
  else {
    code[j++]=c;
    }
  if (j+1>=scode) {
    if (truncated) *truncated=1;
    break;
    }
  }

code[j]=0;
*icode=j;

DONE;
}
