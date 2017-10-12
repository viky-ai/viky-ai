/*
 *  Used for file selection in ogm_conf.txt, sections
 *  [DirSelect], [FileSelect], [FileSystemSelect]
 *  Copyright (c) 1999-2000  Ogmios by Patrick Constant
 *  Dev : December 1999, January 2000
 *  Version 1.2
*/
#include <logfix.h>
#include <loggen.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <logpath.h>

#define DEVERMINE

#define DOgMaxSelectionPerLine  7

struct og_select_line {
  char type; int nb_start;
  int start[DOgMaxSelectionPerLine];
  int length[DOgMaxSelectionPerLine];
  };

struct og_select_section {
  int isl;  struct og_select_line *sl;
  unsigned char *B;
  };

/*  'ds' is DirSelect section
 *  'fs' is FileSelect section
 *  'fss' is FileSystemSelect section
*/
struct og_select {
  struct og_select_section ds;
  struct og_select_section fs;
  struct og_select_section fss;
  int MajToMin;
  char *where;
  };


static int OgMatchCanon(char *,
                 struct og_select_section *,
                 char *);
static int OgGetCanon(char *, char *);
static int OgCompileSelectSection(struct og_select *, const char *, const char *,
                                    struct og_select_section *, const char *);
static int AddSelectLine(char, struct og_select_section *,
                           int, int);
static int ModifyCanon(int, char *, int *, char *);






PUBLIC(void *) OgFileSelectInit(char *ogm_conf_in, const char *exclude_path, char *where)
{
int retour;
char value[256],ogm_conf[DPcPathSize],*DOgMAIN,*DOgINSTANCE;
char erreur[DPcSzErr];
struct og_select *ogselect;

if(!Ogstricmp(ogm_conf_in,DOgFileOgmConf_Txt) && (DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  sprintf(ogm_conf,"%s/%s/%s/%s",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE,DOgFileOgmConf_Txt_instance);  }
else {
  strcpy(ogm_conf,ogm_conf_in); }

IFn(ogselect=(struct og_select *)malloc(sizeof(struct og_select))) {
  DPcSprintf(erreur,"OgFileSelectInit: impossible to malloc ogselect (%d bytes)"
            ,sizeof(struct og_select));
  PcErr(-1,erreur); return(0);
  }

memset(ogselect,0,sizeof(struct og_select));
ogselect->where = where;

IF(retour=OgConfGetVar(ogm_conf,"MajToMin",value,256,1)) return(0);
if (retour) {
  OgMessageLog( DOgMlogInLog, where, 0
              , "MajToMin is: '%s'", value);
  if (value[0]=='y') ogselect->MajToMin = 1;
  else ogselect->MajToMin = 0;
  }
else {
  OgMessageLog( DOgMlogInLog, where, 0
    ,"MajToMin not defined in '%s', using 'yes'",ogm_conf);
  ogselect->MajToMin = 1;
  }
OgMessageLog( DOgMlogInLog, where, 0
            , "MajToMin is %d",ogselect->MajToMin);

IF(OgCompileSelectSection(ogselect,ogm_conf
  ,"DirSelect",&(ogselect->ds),0)) return(0);
IF(OgCompileSelectSection(ogselect,ogm_conf
  ,"FileSelect",&(ogselect->fs),0)) return(0);

IF(OgCompileSelectSection(ogselect,ogm_conf
  ,"FileSystemSelect",&(ogselect->fss),exclude_path)) return(0);

return((void *)ogselect);
}







PUBLIC(void *) OgFileSelectZoneInit(char *ogm_conf, char *zone, char *where)
{
int retour;
char value[256];
char erreur[DPcSzErr];
struct og_select *ogselect;

IFn(ogselect=(struct og_select *)malloc(sizeof(struct og_select))) {
  DPcSprintf(erreur,"OgFileSelectInit: impossible to malloc ogselect (%d bytes)"
            ,sizeof(struct og_select));
  PcErr(-1,erreur); return(0);
  }

memset(ogselect,0,sizeof(struct og_select));
ogselect->where = where;

IF(retour=OgConfGetVar(ogm_conf,"MajToMin",value,256,1)) return(0);
if (retour) {
  OgMessageLog( DOgMlogInLog, where, 0
              , "MajToMin is: '%s'", value);
  if (value[0]=='y') ogselect->MajToMin = 1;
  else ogselect->MajToMin = 0;
  }
else {
  OgMessageLog( DOgMlogInLog, where, 0
    ,"MajToMin not defined in '%s', using 'yes'",ogm_conf);
  ogselect->MajToMin = 1;
  }
OgMessageLog( DOgMlogInLog, where, 0
            , "MajToMin is %d",ogselect->MajToMin);

IF(OgCompileSelectSection(ogselect,ogm_conf
  ,zone,&(ogselect->ds),0)) return(0);

return((void *)ogselect);
}










/*
 *  Performs the selection of every file names
 *  contained in 'filename' (one per line)
 *  Each time a file is selected, the function 'func'
 *  is started with an og_selected structure filled with
 *  the following information:
 *    name and length of name of file
 *    type of selection ('x' or 'i' or 'n' for none)
 *    for the three existing sections.
*/
PUBLIC(int) OgFileSelect(void *handle, char *filename, int (*func) (void *, struct og_selected *), void *ptr)
{
FILE *fd;
char buffer[1024];
char erreur[DPcSzErr];

IFn(fd=fopen(filename,"r")) {
  DPcSprintf(erreur,"OgFileSelect: impossible to open '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }

while(fgets(buffer,1024,fd)) {
  int ibuffer=strlen(buffer);
  if (buffer[ibuffer-1]=='\n') buffer[--ibuffer]=0;
  IFE(OgFileSelectLine(handle,ibuffer,buffer,func,ptr));
  }

DONE;
}





PUBLIC(int) OgFileSelectFlush(void *handle)
{
struct og_select *ogselect = (struct og_select *)handle;

IFn(handle) DONE;
DPcFree(ogselect->ds.sl);  DPcFree(ogselect->ds.B);
DPcFree(ogselect->fs.sl);  DPcFree(ogselect->fs.B);
DPcFree(ogselect->fss.sl); DPcFree(ogselect->fss.B);
DPcFree(ogselect);

DONE;
}






PUBLIC(int) OgFileSelectZoneFlush(void *handle)
{
struct og_select *ogselect = (struct og_select *)handle;

IFn(handle) DONE;
DPcFree(ogselect->ds.sl);  DPcFree(ogselect->ds.B);
DPcFree(ogselect);

DONE;
}






PUBLIC(int) OgFileSelectLine(void *handle, int ifile, char *file, int (*func) (void *, struct og_selected *), void *ptr)
{
struct og_select *ogselect = (struct og_select *)handle;
int icanon; char canon[DPcPathSize*2];
struct og_selected selected;
char type;

IFE(OgGetCanon(file,canon)); icanon = strlen(canon);
memset(&selected,0,sizeof(struct og_selected));
IFE(OgMatchCanon(canon,&ogselect->ds,&type));
selected.type_ds = type;
IFE(OgMatchCanon(canon,&ogselect->fs,&type));
selected.type_fs = type;
IFE(OgMatchCanon(canon,&ogselect->fss,&type));
selected.icanon = icanon; selected.canon = canon;
selected.ifile = ifile; selected.file = file;
selected.type_fss = type;

#ifdef DEVERMINE
if (ogselect->where) {
  OgMessageLog( DOgMlogInLog, ogselect->where, 0
     , "'%s' -> '%s' ds=%c fs=%c fss=%c",file,canon
    , selected.type_ds, selected.type_fs, selected.type_fss);
  }
#endif

if (func) {
  IFE((*func)(ptr,&selected));
  }
DONE;
}






PUBLIC(int) OgFileSelectZoneLine(void *handle, int ifile, char *file, int (*func) (void *, struct og_selected *), void *ptr)
{
struct og_select *ogselect = (struct og_select *)handle;
int icanon; char canon[DPcPathSize*2];
struct og_selected selected;
char type;

IFE(OgGetCanon(file,canon)); icanon = strlen(canon);
memset(&selected,0,sizeof(struct og_selected));
IFE(OgMatchCanon(canon,&ogselect->ds,&type));
selected.type_ds = type;
selected.icanon = icanon; selected.canon = canon;
selected.ifile = ifile; selected.file = file;

#ifdef DEVERMINE
if (ogselect->where) {
  OgMessageLog( DOgMlogInLog, ogselect->where, 0
     , "'%s' -> '%s' ds=%c fs=%c fss=%c",file,canon
    , selected.type_ds, selected.type_fs, selected.type_fss);
  }
#endif

if (func) {
  IFE((*func)(ptr,&selected));
  }
DONE;
}





static int OgMatchCanon(char *canon, struct og_select_section *ss, char *type)
{
char *s;
int i,j,matched;
*type = 'n';
for (i=0; i<ss->isl; i++) {
  matched=1; s = canon;
  for (j=0; j<ss->sl[i].nb_start; j++) {
    IFn(s = strstr(s,ss->B + ss->sl[i].start[j])) { matched=0; break; }
    }
  if (matched) { *type = ss->sl[i].type; return(1); }
  }
return(0);
}



/*
 *  Calculated the canonical form of a file, according
 *  to the description in ogm_conf.txt. Bundled files
 *  (zip or Lotus Notes files for example) have the form
 *  c:\toto.zip<sep>zip<sep>a/b.txt<sep><some other info>
 *  are transformed into something like:
 *  [c:\toto;zip:zip:a;/b.txt]
*/

static int OgGetCanon(char *file, char *canon)
{
int i,j,k=0,len=strlen(file);
int dir,slash,point=0,iref=0;

canon[k++]='['; dir=1;
for (i=0; i<len; i++) {
  if (file[i]==DOgSepReference) {
    if (iref < 2) {
      point=0; canon[k++]=':';
      if (iref == 1) {
        for (dir=0,j=i+1; j<len && file[j] != DOgSepReference; j++) {
          if (file[j]=='\\' || file[j]=='/') { dir=1; break; }
          }
        if (!dir) canon[k++]='+';
        }
      iref++;
      continue;
      }
    /** Third separator indicates end of filename **/
    else break;
    }
  if (file[i]=='\\' || file[i]=='/') {
    if (point==0 && i>0 && file[i-1]!=':' && file[i-1]!='[') canon[k++]=';';
    point=0;
    for (dir=0,j=i+1; j<len && file[j] != DOgSepReference; j++) {
      if (file[j]=='\\' || file[j]=='/') { dir=1; break; }
      }
    slash=1;
    }
  else slash=0;
  /**if (dir && file[i]=='.') { point=1; canon[k++]=';'; } **/
  if (file[i]=='.') {
    point=1;
    if (dir) canon[k++]=';';
    else canon[k++]=PcTolower(file[i]);
    }
  else canon[k++]=PcTolower(file[i]);
  if (slash && !dir) canon[k++]='+';
  }
if (!point) canon[k++]='.';
canon[k++]=']';
canon[k]=0;
/** removing all differences between '/' and '\\' **/
for (i=0; canon[i]; i++) {
  if (canon[i]=='\\') canon[i]='/';
  }
DONE;
}





static int OgCompileSelectSection(struct og_select *ogselect, const char *filename,
                                    const char *section, struct og_select_section *ss,
                                    const char *exclude_path)
{
FILE *fd;
char erreur[DPcSzErr];
char lwsection[1024], buffer[1024], buf[1024];
int iline, start_line, end_line;
int nbsl,nbi,nbx,sB=0,iB;
int i,j,state,isection;
int ibuffer,ibuf;

IFn(fd=fopen(filename,"r")) {
  DPcSprintf(erreur,"OgCompileSelectSection: impossible to open '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }

IFn(isection=strlen(section)) {
  DPcSprintf(erreur,"OgCompileSelectSection: null variable name");
  PcErr(-1,erreur); DPcErr;
  }

strcpy(lwsection,section);
PcStrlwr(lwsection);

nbi=nbx=0;
state=1; start_line=end_line=iline=0;
while(fgets(buffer,1024,fd)) {
  ibuffer=strlen(buffer); iline++;
  if (buffer[0]==';') continue;
  for(i=0; i<ibuffer; i++) {
    /**if (buffer[i]==';') { ibuffer=i; buffer[i]=0; break; }   **/
    if (!isspace((int)buffer[i])) break;
    if (buffer[i]=='[') break;
    }
  if (state==1) {
    if (buffer[i]=='[') {
      PcStrlwr(buffer);
      IFx(strstr(buffer,lwsection)) {
        start_line=iline; state=2;
        }
      }
    }
  else if (state==2) {
    if (buffer[i]=='[') { end_line=iline; break; }
    for(i=0; i<ibuffer; i++) {
      if (PcIsspace(buffer[i])) continue;
      else if (buffer[i]=='i' || buffer[i]=='I') { nbi++; sB+=ibuffer; }
      else if (buffer[i]=='x' || buffer[i]=='X') { nbx++; sB+=ibuffer; }
      break;
      }
    }
  }

if (start_line==0) {
#ifdef DEVERMINE
  if (ogselect->where) {
    OgMessageLog( DOgMlogInLog, ogselect->where, 0
      , "OgCompileSelectSection: no section '%s'",section);
    }
#endif
  DONE;
  }
/** This happens when arriving at the end of the file **/
if (end_line==0) end_line=iline+1;

IFn(sB) DONE;

/* Buffer can be bigger because of ';' that are added
 * when canonizing the select lines, thus we multiply by 2 */
sB*=2;
nbsl=nbi+nbx;
IFn(nbsl) DONE;
if (exclude_path) {
  sB += DPcPathSize;
  nbsl++;
  }

IFn(ss->B=(unsigned char *)malloc(sB+10)) {
  DPcSprintf(erreur,"OgCompileSelectSection: impossible to malloc ss->B (%d bytes)",sB);
  PcErr(-1,erreur); DPcErr;
  }


IFn(ss->sl=(struct og_select_line *)malloc(nbsl*sizeof(struct og_select_line))) {
  DPcSprintf(erreur,"OgCompileSelectSection: impossible to malloc ss->sl (%d bytes)"
            ,nbsl*sizeof(struct og_select_line));
  PcErr(-1,erreur); DPcErr;
  }
memset(ss->sl,0,nbsl*sizeof(struct og_select_line));

rewind(fd);

iline=0; iB=0; ss->isl=0;
while(fgets(buffer,1024,fd)) {
  ibuffer=strlen(buffer);
  if (buffer[ibuffer-1]=='\n') buffer[--ibuffer]=0;
  iline++;
  if (iline <= start_line) continue;
  if (iline >= end_line) break;
  if (buffer[0]==';') continue;
  PcStrlwr(buffer);
  for(i=0; i<ibuffer; i++) {
    if (PcIsspace(buffer[i])) continue;
    else if (buffer[i]=='i' || buffer[i]=='x') {
      IFE(ModifyCanon(ibuffer,buffer,&ibuf,buf));
      if (ibuf-i > 0) {
        memcpy(ss->B+iB, buf+i, ibuf-i);
        IFE(AddSelectLine(buffer[i],ss,iB,ibuf-i));
        /** +1 for the zero at the end of the string **/
        iB += ibuf-i+1; ss->isl++;
        }
      }
    break;
    }
  }

fclose(fd);

if (exclude_path) {
  char work_exclude_path[DPcPathSize];
  int len;
  strncpy(work_exclude_path, exclude_path, sizeof work_exclude_path);
  len = strlen(work_exclude_path);
  if (work_exclude_path[len-1] != '\\' && work_exclude_path[len-1] != '/') {
    strcpy(work_exclude_path+len,DPcDirectorySeparator);
    }
  PcStrlwr(work_exclude_path);
  OgGetCanon(work_exclude_path,buffer);
  ibuffer = strlen(buffer);
  /** to remove the ending '+.]' **/
  buffer[ibuffer-3] = 0; ibuffer -= 3;
  IFE(ModifyCanon(ibuffer,buffer,&ibuf,buf));
  memcpy(ss->B+iB, buf, ibuf);
  IF(AddSelectLine('x',ss,iB,ibuf)) return(0);
  /** +1 for the zero at the end of the string **/
  iB += ibuf+1; ss->isl++;
  }

#ifdef DEVERMINE
if (ogselect->where) {
  OgMessageLog( DOgMlogInLog, ogselect->where, 0
              , "Analysed section: '%s'",section);
  for (i=0; i<ss->isl; i++) {
    sprintf(buffer, "%3d:%c: ", i, ss->sl[i].type);
    for (j=0; j<ss->sl[i].nb_start; j++) {
      sprintf( buffer+strlen(buffer), "'%s' "
             , ss->B + ss->sl[i].start[j]);
      }
    OgMessageLog( DOgMlogInLog, ogselect->where, 0, "%s", buffer);
    }
  }
#endif

DONE;
}




/*
 *  When the specification is something like ...aaa\...
 *  it is obvious that a ';' should be added except if there is
 *  one before. Thus getting ...aaa;\...
*/

static int ModifyCanon(int ibuffer, char *buffer, int *ibuf, char *buf)
{
int i,j,semicoma_here=0;
for (j=0,i=0; i<ibuffer; i++) {
  if (i>0 && buffer[i-1]!=' ' && (buffer[i]=='/' || buffer[i]=='\\')) {
    if (!semicoma_here && i>0 && buffer[i-1]!=':' && buffer[i-1]!='[') buf[j++]=';';
    semicoma_here=0;
    }
  else if (buffer[i]==';') semicoma_here=1;
  buf[j++]=buffer[i];
  }
*ibuf = j;
buf[j]=0;
/** removing all differences between '/' and '\\' **/
for (i=0; buf[i]; i++) {
  if (buf[i]=='\\') buf[i]='/';
  }
DONE;
}





static int AddSelectLine(char type, struct og_select_section *ss, int startB, int iB)
{
int i,state=1;
struct og_select_line *sl = ss->sl;
int isl = ss->isl;

sl[isl].nb_start=0; sl[isl].type=type;

for (i=1; i<=iB; i++) {
  switch(state) {
    case 1:
      if (i<iB && !isspace(ss->B[startB+i])) {
        sl[isl].start[sl[isl].nb_start] = startB+i;
        state = 2;
        }
      break;
    case 2:
      if (i==iB || isspace(ss->B[startB+i])) {
        int n = sl[isl].nb_start;
        sl[isl].length[n] = startB+i - sl[isl].start[n];
        ss->B[sl[isl].start[n] + sl[isl].length[n]]=0;
        sl[isl].nb_start++;
        state = 1;
        }
      break;
    }
  }

DONE;
}





