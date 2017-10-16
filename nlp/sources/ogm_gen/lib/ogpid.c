/*
 *	Encapsulation of Sleep function
 *	Copyright (c) 1997-2000	Ogmios by M'hammed El Ajli and Patrick Constant
 *	Dev : October 1997, January 1998, January 2000, January 2005
 *	Version 1.3
*/
#include <loggen.h>
#include <logpath.h>
#include <string.h>
#include <ctype.h>

#if (DPcSystem == DPcSystemWin32)
int VerifyProcessName(unsigned, char *, char *, int);
#endif


/*
 * Function that tests the existence of a Process
 * Returns 1 if the process with pid 'pid' exists, 
 * returns 0 otherwise.
*/

#if (DPcSystem == DPcSystemUnix)
#include <unistd.h>

/*
 * The function "getpgid" return the group ID of the process
 * if the process doesn't exist the function return -1 and 
 * the GUID in the other cases.
*/
 
int OgProcessExists(unsigned int pid)
{
if(getpgid(pid) == -1) return(0);
else return(2);
}

#else
#if (DPcSystem == DPcSystemWin32)



/*
 *  Returns 1 if the process exists and is reacheable with a low access priority
 *  and 2 if the process exists  and is reachable with a standard access priority
 *  "standard" means specifically "STANDARD_RIGHTS_REQUIRED", but, typically
 *  this means that synchronization (shared memory) is possible between server
 *  and indexing chain.
*/

int OgProcessExists(pid)
unsigned pid;
{
HANDLE hp;
IFn(hp=OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,pid)) {
  /* OgSysErrMes(GetLastError(),DPcSzErr,erreur);
   * printf("pid 0x%x not found: %s\n",pid,erreur); */
  return(0);
  }
CloseHandle(hp);
IFn(hp=OpenProcess(STANDARD_RIGHTS_REQUIRED,FALSE,pid)) {
  /* OgSysErrMes(GetLastError(),DPcSzErr,erreur);
   * printf("pid 0x%x not found: %s\n",pid,erreur); */
  return(1);
  }
CloseHandle(hp);
return(2);
}

#endif
#endif




/*
 *  Must be called at the beginning of the process, so that
 *  other processes will be able to verify that the process
 *  thus named is running (this check is done using the pid
 *  ofthe process).
*/

PUBLIC(int) OgDeclareNamedPid(char *argv0, char *name)
{
return(OgDirDeclareNamedPid(DOgDirControl,argv0,name));
}



PUBLIC(int) OgDirDeclareNamedPid(char *control_dir, char *argv0, char *name)
{
FILE *fd;
char erreur[DPcSzErr];
char filename[DPcPathSize];
unsigned pid = getpid();

IFn(control_dir) control_dir = DOgDirControl;

IFE(OgDirCleanDeadNamedPid(control_dir,name));

sprintf(filename,"%s/%s_%.8x.pid",control_dir,name,pid);

IFn(fd=fopen(filename,"w")) {
  DPcSprintf(erreur,"OgDeclarePid: impossible to create file '%s'",filename);
  PcErr(-1,erreur); DPcErr;  
  }

fclose(fd);
DONE;
}





PUBLIC(int) OgGetNamedPidPath(int ipath, char *path)
{
FILE *fd;
int retour;
unsigned pid;
char erreur[DPcSzErr];
char file_path[DPcPathSize];
char pattern_path[DPcPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;

pid = getpid();
memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/*%.8x.pid",DOgDirControl,pid);
IFE(retour=OgFindFirstFile(str_file,pattern_path));
if(!retour) {
  DPcSprintf(erreur,"OgGetNamedPidPath: can't get path");
  PcErr(-1,erreur); DPcErr;
  }
sprintf(file_path, "%s/%s",DOgDirControl,str_file->File_Path);
IFn(fd=fopen(file_path,"r")) {
  DPcSprintf(erreur,"OgGetNamedPidPath: impossible to open file '%s'",file_path);
  PcErr(-1,erreur); DPcErr;  
  }
OgFindClose(str_file);
fgets(path,ipath,fd);
fclose(fd);
DONE;
}



PUBLIC(int) OgCleanNamedPid(char *name)
{
return(OgDirCleanNamedPid(DOgDirControl,name));
}


PUBLIC(int) OgDirCleanNamedPid(char *control_dir, char *name)
{
int retour;
char file_path[DPcPathSize];
char pattern_path[DPcPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;

IFn(control_dir) control_dir = DOgDirControl;

memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s_*.pid",control_dir,name);
IFE(retour=OgFindFirstFile(str_file,pattern_path)); if(!retour) DONE;
do {  
  sprintf(file_path, "%s/%s",control_dir,str_file->File_Path);
  remove(file_path);
  }
while (OgFindNextFile(str_file));
OgFindClose(str_file);
DONE;
}




/*
 *  Just as OgGetNamedPidPath but removes only pid files
 *  that correspond to a dead process (a process that does 
 *  not exist anymore)
*/
PUBLIC(int) OgCleanDeadNamedPid(char *name)
{
return(OgDirCleanDeadNamedPid(DOgDirControl,name));
}



PUBLIC(int) OgDirCleanDeadNamedPid(char *control_dir, char *name)
{
int i,j,retour;
char buffer[DPcPathSize];
char file_path[DPcPathSize];
char pattern_path[DPcPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
unsigned mypid = getpid();
char *nil; unsigned pid;

IFn(control_dir) control_dir = DOgDirControl;

memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s_*.pid",control_dir,name);
IFE(retour=OgFindFirstFile(str_file,pattern_path)); if(!retour) DONE;
do {  
  strcpy(buffer,str_file->File_Path);
  for (i=0; buffer[i]!=0; i++) if (buffer[i]=='_') break;
  if (buffer[i] != '_') continue;
  for (j=0; j<8; j++) if (!isxdigit((int)buffer[i+j+1])) break;
  buffer[i+j+1]=0; pid=strtoul(buffer+i+1,&nil,16);
  if (pid == mypid) continue;
  if (OgProcessExists(pid)) {
    #if ( DPcSystem == DPcSystemUnix)
    continue;
    #else
    #if (DPcSystem == DPcSystemWin32)
    if (VerifyProcessName(pid,control_dir,name,0)) continue;
    #endif
    #endif
    }
  sprintf(file_path, "%s/%s",control_dir,str_file->File_Path);
  remove(file_path);
  }
while (OgFindNextFile(str_file));
OgFindClose(str_file);
DONE;
}




/*
 *  Tests if a process with name 'name' is running or not
 *  Return 1 if the process is running and is not the 
 *  process that is calling this function. Returns 0
 *  otherwise. If there are several file named 
 *  this way, it checks whether one of them corresponds
 *  to a running process.
*/
PUBLIC(int) OgRunningNamedPid(char *name)
{
return(OgDirRunningNamedPid(DOgDirControl,name));
}



PUBLIC(int) OgDirRunningNamedPid(char *control_dir, char *name)
{
return(OgDirGetRunningNamedPid(control_dir,name,0));
}



PUBLIC(int) OgDirGetRunningNamedPid(char *control_dir, char *name, unsigned int *ppid)
{
int i,j,retour;
char buffer[DPcPathSize];
char pattern_path[DPcPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
unsigned mypid = getpid();
char *nil; unsigned pid;

IFn(control_dir) control_dir = DOgDirControl;

memset(str_file,0,sizeof(struct og_file));
sprintf(pattern_path, "%s/%s_*.pid",control_dir,name);
IFE(retour=OgFindFirstFile(str_file,pattern_path)); if(!retour) return(0);
retour=0;
do {  
  strcpy(buffer,str_file->File_Path);
  for (i=0; buffer[i]!=0; i++) if (buffer[i]=='_') break;
  if (buffer[i] != '_') continue;
  for (j=0; j<8; j++) if (!isxdigit((int)buffer[i+j+1])) break;
  buffer[i+j+1]=0; pid=strtoul(buffer+i+1,&nil,16);
  if (pid == mypid) continue;
  if (OgProcessExists(pid)) { retour=1; if (ppid) *ppid=pid; break; }
  }
while (OgFindNextFile(str_file));
OgFindClose(str_file);
return(retour);
}




