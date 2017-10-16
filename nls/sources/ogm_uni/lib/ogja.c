/*
 *	General API for japanese character handling.
 *	Copyright (c) 1998-2000	Pertimm, Inc.
 *	Dev : August 2001
 *	Version 1.0
*/
#include <loguni.h>

#define DOgFileCodepagesSjis2uni_Bin    "codepage/sjis2uni.bin"
#define DOgFileCodepagesUni2Sjis_Bin    "codepage/uni2sjis.bin"

/** The two translation table from SJIS to UNICODE and backward. **/
static unsigned short *SjisToUnicode;
static unsigned short *UnicodeToSjis;
static int JaTableAllocated=0;


/*
 *  Basically allocates (0x10000 shorts) and loads the translation tables from files.
*/
PUBLIC(int) OgJaInit()
{
FILE *fd;
size_t i,size;
char *filename;
unsigned short sh;
char erreur[DPcSzErr];
int bigendian=0;

if (JaTableAllocated) DONE;
JaTableAllocated=1;

if (OgBigEndian()) bigendian=1;
size=0x10000*2;

IFn(SjisToUnicode = malloc(size)) {
  sprintf(erreur,"OgJaInit: impossible to malloc %d bytes (SjisToUnicode)",size);
  PcErr(-1,erreur); DPcErr;
  }
filename=DOgFileCodepagesSjis2uni_Bin;
IFn(fd=fopen(filename, "rb")){
  sprintf(erreur,"OgJaInit: impossible to open '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }
if(fread(SjisToUnicode,1,size,fd) != size){
  sprintf(erreur,"OgJaInit: impossible to read '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }
fclose(fd);
if (bigendian) {
  for (i=0; i<size/2; i++) {
    sh =  SjisToUnicode[i];
    sh = ((sh&0xff00)>>8) + ((sh&0xff)<<8);   
    SjisToUnicode[i] = sh;
    }
  }

IFn(UnicodeToSjis = malloc(size)) {
  sprintf(erreur,"OgJaInit: impossible to malloc %d bytes (UnicodeToSjis)",size);
  PcErr(-1,erreur); DPcErr;
  }
filename=DOgFileCodepagesUni2Sjis_Bin;
IFn(fd=fopen(filename, "rb")){
  sprintf(erreur,"OgJaInit: impossible to open '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }
if(fread(UnicodeToSjis,1,size,fd) != size){
  sprintf(erreur,"OgJaInit: impossible to read '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }
fclose(fd);
if (bigendian) {
  for (i=0; i<size/2; i++) {
    sh =  UnicodeToSjis[i];
    sh = ((sh&0xff00)>>8) + ((sh&0xff)<<8);   
    UnicodeToSjis[i] = sh;
    }
  }
DONE;
}


/*
 *  Frees the allocated translation tables.
*/
PUBLIC(int) OgJaFlush()
{
if (!JaTableAllocated) DONE;
DPcFree(SjisToUnicode);
DPcFree(UnicodeToSjis);
DONE;
}




PUBLIC(int) OgJaSjisToUnicode(sjis)
unsigned short sjis;
{
return(SjisToUnicode[sjis]);
}




PUBLIC(int) OgJaUnicodeToSjis(unicode)
unsigned short unicode;
{
return(UnicodeToSjis[unicode]);
}




PUBLIC(int) OgJaJisToUnicode(jis)
unsigned short jis;
{
unsigned short hib, lob, sjis;
hib = (jis >> 8) & 0xff;
lob = jis & 0xff;
lob += (hib & 1) ? 0x1f : 0x7d;
if(lob >= 0x7f) lob++;
hib = ((hib - 0x21) >> 1) + 0x81;
if(hib > 0x9f) hib += 0x40;
sjis = (hib << 8) | lob;
return (OgJaSjisToUnicode(sjis));
}




PUBLIC(int) OgJaUnicodeToJis(unicode)
unsigned short unicode;
{
unsigned short hib, lob, sjis;
sjis = OgJaUnicodeToSjis(unicode);
hib = (sjis >> 8) & 0xff;
lob = sjis & 0xff;
hib -= (hib <= 0x9f) ? 0x71 : 0xb1;
hib = (hib << 1) + 1;
if(lob > 0x7f) lob--;
if(lob >= 0x9e){
  lob -= 0x7d;
  hib++;
  }
else lob -= 0x1f;
return (hib << 8) | lob;
}




unsigned int hankaku2zen(int hankaku)
{
static unsigned int z[64] = {
  0x2121,0x2123,0x2156,0x2157,0x2122,0x2126,0x2572,0x2521,
  0x2523,0x2525,0x2527,0x2529,0x2563,0x2565,0x2567,0x2543,
  0x213c,0x2522,0x2524,0x2526,0x2528,0x252a,0x252b,0x252d,
  0x252f,0x2531,0x2533,0x2535,0x2537,0x2539,0x253b,0x253d,
  0x253f,0x2541,0x2544,0x2546,0x2548,0x254a,0x254b,0x254c,
  0x254d,0x254e,0x254f,0x2552,0x2555,0x2558,0x255b,0x255e,
  0x255f,0x2560,0x2561,0x2562,0x2564,0x2566,0x2568,0x2569,
  0x256a,0x256b,0x256c,0x256d,0x256f,0x2573,0x212b,0x212c };
if (hankaku < 0xa0 || hankaku > 0xdf) return 0;
return z[hankaku - 0xa0];
}




PUBLIC(int) OgJaEucToUnicode(euc)
unsigned short euc;
{
unsigned short jis;

if( (euc & 0xff00) == 0x8e00)
  jis = hankaku2zen(euc & 0xff);
else jis = euc & ~0x8080;

return (OgJaJisToUnicode(jis));
}




PUBLIC(int) OgJaUnicodeToEuc(unicode)
unsigned short unicode;
{
unsigned short sjis, hib, lob;
sjis = OgJaUnicodeToSjis(unicode);
hib = (sjis >> 8) & 0xff;
lob = sjis & 0xff;
hib -= (hib <= 0x9f) ? 0x71 : 0xb1;
hib = (hib << 1) + 1;
if(lob >= 0x9e){
  lob -= 0x7e;
  hib++;
  }
else if(lob > 0x7f) lob -= 0x20;
else lob -= 0x1f;
hib |= 0x80;
lob |= 0x80;
return (hib << 8) | lob;
}



PUBLIC(int) OgUniCjkKanjiHiragana(int c)
{
if (0x3041<=c && c<=0x309E) return(1);
else if (0x4E00<=c && c<=0x9FA5) return(1);
return(0);
}


