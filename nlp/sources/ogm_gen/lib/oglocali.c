/*
 *	Localizes a son URL with regard to a father URL
 *	Copyright (c) 2006 Pertimm by Patrick Constant
 *	Dev: January 2006
 *	Version 1.1
*/
#include <loggen.h>


/*#define DEVERMINE*/


struct og_dir {
  int start,length;
  };

static int OgGetDir(char *, int *, struct og_dir *);




PUBLIC(int) OgLocalize(int father_isdir, char *father, char *son, char *url, char *where)
{
int i,ifather_dir,idir,imin,start_dir,nb_dp;
struct og_dir father_dir[DPcPathSize], dir[DPcPathSize]; 
#ifdef DEVERMINE
char buffer[DPcPathSize];
#endif

IFE(OgGetDir(father,&ifather_dir,father_dir));

#ifdef DEVERMINE
buffer[0]=0;
for (i=0;i<ifather_dir;i++) {
  sprintf(buffer+strlen(buffer),"|%.*s", father_dir[i].length, father+father_dir[i].start);
  }
OgMessageLog(DOgMlogInLog,where,0
  , "localize on father: '%s' %2d %s|", father, ifather_dir, buffer);
#endif

IFE(OgGetDir(son,&idir,dir));

#ifdef DEVERMINE
buffer[0]=0;
for (i=0;i<idir;i++) {
  sprintf(buffer+strlen(buffer),"|%.*s", dir[i].length, son+dir[i].start);
  }
OgMessageLog(DOgMlogInLog,where,0
  , "localize on son   : '%s' %2d %s|", son, idir, buffer);
#endif

imin=ifather_dir;
if (imin>idir) imin=idir;
for (i=0; i<imin; i++) {
  if (father_dir[i].length != dir[i].length) break;
  if (memcmp(father+father_dir[i].start,son+dir[i].start,dir[i].length)) break;
  }
start_dir=i;
nb_dp=ifather_dir-i;
url[0]=0;
for (i=(father_isdir?0:1); i<nb_dp; i++) {
  sprintf(url+strlen(url),"../");
  }
for (i=start_dir; i<idir; i++) {
  char *slash=(i+1==idir)?"":"/";
  sprintf(url+strlen(url),"%.*s%s",dir[i].length,son+dir[i].start,slash);  
  }

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog,where,0
  , "localize result   : '%s'\n", url);
#endif

DONE;
}






static int OgGetDir(char *path, int *idi, struct og_dir *di)
{
int i, c, state=1, inp=0, end=0; 
for (i=0; !end; i++) {
  c = path[i]; if (c==0) { c='/'; end=1; }
  switch (state) {
    /** at the beginning or between directory names **/
    case 1:
      if (c=='/' || c=='\\') state=1;
      else {
        di[inp].start = i; di[inp].length = 0;
        state=2;
        } 
      break;
    /** in directory name **/
    case 2:
      if (c=='/' || c=='\\') {
        di[inp].length = i - di[inp].start;
        inp++; state=1;
        }
      break;
    }
  } 
*idi=inp;
DONE;
}


