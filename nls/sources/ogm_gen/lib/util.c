/*
 *  Some useful functions.
 *  Copyright (c) 2000-2005 Pertimm
 *  Dev : March,May 2000, June 2005
 *  Version 1.2
*/
#include <loggen.h>



#if (DPcSystem == DPcSystemUnix)
#include <strings.h>

static int memcasecmp(const unsigned char *, const unsigned char *, int);



PUBLIC(int) Ogstricmp(const char *chaine1, const char *chaine2)
{
return strcasecmp(chaine1,chaine2);
}


PUBLIC(int) Ogmemicmp(const char *chaine1, const char *chaine2, int length)
{
return memcasecmp(chaine1,chaine2,length);
}


static int memcasecmp(const unsigned char *chaine1, const unsigned char *chaine2, int length)
{
int i,c1,c2;
for (i=0; i<length; i++) {
  c1 = DPcTolower(chaine1[i]);
  c2 = DPcTolower(chaine2[i]);
  if (c1 != c2) return(c1-c2);
  }
return(0);
}

#else
#if (DPcSystem == DPcSystemWin32)

#include <string.h>
#include <stdio.h>


PUBLIC(int) Ogstricmp(chaine1,chaine2)
const char *chaine1;
const char *chaine2;
{
return stricmp(chaine1,chaine2);
}


PUBLIC(int) Ogmemicmp(chaine1,chaine2,length)
const char *chaine1;
const char *chaine2;
int length;
{
return memicmp(chaine1,chaine2,length);
}

#endif
#endif



PUBLIC(int) Ogrename(char *oldname, char *newname, int crucial, char *where, char *c_file, int c_line)
{
int retour;

#ifdef DEVERMINE
MessageInfoLog(0,where,0,"Ogrename: '%s' into '%s'",oldname,newname);
#endif

if ((retour=rename(oldname,newname))) {
  char erreur[DPcSzErr];
  if (!crucial) DONE;
  DPcSprintf(erreur, "Ogrename: Impossible to rename '%s' into '%s' (%s:%d)"
            , oldname, newname, c_file, c_line);
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}




PUBLIC(int) Ogremove(char *path, int crucial, char *where, char *c_file, int c_line)
{
int retour;

#ifdef DEVERMINE
MessageInfoLog(0,where,0,"Ogremove: '%s'",path);
#endif

if ((retour=remove(path))) {
  char erreur[DPcSzErr];
  if (!crucial) DONE;
  DPcSprintf(erreur, "Ogremove: Impossible to remove '%s' (%s:%d)",path,c_file,c_line);
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}

