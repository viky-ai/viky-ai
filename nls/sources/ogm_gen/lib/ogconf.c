/*
 *	Utilities for reading ogm_conf.txt file 
 *	Copyright (c) 1999 Ogmios bu Patrick Constant
 *	Dev : November 1999
 *	Version 1.2
*/
#include <loggen.h>
#include <logpath.h>
#include <string.h>


/**
* Give the current Working Directory using environment variables defining instances
*
* \param value pointer for returning value
* \param ivalue max size for return value
* \return function success 1 if the variable is found, 0 else, -1 on error.
*
* OgConfGetWorkingDirectory(value,ivalue)
**/ 
PUBLIC(int) OgConfGetWorkingDirectory(char *value,int ivalue)
{
char *DOgMAIN,*DOgINSTANCE,path[DPcPathSize*3];
char erreur[DPcSzErr];
int found;

value[0] = 0;

if((DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  if(ivalue<DPcPathSize) {
    sprintf(erreur,"OgConfGetWorkingDirectory (don't call with ivalue < %d)",DPcPathSize);
    PcErr(-1,erreur); DPcErr;  
    }
  sprintf(path,"%s/%s/%s",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE);
  if(strlen(path) > ivalue) {
    sprintf(erreur,"OgConfGetWorkingDirectory failed line %d (path=[%.180s] : size > ivalue=%d)",path,ivalue);
    PcErr(-1,erreur); DPcErr;  
    }
  strcpy(value,path);
  return(1);
  }
else {
  found = OgConfGetVar(DOgFileOgmConf_Txt,"WorkingDirectory",value,ivalue,0);
  return(found);
  }

return(0);
}




/*
 *  Tries to find in the file 'filename' the value of a variable
 *  named 'varname' and put this value in 'value'
 *  The syntaxe is the following:
 *    [varname]
 *    yes|no
 *  if boolean is true, else:
 *    [varname]
 *    =value
 *  returns 1 if the variable is found, 0 else
 *  returns -1 on error.
*/

PUBLIC(int) OgConfGetVar(char *filename_in, char *varname, char *value, int ivalue, int boolean)
{
FILE *fd;
char erreur[DPcSzErr],filename[DPcPathSize],cwd[DPcPathSize];
char lwvarname[1024], buffer[1024];
char *DOgMAIN,*DOgINSTANCE;
int i,j,state,ivarname;

if(!Ogstricmp(filename_in,DOgFileOgmConf_Txt) && (DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  sprintf(filename,"%s/%s/%s/%s",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE,DOgFileOgmConf_Txt_instance);  }
else {
  strcpy(filename,filename_in); }

IFn(fd=fopen(filename,"r")) {
  getcwd(cwd,DPcPathSize);
  sprintf(erreur,"OgConfGetVar: impossible to open '%s' for variable '%s', path is '%s'",filename,varname,cwd);
  PcErr(-1,erreur); DPcErr;  
  }

IFn(ivarname=strlen(varname)) {
  sprintf(erreur,"OgConfGetVar: null variable name");
  PcErr(-1,erreur); DPcErr;  
  }

strcpy(lwvarname,varname);
PcStrlwr(lwvarname);

state=1;
while(fgets(buffer,1024,fd)) {
  int ibuffer=strlen(buffer);
  for(i=0; i<ibuffer; i++) { 
    if (buffer[i]==';') { ibuffer=i; buffer[i]=0; break; }  
    if (buffer[i]=='[') break;
    }
  if (state==1) {
    if (buffer[i]=='[') {
      PcStrlwr(buffer);
      IFx(strstr(buffer,lwvarname)) state=2;
      }
    }
  else if (state==2) {
    PcStrlwr(buffer);
    if (buffer[i]=='[') break;
    if (boolean) {
      IFx(strstr(buffer,"no")) { state=3; strcpy(value,"no"); break; }
      else { state=3; strcpy(value,"yes"); break; }
      }
    else {
      for(i=0; i<ibuffer; i++) { 
        if (buffer[i]=='=') break;
        }
      if (buffer[i]=='=') {
        for(i++,j=0; i<ibuffer; i++) {
          if (PcIsspace(buffer[i])) {
            if (j==0) continue; 
            else break;
            }
          value[j++]=buffer[i];
          if (j>=ivalue) break;
          }
        value[j]=0;
        state=3; break;
        }
      }
    }
  }
fclose(fd);
if (state==3) return(1);
else return(0);
}





/*
 *  Tries to find in the file 'filename' the value of a variable
 *  named 'varname' and put this value in 'value'
 *  The syntaxe is the following:
 *    [varname]
 *    value
 *  value is the whole immediately following line
 *  returns 1 if the variable is found, 0 else
 *  returns -1 on error.
*/

PUBLIC(int) OgConfGetVarLine(char *filename_in, char *varname, char *value, int ivalue)
{
FILE *fd;
char erreur[DPcSzErr],filename[DPcPathSize],*DOgMAIN,*DOgINSTANCE;
char lwvarname[1024], buffer[1024];
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

strcpy(lwvarname,varname);
PcStrlwr(lwvarname);

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
    strcpy(value,buffer);
    OgTrimString(value,value);
    state=3; break;
    }
  }
fclose(fd);
if (state==3) return(1);
else return(0);
}










/*
 *  Tries to find in the file 'filename' the value of a variable
 *  named 'varname' and put this value in 'value'
 *  The syntaxe is the following:
 *    varname value
 *    white chars can be put anywhere
 *  returns 1 if the variable is found, 0 else
 *  returns -1 on error.
*/

PUBLIC(int) OgDipperConfGetVar(char *filename_in, char *varname, char *value, int ivalue)
{
FILE *fd;
char filename[DPcPathSize],*DOgMAIN,*DOgINSTANCE;
char lwvarname[1024], buffer[1024];
int i,ivarname;
int retour=0;

if(!Ogstricmp(filename_in,DOgFileOgmConf_Txt) && (DOgMAIN=getenv("DOgMAIN")) && (DOgINSTANCE=getenv("DOgINSTANCE"))) {
  sprintf(filename,"%s/%s/%s/%s",DOgMAIN,DOgPathMAINtoINSTANCES,DOgINSTANCE,DOgFileOgmConf_Txt_instance);  }
else {
  strcpy(filename,filename_in); }

/** value is empty when not found **/
*value=0;

IFn(ivarname=strlen(varname)) {
  return(0);
  }

IFn(fd=fopen(filename,"r")) {
  return(0);
  }

strcpy(lwvarname,varname);
PcStrlwr(lwvarname);

while(fgets(buffer,1024,fd)) {
  int equal=0, ibuffer=strlen(buffer);
  int start_name,length_name,start_value=(-1);
  for(i=0; i<ibuffer; i++) { 
    if (buffer[i]=='#') { ibuffer=i; buffer[i]=0; break; }  
    }
  for(i=0; i<ibuffer; i++) { 
    if (isspace(buffer[i])) { equal=i; break; }  
    }
  if (!equal) continue;
  for(i=ibuffer-1; i>=0; i--) { 
    if (!isspace(buffer[i])) { ibuffer=i+1; buffer[i+1]=0; break; }
    }
  for(i=0; i<ibuffer; i++) { 
    if (!isspace(buffer[i])) { start_name=i; break; }  
    }
  for(i=equal-1; i>=0; i--) {  
    if (!isspace(buffer[i])) { length_name=i+1; buffer[i+1]=0; break; }
    } 
  for(i=equal+1; i<ibuffer; i++) { 
    if (!isspace(buffer[i])) { start_value=i; break; }  
    }
  if (start_value < 0) continue;
  if (buffer[start_value]=='"') start_value++;
  if (buffer[ibuffer-1]=='"') { ibuffer--; buffer[ibuffer]=0; }
  PcStrlwr(buffer+start_name);
  if (strcmp(buffer+start_name,lwvarname)) continue;
  strcpy(value,buffer+start_value);
  retour=1;
  break;
  }

fclose(fd);
return(retour);
}





