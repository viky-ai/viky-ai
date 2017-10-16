/*
 *	Calculates a size from a string (generally, an argument string)
 *	Copyright (c) 1999-2000	Ogmios by Patrick Constant
 *	Dev : December 1999, February 2000
 *	Version 1.1
*/
#include <stdlib.h>
#include <loggen.h>



PUBLIC(size_t) OgArgSize(char *s)
{
int length;
size_t factor, size;
length = strlen(s); factor = 1;
if (s[length-1]=='k' || s[length-1]=='K') factor=0x400;
else if (s[length-1]=='m' || s[length-1]=='M') factor=0x100000;
else if (s[length-1]=='g' || s[length-1]=='G') factor=0x40000000;
size=atoi(s)*factor;
return(size);
}



PUBLIC(int) OgArgPeriod(char *s)
{
int length, factor, seconds;
length = strlen(s); factor = 1;
if (s[length-1]=='s' || s[length-1]=='S') factor=1;
else if (s[length-1]=='m' || s[length-1]=='M') factor=60;
else if (s[length-1]=='h' || s[length-1]=='H') factor=3600;
else if (s[length-1]=='d' || s[length-1]=='d') factor=3600*24;
else if (s[length-1]=='w' || s[length-1]=='W') factor=3600*24*7;
seconds=atoi(s)*factor;
return(seconds);
}



PUBLIC(int) OgBigArgSize(char *s, struct og_bnum *big_size)
{
int length, factor, big_factor;
length = strlen(s); factor = 1; big_factor = 1;
if (s[length-1]=='k' || s[length-1]=='K') factor=0x400;
else if (s[length-1]=='m' || s[length-1]=='M') factor=0x100000;
else if (s[length-1]=='g' || s[length-1]=='G') factor=0x40000000;
else if (s[length-1]=='t' || s[length-1]=='T') { factor=0x40000000; big_factor=0x400; }
IFE(OgSetBnum(big_size, atoi(s), 0, 0));
IFE(OgMulBnum(big_size,factor,0));
if (big_factor > 1) {
  IFE(OgMulBnum(big_size,big_factor,0));
  }
DONE;
}




PUBLIC(ogint64_t) OgArgSize64(char *s)
{
int length;
ogint64_t factor, size;
length = strlen(s); factor = 1;
if (s[length-1]=='k' || s[length-1]=='K') factor=0x400;
else if (s[length-1]=='m' || s[length-1]=='M') factor=0x100000;
else if (s[length-1]=='g' || s[length-1]=='G') factor=0x40000000;
else if (s[length-1]=='t' || s[length-1]=='T') { factor=0x10000000000; }
size=atoi(s)*factor;
return(size);
}





PUBLIC(int) OgFormatThousand(unsigned int n, char *c)
{
char buffer[50];
char reverse[50];
int i,j,k,length;

sprintf(buffer,"%u",n);
length = strlen(buffer);
for (i=length-1,j=0,k=0; i>=0; i--) {
  if (k && !(k%3)) reverse[j++] = ' ';
  reverse[j++] = buffer[i]; k++;
  }
length = j;
for (i=0; i<length; i++) {
  c[length-1-i] = reverse[i];
  }
c[length] = 0;
DONE;
}




PUBLIC(int) OgBigFormatThousand(struct og_bnum *bnum, char *c)
{
struct og_bnum bnumd,bnumr,bnum1;
char buffer[1024];
char reverse[1024];
int i,j,k,length;
unsigned d,r;

bnum1 = *bnum;
buffer[0] = 0;

do {
  IFE(OgDivBnum(&bnumd,&bnumr,&bnum1,10,0));
  IFE(OgBnumToUnsigned(&bnumr,&r,1));
  IFE(OgBnumToUnsigned(&bnumd,&d,1));
  sprintf(buffer+strlen(buffer),"%d",r);
  bnum1 = bnumd; 
  }
while(d);

length = strlen(buffer);
for (i=j=k=0; i<length; i++) {
  if (k && !(k%3)) reverse[j++] = ' ';
  reverse[j++] = buffer[i]; k++;
  }
reverse[length=j] = 0;
for (i=length-1,j=0; i>=0; i--) c[j++] = reverse[i]; c[j] = 0;

DONE;
}




PUBLIC(int) Og64FormatThousand(ogint64_t n, char *c, int space)
{
char buffer[1024];
char reverse[1024];
ogint64_t n1,nd,nr;
int i,j,k,length;
unsigned r;

n1 = n;
buffer[0] = 0;

do {
  nd = n1/10; nr = n1%10;
  r = (unsigned)nr;
  sprintf(buffer+strlen(buffer),"%d",r);
  n1 = nd; 
  }
while(nd);

length = strlen(buffer);
for (i=j=k=0; i<length; i++) {
  if (space) { if (k && !(k%3)) reverse[j++] = ' '; }
  reverse[j++] = buffer[i]; k++;
  }
reverse[length=j] = 0;
for (i=length-1,j=0; i>=0; i--) c[j++] = reverse[i]; c[j] = 0;

DONE;
}





/*
 * Convert string 's' to a ogint64_t value.
 * base can go from 2 to 16. 's' can contain
 * spaces.
*/

PUBLIC(ogint64_t) Ogstrto64(char *s, int base)
{
ogint64_t v=0,a;
int i, c, is=strlen(s);

for (i=0; i<is; i++) {
  c=s[i]; if (isspace(c)) continue;
  if ('0' <= c && c <= '9') { a = c-'0'; }
  else if (base > 10 && 'A' <= c && c <= 'F') {  a = 10+c-'A'; }
  else if (base > 10 && 'a' <= c && c <= 'f') {  a = 10+c-'a'; }
  else break;
  if (v > 0xffffffffffffffff/base) { v= 0xffffffffffffffff; break; }
  v *= base;
  if (v > 0xffffffffffffffff - a)  { v= 0xffffffffffffffff; break; }
  v += a;
  }

return(v);
}



