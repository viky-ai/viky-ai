/*
 *  Generic function for handling an automaton that tells which word to delete.
 *	Copyright (c) 2003	Pertimm, Inc. by Patrick Constant
 *	Dev : July 2003
 *	Version 1.0
*/
#include "ogm_auta.h"


#define DOgDeleteWordPeriod     30000 /* 30 seconds between two checks of delete.txt */
#define DOgFileLingDelete_Txt	"ling/delete.txt"
#define DOgFileLingDelete_Auf	"ling/delete.auf"

#define DOgAutDeleteWordSize    0x1000 /* 4k */

struct og_ctrl_delete_word {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo; 
  char delete_txt[DPcPathSize];
  char delete_auf[DPcPathSize];
  time_t clock_start;
  time_t og_mtime;
  void *ha;
  };

struct og_word_info {
  int iword; unsigned char *word;
  int language,keep_accent,keep_case,del;
  };



STATICF(int) OgDeleteWordCompile(pr(struct og_ctrl_delete_word *));
STATICF(int) OgDeleteWordCompile1(pr_(struct og_ctrl_delete_word *) pr(unsigned char *));
STATICF(int) OgDeleteWord1(pr_(struct og_ctrl_delete_word *) pr_(int) 
                           pr_(int) pr_(unsigned char *) pr_(int) pr_(unsigned char *) 
                           pr(struct og_word_info *));




PUBLIC(void *) OgDeleteWordInit()
{
struct og_aut_param caut_param, *aut_param=&caut_param;
struct og_ctrl_delete_word *ctrl_delete_word;
char erreur[DPcSzErr];
struct stat filestat;
int must_compile;

IFn(ctrl_delete_word=(struct og_ctrl_delete_word *)malloc(sizeof(struct og_ctrl_delete_word))) {
  sprintf(erreur,"OgCoocInit: malloc error on ctrl_delete_word");
  PcErr(-1,erreur); return(0);
  }

memset(ctrl_delete_word,0,sizeof(struct og_ctrl_delete_word));
strcpy(ctrl_delete_word->delete_txt,DOgFileLingDelete_Txt);
strcpy(ctrl_delete_word->delete_auf,DOgFileLingDelete_Auf);
ctrl_delete_word->loginfo = &ctrl_delete_word->cloginfo;
ctrl_delete_word->ha = 0;


IFn(stat(ctrl_delete_word->delete_txt,&filestat)) {
  ctrl_delete_word->og_mtime = filestat.st_mtime;
  }
else {
  /** text file does not exist **/ 
  ctrl_delete_word->og_mtime=0;
  return(ctrl_delete_word);
  }

must_compile=0;
if (OgFileExists(ctrl_delete_word->delete_auf)) {
  IFn(stat(ctrl_delete_word->delete_auf,&filestat)) {
    if (ctrl_delete_word->og_mtime > filestat.st_mtime) must_compile=1;
    }
  else must_compile=1;
  }
else must_compile=1;

if (must_compile) {
  IF(OgDeleteWordCompile(ctrl_delete_word)) return(0);
  }
else {
  memset(aut_param,0,sizeof(struct og_aut_param));
  aut_param->herr=ctrl_delete_word->herr; 
  aut_param->hmutex=ctrl_delete_word->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
  aut_param->loginfo.where = ctrl_delete_word->loginfo->where;
  aut_param->state_number = 0;
  sprintf(aut_param->name,"auta delete_word");
  IFn(ctrl_delete_word->ha=OgAutInit(aut_param)) return(0);  
  IF(OgAufRead(ctrl_delete_word->ha,ctrl_delete_word->delete_auf)) return(0);
  }

ctrl_delete_word->clock_start=OgClock();

return(ctrl_delete_word);
}






STATICF(int) OgDeleteWordCompile(ctrl_delete_word)
struct og_ctrl_delete_word *ctrl_delete_word;
{
struct og_aut_param caut_param, *aut_param=&caut_param;
char line[DPcPathSize];
char erreur[DPcSzErr];
FILE *fd;
int i;

IFn(fd=fopen(ctrl_delete_word->delete_txt,"r")) {
  sprintf(erreur,"OgDeleteWordCompile: Impossible to open '%s'",ctrl_delete_word->delete_txt);
  PcErr(-1,erreur); DPcErr;
  }

IFE(OgAutFlush(ctrl_delete_word->ha));

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_delete_word->herr; 
aut_param->hmutex=ctrl_delete_word->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = ctrl_delete_word->loginfo->where;
aut_param->state_number = DOgAutDeleteWordSize;
sprintf(aut_param->name,"delete_word");
IFn(ctrl_delete_word->ha=OgAutInit(aut_param)) DPcErr;  

while(fgets(line,DPcPathSize,fd)) {
  strtok(line,"\n");
  for (i=0; line[i]; i++) if (line[i]=='#') { line[i]=0; break; }
  for (i=0; line[i]; i++) if (!isspace(line[i])) break;
  if (line[i]==0) continue;
  IFE(OgDeleteWordCompile1(ctrl_delete_word,line));
  }

fclose(fd);

/** Not really necessary, because size is small **/
/* IFn(OgAum(ctrl_delete_word->ha)) DPcErr; */
IFE(OgAuf(ctrl_delete_word->ha,0)); 
IFE(OgAufWrite(ctrl_delete_word->ha,ctrl_delete_word->delete_auf));

DONE;
}






STATICF(int) OgDeleteWordCompile1(ctrl_delete_word,line)
struct og_ctrl_delete_word *ctrl_delete_word;
unsigned char *line;
{
int i,j,c,state=1,end=0;
void *ha = ctrl_delete_word->ha;
int start_name=0,end_name=0,start_value=0;
struct og_word_info cinfo, *info=&cinfo;
int ibuffer; unsigned char buffer[1024];
int is; unsigned char *s;
int added;

memset(info,0,sizeof(struct og_word_info));

for (i=0; !end; i++) {
  if (line[i]) c=line[i]; 
  else { c=':'; end=1; }
  switch(state) {
    case 1:
      if (c==':') {
        info->word = line;
        info->iword = i;
        start_name=i+1;
        state=2;
        }
      break;
    case 2:
      if (c==':') {
        is = i-start_name; s = line+start_name;
        if (is==3 && !memcmp(s,"del",is)) info->del=1;
        start_name=i+1;
        }
      else if (c=='=') {
        end_name=i; start_value=i+1;
        state=3;
        }
      break;
    case 3:
      if (c==':') {
        is = end_name-start_name; s = line+start_name;
        if (is==1 && !memcmp(s,"l",is)) {
          is=i-start_value; s = line+start_value;
          if (is==2) {
            info->language=OgCodeToIso639(s);
            }
          }
        if (is==1 && !memcmp(s,"k",is)) {
          is=i-start_value; s = line+start_value;
          if (is==2) {
            if (!memcmp(s,"ka",is)) info->keep_accent=1;
            if (!memcmp(s,"kc",is)) info->keep_case=1;
            }
          }
        start_name=i+1;
        state=2;
        }
      break;
    }
  }

memcpy(buffer,info->word,info->iword); ibuffer=info->iword;
for (i=0; i<info->iword; i++) {
  buffer[i]=PcTolower(PcUnaccent(info->word[i]));
  if (buffer[i]=='-') buffer[i]=' ';
  }
ibuffer=i;

added=0;
buffer[ibuffer++]=':'; memcpy(buffer+ibuffer,OgIso639ToCode(info->language),2); ibuffer+=2;
if (info->del) {
  i=ibuffer; memcpy(buffer+i,":del",4); i+=4;
  IFE(OgAutAdd(ha,i,buffer)); added=1;
  }
if (info->keep_accent) {
  i=ibuffer; memcpy(buffer+i,":ka",3); i+=3;
  IFE(OgAutAdd(ha,i,buffer)); added=1;
  }
if (info->keep_case) {
  i=ibuffer; memcpy(buffer+i,":kc",3); i+=3;
  IFE(OgAutAdd(ha,i,buffer)); added=1;
  }
if (info->keep_accent || info->keep_case) {
  i=ibuffer; memcpy(buffer+i,":w=",3); i+=3;
  if (info->keep_accent && info->keep_case) {
    for (j=0; j<info->iword; j++) {
      buffer[i+j]=info->word[j];
      if (buffer[i+j]=='-') buffer[i+j]=' ';
      }
    i += j;
    }
  else if (info->keep_accent) {
    for (j=0; j<info->iword; j++) {
      buffer[i+j]=PcTolower(info->word[j]);
      if (buffer[i+j]=='-') buffer[i+j]=' ';
      }
    i += j;
    }
  else if (info->keep_case) {
    for (j=0; j<info->iword; j++) {
      buffer[i+j]=PcUnaccent(info->word[j]);
      if (buffer[i+j]=='-') buffer[i+j]=' ';
      }
    i += j;
    }
  IFE(OgAutAdd(ha,i,buffer)); added=1;
  }

if (!added) {
  IFE(OgAutAdd(ha,ibuffer,buffer));
  }

DONE;
}





/*
 *  Returns 0 is word is ok, 1, if word should be deleted from coocurrences and glimpses
 *  Returns 2 if word should be deleted from index and search. Return -1 on error.
*/

PUBLIC(int) OgDeleteWord(ptr,is_unicode,iword,word,language)
void *ptr; int is_unicode,iword; unsigned char *word;
int language;
{
struct og_ctrl_delete_word *ctrl_delete_word = (struct og_ctrl_delete_word *)ptr;
int retour,nstate0,nstate1,iline,iout; oindex states[DPcAutMaxBufferSize+10];
struct og_word_info cinfo, *info=&cinfo;
unsigned char line[1024],out[1024];
int i,iw; unsigned char w[1024];
int irw; unsigned char rw[1024];
time_t clock_end = OgClock();
int word_language,deleted;
struct stat filestat;

IFn(ptr) return(0);

if (clock_end-ctrl_delete_word->clock_start > DOgDeleteWordPeriod) {
  IFn(stat(ctrl_delete_word->delete_txt,&filestat)) {
    if (filestat.st_mtime != ctrl_delete_word->og_mtime) {
      IFE(OgDeleteWordCompile(ctrl_delete_word));
      }
    }
  }
IFn(ctrl_delete_word->ha) return(0);

memset(info,0,sizeof(struct og_word_info)); irw=0;
info->language=-1;

/*  possible values are:
 *    word:fr
 *    word:--
 *    word:en:del
 *    word:fr:ka
 *    word:fr:kc
 *    word:fr:w=cheval
 *  we always have a language ("--" is no language) and then possibly
 *  :del :ka or :kc */
if (is_unicode) { OgUtoA(word,w,iword); iw=strlen(w); }
else { memcpy(w,word,iword); iw=iword; }

for (i=0; i<iw; i++) {
  line[i]=PcTolower(PcUnaccent(w[i]));
  if (line[i]=='-') line[i]=' ';
  }
line[i++]=':'; iline=i; word_language=-1;
if ((retour=OgAufScanf(ctrl_delete_word->ha,iline,line,&iout,out,&nstate0,&nstate1,states))) {
  do {
    if (retour == ERREUR)
      break;
    if (iout < 2) continue;
    word_language = OgCodeToIso639(out);
    if (word_language != info->language) {
      if (info->language != -1) {
        info->language = word_language;
        IFE(deleted=OgDeleteWord1(ctrl_delete_word,language,iw,w,irw,rw,info));
        memset(info,0,sizeof(struct og_word_info)); irw=0;
        if (deleted) return(deleted);
        }
      }
    if (iout < 5) continue;
    if (!memcmp(out+3,"ka",2)) info->keep_accent=1;
    if (!memcmp(out+3,"kc",2)) info->keep_case=1;
    if (!memcmp(out+3,"w=",2)) irw=iout-5; memcpy(rw,out+5,irw);
    if (iout < 6) continue;
    if (!memcmp(out+3,"del",3)) info->del=1;
    IFE(retour);
    }
  while((retour=OgAufScann(ctrl_delete_word->ha,&iout,out,nstate0,&nstate1,states)));
  }

if (word_language != info->language) {
  info->language = word_language;
  IFE(deleted=OgDeleteWord1(ctrl_delete_word,language,iw,w,irw,rw,info));
  if (deleted) return(deleted);
  }
return(0);
}






STATICF(int) OgDeleteWord1(ctrl_delete_word,language,iw,w,irw,rw,info)
struct og_ctrl_delete_word *ctrl_delete_word;
int language,iw; unsigned char *w;
int irw; unsigned char *rw;
struct og_word_info *info;
{
int valid_language=0,valid_keep=0;
int i,iword; unsigned char word[1024];

if (language == DOgLangNil) valid_language=1;
if (info->language == DOgLangNil) valid_language=1;
if (info->language == language) valid_language=1;
if (!valid_language) return(0);

if (info->keep_accent && info->keep_case) {
  if (iw==irw && !memcmp(w,rw,iw)) valid_keep=1;
  }
else if (info->keep_accent) {
  iword=iw; for (i=0; i<iw; i++) word[i]=PcTolower(w[i]);
  if (iw==iword && !memcmp(word,rw,iword)) valid_keep=1;
  }
else if (info->keep_case) {
  iword=iw; for (i=0; i<iw; i++) word[i]=PcUnaccent(w[i]);
  if (iw==iword && !memcmp(word,rw,iword)) valid_keep=1;
  }
else valid_keep=1;
if (!valid_keep) return(0);
if (info->del) return(2);
return(1);
}






PUBLIC(int) OgDeleteWordFlush(ptr)
void *ptr;
{
struct og_ctrl_delete_word *ctrl_delete_word = (struct og_ctrl_delete_word *)ptr;
IFn(ptr) DONE;
IFE(OgAutFlush(ctrl_delete_word->ha));
DPcFree(ctrl_delete_word);
DONE;
}



