/*
 *	Getting information from a process id.
 *	Copyright (c) 2003 Pertimm by Patrick Constant
 *	Dev : May 2003
 *	Version 1.0
*/
#include <loggen.h>
#include <tlhelp32.h>


/*
 *  Gets Process full path name from a pid. On 95 uses the method 
 *  of listing all the processes and picking the one searched. Also on 95, 
 *  trying to find all the modules of a given process is no more interesting
 *  because the first module is not necessarily the executable path (tested on 
 *  ogm_fltr.exe returns a dll), and so we would need to examine all module, which
 *  is the same as listing the process.
 *  Returns 1 if the process was found, returns 0 otherwise. Returns -1 on error.
*/

PUBLIC(int) OgPidName(pid,sprocess_name,process_name)
unsigned pid; int sprocess_name; 
char *process_name;
{
char erreur[DPcSzErr];
OSVERSIONINFO osver;
HINSTANCE hInstLib;
HANDLE hProcess;
HMODULE hMod;
DWORD dwSize2;
HANDLE hSnapShot;
PROCESSENTRY32 procentry;
BOOL bFlag;
int retour;
int iprocess_name;

// ToolHelp Function Pointers.
HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD);
BOOL (WINAPI *lpfProcess32First)(HANDLE,LPPROCESSENTRY32);
BOOL (WINAPI *lpfProcess32Next)(HANDLE,LPPROCESSENTRY32);
 
// PSAPI Function Pointers.
BOOL (WINAPI *lpfEnumProcesses)( DWORD *, DWORD cb, DWORD * );
BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *, DWORD, LPDWORD );
DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE, LPTSTR, DWORD );

retour=0;
process_name[0]=0;

// Check to see if were running under Windows95 or Windows NT.
osver.dwOSVersionInfoSize = sizeof(osver) ;
if(!GetVersionEx(&osver)) {
  sprintf(erreur,"OgPidName: impossible to GetVersionEx");
  PcErr(-1,erreur); DPcErr;
  }
// If Windows NT:
if(osver.dwPlatformId == VER_PLATFORM_WIN32_NT)  {
  // Load library and get the procedures explicitly. We do
  // this so that we don't have to worry about modules using
  // this code failing to load under Windows 95, because
  // it can't resolve references to the PSAPI.DLL.
  IFn(hInstLib = LoadLibraryA("PSAPI.DLL")) {
    sprintf(erreur,"OgPidName: impossible to LoadLibraryA 'PSAPI.DLL'");
    PcErr(-1,erreur); DPcErr;
    }
  // Get procedure addresses.
  lpfEnumProcesses = (BOOL(WINAPI *)(DWORD *,DWORD,DWORD*)) GetProcAddress(hInstLib, "EnumProcesses");
  lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,DWORD, LPDWORD)) 
    GetProcAddress( hInstLib, "EnumProcessModules");
  lpfGetModuleFileNameEx =(DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD))
    GetProcAddress( hInstLib, "GetModuleFileNameExA");
  if( lpfEnumProcesses == NULL ||
      lpfEnumProcessModules == NULL ||
      lpfGetModuleFileNameEx == NULL) {
    FreeLibrary(hInstLib);
    sprintf(erreur,"OgPidName: impossible to GetProcAddress some functions from 'PSAPI.DLL'");
    PcErr(-1,erreur); DPcErr;
    }
  // Open the process (if we can... security does not
  // permit every process in the system).
  IFn(hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,FALSE, pid)) {
    // This the normal way that the process is not found
    FreeLibrary(hInstLib);
    return(0);
    }
  // Here we call EnumProcessModules to get only the
  // first module in the process this is important,
  // because this will be the .EXE module for which we
  // will retrieve the full path name in a second.
  if(lpfEnumProcessModules(hProcess, &hMod, sizeof(hMod), &dwSize2)) {
    // Get Full pathname.
    iprocess_name=lpfGetModuleFileNameEx( hProcess, hMod, process_name, sprocess_name);
    if (iprocess_name > 0) retour=1;
    // No CloseHandle on hMod as specified in Microsoft's documentation.
    }
  CloseHandle(hProcess);
  }
// If Windows 95:
else if(osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
  IFn(hInstLib = LoadLibraryA("Kernel32.DLL")) {
    sprintf(erreur,"OgPidName: impossible to LoadLibraryA 'Kernel32.DLL'");
    PcErr(-1,erreur); DPcErr;
    }

  // Get procedure addresses.
  // We are linking to these functions of Kernel32
  // explicitly, because otherwise a module using
  // this code would fail to load under Windows NT,
  // which does not have the Toolhelp32
  // functions in the Kernel 32.
  lpfCreateToolhelp32Snapshot=(HANDLE(WINAPI *)(DWORD,DWORD)) 
    GetProcAddress(hInstLib,"CreateToolhelp32Snapshot");
  lpfProcess32First=(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
    GetProcAddress( hInstLib, "Process32First");
  lpfProcess32Next=(BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32))
    GetProcAddress( hInstLib, "Process32Next");
  if( lpfProcess32Next == NULL ||
      lpfProcess32First == NULL ||
      lpfCreateToolhelp32Snapshot == NULL) {
    FreeLibrary(hInstLib);
    sprintf(erreur,"OgPidName: impossible to GetProcAddress some functions from 'Kernel32.DLL'");
    PcErr(-1,erreur); DPcErr;
    }

  // Get a handle to a Toolhelp snapshot of the systems processes.
  hSnapShot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
  if( hSnapShot==INVALID_HANDLE_VALUE) {
    // This the normal way that the process is not found
    FreeLibrary(hInstLib);
    return(0);
    }

  // Get the first process' information.
  procentry.dwSize = sizeof(PROCESSENTRY32) ;
  bFlag = lpfProcess32First( hSnapShot, &procentry );
  // While there are processes, and we have not found our process keep looping.
  while(bFlag) {
    if (procentry.th32ProcessID == pid) {
      strcpy(process_name,procentry.szExeFile);
      retour=1;
      break;
      }
    procentry.dwSize = sizeof(PROCESSENTRY32) ;
    bFlag = lpfProcess32Next( hSnapShot, &procentry );
    }
  CloseHandle(hSnapShot);
  }
else {
  sprintf(erreur,"OgPidName: unknown Windows system");
  PcErr(-1,erreur); DPcErr;
  }
// Free the library.
FreeLibrary(hInstLib);
return(retour);
}

