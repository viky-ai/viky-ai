/*
 *	Fonctions de logging sur un fichier.
 *	Copyright (c) 1997	Ogmios par Patrick Constant
 *	Dev : Août 1997
 *	Version 1.0
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <lpcosys.h>
#include <lpclog.h>
#include <lpcmcr.h>
#include <lpclat.h>


/*
 *	This function logs in message 'mes' into file 'file'
 *	if 'mes' is a zero pointeur. the file is emptied,
 *  else it's appended to the end of the file.
*/

PUBLIC(int) OgLog(char *mes, char *file)
{
FILE *fd;
if (mes==0) {
  if (file) if ((fd=fopen(file,"w"))!=0) fclose(fd);
  return(0);
  }
if (file) if ((fd=fopen(file,"a"))!=0) {
  fprintf(fd,"%s",mes);
  fclose(fd);
  }
return(0);
}




/*
 *	This function logs in message 'mes' into file 'file'
 *	if 'mes' is a zero pointeur. the file is emptied,
 *  	else it's appended to the end of the file.
 *	in fact 'file' represents the beginning of the
 *	name of the file. the file is actually two files
 *	with name "file1.log" and "file2.log"
 *	'size' is the max size of those files.
 *	When one file has reached this size, 
 *	the second file is used. And when the second file
 *	has reached this size, the first file is reinitialized.
*/



PUBLIC(int) OgLimLog(char *mes, char *file, int size)
{
FILE *fd1,*fd2;
FILE *fdc,*fdb;
time_t time_of_day;
char file1[DPcPathSize];
char file2[DPcPathSize];
char *filec,*fileb;
struct stat stat1;
struct stat stat2;
int file_sizec=0;
int file_sizeb=0;
char start1[5]="N";
char start2[5]="N";
int retour1;
int retour2;

if (!file) DONE;
sprintf(file1,"%s1.log",file);
sprintf(file2,"%s2.log",file);
time_of_day=time(NULL);

if (mes==0) {
  if ((fd1=fopen(file1,"w"))!=0) {
    fprintf(fd1,"C, started at %s\n",OgGmtime( &time_of_day));
    fclose(fd1);
    }
  if ((fd2=fopen(file2,"w"))!=0) {
    fprintf(fd2,"B, started at %s\n",OgGmtime( &time_of_day));
    fclose(fd2);
    }
  DONE;
  }

retour1=stat(file1,&stat1);
retour2=stat(file2,&stat2);

/** Au moins un fichier n'existe pas **/
/** On reinitialise les logs **/
if (retour1!=0 || retour2!=0) {
  if ((fd1=fopen(file1,"w"))!=0) {
    fprintf(fd1,"C, started at %s\n",OgGmtime(&time_of_day));
    fputs(mes,fd1);
    fclose(fd1);
    }
  if ((fd2=fopen(file2,"w"))!=0) {
    fprintf(fd2,"B, started at %s\n",OgGmtime(&time_of_day));
    fclose(fd2);
    }
  DONE;
  }

/** les deux fichiers existent **/
/** on va rechercher lequel est courant **/
/** Si le courant (C) est plus grand que size **/
/** il devient le before (B) et l'ancien before **/
/** est remis à zéro en devenant le courant **/
if ((fd1=fopen(file1,"r"))!=0) {
  fread(start1,1,1,fd1);
  fclose(fd1);
  }
if ((fd2=fopen(file2,"r"))!=0) {
  fread(start2,1,1,fd2);
  fclose(fd2);
  }
if (start1[0]=='C') {
  filec=file1; file_sizec=stat1.st_size;
  fileb=file2; file_sizeb=stat2.st_size;
  }
else {
  filec=file2; file_sizec=stat2.st_size;
  fileb=file1; file_sizeb=stat1.st_size;
  }

/** size if enough, just write in the file **/
if (file_sizec<size) {
  if ((fdc=fopen(filec,"a"))!=0) {
    fputs(mes,fdc);
    fclose(fdc);
    }
  DONE;
  }
/** size if too big one must swap **/
if ((fdc=fopen(filec,"r+"))!=0) {
  fwrite("B",1,1,fdc);
  fclose(fdc);
  }
if ((fdb=fopen(fileb,"w"))!=0) {
  fprintf(fdb,"C, started at %s\n",OgGmtime(&time_of_day));
  fputs(mes,fdb);
  fclose(fdb);
  }
DONE;
}


