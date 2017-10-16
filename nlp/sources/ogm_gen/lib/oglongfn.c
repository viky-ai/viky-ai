/*
 *	Getting information from a process id.
 *	Copyright (c) 2003 Pertimm by Patrick Constant
 *	Dev : May 2003
 *	Version 1.0
*/
#include <loggen.h>


/*  Gets the long filename of the file, we suppose that 
 *  filename is a full path, which is always the case here.
*/

#if (DPcSystem == DPcSystemUnix)

PUBLIC(int) OgGetLongFileName(filename,cleaned_long_filename)
char *filename, *cleaned_long_filename;
{
CleanFilename(filename,cleaned_long_filename);
DONE;
}

#else
#if (DPcSystem == DPcSystemWin32)


PUBLIC(int) OgGetLongFileName(filename,cleaned_long_filename)
char *filename, *cleaned_long_filename;
{
int i,sep,ifilename;
HANDLE FindHandle;	
WIN32_FIND_DATA FindFileData;
char long_filename[DPcPathSize];
char erreur[DPcSzErr];

for (i=0; filename[i]; i++) if (filename[i]=='/') filename[i]='\\';
ifilename = i;

if (ifilename==2 && filename[1] == ':') {
  strcpy(long_filename,filename);
  PcStrlwr(long_filename); CleanFilename(long_filename,cleaned_long_filename);
  DONE;
  }

//if (filename[1] != ':' && filename[2] != '\\') {
if (!OgIsAbsolutePath(filename)) {
  strcpy(long_filename,filename);
  PcStrlwr(long_filename); CleanFilename(long_filename,cleaned_long_filename);
  sprintf(erreur,"OgGetLongFileName: not a full path '%s'",filename);
  PcErr(-1,erreur); DPcErr;
  }
sprintf(long_filename,"%.*s",3,filename);

/** We must start at 3 because we don't want the first '\' but sep must be 2 **/
for (i=3,sep=2; i<ifilename; i++) {
  if (filename[i]!='\\') continue;
  filename[i]=0;
  FindHandle = FindFirstFile(filename,&FindFileData);
  if (FindHandle == INVALID_HANDLE_VALUE) {
    filename[i]='\\';
    sprintf(long_filename+strlen(long_filename),"%s",filename+sep+1);
    PcStrlwr(long_filename); CleanFilename(long_filename,cleaned_long_filename);
    DONE;
	}
  sprintf(long_filename+strlen(long_filename),"%s\\",FindFileData.cFileName);
  FindClose(FindHandle);
  filename[i]='\\';
  sep=i; 
  }

/** Last we handle the file itself **/
FindHandle = FindFirstFile(filename,&FindFileData);
if (FindHandle == INVALID_HANDLE_VALUE) {
  sprintf(long_filename+strlen(long_filename),"%s",filename+sep+1);
  PcStrlwr(long_filename); CleanFilename(long_filename,cleaned_long_filename);
  DONE;
  }
sprintf(long_filename+strlen(long_filename),"%s",FindFileData.cFileName);
PcStrlwr(long_filename); CleanFilename(long_filename,cleaned_long_filename);
FindClose(FindHandle);
DONE;
}

#endif
#endif

