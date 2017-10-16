/*
 *  Generic function for file that create problem to the filter.
 *	Copyright (c) 2004-2006 Pertimm, by Patrick Constant
 *	Dev : August 2004, June 2006
 *	Version 1.1
*/
#include "ogm_auta.h"


#define DOgFileDataQuarantine_Txt	"data/quarantine.txt"

#define DOgAutQuarantineSize    0x2000 /* 8k */


/*
 *  The quarantine process is different when it is used in moni and fltr
 *  1. moni simple reads the quarantine file when a file has crashed ogm_fltr
 *     reading the quaranting file means putting it into quarantine
 *     then it writes the quarantine file just after and before 
 *     starting ogm_fltr again, which avoids the need of a synchronization.
 *  2. fltr reads the quarantine file at initialization, and writes it 
 *     each time a file needs to be quarantine to avoid loosing information
 *     in crashes.
*/

struct og_ctrl_quarantine {
  void *herr; ogmutex_t *hmutex;
  char quarantine_txt[DPcPathSize];
  char where[DPcPathSize];
  void *ha;
  };


STATICF(int) OgQuarantineRead(pr(struct og_ctrl_quarantine *));
STATICF(int) OgQuarantineWrite(pr(struct og_ctrl_quarantine *));
STATICF(int) OgQuarantineRead1(pr_(struct og_ctrl_quarantine *) pr(unsigned char *));




PUBLIC(void *) OgQuarantineInit(where)
char *where;
{
struct og_aut_param caut_param, *aut_param=&caut_param;
struct og_ctrl_quarantine *ctrl_quarantine;
char erreur[DPcSzErr];

IFn(ctrl_quarantine=(struct og_ctrl_quarantine *)malloc(sizeof(struct og_ctrl_quarantine))) {
  sprintf(erreur,"OgCoocInit: malloc error on ctrl_quarantine");
  PcErr(-1,erreur); return(0);
  }

memset(ctrl_quarantine,0,sizeof(struct og_ctrl_quarantine));
strcpy(ctrl_quarantine->quarantine_txt,DOgFileDataQuarantine_Txt);
ctrl_quarantine->ha = 0;

memset(aut_param,0,sizeof(struct og_aut_param));
aut_param->herr=ctrl_quarantine->herr; 
aut_param->hmutex=ctrl_quarantine->hmutex;
aut_param->loginfo.trace = DOgAutTraceMinimal+DOgAutTraceMemory; 
aut_param->loginfo.where = where;
aut_param->state_number = DOgAutQuarantineSize;
sprintf(aut_param->name,"auta quarantine");
IFn(ctrl_quarantine->ha=OgAutInit(aut_param)) return(0);

if (where) strcpy(ctrl_quarantine->where,where);

IF(OgQuarantineRead(ctrl_quarantine)) return(0);

return(ctrl_quarantine);
}



/*
 *  Indicates that the file reference must be quarantined.
 *  a file quarantine.dat is created in data directory with
 *  human readable format:
 *  <quarantine_level>:<filename>
 *  quarantine_level is the number of times the file has been 
 *  put into quarantine. File that are quarantined are:
 *  1. file that crash the filter (done by moni)
 *  2. file whose content cannot be analysed and are thus 
 *     analysed as name-only.
 *  In case 1 increment is 1 because moni is not sure
 *  In case 2 increment is DOgMaxQuarantineLevel
*/

PUBLIC(int) OgQuarantined(ptr,file_ref)
void *ptr; char *file_ref;
{
struct og_ctrl_quarantine *ctrl_quarantine = (struct og_ctrl_quarantine *)ptr;
int retour,nstate0,nstate1,iout; oindex states[DPcAutMaxBufferSize+10];
unsigned char out[DPcAutMaxBufferSize+9];
char buffer[DPcPathSize];

sprintf(buffer,"%s\t",file_ref);

if ((retour=OgAutScanf(ctrl_quarantine->ha,-1,buffer,&iout,out,&nstate0,&nstate1,states))) {
  do {
    int quarantine_level;
    IFE(retour);
    quarantine_level=atoi(out);
    if (quarantine_level>=DOgMaxQuarantineLevel) return(1);
    }
  while((retour=OgAutScann(ctrl_quarantine->ha,&iout,out,nstate0,&nstate1,states)));
  } 

return(0);
}





PUBLIC(int) OgQuarantineSet(ptr,file_ref,quarantine_incr)
void *ptr; char *file_ref;
int quarantine_incr;
{
struct og_ctrl_quarantine *ctrl_quarantine = (struct og_ctrl_quarantine *)ptr;
int retour,nstate0,nstate1,iout; oindex states[DPcAutMaxBufferSize+10];
unsigned char oout[DPcAutMaxBufferSize+9];
unsigned char out[DPcAutMaxBufferSize+9];
unsigned char del[DPcAutMaxBufferSize+9];
int ifile_ref,max_quarantine_level=0;
char buffer[DPcPathSize];

ifile_ref=strlen(file_ref);
IFn(ifile_ref) DONE;

IFE(OgQuarantineRead(ctrl_quarantine));

sprintf(buffer,"%s\t",file_ref);

oout[0]=0;
if ((retour=OgAutScanf(ctrl_quarantine->ha,-1,buffer,&iout,out,&nstate0,&nstate1,states))) {
  do {
    int quarantine_level;
    IFE(retour);
    quarantine_level=atoi(out);
    if (max_quarantine_level<=quarantine_level) max_quarantine_level=quarantine_level;
    sprintf(del,"%s%s",buffer,oout); IFE(OgAutDel(ctrl_quarantine->ha,strlen(del),del)); 
    strcpy(oout,out);    
    }
  while((retour=OgAutScann(ctrl_quarantine->ha,&iout,out,nstate0,&nstate1,states)));
  } 

if (oout[0]) {
  sprintf(del,"%s%s",buffer,oout); IFE(OgAutDel(ctrl_quarantine->ha,strlen(del),del)); 
  }

max_quarantine_level += quarantine_incr;
if (max_quarantine_level > DOgMaxQuarantineLevel) max_quarantine_level = DOgMaxQuarantineLevel;
sprintf(buffer,"%s\t%d",file_ref,max_quarantine_level);
IFE(OgAutAdd(ctrl_quarantine->ha,strlen(buffer),buffer));

IFE(OgQuarantineWrite(ctrl_quarantine));

if (ctrl_quarantine->where[0]) {
  OgMessageLog(DOgMlogInLog,ctrl_quarantine->where,0
    ,"OgQuarantineSet: setting q-level %d to '%s'", max_quarantine_level, file_ref);
  }

DONE;
}




STATICF(int) OgQuarantineRead(ctrl_quarantine)
struct og_ctrl_quarantine *ctrl_quarantine;
{
char line[DPcPathSize];
FILE *fd;
int i;

IFn(fd=fopen(ctrl_quarantine->quarantine_txt,"r")) DONE;

if (ctrl_quarantine->where[0]) {
  OgMessageLog(DOgMlogInLog,ctrl_quarantine->where,0
    ,"OgQuarantineRead: reading '%s'", ctrl_quarantine->quarantine_txt);
  }

while(fgets(line,DPcPathSize,fd)) {
  strtok(line,"\n");
  for (i=0; line[i]; i++) if (line[i]=='#') { line[i]=0; break; }
  for (i=0; line[i]; i++) if (!isspace(line[i])) break;
  if (line[i]==0) continue;
  IFE(OgQuarantineRead1(ctrl_quarantine,line));
  }


fclose(fd);
DONE;
}



/*
 *  Format of the line is:
 *  <quarantine_level>:<filename>
*/
STATICF(int) OgQuarantineRead1(ctrl_quarantine,line)
struct og_ctrl_quarantine *ctrl_quarantine;
unsigned char *line;
{
int i,dp=(-1);
char buffer[DPcPathSize];

for (i=0; line[i]; i++) {
  if (line[i]==':') { dp=i; break; }
  }

if (dp<0) DONE;

sprintf(buffer,"%s\t%.*s",line+dp+1,dp,line);
IFE(OgAutAdd(ctrl_quarantine->ha,strlen(buffer),buffer));

DONE;
}





STATICF(int) OgQuarantineWrite(ctrl_quarantine)
struct og_ctrl_quarantine *ctrl_quarantine;
{
int retour,nstate0,nstate1,iout; oindex states[DPcAutMaxBufferSize+10];
unsigned char out[DPcAutMaxBufferSize+9];
char erreur[DPcSzErr];
FILE *fd;

IFn(fd=fopen(ctrl_quarantine->quarantine_txt,"w")) {
  sprintf(erreur,"OgQuarantineWrite: Impossible to open '%s'",ctrl_quarantine->quarantine_txt);
  PcErr(-1,erreur); DPcErr;
  }

if (ctrl_quarantine->where[0]) {
  OgMessageLog(DOgMlogInLog,ctrl_quarantine->where,0
    ,"OgQuarantineWrite: writing '%s'", ctrl_quarantine->quarantine_txt);
  }

if ((retour=OgAutScanf(ctrl_quarantine->ha,0,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    int i,dp=(-1);
    IFE(retour);
    for (i=0; i<iout; i++) {
      if (out[i]=='\t') { dp=i; break; }
      }
    if (dp<0) continue;
    fprintf(fd,"%s:%.*s\n",out+dp+1,dp,out);
    }
  while((retour=OgAutScann(ctrl_quarantine->ha,&iout,out,nstate0,&nstate1,states)));
  } 

fclose(fd);

DONE;
}




PUBLIC(int) OgQuarantineFlush(ptr)
void *ptr;
{
struct og_ctrl_quarantine *ctrl_quarantine = (struct og_ctrl_quarantine *)ptr;
IFE(OgAutFlush(ctrl_quarantine->ha));
DPcFree(ctrl_quarantine);
DONE;
}



