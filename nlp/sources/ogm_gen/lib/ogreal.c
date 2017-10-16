/*
 *  handling real numbers per Pertimm design
 *  Copyright (c) 2001  Ogmios
 *  Dev : September 2001
 *  Version 1.1
*/
#include <math.h>
#include <loggen.h>



/*
 *  Writes the Ogmios real number corresponding to the given unsigned
 *  into the 'or' buffer whose size is 'sor'. 'or' is zero terminated.
 *  return the length of 'or' if the Ogmios real number has been successfully
 *  written and -1 on error (usually because the string is too small).
 *  'u' goes from 0 to 0xffffffff = 4294967295, so the real zone is 'F'.
 *  Also, the matissa value cannot be bigger than strlen("4294967295") = 10.
 *  Thus, sor must be bigger than 2 + 10 + 1 = 13.
*/
PUBLIC(int) OgUnsignedToOgReal(unsigned int u, int sor, unsigned char *or)
{
int i,j,k,start,exp;
unsigned rest,num = u;
unsigned char rev[20];

if (sor < 13) {
  char erreur[DPcSzErr];
  DPcSprintf(erreur,"OgUnsignedToOgReal: sor = %d < 13",sor);
  PcErr(-1,erreur); DPcErr;
  }

start = 1; i = 0; exp=0;
while (num) {
  rest = num%10;
  num = num/10; exp++;
  if (start && rest==0) continue;
  rev[i++] = 0x30 + rest;
  start = 0;
  }
if (exp==0) { or[0]='D'; or[1]=0; return(1); }

or[0]='F'; or[1] = exp - 1;
for (j=i-1,k=2; j>=0; j--) or[k++]=rev[j];
or[k]=0;

return(k);
}


/*
 *  Writes the Ogmios real number corresponding to the given integer
 *  into the 'or' buffer whose size is 'sor'. 'or' is zero terminated.
 *  return the length of 'or' if the Ogmios real number has been successfully
 *  written and -1 on error (usually because the string is too small).
 *  'i' goes from -0x8fffffff to 0x8fffffff = 2415919103, positive value
 *  is handle by OgUnsignedToOgReal and for negative values, the real zone is B
 *  (complemented mantissa and exponent).
 *  Also, the matissa value cannot be bigger than strlen("2415919103") = 10.
 *  Thus, sor must be bigger than 2 + 10 + 1 = 13.
*/

PUBLIC(int) OgIntToOgReal(int i, int sor, unsigned char *or)
{
int j,inor;
unsigned char nor[20];

if (i>=0) return(OgUnsignedToOgReal((unsigned)i,sor,or));

i = -i;

IFE(inor=OgUnsignedToOgReal((unsigned)i,20,nor));
or[0] = 'B'; or[1] = ~nor[1];
for (j=2; j<inor; j++) {
  if (j+1==inor) or[j]=0x30 + 10 - (nor[j]-0x30);
  else or[j]= 0x30 + 9 - (nor[j]-0x30);
  }
or[j]=0;
return(j);
}




PUBLIC(int) OgDoubleToOgReal(double d, int sor, unsigned char *or)
{
char s[128];
sprintf(s,"%e",d);
return(OgStringToOgReal(s,sor,or));
}




/*
 *  Reads a string that is representing a real number. This string
 *  has the following format (taken from atof):
 *  [whitespace] [sign] [digits] [.digits] [ {d | D | e | E }[sign]digits]
 *  A whitespace consists of space and/or tab characters, which are ignored;
 *  sign is either plus (+) or minus (-); and digits are one or more decimal digits.
 *  If no digits appear before the decimal point, at least one must appear after the decimal point.
 *  The decimal digits may be followed by an exponent, which consists of an
 *  introductory letter ( d, D, e, or E) and an optionally signed decimal integer.
 *  To this specification, we add:
 *   - the fact that we can add spaces anywhere in the string.
 *   - the fact that '.' is the same as ','
*/

PUBLIC(int) OgStringToOgReal(char *s, int sor, unsigned char *or)
{
int im1=0,im2=0,ie=0,offset_e=0,exp,ior,iexp,inm;
int i,state=1,m_sign='+',e_sign='+';
char nexp[DOgMaxRealElement];
char nm[DOgMaxRealElement];
char m1[DOgMaxRealElement];
char m2[DOgMaxRealElement];
char e[DOgMaxRealElement];
char erreur[DPcSzErr];
unsigned char *p;

/** First, we parse the different elements **/
for (i=0; s[i]; i++) {
  int c=s[i];
  if (isspace(c)) continue;
  switch(state) {
    /** start of mantissa **/
    case 1:
      if (isdigit(c)) {
        m1[im1++]=c; state=2;
        }
      else if (c=='+') state=2;
      else if (c=='-') { m_sign='-'; state=2; }
      else if (c=='.' || c==',') state=3;
      else if (c=='d' || c=='D' || c=='e' || c=='E') state=4;
      else goto bad_format;
      break;

    /** in mantissa before comma **/
    case 2:
      if (isdigit(c)) {
        m1[im1++]=c; state=2;
        }
      else if (c=='.' || c==',') state=3;
      else if (c=='d' || c=='D' || c=='e' || c=='E') state=4;
      else goto bad_format;
      break;

    /** in mantissa after comma **/
    case 3:
      if (isdigit(c)) {
        m2[im2++]=c; state=3;
        }
      else if (c=='d' || c=='D' || c=='e' || c=='E') state=4;
      else goto bad_format;
      break;

    /** start of exponent **/
    case 4:
      if (isdigit(c)) {
        e[ie++]=c; state=5;
        }
      else if (c=='+') state=5;
      else if (c=='-') { e_sign='-'; state=5; }
      else goto bad_format;
      break;

    /** in exponent **/
    case 5:
      if (isdigit(c)) {
        e[ie++]=c; state=5;
        }
      else goto bad_format;
      break;
    }
  }

m1[im1]=0;
m2[im2]=0;
e[ie]=0;

/** Second, we canonize the different elements **/
/** removing zeros at the beginning of m1 **/
for (i=0; i<im1; i++) {
  if (m1[i]!='0') break;
  }
if (i>0) {
  memmove(m1,m1+i,im1-i);
  im1-=i; m1[im1]=0;
  }
/** removing zeros at the end of m2 **/
for (i=im2-1; i>=0; i--) {
  if (m2[i]!='0') break;
  }
im2=i+1; m2[im2]=0;

/** keeping only one digit in m1 **/
if (im1>1) {
  offset_e=im1-1;
  memmove(m2+im1-1,m2,im2);
  memcpy(m2,m1+1,im1-1);
  im2=im1-1+im2; m2[im2]=0;
  im1=1; m1[im1]=0;
  /** removing zeros again at the end of m2 **/
  for (i=im2-1; i>=0; i--) {
    if (m2[i]!='0') break;
    }
  im2=i+1; m2[im2]=0;
  }
else if (im1<1) { /* im1==0 */
  if (im2>0) {
    /** remove zeros at the beginning of m2 **/
    for (i=0; i<im2; i++) {
      if (m2[i]!='0') break;
      }
    if (i>0) {
      memmove(m2,m2+i,im2-i);
      im2-=i; m2[im2]=0;
      }
    offset_e=-i-1;
    m1[0]=m2[0]; m1[1]=0;
    memmove(m2,m2+1,im2-1);
    im2--; m2[im2]=0;
    im1=1;
    }
  }

exp=atoi(e);
if (e_sign=='+') exp+=offset_e;
else exp-=offset_e;
if (exp<0) {
  if (e_sign=='+') e_sign='-';
  else e_sign='+';
  exp = -exp;
  }

//printf("%c%s.%s E %c%d\n",m_sign,m1,m2,e_sign,exp);

/** Third, calculate the ogreal **/
if (im1==0) { or[0]='D'; or[1]=0; return(1); }

ior=0;
if (m_sign=='+') {
  if (e_sign=='+') {
    p=or; *p++='F'; OggNout(exp,&p);
    }
  else {
    p=or; OggNout(exp,&p);
    iexp = p-or;
    for (i=0; i<iexp; i++) {
      nexp[i]=0xff-or[i];
      }
    p=or; *p++='E';
    for (i=0; i<iexp; i++) {
      *p++=nexp[i];
      }
    }
  if (im1) memcpy(p,m1,im1);
  if (im2) memcpy(p+im1,m2,im2);
  ior = (p-or) + im1 + im2;
  }
else {
  if (e_sign=='+') {
    p=or; OggNout(exp,&p);
    iexp = p-or;
    for (i=0; i<iexp; i++) {
      nexp[i]=0xff-or[i];
      }
    p=or; *p++='B';
    for (i=0; i<iexp; i++) {
      *p++=nexp[i];
      }
    }
  else {
    p=or; *p++='C'; OggNout(exp,&p);
    }
  if (im1) memcpy(nm,m1,im1);
  if (im2) memcpy(nm+im1,m2,im2);
  inm = im1+im2;
  for (i=0; i<inm; i++) {
    if (i+1==inm) *p++ = 0x30 + 10 - (nm[i]-0x30);
    else *p++ = 0x30 + 9 - (nm[i]-0x30);
    }
  ior = (p-or);
  }

or[ior]=0;
return(ior);

bad_format:
sprintf(erreur,"OgStringToOgReal: bad format state=%d",state);
PcErr(-1,erreur); DPcErr;
}





/*
 *  'or' is an Ogmios Real number. This function returns error
 *  when it cannot tranlate into an unsigned int. if truncate is true,
 *  it will truncate the digits after the comma, otherwise,
 *  it sends also an error if there are digit after the coma.
 *  It does not test some overflows of the unsigned number created.
 *  returns the length read in  'or' (so or[length]==0).
*/

PUBLIC(int) OgRealToUnsigned(unsigned char *or, unsigned int *unb, int truncate)
{
unsigned unum;
char erreur[DPcSzErr];
int i, ior, start_mantissa, length_mantissa;
if (or[0]=='D') { *unb=0; return(1); }
else if (or[0]!='F') {
  DPcSprintf(erreur,"OgRealToUnsigned: out of bound: type '%c'",or[0]);
  PcErr(-1,erreur); DPcErr;
  }
if (or[1]>9) {
  DPcSprintf(erreur,"OgRealToUnsigned: out of bound: exponent %d",or[1]);
  PcErr(-1,erreur); DPcErr;
  }
start_mantissa=2;
for (i=start_mantissa; or[i]; i++); ior=i;
length_mantissa = i - start_mantissa;
if (length_mantissa > or[1]+1) {
  if (truncate) length_mantissa = or[1]+1;
  else {
    DPcSprintf(erreur,"OgRealToUnsigned: not a pure unsigned number");
    PcErr(-1,erreur); DPcErr;
    }
  }
unum=or[start_mantissa] - 0x30;
for (i=1; i<length_mantissa; i++) {
  unum *= 10;
  unum += or[start_mantissa+i]  - 0x30;
  }
for (i=0; i<or[1]-length_mantissa+1; i++) {
  unum *= 10;
  }
*unb = unum;
return(ior);
}



/*
 *  'or' is an Ogmios Real number. This function returns error
 *  when it cannot tranlate into an int. if truncate is true,
 *  it will truncate the digits after the comma, otherwise,
 *  it sends also an error if there are digit after the coma.
 *  It does not test some overflows of the int number created.
 *  returns the length read in  'or' (so or[length]==0).
*/

PUBLIC(int) OgRealToInt(unsigned char *or, int *inb, int truncate)
{
int inum,ior;
unsigned char exp;
char erreur[DPcSzErr];
unsigned char mantissa[20];
int i, start_mantissa, length_mantissa;
if (or[0]=='D') { *inb=0; return(1); }
else if (or[0]=='F') {
  unsigned unb;
  IFE(ior=OgRealToUnsigned(or,&unb,truncate));
  if (unb > 0x7fffffff) {
    DPcSprintf(erreur,"OgRealToInt: overflow but would have worked as unsigned");
    PcErr(-1,erreur); DPcErr;
    }
  *inb=(int)unb;
  return(ior);
  }
if (or[0]!='B') {
  DPcSprintf(erreur,"OgRealToInt: out of bound: type '%c'",or[0]);
  PcErr(-1,erreur); DPcErr;
  }
/** exponent and mantissa are complemented, so we need to uncomplement them **/
exp = ~or[1];
if (exp>9) {
  DPcSprintf(erreur,"OgRealToInt: out of bound: exponent %d",exp);
  PcErr(-1,erreur); DPcErr;
  }
start_mantissa=2;
for (i=start_mantissa; or[i]; i++); ior=i;
length_mantissa = i - start_mantissa;
if (length_mantissa > exp+1) {
  if (truncate) length_mantissa = exp+1;
  else {
    DPcSprintf(erreur,"OgRealToInt: not a pure integer number");
    PcErr(-1,erreur); DPcErr;
    }
  }
for (i=0; i<length_mantissa; i++) {
  if (i+1==length_mantissa) mantissa[i] = 10 -(or[start_mantissa+i]-0x30);
  else mantissa[i] = 9 - (or[start_mantissa+i]-0x30);
  }

inum=mantissa[0];
for (i=1; i<length_mantissa; i++) {
  inum *= 10;
  inum += mantissa[i];
  }
for (i=0; i<exp-length_mantissa+1; i++) {
  inum *= 10;
  }
*inb = -inum;
return(ior);
}




PUBLIC(int) OgRealToDouble(unsigned char *or, double *r)
{
int ior;
char s[1024];
IFE(ior=OgRealToString(or,1024,s));
*r = atof(s);
return(ior);
}






PUBLIC(int) OgRealLength(unsigned char *or)
{
int ior;
struct og_dispatched_real dr;
IFE(ior=OgRealToStruct(or,&dr));
return(ior);
}






PUBLIC(int) OgRealToStruct(unsigned char *or, struct og_dispatched_real *dr)
{
int i,ior;
unsigned char *p;
int im; char m[DOgMaxRealElement];
int isexp; char sexp[DOgMaxRealElement];
char erreur[DPcSzErr];

dr->zero = 0;
dr->infinite = 0;
dr->m_sign[0]=0;
dr->e_sign[0]=0;

if (or[0]=='G') {
  dr->infinite = 1;
  ior=1;
  }
else if (or[0]=='F') {
  p = or+1;
  IFE(dr->exp=OggNin4(&p));
  dr->m1[0]=p[0]; dr->m1[1]=0;
  strcpy(dr->m2,p+1);
  /** dr->exp can be zero, so strlen(or) does not work **/
  ior=(p-or)+1+strlen(dr->m2);
  }
else if (or[0]=='E') {
  p = or+1;
  sexp[0]=0xff-p[0];
  IFE(isexp=OggNinlen(sexp));
  for (i=1; i<isexp; i++) {
    sexp[i]=0xff-p[i];
    }
  p = sexp;
  IFE(dr->exp=OggNin4(&p));
  p = or+1+isexp;
  dr->m1[0]=p[0]; dr->m1[1]=0;
  if (p[1]) strcpy(dr->m2,p+1);
  else dr->m2[0]=0;
  strcpy(dr->e_sign,"-");
  ior=strlen(or);
  }
else if (or[0]=='D') {
  dr->zero=1;
  ior=1;
  }
else if (or[0]=='C') {
  p = or+1;
  IFE(dr->exp=OggNin4(&p));
  for (im=0; p[im]; im++);
  for (i=0; i<im; i++) {
    if (i+1==im) m[i] = 0x30 + 10 -(p[i]-0x30);
    else m[i] = 0x30 + 9 - (p[i]-0x30);
    }
  m[im]=0;
  dr->m1[0]=m[0]; dr->m1[1]=0;
  if (im>1) strcpy(dr->m2,m+1);
  else dr->m2[0]=0;
  strcpy(dr->m_sign,"-");
  strcpy(dr->e_sign,"-");
  ior=strlen(or);
  }
else if (or[0]=='B') {
  p = or+1;
  sexp[0]=0xff-p[0];
  IFE(isexp=OggNinlen(sexp));
  for (i=1; i<isexp; i++) {
    sexp[i]=0xff-p[i];
    }
  p = sexp;
  IFE(dr->exp=OggNin4(&p));
  p = or+1+isexp;
  for (im=0; p[im]; im++);
  for (i=0; i<im; i++) {
    if (i+1==im) m[i] = 0x30 + 10 -(p[i]-0x30);
    else m[i] = 0x30 + 9 - (p[i]-0x30);
    }
  m[im]=0;
  dr->m1[0]=m[0]; dr->m1[1]=0;
  if (im>1) strcpy(dr->m2,m+1);
  else dr->m2[0]=0;
  strcpy(dr->m_sign,"-");
  ior=strlen(or);
  }
else if (or[0]=='A') {
  dr->infinite = -1;
  ior=1;
  }
else {
  DPcSprintf(erreur,"OgRealToString: Wrong real type");
  PcErr(-1,erreur); DPcErr;
  }

return(ior);
}




PUBLIC(int) OgRealToString(unsigned char *or, int is, char *s)
{
int ior;
struct og_dispatched_real dr;
IFE(ior=OgRealToStruct(or,&dr));
if (dr.infinite > 0) {
  strcpy(s,"Plus Infinite");
  }
else if (dr.infinite == 0) {
  if (dr.zero) strcpy(s,"0");
  else sprintf(s,"%s%s.%sE%s%d",dr.m_sign,dr.m1,dr.m2,dr.e_sign,dr.exp);
  }
else { /* infinite < 0 */
  strcpy(s,"Minus Infinite");
  }
return(ior);
}



/*
 * Some people prefer a more 'readable' format
 */
PUBLIC(int) OgRealToHumanString(unsigned char *or, int is, char *s)
{
int ior;
char string[1024];
IFE(ior=OgRealToString(or,1024,string));
double d = atof(string);
sprintf(s,"%.6f",d);
return(ior);
}


/*
 * Round the real number 'or' of length 'ior'. New length is '*iorr' and new real number
 * is in orr, except if orr is zero, in that case, or is directly modified.
 * n and flag is used the same Excel functions are used:
 * flag = -1 -> ARRONDI.INF
 * ARRONDI.INF(3,2;0) �gale 3
 * ARRONDI.INF(76,9;0) �gale 76
 * ARRONDI.INF(3,14159;3) �gale 3,141
 * ARRONDI.INF(-3,14159;1) �gale -3,1
 * ARRONDI.INF(31415,92654;-2) �gale 31400
 * flag = 0 -> ARRONDI
 * ARRONDI(2,15; 1) �gale 2,2
 * ARRONDI(2,149; 1) �gale 2,1
 *  * ARRONDI(-1,475; 2) �gale -1,48
 * ARRONDI(21,5; -1) �gale 20
 * flag = 1 -> ARRONDI.SUP
 * ARRONDI.SUP(3,2;0) �gale 4
 * ARRONDI.SUP(76,9;0) �gale 77
 * ARRONDI.SUP(3,14159;3) �gale 3,142
 * ARRONDI.SUP(-3,14159;1) �gale -3,2
 * ARRONDI.SUP(31415,92654;-2) �gale 31500
*/

PUBLIC(int) OgRoundReal(int ior, unsigned char *or, int *iorr, unsigned char *orr, int n, int flag)
{
unsigned char *o;
struct og_dispatched_real dr;
char s[256];
double d,i,r;

if (orr) o = orr; else o = or;
IFE(OgRealToStruct(or,&dr));

if (dr.infinite > 0) {
  *iorr = ior; if (orr) memcpy(orr,or,ior); orr[ior]=0; DONE;
  }
else if (dr.infinite == 0) {
  if (dr.zero) { *iorr = ior; if (orr) memcpy(orr,or,ior); orr[ior]=0; DONE; }
  }
else { /* infinite < 0 */
  *iorr = ior; if (orr) memcpy(orr,or,ior); orr[ior]=0; DONE;
  }

if (dr.e_sign[0]=='-') {
  if (dr.exp - n < 0) { dr.exp = -dr.exp +n; dr.e_sign[0]=0; }
  else dr.exp -= n;
  }
else {
  if (dr.exp + n < 0) { dr.exp = -dr.exp -n; strcpy(dr.e_sign,"-"); }
  else dr.exp += n;
  }
sprintf(s,"%s%s.%sE%s%d",dr.m_sign,dr.m1,dr.m2,dr.e_sign,dr.exp);

d = atof(s);
if (flag < 0) d = floor(d);
else if (flag==0) {
  r = modf(d,&i); /* rint(d) does not exist on Windows */
  if (r>=0.5) d = i+1;
  else d = i;
  }
else d = ceil(d);

IFE(OgDoubleToOgReal(d,DOgMaxRealElement,o));
IFE(OgRealToStruct(o,&dr));

if (dr.zero) sprintf(s,"0");
else {
  if (dr.e_sign[0]=='-') {
    if (dr.exp + n < 0) { dr.exp = -dr.exp -n; dr.e_sign[0]=0; }
    else dr.exp += n;
    }
  else {
    if (dr.exp - n < 0) { dr.exp = -dr.exp +n; strcpy(dr.e_sign,"-"); }
    else dr.exp -= n;
    }
  sprintf(s,"%s%s.%sE%s%d",dr.m_sign,dr.m1,dr.m2,dr.e_sign,dr.exp);
  }
IFE(*iorr = OgStringToOgReal(s,DOgMaxRealElement,o));
DONE;
}

