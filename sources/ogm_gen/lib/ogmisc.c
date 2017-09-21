#include <lpcosys.h>
#if (DPcSystem==DPcSystemWin32)
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#else
#include <sys/utsname.h>
#endif
#include <stdio.h>
#include <loggen.h>

/**
 * return a static strings which contains OS version
 * For windows, see http://msdn.microsoft.com/en-us/library/ms724451(VS.85).aspx
 **/
PUBLIC(char *) OgGetOsInfo(void) {
  static char info[256];

#if (DPcSystem==DPcSystemWin32)
  OSVERSIONINFO vinfo;

  info[0]='\0';
  vinfo.dwOSVersionInfoSize = sizeof(vinfo);
  if(!GetVersionEx(&vinfo)) return info;

  switch(vinfo.dwMajorVersion) {
  case 5: { // Windows 2000, 2003, XP
    switch(vinfo.dwMinorVersion) {
    case 0: // Windows 2000
      snprintf(info,255,"%s %d.%d.%d %s",
         "Windows 2K",
         vinfo.dwMajorVersion,vinfo.dwMinorVersion,
         vinfo.dwBuildNumber,vinfo.szCSDVersion);
      info[255]='\0';
      break;
    case 1: // Windows XP
      snprintf(info,255,"%s %d.%d.%d %s",
         "Windows XP",
         vinfo.dwMajorVersion,vinfo.dwMinorVersion,
         vinfo.dwBuildNumber,vinfo.szCSDVersion);
      info[255]='\0';
      break;
    case 2: // Windows 2003, XP Pro x64
      snprintf(info,255,"%s %d.%d.%d %s",
         "Windows 2003",
         vinfo.dwMajorVersion,vinfo.dwMinorVersion,
         vinfo.dwBuildNumber,vinfo.szCSDVersion);
      info[255]='\0';
      break;
    }
  }
  case 6: // Windows Vista, 2008
    switch(vinfo.dwMinorVersion) {
    case 0: // Windows 2008, Vista
      snprintf(info,255,"%s %d.%d.%d %s",
         "Windows Vista",
         vinfo.dwMajorVersion,vinfo.dwMinorVersion,
         vinfo.dwBuildNumber,vinfo.szCSDVersion);
      info[255]='\0';
      break;
    }
  }
#else
  struct utsname osname;

  info[0] = '\0';
  if (uname(&osname) < 0) return info;
  sprintf(info, "%s %s",osname.sysname, osname.release);
#endif
  return info;
}

/** @ingroup libmiscAPI
 * Convert NT Handle to CRT FILE *
 *
 * \param handle
 * \return CRT FILE *
 *
 * OgFileToHandle(fp)
 **/
PUBLIC(FILE *) OgHandleToFile(NATIVE_FILE handle,char *mode)
{
#if (DPcSystem==DPcSystemWin32)
  FILE *fp;
  int fd;
  int flags = _O_RDWR | _O_CREAT;

  if (strstr(mode,"a")) flags |= _O_APPEND;
  fd = _open_osfhandle((long) handle,flags); // Associates a C run-time file descriptor with an existing operating-system file handle
  fp = _fdopen(fd,mode);
  return  fp;
#else
  return fdopen(handle,mode);
#endif
}

/**
 * @defgroup libmiscAPI Libmisc : Toolbox for UNIX/Windows systems
 **/

/** @ingroup libmiscAPI
 * lock all file with shared of exclusive access
 *
 * \param hFile (native_file descriptor)
 * \param mode (FILE_LOCKSHARED, FILE_LOCKEXCLUSIVE, FILE_UNLOCK)
 * \return 0 for success, -1 otherwise
 *
 * OgfileLock(fp,mode)
 **/
PUBLIC(int) OgFileLock(NATIVE_FILE hFile,int mode)
{
#if (DPcSystem==DPcSystemWin32)
  DWORD nNumberOfBytesToLockLow = 0xffffffff;
  DWORD nNumberOfBytesToLockHigh = 0xffffffff;
  OVERLAPPED lpOverlapped;
  int  res;

  lpOverlapped.Offset = 0;
  lpOverlapped.OffsetHigh = 0;
  lpOverlapped.hEvent = NULL;
  if (mode == FILE_UNLOCK) {
    res = UnlockFileEx(hFile,                      // handle of file to lock
           0,                          // reserved, must be set to zero
           nNumberOfBytesToLockLow,    // low-order 32 bits of length to lock
           nNumberOfBytesToLockHigh,   // high-order 32 bits of length to lock
           &lpOverlapped);             // addr. of structure with lock region start offset
    if (res == 0) return -1;
    return 0;
  }

  if (mode == FILE_LOCKSHARED || mode == FILE_LOCKEXCLUSIVE) {
    res = LockFileEx(hFile,                    // handle of file to lock
         mode,                     // functional behavior modification flags
         0,                        // reserved, must be set to zero
         nNumberOfBytesToLockLow,  // low-order 32 bits of length to lock
         nNumberOfBytesToLockHigh, // high-order 32 bits of length to lock
         &lpOverlapped);           // addr. of structure with lock region start offset
    if (res == 0) return -1;
    return 0;
  }
  return(-1);
#else
  struct flock lck;

  switch (mode)  {
  case FILE_LOCKSHARED:
    lck.l_type = F_RDLCK;
    break;
  case FILE_LOCKEXCLUSIVE:
    lck.l_type = F_WRLCK;
    break;
  case FILE_UNLOCK:
    lck.l_type = F_UNLCK;
    break;
  }
  lck.l_whence = SEEK_SET;
  lck.l_start = lck.l_len = 0;

  // lock loop : if a shared or exclusive lock cannot be set, fcntl() return immediately with -1
  while (fcntl (hFile, F_SETLKW, &lck) == -1) {
    if (errno != EINTR) return -1;
  }
  return 0;
#endif
}

/** @ingroup libmiscAPI
 * sync file to disk
 *
 * \param fp file descriptor
 * \return 0 for success, -1 otherwise
 *
 * OgfileSync(fp)
 **/
PUBLIC(int) OgFileSync(NATIVE_FILE hFile)
{
#if (DPcSystem==DPcSystemWin32)
 if (!FlushFileBuffers(hFile)) return -1;
 return 0;
#else
 if (hFile <0) return -1;
 return fsync(hFile);
#endif
}

/** @ingroup libmiscAPI
 * set EOF to size
 * (warning: tested only for size=0)
 *
 * \param fp   file descriptor
 * \param size
 * \return 0 : success, -1 : error
 **/
PUBLIC(int) OgFileTruncate(NATIVE_FILE hFile, int size) {
#if (DPcSystem==DPcSystemWin32)
  if (SetFilePointer(hFile,  // handle of file
         0,  // number of bytes to move file pointer
         NULL,  // address of high-order word of distance to move
         FILE_BEGIN // how to move
         ) == 0xFFFFFFFF) return -1;

  if (SetEndOfFile(hFile) == 0) return -1;
#else
  if (ftruncate(hFile,0) !=0) return -1;
#endif
  return 0;
}

/** @ingroup libmiscAPI
 * delete any suffix beginning with the last slash ('/')
 * example : OgDirName("/usr/home/carpetsmoker/dirname.wiki")="/usr/home/carpetsmoker"
 *
 * \param filename
 * \return string
 **/
PUBLIC(char *) OgDirName(char *filename)
{
#if (DPcSystem==DPcSystemWin32)
  unsigned int i;
  for(i=0; i<=strlen(filename); i++){
    if(filename[i] == '/' || filename[i] == '\\'){
      filename[i] = '\0';
      return filename;
    }
  }
  return "\\";
#else
  return dirname(filename);
#endif
}

/** @ingroup libmiscAPI
 * delete any prefix up to the last slash ('/') character
 * example : OgBaseName("/usr/home/jsmith/basename.wiki")="basename.wiki"
 *
 * \param filename
 * \return string
 **/
PUBLIC(char *) OgBaseName(char *filename)
{
  int i;
  for(i=strlen(filename); i>=0; i--){
    if(filename[i] == '/' || filename[i] == '\\') return (&filename[i+1]);
  }
  return filename;
}

PUBLIC(int) OgGetTimeOfDay(struct timeval *tvp, void *tzp)
{
#if (DPcSystem==DPcSystemWin32)
  SYSTEMTIME systime;
  FILETIME ftime;
  FILETIME fepoch;
  SYSTEMTIME sepoch;
  LARGE_INTEGER ltime,etime;
  // Windows filetime starts 1/1/1601 whereas gettimeofday starts 1/1/1970,
  // causing some confusion later ... Hence the conversion
  GetSystemTime(&systime);
  SystemTimeToFileTime(&systime,&ftime);

  sepoch.wYear = 1970;
  sepoch.wMonth = 1;
  sepoch.wDay = 1;
  sepoch.wHour = 0;
  sepoch.wMinute = 0;
  sepoch.wSecond = 0;
  sepoch.wMilliseconds = 0;
  SystemTimeToFileTime(&sepoch,&fepoch);

  etime.LowPart = fepoch.dwLowDateTime;
  etime.HighPart = fepoch.dwHighDateTime;
  ltime.LowPart=ftime.dwLowDateTime;
  ltime.HighPart=ftime.dwHighDateTime;

  ltime.QuadPart = ltime.QuadPart - etime.QuadPart;

  tvp->tv_sec = (long)(ltime.QuadPart/10000000);
  tvp->tv_usec = (long)((ltime.QuadPart%10000000)/10);
  DONE;
#else
  return gettimeofday(tvp,tzp);
#endif
}

PUBLIC(struct tm *)OgLocalTime(const time_t *clck, struct tm *res)
{
#if (DPcSystem==DPcSystemWin32)
  struct tm* ltres;

  ltres=localtime(clck);
  *res=*ltres;
  return(res);
#else
  return localtime_r(clck,res);
#endif
}


PUBLIC(struct tm *)OgGmtTime(const time_t *clck, struct tm *res)
{
#if (DPcSystem==DPcSystemWin32)
  struct tm* ltres;

  ltres=gmtime(clck);
  *res=*ltres;
  return(res);
#else
  return gmtime_r(clck,res);
#endif
}


/** @ingroup libmiscAPI
 *
 * write in buffer an iso8601 time format
 * \param buffer an allocated buffer
 * \param size buffer size
 * \return @buffer
 **/
PUBLIC (char *) OgGetTimeISO8601(char *buffer, int size) {
  struct timeval  tv;
  struct tm       tpm;
  time_t          tp;
  char            date[256];

  time(&tp);
  OgGmtTime(&tp,&tpm);
  strftime(date,256,"%Y-%m-%dT%H:%M:%S",&tpm);
  OgGetTimeOfDay(&tv,NULL);
  snprintf(buffer,size,"%s.%03d%03dZ",date,(int)tv.tv_usec/1000, (int)tv.tv_usec%1000); buffer[size-1]='\0';
  return buffer;
}

/** @ingroup libmiscAPI
* this function is for computing the time difference between timeval x and y
* the result is stored in result. Example :
* \code
  struct timeval start, stop, echodelay; // start, stop and echo delay times

  OgGetTimeOfDay(&start,NULL);
  ... do stuff
  OgGetTimeOfDay(&stop, NULL);
  OgElapseTime(&echodelay, &stop, &start); // compute time delay

  printf("Echo delay is %ds and %dus\n", echodelay.tv_sec, echodelay.tv_usec);
  \endcode
*
* \param result
* \param stop
* \param start
**/
PUBLIC(int) OgElapseTime(struct timeval *result, struct timeval *stop, struct timeval *start)
{
  // Perform the carry for the later subtraction by updating y.
  if (stop->tv_usec < start->tv_usec) {
    int nsec = (start->tv_usec - stop->tv_usec) / 1000000 + 1;
    start->tv_usec -= 1000000 * nsec;
    start->tv_sec += nsec;
  }
  if (stop->tv_usec - start->tv_usec > 1000000) {
    int nsec = (stop->tv_usec - start->tv_usec) / 1000000;
    start->tv_usec += 1000000 * nsec;
    start->tv_sec -= nsec;
  }

  // Compute the time remaining to wait.  tv_usec is certainly positive.
  result->tv_sec = stop->tv_sec - start->tv_sec;
  result->tv_usec = stop->tv_usec - start->tv_usec;

  // Return 1 if result is negative.
  return stop->tv_sec < start->tv_sec;
}

/** @ingroup libmiscAPI
 * micro seconds sleep
 *
 * OgUsleep(microseconds)
 **/
PUBLIC(void) OgUsleep(unsigned long microseconds) {
#if (DPcSystem==DPcSystemWin32)
  Sleep((DWORD) (microseconds/1000));
#else
  struct timespec tmReq;
  tmReq.tv_sec = microseconds / (1000 * 1000);
  tmReq.tv_nsec = (microseconds % (1000 * 1000)) * 1000;
  nanosleep (&tmReq, NULL);
#endif
}

/** @ingroup libmiscAPI
 * return last error number
 *
 * \return error number
 *
 * OgErrno()
 **/
PUBLIC(int) OgErrno(void)
{
#if (DPcSystem==DPcSystemWin32)
  return WSAGetLastError();
#else
  return errno;
#endif
}

/** @ingroup libmiscAPI
 * return last error message
 *
 * \return msg string (must be freed by caller)
 * \deprecated use OgStrError
 * OgWriteErrnoMsg()
 **/
PUBLIC(char *) OgWriteErrnoMsg(void) {
  char *msg;
#if (DPcSystem==DPcSystemWin32)
  HMODULE hModule = NULL; // default to system source
  DWORD dwLastError = GetLastError();
  LPSTR MessageBuffer;
  DWORD dwBufferLength;

  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;

  // if dwLastError is in the network range, load the message source
  if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
    hModule = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if(hModule != NULL) dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
  }

  // call FormatMessage() to allow for message text to be acquired
  // from the system or the supplied module handle
  if(dwBufferLength = FormatMessage(dwFormatFlags,
            hModule, // module to get message from (NULL == system)
            dwLastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
            (LPSTR) &MessageBuffer,0, NULL)) {
    msg=strdup(MessageBuffer);
    LocalFree(MessageBuffer);   // free the buffer allocated by the system
  }
  if(hModule != NULL) FreeLibrary(hModule);    // if we loaded a message source, unload it
#else
  msg=strdup(strerror(errno));
#endif
  return msg;
}

/** @ingroup libmiscAPI
 * return last error message
 *
 * \param  msg an allocated buffer for error msg
 * \return @msg
 *
 * OgWriteErrnoMsg()
 **/
PUBLIC(char *) OgStrError(char *msg) {
#if (DPcSystem==DPcSystemWin32)
  HMODULE hModule = NULL; // default to system source
  DWORD dwLastError = GetLastError();
  LPSTR MessageBuffer;
  DWORD dwBufferLength;

  DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;

  // if dwLastError is in the network range, load the message source
  if(dwLastError >= NERR_BASE && dwLastError <= MAX_NERR) {
    hModule = LoadLibraryEx(TEXT("netmsg.dll"), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if(hModule != NULL) dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
  }

  // call FormatMessage() to allow for message text to be acquired
  // from the system or the supplied module handle
  if(dwBufferLength = FormatMessage(dwFormatFlags,
            hModule, // module to get message from (NULL == system)
            dwLastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
            (LPSTR) &MessageBuffer,0, NULL)) {
    memcpy(msg,MessageBuffer,dwBufferLength);
    msg[dwBufferLength-1]='\0';
    CharToOem(msg,msg); // convert char to OEM
    LocalFree(MessageBuffer);   // free the buffer allocated by the system
  }
  if(hModule != NULL) FreeLibrary(hModule);    // if we loaded a message source, unload it
#else
  strcpy(msg,strerror(errno));
#endif
  return msg;
}


