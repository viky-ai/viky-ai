#include <lpcosys.h>

#if DPcSystem==DPcSystemWin32
#include <winsock2.h>
#include <windows.h>
#include <errno.h>
#else
#include <sys/stat.h>
#endif
#include <loggen.h>



/**
 * Write message "data" to pipe
 * 
 * \param fifoID
 * \param data
 * \param size |data|
 * \return 0: ok
 **/
PUBLIC(int) OgWritePipe(void *herr, NATIVE_FILE fifoID, const char *data, int size) {
  char erreur[DOgErrorSize];
 
#if DPcSystem==DPcSystemWin32
  int        written;
  OVERLAPPED ovlp;
#endif
  if (fifoID<0 && fifoID!=OgStdNull()) fifoID=OgStdOutput();	

#if DPcSystem==DPcSystemWin32
  ovlp.Offset=ovlp.OffsetHigh=0;
  ovlp.hEvent=NULL;
  if (!WriteFile(fifoID,data,size,&written,&ovlp)) {
    char *errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgWritePipe error: %s",errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    DPcErr;
  }
#else
  if (write(fifoID,data,size)<0) {
    char *errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgWritePipe error: %s",errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    DPcErr;  
  }
#endif
  DONE;
}

/** @ingroup libpsyncAPI
 *  Open or create a fifoPath to write 
 *  
 * \param fifoPath
 * \return 0: error (>0 fifoID)
 *
**/
PUBLIC(NATIVE_FILE) OgOpenWriterPipe(void *herr, const char *fifoPath) 
{
  NATIVE_FILE fifoID=0;
  char        erreur[DOgErrorSize];
  char        *errmsg;

#if DPcSystem==DPcSystemWin32
  char        fifoName[256];
  char       *baseName;

  baseName= OgBaseName((char *) fifoPath);
  if (strcmp(baseName,".") != 0 && strcmp(baseName,"..") != 0 && strcmp(baseName,"\\") != 0) {
    snprintf(fifoName,255,"\\\\.\\mailslot\\%s",baseName);
  }
  else strncpy(fifoName,"\\\\.\\mailslot\\OgFifo",255);
  if((fifoID = CreateFile(fifoName, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
			  (LPSECURITY_ATTRIBUTES) NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,(HANDLE) NULL))==INVALID_HANDLE_VALUE) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenWriterPipe error: (%s) %s",fifoName, errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    return 0;
  }
#else
  // non blocking mode since pipe may have been broken 
  if ((fifoID = open(fifoPath, O_WRONLY )) == -1) { 
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenWriterPipe error: (%s) %s",fifoPath,errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    return 0;
  }
  // set blocking mode for write 
  if (fcntl(fifoID, F_SETFL, O_WRONLY) == -1) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenWriterPipe error: (%s) %s",fifoPath,errmsg);
    free(errmsg);
    OgErr(herr,erreur);
    return 0;
  }
  // set close on exec 
  if (fcntl (fifoID, F_SETFD, 1) == -1) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenWriterPipe error: (%s) %s",fifoPath,errmsg);
    free(errmsg);
    OgErr(herr,erreur);
    return 0;
  }
#endif
  return fifoID;
}

/** @ingroup libpsyncAPI
 *  Open or create a fifoPath to read in it
 *  
 * \param fifoPath
 * \return 0: error (>0 fifoID)
 *
 * \note : In Windows, we use "mailslots" to implement pipe instead of
 * "Named pipes" because mailslots allows broadcast messages using
 * datagrams.
 **/
PUBLIC(NATIVE_FILE) OgOpenReaderPipe(void *herr, const char *fifoPath) {
  NATIVE_FILE fifoID=0;
  char        erreur[DOgErrorSize];
  char        *errmsg;

#if DPcSystem==DPcSystemWin32
  PSECURITY_DESCRIPTOR pSD;
  SECURITY_ATTRIBUTES  sa;
  char                 fifoName[256];
  char                *baseName;
  
  baseName= OgBaseName((char *) fifoPath);
  if (strcmp(baseName,".") != 0 && strcmp(baseName,"..") != 0 && strcmp(baseName,"\\") != 0) {
    snprintf(fifoName,255,"\\\\.\\mailslot\\%s",baseName);
  }
  else strncpy(fifoName,"\\\\.\\mailslot\\OgFifo",255);
 
  IFn (pSD = (PSECURITY_DESCRIPTOR) malloc(SECURITY_DESCRIPTOR_MIN_LENGTH)) {
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe malloc error");
    OgErr(herr,erreur); 
    return 0;
  }
  if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) {
    free(pSD);
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: %s",errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    return 0;
  }
  // add a NULL disc. ACL to the security descriptor.
  if (!SetSecurityDescriptorDacl(pSD, TRUE, (PACL) NULL, FALSE)) {
    free(pSD);
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: %s",errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    return 0;
  }
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = pSD;
  sa.bInheritHandle = FALSE;
  if ((fifoID = CreateMailslot(fifoName, 0,MAILSLOT_WAIT_FOREVER, &sa))==INVALID_HANDLE_VALUE) {
    free(pSD);
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoName, errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    return 0;
  }
  // TODO: free(pSD?)
#else
  struct stat statBuf;
  
  if (stat(fifoPath,&statBuf)==0) {  // file exist
    if (!S_ISFIFO(statBuf.st_mode)) { // ... but not a fifo => error
      errmsg = OgWriteErrnoMsg();
      snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: %s is not a pipe (%s)",fifoPath,errmsg);
      free(errmsg);
      OgErr(herr,erreur); 
      return 0;
    }
  }
  else { // file does not exist 
    if (errno!=ENOENT) { 
      errmsg = OgWriteErrnoMsg();
      snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoPath,errmsg);
      free(errmsg);
      OgErr(herr,erreur); 
      return 0;
    }
    //create fifo 
    if (mkfifo(fifoPath, 0666) == -1) {
      errmsg = OgWriteErrnoMsg();
      snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoPath,errmsg);
      free(errmsg);
      OgErr(herr,erreur); 
      return 0;
    }
    // set access mode since not well set at creation time (...)
    if (chmod(fifoPath, 0666) == -1) {
      errmsg = OgWriteErrnoMsg();
      snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoPath,errmsg);
      free(errmsg);
      OgErr(herr,erreur); 
      return 0;
    }
  }
  // non blocking mode since pipe may have been broken 
  if ((fifoID = open(fifoPath, O_RDONLY | O_NDELAY )) == -1) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoPath,errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    return 0;
  }
  // set blocking mode for next read
  if (fcntl(fifoID, F_SETFL, O_RDONLY) == -1) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoPath,errmsg);
    free(errmsg);
    OgErr(herr,erreur);
    return 0;
  }
  // set close on exec 
  if (fcntl (fifoID, F_SETFD, 1) == -1) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgOpenReaderPipe error: (%s) %s",fifoPath,errmsg);
    free(errmsg);
    OgErr(herr,erreur);
    return 0;
  }
#endif
  return fifoID;
}



/** @ingroup libpsyncAPI
 *  Close a pipe
 *  
 * \param fifoID
 * \return 0: ok
 *
**/
PUBLIC(int) OgClosePipe(void *herr, NATIVE_FILE fifoID)
{
#if DPcSystem==DPcSystemWin32
  CloseHandle(fifoID);
#else 
  if (close(fifoID) == -1) {
    char erreur[DOgErrorSize];
    char *errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgClosePipe error: %s",errmsg);
    free(errmsg);
    OgErr(herr,erreur); 
    DPcErr;
  }
#endif
  DONE;
}



/**
 * Read a message "data" from pipe 
 * This function is in blocking read mode:
 *   1) the thread is blocked until a data is written in pipe
 *   2) except if all pipe writers are killed => 
 *       select out with value 1 to 
 *         
 * 
 * \param fifoID
 * \param data
 * \param size |data| (buffer size)
 * \return >=0: len of received message <0: error
 **/
PUBLIC(int) OgReadPipe(void *herr, NATIVE_FILE fifoID, char *buffer, int size) {    
  char erreur[DOgErrorSize];
  int  len=0;
  char *errmsg;

#if DPcSystem==DPcSystemWin32
  if (!ReadFile(fifoID,buffer,size-1,&len,NULL)) {
    errmsg = OgWriteErrnoMsg();
    snprintf(erreur,DOgErrorSize-1, "OgReadPipe error: %s",errmsg);
    free(errmsg);
    OgErr(herr,erreur);     
    DPcErr;
  }
#else
  fd_set rfds;
  struct timeval tv;
  int    retval;

  do {
    FD_ZERO(&rfds);
    FD_SET(fifoID,&rfds);
    tv.tv_sec=1;
    tv.tv_usec=0;

    retval=select(fifoID+1,&rfds,NULL,NULL,&tv); // waiting for a data in pipe (retval=0 => timeout)
    if (retval<0) {
      errmsg = OgWriteErrnoMsg();
      snprintf(erreur,DOgErrorSize-1, "OgReadPipe error: %s",errmsg);
      free(errmsg);
      OgErr(herr,erreur);
      DPcErr;
    }
    else if (retval>0) { // received data or all writer pipe are closed
      // read data
      if ((len=read(fifoID,buffer,size-1))<0) {
      errmsg = OgWriteErrnoMsg();
      snprintf(erreur,DOgErrorSize-1, "OgReadPipe error: %s",errmsg);
      free(errmsg);
      OgErr(herr,erreur); 
      DPcErr;
      }
      if (len==0) 	OgUsleep(1000*1000);  // all writer pipe are closed
    }
  } 
  while (len==0);
  
#endif
  return len;
}
/** @ingroup libpsyncAPI
 *  Delete pipe
 *  
 * \param fifoPath
 * \return 0: ok
 **/
PUBLIC(int) OgDeletePipe(void *herr, const char *fifoPath) {
#if DPcSystem==DPcSystemWin32
  return 0; // not make sense in WIN32; mailslots are ref counted,and destroyed when the refcount reach 0.
#else
  return unlink(fifoPath);
#endif
}


 
