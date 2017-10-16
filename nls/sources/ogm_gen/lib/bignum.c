/*
 *  Function that work on big numbers 
 *	Copyright (c) 2000 Ogmios by Patrick Constant
 *	Dev : May 2000
 *	Version 1.0
*/
#include <loggen.h>



static int OgMulBnum16(struct og_bnum *, unsigned);




PUBLIC(int) OgSetBnum(struct og_bnum *bnum, unsigned int n, int minus, int ifrom)
{
char erreur[DPcSzErr];

if (ifrom+1 >= DOgMaxBnum) {
  DPcSprintf(erreur,"OgSetBnum: too big ifrom (%d) max is %d",ifrom,DOgMaxBnum-2);
  PcErr(-1,erreur); DPcErr;
  }

memset(bnum,0,sizeof(struct og_bnum));
bnum->n[ifrom] = n & 0xffff;
bnum->n[ifrom+1] = n >> 16;
bnum->minus = minus;
DONE;
}





PUBLIC(int) OgCopyBnum(struct og_bnum *bnum1, struct og_bnum *bnum2)
{
memcpy(bnum1,bnum2,sizeof(struct og_bnum));
DONE;
}





/*
 *  Compare absolute values for bnum1 and bnum2 and returns
 *  1 if bnum1 > bnum2, -1 else if bnum2 > bnum1, else returns 0. 
*/

PUBLIC(int) OgAcmpBnums(struct og_bnum *bnum1, struct og_bnum *bnum2)
{
int i;
for (i=DOgMaxBnum-1; i>=0; i--) {
  if (bnum1->n[i] > bnum2->n[i]) return(1);
  else if (bnum1->n[i] < bnum2->n[i]) return(-1);
  }
return(0);
}




/*
 *  Adds bnum1 and bnum2, result in bnum.
*/

PUBLIC(int) OgAddBnums(struct og_bnum *bnum, struct og_bnum *bnum1, struct og_bnum *bnum2)
{
int i;
char erreur[DPcSzErr];
unsigned result, remain=0;

memset(bnum,0,sizeof(struct og_bnum));

if (bnum1->minus == bnum2->minus) {
  for (i=0; i<DOgMaxBnum; i++) {
    result = bnum1->n[i] + bnum2->n[i] + remain;
    remain = result >> 16;
    bnum->n[i] = (unsigned short)(result & 0xffff);
    }
  if (remain) {
    DPcSprintf(erreur,"OgAddBnums: overflow on result");
    PcErr(-1,erreur); DPcErr;
    }
  }
else {
  struct og_bnum *bnuma,*bnumb;
  if (OgAcmpBnums(bnum1,bnum2) >= 0) {
    bnuma = bnum1; bnumb = bnum2;
    }
  else {
    bnuma = bnum2; bnumb = bnum1;
    }
  for (i=0; i<DOgMaxBnum; i++) {
    if (bnuma->n[i] >= bnumb->n[i]+remain) {
      result = bnuma->n[i] - (bnumb->n[i]+remain);
      remain = 0;
      }
    else {
      result = 0x10000 + bnuma->n[i] - (bnumb->n[i]+remain);
      remain = 1;
      }
    bnum->n[i] = result;
    }
  bnum->minus = bnuma->minus;
  }
DONE;
}




PUBLIC(int) OgSlideBnum(struct og_bnum *bnum, int slide)
{
int i;
char erreur[DPcSzErr];
int start = DOgMaxBnum-slide;

if (start < 0) {
  DPcSprintf(erreur,"OgSlideBnum: slide %d too big",slide);
  PcErr(-1,erreur); DPcErr;
  }
for (i=start; i<DOgMaxBnum; i++) {
  if (bnum->n[i] != 0) {
    DPcSprintf(erreur,"OgSlideBnum: number will be truncated with slide %d",slide);
    PcErr(-1,erreur); DPcErr;
    }
  }

for (i=start-1; i>=0; i--) {
  bnum->n[i+slide] = bnum->n[i];
  }

for (i=0; i<slide; i++) bnum->n[i]=0;

DONE;
}





static int OgMulBnum16(struct og_bnum *bnum, unsigned int n)
{
int i;
unsigned result;
unsigned remain;
char erreur[DPcSzErr];

if (n > 0xffff) {
  DPcSprintf(erreur,"OgMulBnum16: overflow 0x%x > 0xffff, use OgMulBnum",n);
  PcErr(-1,erreur); DPcErr;
  }
remain = 0;

for (i=0; i<DOgMaxBnum; i++) {
  result = n * bnum->n[i] + remain;
  remain = result >> 16;

  bnum->n[i] = (unsigned short)(result & 0xffff);

  if (remain && i+1 == DOgMaxBnum) {
    DPcSprintf(erreur,"OgMulBnum16: overflow for size %d",DOgMaxBnum);
    PcErr(-1,erreur); DPcErr;
    }
  }
DONE;
}





PUBLIC(int) OgMulBnum(struct og_bnum *bnum, unsigned int n, int minus)
{
struct og_bnum bnum1;
struct og_bnum bnum2;
unsigned n1,n2;

n1 = n & 0xffff;
n2 = n >> 16;

memcpy(&bnum1,bnum,sizeof(struct og_bnum));
IFE(OgMulBnum16(&bnum1,n1));

memcpy(&bnum2,bnum,sizeof(struct og_bnum));
IFE(OgMulBnum16(&bnum2,n2));

IFE(OgSlideBnum(&bnum2,1));

IFE(OgAddBnums(bnum,&bnum1,&bnum2));

if (minus == bnum->minus) bnum->minus=0;
else bnum->minus=1;

DONE;
}






PUBLIC(int) OgAddBnum(struct og_bnum *bnum, unsigned int n, int minus)
{
struct og_bnum bnum1;
struct og_bnum bnumr;
IFE(OgSetBnum(&bnum1,n,minus,0));
IFE(OgAddBnums(&bnumr,bnum,&bnum1));
memcpy(bnum,&bnumr,sizeof(struct og_bnum));
DONE;
}





PUBLIC(int) OgAddBnus(struct og_bnum *bnum1, struct og_bnum *bnum2)
{
struct og_bnum bnumr;
IFE(OgAddBnums(&bnumr,bnum1,bnum2));
memcpy(bnum1,&bnumr,sizeof(struct og_bnum));
DONE;
}




/*
 * B is the hexadecimal string representation of the bnum.
*/

PUBLIC(int) OgBnumToStr(struct og_bnum *bnum, int iB, char *B)
{
int i,start=1,print;
if (iB < DOgMaxBnum*5 + 10) {
  sprintf(B,"buffer too short: %d < %d",iB,DOgMaxBnum*5 + 10);
  DONE;
  }
sprintf(B,"%s",(bnum->minus)?"-":"");
for (i=DOgMaxBnum-1; i>=0; i--) {
  if (start) {
    IFn(bnum->n[i]) print=0; 
    else { start=0; print=1; } 
    }
  else print=1;
  if (print) { sprintf(B+strlen(B),"%.4x",bnum->n[i]); }
  }
DONE;
}




/*
 * B is the hexadecimal string representation of the bnum.
*/

PUBLIC(int) OgStrToBnum(char *B, struct og_bnum *bnum)
{
int i,iB=strlen(B);
int iend=iB,ibnum=0;
char buffer[16],*nil;

OgSetBnum(bnum,0,0,0);

for (i=iB-1; i>=0; i--) {
  if (iend-i == 4) {
    memcpy(buffer,B+i,4); buffer[4]=0;
    bnum->n[ibnum++]=(unsigned short)strtol(buffer,&nil,16);
    iend=i;
    }
  }
if (iend>0) {
  memcpy(buffer,B,iend); buffer[iend]=0;
  bnum->n[ibnum++]=(unsigned short)strtol(buffer,&nil,16);
  }

DONE;
}





/*
 *  Integer division of bnum1 by bnum2 result in bnumd, rest of division in bnumr
 *  sign of bnumd is sign of bnum1 * bnum2, sign of bnumr is sign if bnum1.
*/



PUBLIC(int) OgDivBnums(struct og_bnum *bnumd, struct og_bnum *bnumr, struct og_bnum *bnum1, struct og_bnum *bnum2)
{
unsigned division;
char erreur[DPcSzErr];
struct og_bnum b1,b2;
int i,start1,start2;

memset(bnumd,0,sizeof(struct og_bnum));
memset(bnumr,0,sizeof(struct og_bnum));
memcpy(&b1,bnum1,sizeof(struct og_bnum));

bnumr->minus = bnum1->minus; 
if (bnum1->minus == bnum2->minus) bnumd->minus=0;
else bnumd->minus=1;

for (i=DOgMaxBnum-1; i>=0; i--) if (bnum2->n[i]) break;
if (i<0) {
  DPcSprintf(erreur,"OgDivBnums: impossible to divide by zero");
  PcErr(-1,erreur); DPcErr;
  }
start2=i;

while(1) {
  for (i=DOgMaxBnum-1; i>=0; i--) if (b1.n[i]) break;
  if (i<0) DONE;
  start1=i;
  if (start2 > start1) {
    memcpy(bnumr,&b1,sizeof(struct og_bnum));
    DONE;
    }
  division = b1.n[start1] / bnum2->n[start2];
  if (division==0) {
    if (start1 == start2) {
      memcpy(bnumr,&b1,sizeof(struct og_bnum));
      DONE;
      }
    if (start1-1 >= 0) {
      /* This operation must be separated, because otherwise it works with integers 
       * and creates errors because of falsely negative values. 
       * division = ((b1.n[start1]<<16) + b1.n[start1-1]) / bnum2->n[start2]; */
      division = b1.n[start1]<<16;
      division += b1.n[start1-1];
      division /= bnum2->n[start2];
      start1--;
      }
    else { 
      /** Here, start1 == 0 **/
      bnumr->n[start1] = b1.n[start1]; DONE; 
      }
    }
  memcpy(&b2,bnum2,sizeof(struct og_bnum));
  IFE(OgMulBnum16(&b2,division));
  IFE(OgSlideBnum(&b2,start1 - start2));

  while (OgAcmpBnums(&b1,&b2) < 0) {
    division--;
    if (division <= 0) {
      memcpy(bnumr,&b1,sizeof(struct og_bnum));
      DONE;
      }
    memcpy(&b2,bnum2,sizeof(struct og_bnum));
    IFE(OgMulBnum16(&b2,division));
    IFE(OgSlideBnum(&b2,start1 - start2));
    }
  b2.minus=1;
  IFE(OgAddBnus(&b1,&b2));
  bnumd->n[start1 - start2]=division;
  }

DONE;
}





PUBLIC(int) OgDivBnum(struct og_bnum *bnumd, struct og_bnum *bnumr, struct og_bnum *bnum1, unsigned int n, int minus)
{
struct og_bnum bnum2;
IFE(OgSetBnum(&bnum2,n,minus,0));
return(OgDivBnums(bnumd,bnumr,bnum1,&bnum2));
DONE;
}





PUBLIC(int) OgMinusBnum(struct og_bnum *bnum)
{
bnum->minus = (bnum->minus?0:1);
DONE;
}




/*
 *  Translates an big number to an unsigned int
 *  when it is possible. minus sign is an error.
 *  if 'truncation!=0' then value if bigger
 *  is truncated to biggest unsigned value.
 *  and function returns 1. Else bigger value
 *  than biggest unsigned value implies an error.
 *  Also, if 'truncation!=0' minus sign is not taken 
 *  into account, else error on minus sign.
*/

PUBLIC(int) OgBnumToUnsigned(struct og_bnum *bnum, unsigned int *n, int truncation)
{
int i;
char erreur[DPcSzErr];
if (bnum->minus && !truncation) {
  DPcSprintf(erreur,"OgBnumToUnsigned: minus big number");
  PcErr(-1,erreur); DPcErr;
  }
for (i=DOgMaxBnum-1; i>=0; i--) if (bnum->n[i]) break;
if (i>1) {
  if (truncation) {
    *n = 0xffffffff;
    return(1);
    }
  else {
    DPcSprintf(erreur,"OgBnumToUnsigned: too big big number");
    PcErr(-1,erreur); DPcErr;
    }
  }
*n = (bnum->n[1]<<16) + bnum->n[0];
return(0);
}





/*
 *  Translates an big number to an integer when it is possible.
 *  if 'truncation!=0' then value if bigger
 *  is truncation to biggest int value.
 *  and function returns 1. Else bigger value
 *  than biggest int value implies an error.
*/

PUBLIC(int) OgBnumToInt(struct og_bnum *bnum, int *n, int truncation)
{
int i;
char erreur[DPcSzErr];
for (i=DOgMaxBnum-1; i>=0; i--) if (bnum->n[i]) break;
if (i>1 || bnum->n[1]>0x7fff) {
  if (truncation) {
    *n = 0x7fffffff;
    if (bnum->minus) *n = -(*n);
    return(1);
    }
  else {
    DPcSprintf(erreur,"OgBnumToInt: too big big number");
    PcErr(-1,erreur); DPcErr;
    }
  }
*n = (bnum->n[1]<<16) + bnum->n[0];
if (bnum->minus) *n = -(*n);
return(0);
}





PUBLIC(int) OgHighLowToBnum(unsigned int high, unsigned int low, struct og_bnum *bnum)
{
IFE(OgSetBnum(bnum,high,0,2));
IFE(OgAddBnum(bnum,low,0));
DONE;
}



#if (DPcSystem == DPcSystemWin32)

PUBLIC(int) OgLong64ToBnum(n,bnum)
__int64 n; struct og_bnum *bnum;
{
unsigned high,low;
high = (unsigned)(n>>32);
low = (unsigned)(n & 0xffffffff);
return(OgHighLowToBnum(high,low,bnum));
DONE;
}

#endif



PUBLIC(int) OgBnumToHighLow(struct og_bnum *bnum, unsigned int *high, unsigned int *low, int truncation)
{
int i;
char erreur[DPcSzErr];
for (i=DOgMaxBnum-1; i>=0; i--) if (bnum->n[i]) break;
if (i>3) {
  if (truncation) {
    *high = 0xffffffff;
    *low = 0xffffffff;
    return(1);
    }
  else {
    DPcSprintf(erreur,"OgBnumToHighLow: too big big number");
    PcErr(-1,erreur); DPcErr;
    }
  }
*high = (bnum->n[3]<<16) + bnum->n[2];
*low = (bnum->n[1]<<16) + bnum->n[0];
return(0);
}





PUBLIC(int) OgBnumIsZero(struct og_bnum *bnum)
{
int i;
for (i=0; i<DOgMaxBnum; i++) {
  if (bnum->n[i]) return(0);
  }
return(1);
}





PUBLIC(int) OgBnumToDouble(struct og_bnum *bnum, double *n)
{
int i,j; 
double dnum;

*n = 0;
for (i=DOgMaxBnum-1; i>=0; i--) { 
  if (bnum->n[i]==0) continue;
   dnum = bnum->n[i];
   for (j=i; j>0; j--) {
     dnum *= 0x10000;
     }   
  *n += dnum;
  }
if (bnum->minus) *n = -(*n);
DONE;
}



#if (DPcSystem == DPcSystemWin32)

PUBLIC(int) OgBnumToLong64(bnum,n,truncation)
struct og_bnum *bnum; __int64 *n;
int truncation;
{
int i;
char erreur[DPcSzErr];
for (i=DOgMaxBnum-1; i>=0; i--) if (bnum->n[i]) break;
if (i>3 || bnum->n[3]>0x7fff) {
  if (truncation) {
    *n = (__int64)0x7fffffffffffffff;
    if (bnum->minus) *n = -(*n);
    return(1);
    }
  else {
    DPcSprintf(erreur,"OgBnumToInt: too big big number");
    PcErr(-1,erreur); DPcErr;
    }
  }
*n  = bnum->n[3]; *n <<= 16;
*n += bnum->n[2]; *n <<= 16;
*n += bnum->n[1]; *n <<= 16;
*n += bnum->n[0];
if (bnum->minus) *n = -(*n);
return(0);
}

#endif

#if (DPcArch == DPcArchtru64)

PUBLIC(int) OgBnumToLong64(bnum,n,truncation)
struct og_bnum *bnum; long *n;
int truncation;
{
int i;
char erreur[DPcSzErr];
for (i=DOgMaxBnum-1; i>=0; i--) if (bnum->n[i]) break;
if (i>3 || bnum->n[3]>0x7fff) {
  if (truncation) {
    *n = (long)0x7fffffffffffffff;
    if (bnum->minus) *n = -(*n);
    return(1);
    }
  else {
    DPcSprintf(erreur,"OgBnumToInt: too big big number");
    PcErr(-1,erreur); DPcErr;
    }
  }
*n  = bnum->n[3]; *n <<= 16;
*n += bnum->n[2]; *n <<= 16;
*n += bnum->n[1]; *n <<= 16;
*n += bnum->n[0];
if (bnum->minus) *n = -(*n);
return(0);
}

#endif



