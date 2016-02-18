/*
 *	Fonctions permettant de mesurer le temps cpu.
 *	Copyright (c) 1997,98	Ogmios par Patrick Constant
 *	Dev : Juin 1997, Janvier 1998
 *	Version 1.1
*/
#include <lpcosys.h>
#include <lpctime.h>


/*
 *	Function used to get information on CPU time.
 *	first you set a time stamp by calling :
 *	  struct scpu_time cpu_time;
 *	  OgCpuTime(0,0,&cpu_time);
 *	then the part of the program you want to mesure is executed
 *	then you write a message 'mes' to a file 'file'.
 *	  OgCpuTime("Time elapsed : ","myfile.log",,&cpu_time);
 *	The information is appended to the end of the file.
 *	To clear the file you can call :
 *	  OgCpuTime(0,"myfile.log",&cpu_time);
 *	instead of OgCpuTime(0,0,&cpu_time);
*/

#if (DPcSystem == DPcSystemUnix)

#include <sys/types.h>
#include <sys/times.h>

PUBLIC(int) OgCpuTime(char *mes, char *file, struct scpu_time *cpu_time)
{
FILE *fd;
long duser,dsys;
struct tms cpu_time_unx;
struct scpu_time new_cpu_time;

if (mes==0) {
  times(&cpu_time_unx);
  cpu_time->user=cpu_time_unx.tms_utime*100L/60L;
  cpu_time->sys=cpu_time_unx.tms_stime*100L/60L;
  if (file) if ((fd=fopen(file,"w"))!=0) fclose(fd);
  duser=0; dsys=0;
  }
else {
  times(&cpu_time_unx);
  new_cpu_time.user=cpu_time_unx.tms_utime*100L/60L;
  new_cpu_time.sys=cpu_time_unx.tms_stime*100L/60L;
  duser=new_cpu_time.user-cpu_time->user;
  dsys=new_cpu_time.sys-cpu_time->sys;
  if (file) if ((fd=fopen(file,"a"))!=0) {
    fprintf(fd,"%suser:%lu.%lu - sys:%lu.%lu\n",mes,
      duser/100L,duser%100L,dsys/100L,dsys%100L);
    fclose(fd);
    }
  memcpy(cpu_time,&new_cpu_time,sizeof(struct scpu_time));
  }
return(duser+dsys);
}


#else
#if (DPcSystem == DPcSystemWin32)

#include <time.h>

PUBLIC(int) OgCpuTime(mes,file,cpu_time)
char *mes, *file; struct scpu_time *cpu_time;
{
FILE *fd;
long duser,dsys;
clock_t cpu_time_win32;
struct scpu_time new_cpu_time;

if (mes==0) {
  cpu_time_win32=clock();
  cpu_time->user=cpu_time_win32*100L/CLOCKS_PER_SEC;
  cpu_time->sys=0L;
  if (file) if ((fd=fopen(file,"w"))!=0) fclose(fd);
  duser=0; dsys=0;
  }
else {
  cpu_time_win32=clock();
  new_cpu_time.user=cpu_time_win32*100L/CLOCKS_PER_SEC;
  new_cpu_time.sys=0L;
  duser=new_cpu_time.user-cpu_time->user;
  dsys=new_cpu_time.sys-cpu_time->sys;
  if (file) if ((fd=fopen(file,"a"))!=0) {
    fprintf(fd,"%suser:%lu.%lu - sys:%lu.%lu\n",mes,
      duser/100L,duser%100L,dsys/100L,dsys%100L);
    fclose(fd);
    }
  memcpy(cpu_time,&new_cpu_time,sizeof(struct scpu_time));
  }
return(duser+dsys);
}

#else
ERREUR
#endif
#endif





