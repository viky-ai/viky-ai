/*
 *	Handling processes with a specific attention to windows. 
 *	Copyright (c) 1997-2003	Pertimm by Patrick Constant
 *	Dev : October 1997, January 1998, January 2000, May 2003, January 2005
 *	Version 1.4
*/
#include <loggen.h>
#include <logpath.h>
#include <string.h>
#include <ctype.h>


/*#define DEVERMINE*/

#if (DPcSystem == DPcSystemWin32)
int VerifyProcessName(unsigned, char *, char *, int);
#endif


/*
 *  Tests if a process with name 'name' is running or not
 *  Return 1 or 2 if the process is running and is not the 
 *  process that is calling this function. Returns 0
 *  otherwise. If there are several file named 
 *  this way, it checks whether one of them corresponds
 *  to a running process. Difference between values 1
 *  and 2 is the fact that value indicates less access
 *  right to the process as value 2. Typically value
 *  is not enough to synchronize server and indexing chain
 *  while value 2 is.
*/
PUBLIC(int) OgRunningNamedPidW(char *name)
{
return(OgDirRunningNamedPidW(DOgDirControl,name));
}


PUBLIC(int) OgDirRunningNamedPidW(char *control_dir, char *name)
{
return(OgDirGetRunningNamedPidW(control_dir,name,0));
}



PUBLIC(int) OgDirGetRunningNamedPidW(char *control_dir, char *name, unsigned int *ppid)
{
int i,j,retour,retour_pe;
char buffer[DPcPathSize];
char pattern_path[DPcPathSize];
struct og_file cstr_file;
struct og_file *str_file = &cstr_file;
unsigned mypid = getpid();
char *nil; unsigned pid;
char *DOgMAIN;

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
  #ifdef DEVERMINE
  {
  char current_executable_path[DPcPathSize];
  char current_executable_program[DPcPathSize];
  char long_current_executable_program[DPcPathSize];
  OgGetExecutablePath(0,current_executable_path);
  GetModuleFileName(NULL,current_executable_program,DPcPathSize);
  OgGetLongFileName(current_executable_program,long_current_executable_program);
  OgMessageLog(DOgMlogInLog+DOgMlogDateIn,"ogpidw",0,"Testing pid=0x%x=%d for '%s'",pid,pid,long_current_executable_program);
  }
  #endif
  /** retour_pe can be 1 (few access rights) or 2 (more access rights) **/
  if (retour_pe=OgProcessExists(pid)) {
    #if ( DPcSystem == DPcSystemUnix)
    /** pid are unique under Unix **/
    retour=retour_pe; 
    #else
    #if (DPcSystem == DPcSystemWin32)
    /* pid can be reused by another program under Windows, 
     * thus we use a specific window name. In fact, it is not 
     * always possible to get the window name, e.g. when the program
     * is a CGI-script and does not have enough rights to access the 
     * window name. Thus we have to use the method of checking
     * the full path name of the process defined by the process id
     * This is not very simple to do but it seems to be better 
     * than looking for the window name. */
    #if 0
    char name_pid[1024];
    sprintf(name_pid,"%s_%.8x",name,pid);
    if (FindWindow(name,name_pid)) { retour=retour_pe; if (ppid) *ppid=pid; }
    #else
    if(DOgMAIN=getenv("DOgMAIN")) {
      char fake_control_dir[DPcPathSize];
      sprintf(fake_control_dir,"%s/private/bin/fake_control",DOgMAIN);
      if (VerifyProcessName(pid,fake_control_dir,name,1)) { retour=retour_pe; if (ppid) *ppid=pid; }
      }
    else {
      if (VerifyProcessName(pid,control_dir,name,1)) { retour=retour_pe; if (ppid) *ppid=pid; }
      }
    #endif
    #endif
    #endif
    break; 
    }
  }
while (OgFindNextFile(str_file));
OgFindClose(str_file);
return(retour);
}




#if (DPcSystem == DPcSystemWin32)

int VerifyProcessName(pid,control_dir,name,valid_on_error)
unsigned pid; char *control_dir, *name;
int valid_on_error;
{
int i,ilong_process_name,ifull_control_dir,slash;
char long_current_executable_program[DPcPathSize];
char current_executable_program[DPcPathSize];
char current_executable_path[DPcPathSize];
char long_process_name[DPcPathSize];
char full_control_dir[DPcPathSize];
char pattern_exename[DPcPathSize];
char process_name[DPcPathSize];
char erreur[DPcSzErr];
char *real_exename;
int retour;

OgGetExecutablePath(0,current_executable_path);
GetModuleFileName(NULL,current_executable_program,DPcPathSize);
OgGetLongFileName(current_executable_program,long_current_executable_program);

IF(retour=OgPidName(pid,DPcPathSize,process_name)) {
  PcErrLast(-1,erreur);
  OgMessageLog(DOgMlogInLog+DOgMlogDateIn,"ogpidw",0,"From current program '%s'",long_current_executable_program);
  OgMessageLog(DOgMlogInLog,"ogpidw",0,"while checking '%s'",control_dir);
  OgMessageLog(DOgMlogInLog,"ogpidw",0,"warning: %s",erreur);
  /** If we have an error we consider the process as running **/
  return(valid_on_error);
  }
/* July 10th 2006: adding this line (process_name has not been found)
 * This make the system think there is a program while it is not true */
IFn(retour) return(0);
/* We retrieve a process name such as "C:\PROGRA~1\Adaptec\DirectCD\directcd.exe"
 * We then need to check that this corresponds control_dir, i.e. we have:
 * <pertimm_dir>/control and <pertimm_dir>/process_name.exe */
OgGetLongFileName(process_name,long_process_name);

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog+DOgMlogDateIn,"ogpidw",0,"\nlong_current_executable_program '%s'",long_current_executable_program);
OgMessageLog(DOgMlogInLog,"ogpidw",0,"process_name '%s'",process_name);
OgMessageLog(DOgMlogInLog,"ogpidw",0,"long_process_name '%s'",long_process_name);
#endif

/** We check if control_dir is absolute to avoid concatenation **/
if (control_dir[0]=='/' || control_dir[0]=='\\' || control_dir[1]==':') {
  strcpy(full_control_dir,control_dir);
  }
else {
  sprintf(full_control_dir,"%s/%s",current_executable_path,control_dir);
  }

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog,"ogpidw",0,"full_control_dir '%s'",full_control_dir);
#endif

slash=(-1); ilong_process_name=strlen(long_process_name);
for (i=ilong_process_name-1; i>=0; i--) {
  if (long_process_name[i]=='/' || long_process_name[i]=='\\') { slash=i; break; }
  }
/** should not happen, we consider the process as running **/
IF(slash) return(1);
long_process_name[slash]=0;
real_exename = long_process_name+slash+1;

sprintf(pattern_exename,"ogm_%s.exe",name);
if (strcmp(pattern_exename,real_exename)) return(0);

slash=(-1); ifull_control_dir=strlen(full_control_dir);
for (i=ifull_control_dir-1; i>=0; i--) {
  if (full_control_dir[i]=='/' || full_control_dir[i]=='\\') { slash=i; break; }
  }
/** should not happen, we consider the process as running **/
IF(slash) return(1);
full_control_dir[slash]=0;
PcStrlwr(full_control_dir);
OgNormalizePath(full_control_dir,full_control_dir);
OgNormalizePath(long_process_name,long_process_name);

#ifdef DEVERMINE
OgMessageLog(DOgMlogInLog,"ogpidw",0,"long_process_name '%s'",long_process_name);
OgMessageLog(DOgMlogInLog,"ogpidw",0,"full_control_dir  '%s'",full_control_dir);
#endif

if (strcmp(full_control_dir,long_process_name)) return(0);
return(1);
}

#endif


