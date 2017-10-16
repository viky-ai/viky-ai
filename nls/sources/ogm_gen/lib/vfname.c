/*
 *	Valid filename on a file system (Linux Or Windows)
 *	Copyright (c) 2004-2005 Pertimm by Patrick Constant
 *	Dev : December 2004, January, April 2005
 *	Version 1.2
*/
#include <loggen.h>



/*
 *  Returns 1 if the file can be safely written in the file system and 0 otherwise. 
 *  We get all invalid names from Linux and Windows and set them together, 
 *  so that we have same filename under Linux or Windows. 
 *  Some chars need à '\' under Linux, because they are special chars: " !"$&'()*[\`{|}".
 *  On websites using Lotus Notes such as www.alcan.com the '$' is 
 *  commonly used (directory "$file"), so we keep it as normal char.
*/

PUBLIC(int) OgValidFilename(int iurl, char *url)
{
static unsigned char invalid[256+9];
static int calculated_invalid=0;
int i; unsigned char c;

if (!calculated_invalid) {  
  memset(invalid,0,256*sizeof(unsigned char));
  for (i=0; i<=31; i++) invalid[i]=1;
  /*  33 ! */     /*  34 " */     /*  38 & */     /*  39 ' */ 
  invalid[33]=1;  invalid[34]=1;  invalid[38]=1;  invalid[39]=1;
  /*  40 ( */     /*  41 ) */     /*  42 * */     /*  58 : */     /*  60 < */
  invalid[40]=1;  invalid[41]=1;  invalid[42]=1;  invalid[58]=1;  invalid[60]=1;  
  /*  62 > */     /*  63 ? */     /*  91 [ */     /*  92 \ */     /*  93 ] */  
  invalid[62]=1;  invalid[63]=1;  invalid[91]=1;  invalid[92]=1;  invalid[93]=1;  
  /*  94 ^ */     /*  96 ` */    /* 123 { */      /* 124 | */     /* 125 } */ 
  invalid[94]=1;  invalid[96]=1; invalid[123]=1;  invalid[124]=1; invalid[125]=1;
  for (i=127; i<=160; i++) invalid[i]=1;
  calculated_invalid=1;
  }

for (i=0; i<iurl; i++) {
  c=(unsigned char)url[i];
  if (invalid[c]) return(0);
  } 
return(1);
}





/*
 * Same as OgValidFilename but replace every invalid char with 
 * the corresponding 'replacement' char.
*/

PUBLIC(int) OgValidateFilename(int iurl, char *url, int replacement)
{
static unsigned char invalid[256+9];
static int calculated_invalid=0;
int i; unsigned char c;

if (!calculated_invalid) {  
  memset(invalid,0,256*sizeof(unsigned char));
  for (i=0; i<=31; i++) invalid[i]=1;
  /*  33 ! */     /*  34 " */     /*  38 & */     /*  39 ' */ 
  invalid[33]=1;  invalid[34]=1;  invalid[38]=1;  invalid[39]=1;
  /*  40 ( */     /*  41 ) */     /*  42 * */     /*  58 : */     /*  60 < */
  invalid[40]=1;  invalid[41]=1;  invalid[42]=1;  invalid[58]=1;  invalid[60]=1;  
  /*  62 > */     /*  63 ? */     /*  91 [ */     /*  92 \ */     /*  93 ] */  
  invalid[62]=1;  invalid[63]=1;  invalid[91]=1;  invalid[92]=1;  invalid[93]=1;  
  /*  94 ^ */     /*  96 ` */    /* 123 { */      /* 124 | */     /* 125 } */ 
  invalid[94]=1;  invalid[96]=1; invalid[123]=1;  invalid[124]=1; invalid[125]=1;
  for (i=127; i<=160; i++) invalid[i]=1;
  calculated_invalid=1;
  }

for (i=0; i<iurl; i++) {
  c=(unsigned char)url[i];
  if (invalid[c]) url[i]=replacement;
  } 
DONE;
}




