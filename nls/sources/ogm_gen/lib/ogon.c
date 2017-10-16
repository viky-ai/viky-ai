/*
 *	Getting Original Name from Mirror 
 *	Copyright (c) 2002-2004 Pertimm by Patrick Constant
 *	Dev : January 2002, December 2004, January 2005
 *	Version 1.2
*/
#include <loggen.h>


#define DOgMaxHeaderSize    4096

/*
 * For the moment, we simply get original Url from HTTrack's program
 * <!-- Mirrored from www.sandstream.com/ftthmeeting-b.html by HTTrack Website 
 *      Copier/3.x [XR&CO'2001], Mon, 14 Jan 2002 19:21:21 GMT -->
 * and now (16 mars 2004):
 * <!-- Mirrored from http://www.glaverbel.com/fr/news/print.cfm?content=prod_news/2002/03_2002/images.cfm 
 * by Pertimm Website Copier version 2.68 at Mon Mar 15 01:21:34 2004 -->
*/ 

PUBLIC(int) OgGetOriginalName(int is_file, int ib, char *b, int mon, int *ion, char *on)
{
char erreur[DPcSzErr];
char buffer[DOgMaxHeaderSize+9];
int i,state,start_url,length_url;
char *mirrored = "Mirrored from ";
char *MIRRORED = "MIRRORED FROM ";
int imirrored,smirrored = strlen(mirrored);
char *by = " by ";
char *BY = " BY ";
int iby,sby = strlen(by);
int is; char *s;
int retour=0;

if (is_file) {
  char *fileslash = "file:///";
  int sfileslash = strlen(fileslash);
  FILE *fd; char *filename=b;
  if (ib > sfileslash) {
    if (!Ogmemicmp(b,fileslash,sfileslash)) {
      filename=b+sfileslash;
      }
    }

  IFn(fd=fopen(filename,"r")) {
    sprintf(erreur,"OgGetOriginalName: impossible to open '%.180s'",filename);
    PcErr(-1,erreur); DPcErr;  
    }
  is=fread(buffer,1,DOgMaxHeaderSize-1,fd);
  s = buffer; s[is]=0;
  fclose(fd);
  }
else { is=ib; s=b; }

state=1;
imirrored=0; iby=0;
start_url=0; length_url=0;
for (i=0; i<is; i++) {
  int c = s[i];
  switch(state) {
    case 1:
      if (c == mirrored[imirrored] || c == MIRRORED[imirrored]) {
        imirrored++; 
        if (imirrored >=smirrored) {
          start_url=i+1;
          state=2;
          }
        }
      else imirrored=0;
      break;
    case 2:
      /** Those chars are not allowed in the original name **/
      if (c == '\r' || c == '\n') state=1;
      if (c == by[iby] || c == BY[iby]) {
        iby++; 
        if (iby >= sby) {
          length_url = i-iby-start_url+1;
          goto endOgGetOriginalUrl;
          }
        }
      else iby=0;
      break;
    }
  }

endOgGetOriginalUrl:

if (length_url == 0) { *ion=0; on[0]=0; return(retour); }

if (length_url > mon-1) { length_url = mon-1; retour=1; }

*ion = length_url;
memcpy(on,s+start_url,*ion);
on[*ion]=0;

return(retour);
}




PUBLIC(int) OgDownloadedFilenameToAssociatedFilename(unsigned char *downloaded_filename, unsigned char* associated_filename)
{
int downloaded_filename_length=strlen(downloaded_filename);
int i,slash=(-1);

for (i=downloaded_filename_length; i>=0; i--) {
  if (downloaded_filename[i]=='/' || downloaded_filename[i]=='\\') {
    slash=i; break;
    }
  }

if (slash >= 0) {  
  sprintf(associated_filename,"%.*s/.pertimm/%s.xml",slash,downloaded_filename,downloaded_filename+slash+1);
  }
else {
  sprintf(associated_filename,".pertimm/%s.xml",downloaded_filename);
  }
DONE;
}




