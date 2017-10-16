/*
 *	Converting from Codepage to Codepage, via Unicode.
 *	Copyright (c) 2010 Pertimm by Loïs Rigouste
 *	Dev : February 2010
 *	Version 1.0
*/
#include <loguni.h>

#define DOgWhereName            "oguc"

PUBLIC(int) OgFilesCpToCp(input_file,output_file,input_codepage,output_codepage)
unsigned char *input_file; unsigned char *output_file;
int input_codepage; int output_codepage;
{
struct stat filestat; char erreur[DPcSzErr];
int sincode,iincode; unsigned char *incode;
int soutcode,ioutcode; unsigned char *outcode;
FILE *fd;

IFx(stat(input_file,&filestat)) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to stat '%s'",input_file);
  PcErr(-1,erreur); DPcErr;
  }
sincode = filestat.st_size;

/** We add +4 because of the zero at the end of the output buffer **/
if (input_codepage==DOgCodePageUTF8 && output_codepage==DOgCodePageUTF8) {
  soutcode = sincode+4;
  }
else if (input_codepage==DOgCodePageUTF8 && output_codepage!=DOgCodePageUTF8) {
  soutcode = sincode+4;
  }
else if (input_codepage!=DOgCodePageUTF8 && output_codepage==DOgCodePageUTF8) {
  soutcode = 3*sincode+4; /** Max possible extension (e.g.: only Japanese) **/
  }
else {
  soutcode = sincode+4;
  }

IFn(incode=(unsigned char *)malloc(sincode+9)) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to malloc %d bytes for input",sincode);
  PcErr(-1,erreur); DPcErr;
  }
IFn(outcode=(unsigned char *)malloc(soutcode+9)) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to malloc %d bytes for output",soutcode);
  PcErr(-1,erreur); DPcErr;
  }

IFn(fd=fopen(input_file,"rb")) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to fopen '%s'",input_file);
  PcErr(-1,erreur); DPcErr;
  }

IFn(iincode=fread(incode,1,sincode,fd)) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to fread '%s'",input_file);
  PcErr(-1,erreur); DPcErr;
  }

fclose(fd);

IFE(OgCpToCp(iincode,incode,soutcode,&ioutcode,outcode,input_codepage,output_codepage,0,0));

IFn(fd=fopen(output_file,"wb")) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to fopen '%s'",output_file);
  PcErr(-1,erreur); DPcErr;
  }

IFn(fwrite(outcode,1,ioutcode,fd)) {
  sprintf(erreur,"OgFilesCpToCp: Impossible to fwrite '%s'",output_file);
  PcErr(-1,erreur); DPcErr;
  }

fclose(fd);
free(incode);
free(outcode);

DONE;
}






PUBLIC(int) OgCpToCp(iincode,incode,soutcode,ioutcode,outcode,incodepage,outcodepage,position,truncated)
int iincode; unsigned char *incode;
int soutcode,*ioutcode; unsigned char *outcode;
int incodepage; int outcodepage;
int *position,*truncated;
{
unsigned char *unicode;
int unicode_size,unicode_length;
char erreur[DPcSzErr];

unicode_size = 2*iincode;

IFn(unicode=(unsigned char *)malloc(unicode_size+9)) {
  sprintf(erreur,"OgCpToCp: Impossible to malloc %d bytes for output",unicode_size);
  PcErr(-1,erreur); DPcErr;
  }

if (incodepage < 0) {
  int nb_ascii_char,nb_utf8_chars,retour;
  IFE(retour=OgCheckUTF8(iincode, incode,&nb_ascii_char,&nb_utf8_chars));
  if (retour) incodepage=DOgCodePageUTF8;
  OgMessageLog( DOgMlogInLog, DOgWhereName, 0
    , "No input codepage, nb_ascii_char=%d nb_utf8_chars=%d retour=%d"
    , nb_ascii_char, nb_utf8_chars, retour);
  if (retour==2) {
    OgMessageLog( DOgMlogInLog, DOgWhereName, 0
      , "This is a clean UTF-8 document");
    }
  else if (retour==1) {
    OgMessageLog( DOgMlogInLog, DOgWhereName, 0
      , "This is possibly a dirty UTF-8 document");
    }
  else if (retour==1) {
    OgMessageLog( DOgMlogInLog, DOgWhereName, 0
      , "This is not a UTF-8 document");
    }
  }

if (incodepage == DOgCodePageUnicode) {
  unicode_length = iincode;
  unicode = incode;
  }
else {  
  IFE(OgCpToUni(iincode, incode, unicode_size
    , &unicode_length, unicode, incodepage, position, truncated));
  }
  
if (outcodepage == DOgCodePageUnicode) {
  *ioutcode = unicode_length;
  memcpy(outcode,unicode,unicode_length);
  }
else {
  IFE(OgUniToCp(unicode_length, unicode, soutcode
    , ioutcode, outcode, outcodepage, position, truncated));
  }

free(unicode);  
  
DONE;
}


