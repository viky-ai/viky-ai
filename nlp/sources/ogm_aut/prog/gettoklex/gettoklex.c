/*
 *  Works on files such as frtotal.auf or entotal.auf and creates a source file
 *  for an automaton that contains toklex in one file: frtotal.toklex
 *  Copyright (c) 2003-2009 Pertimm by Patrick Constant
 *  Dev: May 2003, August 2008
 *  Version 1.1
*/
#include <logaut.h>

struct og_info {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  void *ha_gettoklex;
  };



STATICF(int) AddWord(pr_(FILE *fd) pr_(unsigned char *) pr_(int) pr(char *));
STATICF(int) AddSyncat(pr_(int *) pr_(char *) pr(char));


STATICF(void) OgExit(pr_(void *) pr(char *));


int main(argc,argv)
int argc;
char *argv[];
{
FILE *fd;
int i,retour,len,iout;
int isyncat; char syncat[128];
char name_automat[512], toklex_name[512];
struct og_aut_param caut_param,*aut_param=&caut_param;
oindex states[256]; int nstate0,nstate1;
unsigned char out[512],oldword[512];
struct og_info cinfo,*info=&cinfo;
ogmutex_t mutex;

if (argc < 2)
  { printf("Usage : %s <lexique>.<aut|auz|auf>\n",argv[0]); exit(0); }

memset(info,0,sizeof(struct og_info));
info->loginfo.where="gettoklex";

/** main thread error handle **/
IFn(info->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,info->loginfo.where,0,"gettoklex: OgErrInit error");
  return(1);
  }
/** general mutex handle **/
info->hmutex=&mutex;
IF(OgInitCriticalSection(info->hmutex,"gettoklex")) { 
  OgExit(info->herr,info->loginfo.where); return(1); 
  }

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=info->herr; 
aut_param->hmutex=info->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = info->loginfo.where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"gettoklex");
IFn(info->ha_gettoklex=OgAutInit(aut_param)) {
  OgExit(info->herr,info->loginfo.where); return(1); 
  }

strcpy(name_automat,argv[1]); len=strlen(name_automat);

memcpy(toklex_name,name_automat,len-4);
strcpy(toklex_name+len-4,".toklex");
if ((fd=fopen(toklex_name,"w"))==0) {
  printf("Impossible to open '%s'\n",toklex_name);
  OgExit(info->herr,info->loginfo.where); return(1);
  }

IF(OgAufRead(info->ha_gettoklex,name_automat)) {
  OgExit(info->herr,info->loginfo.where); return(1);
  }

oldword[0]=0; isyncat=0;

if ((retour=OgAufScanf(info->ha_gettoklex,-1,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    IF(retour) {
      OgExit(info->herr,info->loginfo.where); return(1);
      }
    if (!strstr(out,":cs:")) continue;
    //fprintf(fd,"%s\n",out);
    for (i=0; i<iout; i++) {
      if (out[i]==':') { out[i]=0; break; }
      }
    if (strcmp(out,oldword)) {
      if (oldword[0]) {
        IF(AddWord(fd,oldword,isyncat,syncat)) {
          OgExit(info->herr,info->loginfo.where); return(1);
          }
        isyncat=0;
        }
      strcpy(oldword,out);
      }
    for (i=0; i<iout; i++) {
      if (out[i]=='+') AddSyncat(&isyncat,syncat,out[i+1]);
      }
    }
  while((retour=OgAufScann(info->ha_gettoklex,&iout,out,nstate0,&nstate1,states)));
  }
fclose(fd);
return(0);
}



STATICF(int) AddWord(fd,word,isyncat,syncat)
FILE *fd; unsigned char *word;
int isyncat; char *syncat;
{
int i; char c;
syncat[isyncat]=0;
if (isyncat==1 && syncat[0]=='N') DONE;
for (i=0; i<isyncat; i++) {
  switch(syncat[i]) {
    case 'N': c='n'; break;
    case 'A': c='a'; break;
    case 'V': c='v'; break;
    case 'R': c='b'; break;
    default:  c='z';
    }
  if (c!='z') fprintf(fd,"%s:%c\n",word,c);
  }
DONE;
}



STATICF(int) AddSyncat(pisyncat,syncat,c)
int *pisyncat; char *syncat; 
char c;
{
int i,isyncat;
isyncat=*pisyncat;
for (i=0; i<isyncat; i++) {
  if (syncat[i] == c) DONE;
  }
syncat[isyncat++]=c;
*pisyncat=isyncat;
DONE;
}





/*
 *	Exit from program. Prints errors and does the flushing of 
 *	the memory when necessary (function PcAutFlush).
*/





STATICF(void) OgExit(herr,where)
void *herr; char *where;
{
int is_error;
is_error=OgErrLog(herr,where,0,0,0,0);
}


