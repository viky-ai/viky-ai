/*
 *	This is used to delete files that match a given pattern.
 *	Copyright (c) 1998	Ogmios by Patrick Constant
 *	Dev : May 1998
 *	Version 1.0
*/
#include <loggen.h>
#include <sys/stat.h>
#include <glib.h>
#include <glib/gstdio.h>


/*
 *  Generic delete of all files that are in directory 'dir'
 *  and that match the pattern 'pattern_path'. Does not return any error, 
 *  but returns 1 if some deletes have failed. 
 *  Thoses failed deletes are log in "where"
*/

PUBLIC(int) OgDelFiles(char *dir, char *pattern_path, char *where)
{
char file_path[DPcPathSize];
char search_path[DPcPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
int retour, failed_delete = 0;

memset(str_file,0,sizeof(struct og_file));

sprintf(search_path, "%s/%s",dir,pattern_path);

IFE(retour=OgFindFirstFile(str_file,search_path)); 
if(!retour) return(failed_delete);

do {
  struct stat filestat;
  if (!strcmp(str_file->File_Path,".")) continue;
  if (!strcmp(str_file->File_Path,"..")) continue;
  sprintf(file_path,"%s/%s",dir,str_file->File_Path);
  if (stat(file_path,&filestat)==0 && OgIsDir(filestat.st_mode)) continue;

  IF(g_unlink(file_path)) {
    /** Sometimes, files have the wrong attribute **/
#if (DPcSystem == DPcSystemUnix)
#else
#if (DPcSystem == DPcSystemWin32)
    SetFileAttributes(file_path,FILE_ATTRIBUTE_ARCHIVE);
#endif
#endif
    IF(g_unlink(file_path)) {
      OgMessageLog(DOgMlogInLog, where, 0 
        , "OgDelFiles: impossible to delete file '%s'",file_path);
	  failed_delete = 1;
      }
    }
  }
while (OgFindNextFile(str_file));

OgFindClose(str_file);
return(failed_delete);
}
