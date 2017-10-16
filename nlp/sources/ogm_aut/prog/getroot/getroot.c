/*
 *  Works on files such as frall.auf or enall.auf and creates a source file
 *  for an automaton that contains root extractions in one file: frall.root
 *  a family list in a second file frall.fami, and form extractions in a third file
 *  Copyright (c) 1998-2003 Pertimm by Patrick Constant
 *  Dev: September 1998, March, November 2003
 *  Dev: August 2008
 *  Version 1.3
*/
#include <logaut.h>


struct og_info {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  void *ha_getroot;
  };


struct excluded_form {
  int is; unsigned char *s;
  };

struct excluded_form OgExcludedForm[] = {
  { 4, "Vmcp" },   /* Conditionel présent */
  { 4, "Vmif" },   /* Indicatif futur */
  { 4, "Vmii" },   /* Indicatif imparfait */
  { 6, "Vmip1p" },   /* Indicatif présent première personne */
  { 6, "Vmip2p" },   /* Indicatif présent deuxième personne */
//  { 6, "Vmip3p" },   /* Indicatif présent troisième personne */
  { 4, "Vmis" },   /* Indicatif passé */
  { 4, "Vmmp" },   /* Impératif présent */
  { 4, "Vmsi" },   /* Subjonctif imparfait */
  { 4, "Vmsp" },   /* Subjonctif présent */
  { 0, "" }
  };

int WriteRootFamily(pr_(FILE *) pr(unsigned char *));
int WriteForm(pr_(FILE *) pr_(FILE *) pr_(int) pr_(unsigned char *) pr(int));
int GetFemSing(pr_(struct og_info *) pr_(int) pr_(unsigned char *) pr_(unsigned char *) pr(unsigned char *));
int GetFemSing1(pr_(struct og_info *) pr_(unsigned char *) pr(unsigned char *));
int IsFemSing(pr_(struct og_info *) pr(unsigned char *));
int IsExcludedForm(pr(unsigned char *));
STATICF(void) OgExit(pr_(void *) pr(char *));


  int main(argc,argv)
int argc;
char *argv[];
{
int inewout=0,ioldout=0;
int inewform=0,ioldform=0;
int ifemsing=0,ifemplur=0;
unsigned char femsing[512];
FILE *fd_root, *fd_family, *fd_form, *fd_fform;
unsigned char newform[512],oldform[512],zipfami[512],fami[512];
unsigned char out[512],newout[512],oldout[512],termi[100];
char name_automat[512], root_name[512], fami_name[512], form_name[512], fform_name[512];
struct og_aut_param caut_param,*aut_param=&caut_param;
int i,j,retour,len,automat_type,iout,end_entry;
oindex states[256]; int nstate0,nstate1;
struct og_info cinfo,*info=&cinfo;
int newsyncat,syncat=0,offset;
ogmutex_t mutex;

if (argc < 2)
  { printf("Usage : %s <lexique>.<aut|auz|auf>\n",argv[0]); exit(0); }

memset(info,0,sizeof(struct og_info));
info->loginfo.where="getroot";

/** main thread error handle **/
IFn(info->herr=OgErrInit()) {
  OgMessageLog(DOgMlogInLog+DOgMlogInErr,info->loginfo.where,0,"getroot: OgErrInit error");
  return(1);
  }
/** general mutex handle **/
info->hmutex=&mutex;
IF(OgInitCriticalSection(info->hmutex,"getroot")) { 
  OgExit(info->herr,info->loginfo.where); return(1); 
  }

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=info->herr; 
aut_param->hmutex=info->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = info->loginfo.where;
aut_param->state_number = 0x1000;
sprintf(aut_param->name,"getroot");
IFn(info->ha_getroot=OgAutInit(aut_param)) {
  OgExit(info->herr,info->loginfo.where); return(1); 
   }

strcpy(name_automat,argv[1]); len=strlen(name_automat);
if (!strcmp(name_automat+len-4,".aut")) automat_type=1;
else if (!strcmp(name_automat+len-4,".aum")) automat_type=1;
else if (!strcmp(name_automat+len-4,".auz")) automat_type=2;
else if (!strcmp(name_automat+len-4,".auf")) automat_type=3;
else automat_type=0;

oldout[0]=0;
memcpy(root_name,name_automat,len-4);
strcpy(root_name+len-4,".root");
if ((fd_root=fopen(root_name,"wb"))==0) {
  printf("Impossible to open '%s'\n",root_name);
  OgExit(info->herr,info->loginfo.where); return(1);
  }
memcpy(fami_name,name_automat,len-4);
strcpy(fami_name+len-4,".fami");
if ((fd_family=fopen(fami_name,"wb"))==0) {
  printf("Impossible to open '%s'\n",fami_name);
  OgExit(info->herr,info->loginfo.where); return(1);
  }

memcpy(form_name,name_automat,len-4);
strcpy(form_name+len-4,".form");
if ((fd_form=fopen(form_name,"wb"))==0) {
  printf("Impossible to open '%s'\n",form_name);
  OgExit(info->herr,info->loginfo.where); return(1);
  }

memcpy(fform_name,name_automat,len-4);
strcpy(fform_name+len-4,".fform");
if ((fd_fform=fopen(fform_name,"wb"))==0) {
  printf("Impossible to open '%s'\n",fform_name);
  OgExit(info->herr,info->loginfo.where); return(1); 
  }

switch (automat_type) {
  case 1 :
    IF(OgAutRead(info->ha_getroot,name_automat)) {
      OgExit(info->herr,info->loginfo.where); return(1); 
      }
    if ((retour=OgAutScanf(info->ha_getroot,-1,"",&iout,out,&nstate0,&nstate1,states))) {
      do { IF(retour) {
        OgExit(info->herr,info->loginfo.where); return(1);
        }
      out[iout++]='\r'; out[iout++]='\n'; fwrite(out,1,iout,fd_root); }
      while((retour=OgAutScann(info->ha_getroot,&iout,out,nstate0,&nstate1,states)));
      }
    break;
  case 3 :
    IF(OgAufRead(info->ha_getroot,name_automat)) {
      OgExit(info->herr,info->loginfo.where); return(1);
      }
    if ((retour=OgAufScanf(info->ha_getroot,-1,"",&iout,out,&nstate0,&nstate1,states))) {
      do {
        int excluded_form=0;
		    IF(retour) {
          OgExit(info->herr,info->loginfo.where); return(1);
          }
        for (i=0; i<iout-4; i++) {
		  if (!memcmp(out+i,":cs:",4)) break;
		  }
		if (i>=iout-4) continue;
#if 1
		end_entry=i;
  	/** On ne garde pas les racines verbales **/
		/** si une racine nominale existe **/
		newsyncat=0;
		for (j=i; j<iout; j++) {
		  if (out[j]=='+')
		    if (out[j+1]=='N') { newsyncat=1; break; }
		    else if (out[j+1]=='V') { 
              excluded_form=IsExcludedForm(out+j+1);
              newsyncat=2; break; 
              }
		  }
		/** On garde la forme féminin singulier en memoire **/
		/** pour la racine des formes féminin (francais) **/
		if (!strcmp(out+iout-2,"fs")) {
		  ifemsing=end_entry;
		  IF(GetFemSing(info,end_entry,out,femsing,"fs")) {
        OgExit(info->herr,info->loginfo.where); return(1);
        }
		  //memcpy(femsing,out,end_entry); femsing[end_entry]=0;
		  }
		else ifemsing=0;
		if (!strcmp(out+iout-2,"fp")) {
		  ifemplur=end_entry;
		  IF(GetFemSing(info,end_entry,out,femsing,"fp")) {
        OgExit(info->herr,info->loginfo.where); return(1);
        }
		  }
		else ifemplur=0;

		memcpy(newform,out,end_entry); newform[end_entry]=0; inewform=end_entry;
		/** Lorsque la forme est identique, on ne rentre pas la forme si c'est **/
		/** une forme verbale et si il existe une forme nominale (francais) **/
		if (inewform==ioldform && !memcmp(newform,oldform,inewform)) {
		  if (newsyncat==2) if (syncat&1) continue;
		  syncat|=newsyncat;
		  }
        else syncat=newsyncat;
		ioldform=inewform; memcpy(oldform,newform,inewform); oldform[ioldform]=0;

  	if (!memcmp(out+i+4,"0+",2)) {
		  memcpy(fami,out,end_entry); fami[end_entry]=0;
		  IF(WriteRootFamily(fd_family,fami)) {
        OgExit(info->herr,info->loginfo.where); return(1);
        }
		  continue;
		  }
		for (; i<iout; i++) {
			if (out[i]=='+') break;
		  }
		if (i>=iout) continue;
        sprintf(newout,"%s:",newform);
		if (ifemsing || ifemplur) {
		  ifemsing=strlen(femsing);
		  IF(WriteRootFamily(fd_family,femsing)) {
        OgExit(info->herr,info->loginfo.where); return(1);
        }
		  for (i=0; i<ifemsing && i<end_entry; i++) {
		    if (newout[i]!=femsing[i]) break;
		    }
		  if (i<end_entry) offset=end_entry-i; else offset=0;
		  if (i<ifemsing) strcpy(termi,femsing+i); else strcpy(termi,"");
		  sprintf(newout+end_entry+1,"%d%s\r\n",offset,termi);
		  inewout=strlen(newout);
		  }
		else {
          memcpy(newout+end_entry+1,out+end_entry+4,i-(end_entry+4));
		  strcpy(newout+1+i-4,"\r\n"); inewout=1+i-4+2;
		  
	      strcpy(zipfami,out+end_entry+4);
          for (i=0; zipfami[i]!=0; i++) {
	        if (zipfami[i]=='+') break;
	        }
	      zipfami[i]=0;
          for (i=0; zipfami[i]!=0; i++) {
		    if (!isdigit(zipfami[i])) break;
	        }
	      if (zipfami[i]) strcpy(termi,zipfami+i); else termi[0]=0;
	      zipfami[i]=0;
	      offset=atoi(zipfami);
	      memcpy(fami,out,end_entry); fami[end_entry]=0;
	      strcpy(fami+strlen(fami)-offset,termi);
		  IF(WriteRootFamily(fd_family,fami)) {
        OgExit(info->herr,info->loginfo.where); return(1);
        }
		  }


		/** deduplication, automaton is naturally sorted **/
		if (inewout==ioldout && !memcmp(newout,oldout,inewout)) continue;
		/** To remove cases like abaissante:0 **/
		if (!memcmp(newout+inewout-4,":0",2)) continue;
		fwrite(newout,1,inewout,fd_root);
         IFE(WriteForm(fd_form,fd_fform,inewout,newout,excluded_form));
		ioldout=inewout; memcpy(oldout,newout,inewout);
#endif
	    }
      while((retour=OgAufScann(info->ha_getroot,&iout,out,nstate0,&nstate1,states)));
      }
    break;
  default :
    printf("`%s' bad extension (choose .aut, .auz or .auf)\n",name_automat);
  }

fclose(fd_family);
fclose(fd_fform);
fclose(fd_form);
fclose(fd_root);
return(0);
}






int WriteRootFamily(fd,fami)
FILE *fd; unsigned char *fami;
{
unsigned char buffer[1024];
static unsigned char old_fami[1024]={0,0,0};
if (!strcmp(old_fami,fami)) DONE;
strcpy(old_fami,fami);
sprintf(buffer,"%s\r\n",fami);
fwrite(buffer,1,strlen(buffer),fd);
DONE;
}



int WriteForm(fd,ffd,is,s,excluded_form)
FILE *fd,*ffd; int is; 
unsigned char *s;
int excluded_form;
{
int i,deuxpoint=-1,term=-1,idigit=0,offset,len;
unsigned char form[1024], root[1024], digit[1024];
unsigned char buffer[1024];

/** removing \r\n **/
is -= 2; 
/** removing compound words **/
for (i=0; i<is; i++) {
  if (s[i]==' ') DONE;
  }
for (i=0; i<is; i++) {
  if (s[i]==':') deuxpoint=i;
  }
if (deuxpoint<=0) DONE;

term=is;
for (i=deuxpoint+1; i<is; i++) {
  if (isdigit(s[i])) digit[idigit++]=s[i];
  else { term=i; break; }
   }
if (idigit<=0) DONE;

digit[idigit]=0; offset=atoi(digit);

memcpy(form,s,deuxpoint); form[deuxpoint]=0;
len=deuxpoint-offset;
memcpy(root,s,len);
memcpy(root+len,s+term,is-term);
root[len+is-term]=0;

sprintf(buffer,"%s:%d%s\r\n",root,is-term,form+len);

fwrite(buffer,1,strlen(buffer),ffd);

if (excluded_form) DONE;

fwrite(buffer,1,strlen(buffer),fd);
DONE;
}





int GetFemSing(info,iform,form,femsing,gn)
struct og_info *info; int iform; unsigned char *form;
unsigned char *femsing, *gn;
{
int i,retour,offset;
unsigned char entry[1024], ziproot[1024], root[1024], out[1024], termi[100];
oindex states[256]; int nstate0,nstate1;

memcpy(entry,form,iform); entry[iform++]=':'; entry[iform]=0;
if (retour=OgAufScanf(info->ha_getroot,-1,entry,0,out,&nstate0,&nstate1,states)) {
  do {
    IF(retour) {
      OgExit(info->herr,info->loginfo.where); return(1);
      }
    if (!strcmp(out+strlen(out)-2,gn)) {
	    strcpy(ziproot,out+3);
      for (i=0; ziproot[i]!=0; i++) {
	      if (ziproot[i]=='+') break;
	      }
      ziproot[i]=0;
      for (i=0; ziproot[i]!=0; i++) {
		    if (!isdigit(ziproot[i])) break;
	      }
	    if (ziproot[i]) strcpy(termi,ziproot+i); else termi[0]=0;
	    ziproot[i]=0;
	    offset=atoi(ziproot);
	    memcpy(root,form,iform-1); root[iform-1]=0;
	    strcpy(root+strlen(root)-offset,termi);
	    IFE(GetFemSing1(info,root,femsing));
      DONE;
      }
    }
  while(retour=OgAufScann(info->ha_getroot,0,out,nstate0,&nstate1,states));
  }
DONE;
}






int GetFemSing1(info,root,femsing)
struct og_info *info; unsigned char *root;
unsigned char *femsing;
{
int i,retour,ret,iroot,offset;
unsigned char out[1024], zipform[1024], form[1024], termi[100];
oindex states[256]; int nstate0,nstate1;

iroot=strlen(root); root[iroot++]=':'; root[iroot]=0;
if (retour=OgAufScanf(info->ha_getroot,-1,root,0,out,&nstate0,&nstate1,states)) {
  do {
    IFE(retour);
	if (memcmp(out,"f:",2)) continue;
	strcpy(zipform,out+2);
    for (i=0; zipform[i]!=0; i++) {
	  if (!isdigit(zipform[i])) break;
	  }
	if (zipform[i]) strcpy(termi,zipform+i); else termi[0]=0;
	zipform[i]=0;
	offset=atoi(zipform);
	memcpy(form,root,iroot-1); form[iroot-1]=0;
	strcpy(form+strlen(form)-offset,termi);
    IFE(ret=IsFemSing(info,form));
    if (ret) {
      strcpy(femsing,form);
      DONE;
      }
    }
  while(retour=OgAufScann(info->ha_getroot,0,out,nstate0,&nstate1,states));
  }
DONE;
}





int IsFemSing(info,form0)
struct og_info *info; unsigned char *form0;
{
int retour,iform;
unsigned char out[1024], form[1024];
oindex states[256]; int nstate0,nstate1;

strcpy(form,form0);
iform=strlen(form); strcpy(form+iform,":cs:");
if (retour=OgAufScanf(info->ha_getroot,-1,form,0,out,&nstate0,&nstate1,states)) {
  do {
    IFE(retour);
    if (!strcmp(out+strlen(out)-2,"fs")) return(1);
     }
  while(retour=OgAufScann(info->ha_getroot,0,out,nstate0,&nstate1,states));
  }
return(0);
}




int IsExcludedForm(s)
unsigned char *s;
{
int i,is=strlen(s);
for (i=0; OgExcludedForm[i].is; i++) {
  if (OgExcludedForm[i].is > is) continue;
  if (memcmp(s,OgExcludedForm[i].s,OgExcludedForm[i].is)) continue;
  return(1);
  }
return(0);
}




STATICF(void) OgExit(herr,where)
void *herr; char *where;
{
int is_error;
is_error=OgErrLog(herr,where,0,0,0,0);
}


