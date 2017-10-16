/*
 *	This function handles a space file. Which is a file
 *  whose name starts with a given letter, and a decimal number
 *  (10 digit to reach the value 0xffffffff) with .spa extension. 
 *  The file is located in the DOgDirControl directory.
 *  Space files are used to ask space to the monitor.
 *  Copyright (c) 2000 Ogmios
 *  Dev : April 2000
 *  Version 2.0
*/
#include <loggen.h>
#include <logpath.h>


/*
 *  Reads the space file and returns 1 if the file has been read and 0 if not.
 *  We suppose there is only one counter file matching the pattern.
 *  The found number is placed in 'number'.
 *  Returns -1 on error.
*/


PUBLIC(int) OgReadSpaFile(char *letter, unsigned int *number)
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
char pattern_path[DPcPathSize];
char *nil;

*number = 0;
memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s*%s",DOgDirControl,letter,DOgExt_Spa);

IFE(retour=OgFindFirstFile(str_file,pattern_path)); if(!retour) return(0);
strcpy(pattern_path,str_file->File_Path); strtok(pattern_path,".");
*number = strtoul(pattern_path+1,&nil,10);

OgFindClose(str_file);
return(1);
} 




PUBLIC(int) OgWriteSpaFile(char *letter, unsigned int number)
{
FILE *fd;
char erreur[DPcSzErr];
char file_path[DPcPathSize];

IFE(OgRemoveSpaFile(letter));

sprintf(file_path, "%s/%s%.10d%s",DOgDirControl,letter,number,DOgExt_Spa);
IFn(fd=fopen(file_path,"w")) {
  DPcSprintf(erreur,"OgWriteSpaFile: impossible to create file '%s'\n",file_path);
  PcErr(-1,erreur); DPcErr;
  }
fclose(fd);
DONE;
} 




PUBLIC(int) OgRemoveSpaFile(char *letter)
{
int retour;
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
char pattern_path[DPcPathSize];
char file_path[DPcPathSize];
char erreur[DPcSzErr];

memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s*%s",DOgDirControl,letter,DOgExt_Spa);

IFE(retour=OgFindFirstFile(str_file,pattern_path)); 
if(retour) {
  do {
    int remove_cpt = 0;
    sprintf(file_path,"%s/%s",DOgDirControl,str_file->File_Path);
    while (remove_cpt < 3) { if(remove(file_path)==0) break; }
    if (remove_cpt >= 3) {
      DPcSprintf(erreur,"OgRemoveSpaFile: impossible to remove file '%s'\n",file_path);
      PcErr(-1,erreur); DPcErr;
      }
    }
  while (OgFindNextFile(str_file));
  OgFindClose(str_file);
  }
DONE;
} 



