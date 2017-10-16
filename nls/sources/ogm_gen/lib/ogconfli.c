/*
 *	More utilities for reading ogm_conf.txt file 
 *	Copyright (c) 2004 Pertimm by Patrick Constant
 *	Dev : September 2004
 *	Version 1.0
*/
#include <loggen.h>
#include <logpath.h>
#include <string.h>


#define DOgValueNumber  10
#define DOgBaSize       (DOgValueNumber*256)


struct og_conf_lines_info {
  struct og_conf_lines *conf_lines;
  int allocated; char *where;

  int ValueNumber;                 
  int ValueUsed;
  int *Value;

  int BaSize;                 
  int BaUsed;
  char *Ba;
  };


static int OgConfGetVarLines1(struct og_conf_lines_info *, char *);
static int AllocValue(struct og_conf_lines_info *, int **);
static int OgConfGetVarLinesAppendBa(struct og_conf_lines_info *, int, char *);
static int OgConfGetVarLinesTestReallocBa(struct og_conf_lines_info *, int);
static int OgConfGetVarLinesReallocBa(struct og_conf_lines_info *, int);



/*
 *  Tries to find in the file 'filename' the value of a variable
 *  named 'varname' and put this value in 'value'
 *  The syntaxe is the following:
 *    [varname]
 *    value1
 *    value2
 *  value1, value2 is the whole following line.
 *  Section is finished at the the start of a new section [section] or end of file.
 *  returns 1 if at least one variable is found, 0 else
 *  returns -1 on error.
 *  if return value is 1, it is necessary to free the allocated memory
 *  by using function OgConfGetVarLinesFree(conf_lines);
*/

PUBLIC(int) OgConfGetVarLines(char *filename_in, char *varname, struct og_conf_lines *conf_lines, char *where)
{
FILE *fd;
char erreur[DPcSzErr],filename[DPcPathSize],*DOgMAIN,*DOgINSTANCE;
char lwvarname[1024], buffer[1024];
struct og_conf_lines_info cinfo,*info=&cinfo;
int i,state,ivarname;

if(!Ogstricmp(filename_in,DOgFileOgmConf_Txt) && (DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  sprintf(filename,"%s/%s/%s/%s",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE,DOgFileOgmConf_Txt_instance);  }
else {
  strcpy(filename,filename_in); }

IFn(fd=fopen(filename,"r")) {
  DPcSprintf(erreur,"OgConfGetVar: impossible to open '%s'",filename);
  PcErr(-1,erreur); DPcErr;  
  }

IFn(ivarname=strlen(varname)) {
  DPcSprintf(erreur,"OgConfGetVar: null variable name");
  PcErr(-1,erreur); DPcErr;  
  }

strcpy(lwvarname,varname); PcStrlwr(lwvarname);

memset(info,0,sizeof(struct og_conf_lines_info));
info->conf_lines = conf_lines;
info->where = where;

state=1;
while(fgets(buffer,1024,fd)) {
  int ibuffer=strlen(buffer);
  if (buffer[ibuffer-1]=='\n') buffer[--ibuffer]=0;

  if (state==1) {
    for(i=0; i<ibuffer; i++) { 
      if (buffer[i]==';') { ibuffer=i; buffer[i]=0; break; }  
      if (buffer[i]=='[') break;
      }
    if (buffer[i]=='[') {
      PcStrlwr(buffer);
      IFx(strstr(buffer,lwvarname)) state=2;
      }
    }
  else if (state==2) {
    for(i=0; i<ibuffer; i++) { 
      if (buffer[i]==';') { ibuffer=i; buffer[i]=0; break; }  
      }
    OgTrimString(buffer,buffer);
    if (buffer[0]=='[') break;
    IFE(OgConfGetVarLines1(info,buffer));
    }
  }
fclose(fd);
if (info->allocated) {
  info->conf_lines->nb_values = info->ValueUsed;
  info->conf_lines->value = info->Value;
  info->conf_lines->buffer = info->Ba;
  return(1);
  }
else return(0);
}





PUBLIC(int) OgConfGetVarLinesFlush(struct og_conf_lines *conf_lines)
{
DPcFree(conf_lines->buffer);
DPcFree(conf_lines->value);
DONE;
}






static int OgConfGetVarLines1(struct og_conf_lines_info *info, char *line)
{
char erreur[DPcSzErr];
int size,Ivalue,iline;

IFn(line[0]) DONE;

IFn(info->allocated) {
  info->ValueNumber = DOgValueNumber;
  size = info->ValueNumber*sizeof(int);
  IFn(info->Value=(int *)malloc(size)) {
    sprintf(erreur,"OgConfGetVarLines1: malloc error on Value (%d bytes)",size);
    PcErr(-1,erreur); return(0);
   }

  info->BaSize = DOgBaSize;
  size = info->BaSize*sizeof(char);
  IFn(info->Ba=(char *)malloc(size)) {
    sprintf(erreur,"OgConfGetVarLines1: malloc error on Ba (%d bytes)",size);
    PcErr(-1,erreur); return(0);
    } 
  info->allocated=1;

  }

IFE(Ivalue=AllocValue(info,0));

info->Value[Ivalue]=info->BaUsed;
iline=strlen(line)+1; /* for the zero at the end */
IFE(OgConfGetVarLinesAppendBa(info,iline,line));

DONE;
}





static int AllocValue(struct og_conf_lines_info *info, int **pvalue)
{
int i = info->ValueNumber;
int *value = 0;

beginAllocValue:

if (info->ValueUsed < info->ValueNumber) {
  i = info->ValueUsed++; 
  value = info->Value + i;
  }

if (i == info->ValueNumber) {
  char erreur[DPcSzErr];
  unsigned a, b; int *og_value;
  if (info->where) {
    OgMessageLog( DOgMlogInLog, info->where, 0
                , "OgConfGetVarLines: max value number (%d) reached"
                , info->ValueNumber);
    }
  a = info->ValueNumber; b = a + (a>>2) + 1;
  IFn(og_value=(int *)malloc(b*sizeof(int))) {
    sprintf(erreur,"GetValue: malloc error on Value");
    PcErr(-1,erreur); DPcErr;
    }

  memset( og_value + a, 0, (b-a)*sizeof(int));
  memcpy( og_value, info->Value, a*sizeof(int));
  
  DPcFree(info->Value); info->Value = og_value;
  info->ValueNumber = b;

  if (info->where) {
    OgMessageLog( DOgMlogInLog, info->where, 0
                , "OgConfGetVarLines: new value number is %d\n", info->ValueNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur, "OgConfGetVarLines: no more string workspace max is %d", info->ValueNumber);
  PcErr(-1,erreur); DPcErr;
#endif

  goto beginAllocValue;
  }

if (pvalue) *pvalue = value;
return(i);  
}





static int OgConfGetVarLinesAppendBa(struct og_conf_lines_info *info, int is, char *s)
{
if (is<=0) DONE;
IFE(OgConfGetVarLinesTestReallocBa(info,is));
memcpy(info->Ba + info->BaUsed, s, is);
info->BaUsed += is;
DONE;
}




static int OgConfGetVarLinesTestReallocBa(struct og_conf_lines_info *info, int added_size)
{
/** +9 because we want to have always extra chars at the end for safety reasons **/
if (info->BaUsed + added_size +9 > info->BaSize) {
  IFE(OgConfGetVarLinesReallocBa(info,added_size));
  }
DONE;
}




static int OgConfGetVarLinesReallocBa(struct og_conf_lines_info *info, int added_size)
{
unsigned a, a1, b; unsigned char *og_buffer;
char erreur[DPcSzErr];

if (info->where) {
  OgMessageLog( DOgMlogInLog, info->where, 0
              , "OgConfGetVarLinesReallocBa from ogm_url: max Ba size (%d) reached, added_size=%d"
              , info->BaSize,added_size);
  }
a = info->BaSize; a1 = a+added_size; b = a1 + (a1>>2) + 1;
IFn(og_buffer=(unsigned char *)malloc(b*sizeof(unsigned char))) {
  sprintf(erreur,"OgConfGetVarLinesReallocBa from ogm_url: malloc error on Ba");
  PcErr(-1,erreur); DPcErr;
  }

memcpy( og_buffer, info->Ba, a*sizeof(unsigned char));
DPcFree(info->Ba); info->Ba = og_buffer;
info->BaSize = b;

if (info->where) {
  OgMessageLog( DOgMlogInLog, info->where, 0
              , "OgConfGetVarLinesReallocBa from ogm_url: new Ba size is %d", info->BaSize);
  }

#ifdef DOgNoMainBufferReallocation
sprintf(erreur,"OgConfGetVarLinesReallocBa from ogm_url: BaSize reached (%d)",info->BaSize);
PcErr(-1,erreur); DPcErr;
#endif

DONE;
}



