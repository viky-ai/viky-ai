/*
 *  This is filename handling for indexing chain.
 *  Copyright (c) 1997-2000  Ogmios by M'hammed El Ajli and Patrick Constant
 *  Dev : October 1997, January 1998, January 2000
 *  Version 1.2
*/
#include <logpath.h>
#include <loggen.h>
#include <string.h>
#include <stdlib.h>





PUBLIC(int) OgFileExists(og_string filepath)
{
int retour;
struct og_file str_file[1];

memset(str_file,0,sizeof(struct og_file));
IFE(retour=OgFindFirstFile(str_file,filepath));

if(retour) {
   OgFindClose(str_file);
   return(1);
 }
 return(0);
}





PUBLIC(int) OgDirExists(og_string pathname)
{
struct stat sb;
if (stat(pathname, &sb) == 0 && S_ISDIR(sb.st_mode)) return(1);
return(0);
}







PUBLIC(int) OgRemoveEndSlash(char *path)
{
int i, ipath = strlen(path);
for (i=ipath-1; i>=0; i--) {
  if (path[i]=='/' || path[i]=='\\') { path[i]=0; ipath=i; }
  else break;
  }
DONE;
}




/*
 *  Changes "\\" into "\" (or "//" into "/" if necessary.
 *  returns 1 if change have been done 0 otherwise.
 *  Since "\\" can exist at the beginning of the
 *  file name in the case of a Windows network filename,
 *  we start the analysis at the second character.
 *  Used in ogm_ocea and in the file monitors.
*/

PUBLIC(int) CleanFilename(char *file1, char *file2)
{
int i,j,c,state,retour;
state=1; retour=0;

i=j=0; file2[j++] = file1[i++];
for (; file1[i]!=0; i++) {
  c = file1[i];
  switch(state) {
    case 1:
      if (c=='\\' || c=='/') state=2;
      file2[j++] = c;
      break;
    case 2:
      if (c=='\\' || c=='/') retour=1;
      else {
        file2[j++] = c;
        state=1;
        }
      break;
    }
  }
file2[j]=0;
return(retour);
}







PUBLIC(int) OgRenameToStalled(char *filename, int crucial, char *where)
{
int len;
char erreur[DPcSzErr];
unsigned long next_number;
char *ext, stalled_name[DPcPathSize];

len = strlen(filename);

if (filename[len-4] == '.') ext = filename+len-4;
else ext = ".stl";

next_filenumber_to_create(DOgDirStalled,ext,&next_number);
sprintf(stalled_name, "%s/%.8ld%s", DOgDirStalled, next_number, ext);

OgMessageLog(DOgMlogInLog, where, 0
            ,"Renaming '%s' into '%s' because of a problem"
            , filename, stalled_name);

if (rename(filename, stalled_name)) { OgSleep(200);
  if (rename(filename, stalled_name)) {
    DPcSprintf(erreur,"OgRenameToStalled: impossible to rename '%s' into '%s'"
              , filename, stalled_name);
    if (crucial) { PcErr(-1,erreur); DPcErr; }
    else { OgMessageLog(DOgMlogInLog,where,0,"%s",erreur); }
    }
  }
DONE;
}





PUBLIC(void) next_filename_to_create(char *dir,char *ext,char *fileName)
{
  unsigned long int h=0;
  unsigned long int *high=&h;
  char chainfile[9];
  next_filenumber_to_create(dir,ext, high);
  sprintf(chainfile,"%.8ld",*high);
  sprintf(fileName,"%s/%s%s",dir,chainfile,DOgExt_Tmp);
  return;
}


/*
 *  return 0 is file name found
 *  else return -1
*/

PUBLIC(int) DelFilesInDir(char *path, char *ext, unsigned long int *highest)
{
  char    *ptstr;
  char    filedelName[30];
  char    PathOfFile[512];
  unsigned long int next =0;
  struct og_file cstr_file;
  struct og_file *str_file = &cstr_file;
  int retour;

  memset(str_file,0,sizeof(struct og_file));

  sprintf(PathOfFile, "%s/*%s",path,ext);

  /* No file found returns -1 */
  IFE(retour=OgFindFirstFile(str_file,PathOfFile)); if(!retour) return(-1);
  do {
    if((ptstr=strrchr(str_file->File_Path,'.'))==NULL){
    OgFindClose(str_file);
    return(-1);
  }
  sprintf(filedelName,"%s/%s",path,str_file->File_Path)  ;
  *ptstr='\0';
     next=atol(str_file->File_Path);
    if (*highest<=next){remove(filedelName);}
  *ptstr='.';
  }while ( OgFindNextFile(str_file) );
  OgFindClose(str_file);
  return(0);
}



/*
 *  return 0 is file name found
 *  else return -1
*/

PUBLIC(int) next_filenumber_to_create(char *path, char *ext, unsigned long *highest)
{
  int retour;
  char *ptstr;
  char PathOfFile[512];
  unsigned long int next =0;
  struct og_file cstr_file;
  struct og_file *str_file = &cstr_file;

  *highest = 0;
  memset(str_file,0,sizeof(struct og_file));

  sprintf(PathOfFile, "%s/*%s",path,ext);


  /* No file found returns -1 */
  IFE(retour=OgFindFirstFile(str_file,PathOfFile)); if(!retour) return(-1);
  do {
    if((ptstr=strrchr(str_file->File_Path,'.'))==NULL){
                OgFindClose(str_file);
    return(-1);
  }
  *ptstr='\0';
    next=atol(str_file->File_Path);
    if (*highest<=next){*highest = next+1;}
  *ptstr='.';
  }while ( OgFindNextFile(str_file) );
  OgFindClose(str_file);
  return(0);
}


/*
 *  Finds the filename which has the lowest number in its name
 *  the filename has the form : 00000154.ogm for example
 *  'path' is the directory where this file is.
 *  'ext' is the extension of this file
 *  'fileName' is the returned file name (including the path).
 *  'fileSize' is obsolete (should be removed).
 *  Returns  0 is the file is file found
 *        1 or -1 if no file found
*/

PUBLIC(int) find_lowest_filename(char *path, char *ext, char *fileName,
                                 unsigned long int *fileSize)
{
char *ptstr;
int ret,retour=1;
char pattern_path[1024];
unsigned long int current_value=0;
unsigned long int lowest=100000000; /* more than 8 caracters can do */
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
int i; char *s;

memset(str_file,0,sizeof(struct og_file));

sprintf(pattern_path, "%s/*%s",path,ext);

/* No file found returns -1 */
IFE(ret=OgFindFirstFile(str_file,pattern_path)); if(!ret) return(-1);
do {
  /** If no dot, the file is not interesting **/
  if((ptstr=strrchr(str_file->File_Path,'.'))==NULL) continue;
  *ptstr='\0'; s = str_file->File_Path;
  /** If a char in the name is not a digit, file is not interesting **/
  for (i=0; s[i]; i++) { if (!isdigit(s[i])) break; } if (s[i]) continue;
  current_value=atol(str_file->File_Path);
  *ptstr='.';
  if (lowest>=current_value){
    lowest=current_value; retour=0;
    sprintf(fileName,"%s/%s",path,str_file->File_Path);
    }
  }
while ( OgFindNextFile(str_file) );
OgFindClose(str_file);
return(retour);
}


/*
 *  Finds the number of files that have
 *  extension 'ext'. returns 0 if no file found.
*/


PUBLIC(int) find_nb_files(char *path, char *ext)
{
int retour,nb_files=0;
char pattern_path[1024];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;

memset(str_file,0,sizeof(struct og_file));

sprintf(pattern_path, "%s/*%s",path,ext);

/* No file found returns 0 */
IF(retour=OgFindFirstFile(str_file,pattern_path)) return(nb_files);
if (!retour) return(nb_files);
do {
  nb_files++;
  }
while (OgFindNextFile(str_file) );
OgFindClose(str_file);
return(nb_files);
}


/*
 *  Function that waits till a file cant be read.
 *  This file has been typically found by function
 *  'find_lowest_filename'. The found file is create
 *  by a file rename which doesn't release its handle
 *  on the file immediately. This results in an error
 *  in the file opening. This case is rare because
 *  the renaming is very fast. By waiting a few milli-seconds
 *  we give the time for the renaming to release the handle,
 *  and so, the file can be opened.
 *  Returns the number of time the file has been tried
 *  ('nb_calls'), before it is accessed.
 *  if nb_calls>1, there has been a conflict with the renaming.
 *  Returns -1 if the file couldn't be accessed
 *  we use fopen instead of access, because
 *  access works when fopen doesn't...
*/

PUBLIC(int) wait_on_rename(char *filename)
{
FILE *fd;
int nb_calls=1;
while (1) {
  /** 5 milli-seconds between each call **/
  /** test time is 20*5=100 milli-seconds **/
  /** access mode 4 is read permission **/
  IFx(fd=fopen(filename,"rb")) { fclose(fd); break; }
  if (nb_calls++>20) { nb_calls=(-1); break; }
  OgSleep(5);
  }
return(nb_calls);
}




/*
 *  Permet de faire l'�quivalent de
 *  system("REN DELETED\\*.dl0 DELETED\\*.dl1");
*/

PUBLIC(int) patterned_rename(char *where,char *path,char *ext1,char *ext2)
{
int retour;
char file_path1[1024];
char file_path2[1024];
char pattern_path[1024];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;


memset(str_file,0,sizeof(struct og_file));

sprintf(pattern_path, "%s/*%s",path,ext1);

/* No file found returns -1 */
IF(retour=OgFindFirstFile(str_file,pattern_path)) return(-1);
if (!retour) return(-1);
do {
  sprintf(file_path1, "%s/%s",path,str_file->File_Path);
  strcpy(file_path2,file_path1);
  strcpy(file_path2+strlen(file_path2)-4,ext2);
  MessageInfoLog(0,where,0,"renaming '%s' into '%s'",file_path1,file_path2);
  rename(file_path1,file_path2);
  }
while (OgFindNextFile(str_file) );
OgFindClose(str_file);
DONE;
}


PUBLIC(int) patterned_access(char *pattern_path)
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;

memset(str_file,0,sizeof(struct og_file));

/* No file found returns -1 */
IF(retour=OgFindFirstFile(str_file,pattern_path)) return(-1);
if (!retour) return(-1);
OgFindClose(str_file);
/* file is found returns 0 */
return(0);
}


PUBLIC(int) patterned_delete(char *where,char *path,char *pattern_file)
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
char search_path[DogMaxPathSize];
char file_path[DogMaxPathSize];

memset(str_file,0,sizeof(struct og_file));
sprintf(search_path,"%s/%s",path,pattern_file);

IF(retour=OgFindFirstFile(str_file,search_path)) return(-1);
if (!retour) return(-1);

do {
  sprintf(file_path,"%s/%s",path,str_file->File_Path);
  IF(unlink(file_path)) {
    MessageInfoLog(0,where,0,"patterned_delete: impossible to delete file '%s'\n",
    str_file->File_Path);
  continue;
    }
  }
while (OgFindNextFile(str_file));
OgFindClose(str_file);
DONE;
}
