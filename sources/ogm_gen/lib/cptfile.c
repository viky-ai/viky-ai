/*
 *	This function handles a counter file. Which is a file
 *  whose name starts with a given letter, and a decimal number
 *  (10 digit to reach the value 0xffffffff) with .cpt extension. 
 *  The file is located in the DOgDirControl directory.
 *	Copyright (c) 2000 Ogmios by Patrick Constant
 *	Dev : April 2000
 *	Version 1.0
*/
#include <loggen.h>
#include <logpath.h>


/*
 *  Reads the counter file and returns 1 if the file has been read and 0 if not.
 *  We suppose there is only one counter file matching the pattern.
 *  The found number is placed in 'number'.
 *  Returns -1 on error.
*/
PUBLIC(int) OgReadCptFile(char *letter, unsigned int *number)
{
return(OgDirReadExtFile(DOgDirControl,letter,number,DOgExt_Cpt));
}


PUBLIC(int) OgDirReadCptFile(char *control_dir, char *letter, unsigned int *number)
{
return(OgDirReadExtFile(DOgDirControl,letter,number,DOgExt_Cpt));
}


PUBLIC(int) OgReadTotFile(char *letter, unsigned int *number)
{
return(OgDirReadExtFile(DOgDirControl,letter,number,DOgExt_Tot));
}


PUBLIC(int) OgDirReadExtFile(char *control_dir, char *letter, unsigned int *number, char *ext)
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
char pattern_path[DPcPathSize];
char *nil;

*number = 0;
memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s*%s",control_dir,letter,ext);

IFE(retour=OgFindFirstFile(str_file,pattern_path)); if(!retour) return(0);
strcpy(pattern_path,str_file->File_Path); strtok(pattern_path,".");
*number = strtoul(pattern_path+1,&nil,10);

OgFindClose(str_file);
return(1);
} 



PUBLIC(int) OgWriteCptFile(char *letter, unsigned int number)
{
return(OgDirWriteExtFile(DOgDirControl,letter,number,DOgExt_Cpt));
}


PUBLIC(int) OgDirWriteCptFile(char *control_dir, char *letter, unsigned int number)
{
return(OgDirWriteExtFile(DOgDirControl,letter,number,DOgExt_Cpt));
}


PUBLIC(int) OgWriteTotFile(char *letter, unsigned int number)
{
return(OgDirWriteExtFile(DOgDirControl,letter,number,DOgExt_Tot));
}


PUBLIC(int) OgDirWriteExtFile(char *control_dir, char *letter, unsigned int number, char *ext)
{
FILE *fd;
char erreur[DPcSzErr];
char file_path[DPcPathSize];

IFE(OgDirRemoveExtFile(control_dir,letter,ext));

sprintf(file_path, "%s/%s%.10d%s",control_dir,letter,number,ext);
IFn(fd=fopen(file_path,"w")) {
  DPcSprintf(erreur,"OgWriteCptFile: impossible to create file '%s'\n",file_path);
  PcErr(-1,erreur); DPcErr;
  }
fclose(fd);
DONE;
} 





PUBLIC(int) OgRemoveCptFile(char *letter)
{
return(OgDirRemoveExtFile(DOgDirControl,letter,DOgExt_Cpt));
}


PUBLIC(int) OgDirRemoveCptFile(char *control_dir, char *letter)
{
return(OgDirRemoveExtFile(control_dir,letter,DOgExt_Cpt));
}


PUBLIC(int) OgRemoveTotFile(char *letter)
{
return(OgDirRemoveExtFile(DOgDirControl,letter,DOgExt_Tot));
}



PUBLIC(int) OgDirRemoveExtFile(char *control_dir, char *letter, char *ext)
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
char pattern_path[DPcPathSize];
char file_path[DPcPathSize];
char erreur[DPcSzErr];

memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s*%s",control_dir,letter,ext);

IFE(retour=OgFindFirstFile(str_file,pattern_path)); 
if(retour) {
  do {
    int remove_cpt = 0;
    sprintf(file_path,"%s/%s",control_dir,str_file->File_Path);
    while (remove_cpt < 3) { if(remove(file_path)==0) break; }
    if (remove_cpt >= 3) {
      DPcSprintf(erreur,"OgRemoveCptFile: impossible to remove file '%s'\n",file_path);
      PcErr(-1,erreur); DPcErr;
      }
    }
  while (OgFindNextFile(str_file));
  OgFindClose(str_file);
  }
DONE;
} 



