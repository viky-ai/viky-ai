/*
 *  Encapsulation of FindFirstFile function
 *  Encapsulation of FindNextFile function
 *  Encapsulation of FindClose function
 *  Copyright (c) 2000  Ogmios
 *  Dev : March 2000
 *  Version 1.0
*/
#include <loggen.h>


#if (DPcSystem == DPcSystemUnix)

#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>


/*****************************************************
Function which try to identify the path
Return:
   *  1 : Regular Expression (File + Path)
   *  2 : Regular Expression (File in the current path)
   *  3 : Directory (No * or ?)
   *  4 : File (No * or ?)
   *  0 : Nothing found
******************************************************/

static int OgFindTestFile(const char *path)
{
struct stat buf;
char * lpath;
char * tpath;
char ttpath[DPcPathSize];


/*  Match for '/'      */
tpath = strrchr(path,'/');

if(tpath != 0){
   if((tpath+1) != 0){
      strcpy(ttpath,tpath);

/*    Match for '*'         */
      lpath = strrchr(path,'*');
      if(lpath != 0)
         return 1;

/*    Match for '?'         */
      lpath = strrchr(path,'?');
      if(lpath != 0)
         return 1;
   }
}
else{
   strcpy(ttpath,path);

/* Match for '*'         */
   lpath = strrchr(path,'*');
   if(lpath != 0)
      return 2;

/* Match for '?'         */
   lpath = strrchr(path,'?');
   if(lpath != 0)
      return 2;
}

if (stat(path,&buf) == 0) {
  if (S_ISDIR(buf.st_mode)) return 3;
  if (S_ISREG(buf.st_mode)) return 4;
  }
return 0;
}


PUBLIC(int) OgFindFirstFile(struct og_file *str_file, og_string path)
{
char erreur[DPcSzErr];
char sys_erreur[DPcSzErr];
char * lpath;
char tpath[DPcPathSize];
char buffer[DPcPathSize];
char npath[DPcPathSize];
char tmppath[DPcPathSize];
char * point;
char * star;
int reg=0,cpt,cpt2;
struct dirent * dirp;

memset(str_file,0,sizeof(struct og_file));

switch(OgFindTestFile(path)) {
   case 1: strcpy(tpath,path);
           lpath = strrchr(tpath,'/');
           lpath++;
           strcpy(npath,lpath);
           *lpath = 0;
           break;

   case 2: getcwd(tpath,DPcPathSize);
           strcpy(npath,path);
           break;

   case 3:

   case 4: strcpy(tpath,path);
           lpath = strrchr(path,'/');
           if (lpath) {
             lpath++;
             strcpy(str_file->File_Path,lpath);
             }
           else {
             strcpy(str_file->File_Path,path);
             }
           str_file->one_file = 1;
           return 1;
           break;

   default:strcpy(str_file->File_Path,path);
               return 0;
           break;
   }

cpt = 0;
cpt2 = 0;
point = strrchr(npath,'.');
star = strrchr(npath,'*');
if ((point != 0)||(star != 0)){
   if(npath[0] != '*')
      tmppath[cpt2++] = '^';

   while(npath[cpt] != 0){
      switch(npath[cpt]){
         case '.':
                  tmppath[cpt2++] = '\\';
                  tmppath[cpt2++] = '.';
                  break;
         case '*':
                  tmppath[cpt2++] = '.';
                  tmppath[cpt2++] = '*';
                  break;
         default: tmppath[cpt2++]=npath[cpt];
                  break;
      }
      cpt++;
   }
   tmppath[cpt2++] = '$';
   tmppath[cpt2++] = 0;
   strcpy(npath,tmppath);
}

reg=regcomp(&(str_file->comp),npath,REG_NOSUB);

if (reg != 0) {
  regerror(reg,&(str_file->comp),buffer,sizeof buffer);
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgFindFirstFile: regcomp '%s':\n",buffer);
  PcErr(-1,erreur); DPcErr;
  }

IFn((str_file->dp = opendir(tpath))) {
  /*
  OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
  DPcSprintf(erreur,"OgFindFirstFile: impossible to open dir '%s':\n%s"
            ,tpath,sys_erreur);
  regfree(&(str_file->comp));
  PcErr(-1,erreur); DPcErr;
  */
  return 0;
  }

IFn((dirp = readdir(str_file->dp))) {
   OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
   OgFindClose(str_file);
   DPcSprintf(erreur,"OgFindFirstFile: %s scandir an error occured '%s':\n"
             ,tpath,sys_erreur);
   PcErr(-1,erreur);
   DPcErr;
  }
do {
  reg = regexec(&(str_file->comp),dirp->d_name,(size_t) 0, NULL, 0);
  switch(reg) {
    case 0:
      strcpy(str_file->File_Path,dirp->d_name);
      return 1;
      break;
    case REG_NOMATCH:
      str_file->File_Path[0] = 0;
      break;
    default:
      regerror(reg,&(str_file->comp),buffer,sizeof buffer);
      OgFindClose(str_file);
      OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
      DPcSprintf(erreur,"OgFindFirstFile: regexec '%s':\n",buffer);
      PcErr(-1,erreur); DPcErr;
      }
   }
while((dirp = readdir(str_file->dp)));
IFE(OgFindClose(str_file));
return 0;
}




PUBLIC(int) OgFindNextFile(struct og_file *str_file)
{
int reg;
char erreur[DPcSzErr];
char buffer[DPcPathSize];
char sys_erreur[DPcSzErr];
struct dirent * dirp;

if (str_file->one_file == 1) return(0);

while((dirp = readdir(str_file->dp))) {
  reg = regexec(&(str_file->comp),dirp->d_name,(size_t) 0, NULL, 0);

  switch(reg){
    case 0:
      strcpy(str_file->File_Path,dirp->d_name);
      return 1; break;
    case REG_NOMATCH:
      str_file->File_Path[0] = 0;
      break;
    default:
      regerror(reg,&(str_file->comp),buffer,sizeof buffer);
      OgSysErrMes(OgSysErr(),DPcSzSysErr,sys_erreur);
      DPcSprintf(erreur,"OgFindFirstFile: regexec '%s':\n",buffer);
      OgFindClose(str_file);
      PcErr(-1,erreur); DPcErr;
     }
  }
return 0;
}



PUBLIC(int) OgFindClose(struct og_file * str_file)
{
  regfree(&(str_file->comp));

  IFX(str_file->dp)
  {
    closedir(str_file->dp);
    str_file->dp = NULL;
  }

  DONE;
}


#else
#if (DPcSystem == DPcSystemWin32)

#include <windows.h>


PUBLIC(int) OgFindFirstFile(str_file,str)
struct og_file *str_file;
char *str;
{
int i,iutf8;
int istrw; WCHAR strw[DPcPathSize*2];
memset(str_file,0,sizeof(struct og_file));
IFE(OgUTF8ToUnis(strlen(str), str, DPcPathSize*2, &istrw, strw,0,0));
str_file->FindHandle = FindFirstFileW(strw,&(str_file->FindFileData));
if (str_file->FindHandle==INVALID_HANDLE_VALUE) return 0;
istrw = wcslen((str_file->FindFileData).cFileName);
IFE(OgUnisToUTF8(istrw, (str_file->FindFileData).cFileName, 512, &iutf8, str_file->File_Path, 0,0));
return 1;
}

PUBLIC(int) OgFindNextFile(str_file)
struct og_file * str_file;
{
int istrw,iutf8;
int r = FindNextFileW(str_file->FindHandle,&(str_file->FindFileData));
istrw = wcslen((str_file->FindFileData).cFileName);
IFE(OgUnisToUTF8(istrw, (str_file->FindFileData).cFileName, 512, &iutf8, str_file->File_Path, 0,0));
return r;
}

PUBLIC(int) OgFindClose(str_file)
struct og_file * str_file;
{
FindClose(str_file->FindHandle);
DONE;
}

#endif
#endif

