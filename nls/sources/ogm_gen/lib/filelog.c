/*
 *  Writing the content of a file in the log file.
 *  Copyright (c) 1999,2000 Ogmios
 *  Dev : October 1999, January 2000
 *  Version 1.1
*/
#include <loggen.h>


#define DOgCopyBufferSize   4096



PUBLIC(int) OgFileToLog(char *exeFile,char *file_to_read)
{
FILE *file_log;
FILE *ffile_to_read;
char LogFilename[1024];
char buffer[DOgCopyBufferSize];
char *where = exeFile;
unsigned nb_read=0;

sprintf(LogFilename,"log/%s.log",exeFile);

IFn(ffile_to_read=fopen(file_to_read,"r")) {
  MessageInfoLog(0,where,0,"Error opening file '%s'",file_to_read);
  return(1);
  }
IFn(file_log=fopen(LogFilename,"a")) {
  MessageInfoLog(0,where,0,"Error opening file '%s'",LogFilename);
  fclose(ffile_to_read);
  return(1);
  }
while(!feof(ffile_to_read)) { 
  nb_read=fread(buffer,1,DOgCopyBufferSize,ffile_to_read);              
  if(ferror(ffile_to_read)){
    fclose(ffile_to_read); fclose(file_log);
    MessageInfoLog(0,where,0,"Error reading file '%s'",file_to_read);	
    return(1);
    }
  if(nb_read==0) break;
  if(fwrite(buffer,1,nb_read,file_log)!=nb_read) {
    fclose(ffile_to_read); fclose(file_log);
    MessageInfoLog(0,where,0,"Error writing in file '%s'",LogFilename);
	return(1);
    }
  }
fprintf(file_log,"\n");
fclose(ffile_to_read); 
fclose(file_log);
return(0);
}

