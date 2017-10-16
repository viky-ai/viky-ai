/*
 *  64 bit fopen/fseek/ftell/fwrite/fread/fclose 
 *  Only when it is possible. Ftell to be done.
 *  Copyright (c) 2002 Pertimm, Inc. by Patrick Constant
 *  Dev : February 2002
 *  Version 1.0
*/


/** useless except for ogoiwin3.c **/
PUBLIC(int) OgSetIoBufferSize(int size)
{
DONE;
}




PUBLIC(void *) OgFopen(char *filename, char *mode)
{
void *retour;
retour = (void *)fopen(filename,mode);
return(retour);
}




PUBLIC(int) OgFstat(void *stream, struct og_stat *filestat)
{
int retour;
char erreur[DPcSzErr];
struct stat orig_filestat;
FILE *fd = (FILE *)stream;
retour = fstat(fileno(fd),&orig_filestat);
if (retour==0) {
  filestat->atime = orig_filestat.st_atime;
  filestat->mtime = orig_filestat.st_mtime;
  filestat->ctime = orig_filestat.st_ctime;
  if (sizeof(orig_filestat.st_size)==4) {
    filestat->size_high = 0;
    filestat->size_low = orig_filestat.st_size;
    }
  else if (sizeof(orig_filestat.st_size)==8) {
    filestat->size_high = orig_filestat.st_size>>32;
    filestat->size_low = (orig_filestat.st_size & 0xffffffff);
    }
  else {
    sprintf(erreur, "OgFstat: unhandled sizeof(orig_filestat.st_size) = %d"
      , sizeof(orig_filestat.st_size));
    PcErr(-1,erreur); DPcErr;
    }
  if (OgIsDir(orig_filestat.st_mode)) filestat->is_dir  = 1;
  else filestat->is_dir  = 0;
  }
return(retour);
}



/*
 * Using this function in other libraries which are not compiled
 * with the proper macros.
*/

PUBLIC(int) OgSizeOffT(void)
{
return(sizeof(off_t));
}



#if (DPcArch == DPcArchlinux)

PUBLIC(int) OgFseek(void *stream, struct og_bnum *offset, int origin)
{
int retour;
off_t big_offset;
char erreur[DPcSzErr];
unsigned offset_low,offset_high;

IFE(OgBnumToHighLow(offset,&offset_high,&offset_low,1));
if (sizeof(off_t) == 4) {
  /** File size is limited to 2Gb and fseek works for 0x8000000 **/
  if (offset_high || offset_low>=0x80000000) {
    char value[128];
    OgBigFormatThousand(offset,value);
    sprintf(erreur, "OgFseek: offset too big: %s bytes",value);
    PcErr(-1,erreur); DPcErr;
    }
  big_offset = offset_low; 
  retour = fseeko(stream,big_offset,origin);
  }
else if (sizeof(off_t) == 8) {
  big_offset = offset_high; big_offset <<= 32;
  big_offset |= offset_low; 
  retour = fseeko(stream,big_offset,origin);
  }
else {
  sprintf(erreur, "OgFseek: unhandled sizeof(off_t) = %d", sizeof(off_t));
  PcErr(-1,erreur); DPcErr;
  }
if (retour==0) DONE;
DPcErr;
}



PUBLIC(int) OgFtell(void *stream, struct og_bnum *offset)
{
off_t big_offset;
char erreur[DPcSzErr];

big_offset = ftello(stream);
if (big_offset==(off_t)(-1)) {
  sprintf(erreur, "OgFtell: impossible to ftello");
  PcErr(-1,erreur); DPcErr;
  }

if (sizeof(off_t) == 4) {
  IFE(OgSetBnum(offset,big_offset,0,0));
  }
else if (sizeof(off_t) == 8) {
  unsigned high,low;
  high = (unsigned)(big_offset>>32);
  low = (unsigned)(big_offset & 0xffffffff);
  OgHighLowToBnum(high,low,offset);
  }
else {
  sprintf(erreur, "OgFtell: unhandled sizeof(off_t) = %d", sizeof(off_t));
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}




#else
#if (DPcArch == DPcArchsolaris)

PUBLIC(int) OgFseek(stream,offset,origin)
void *stream; struct og_bnum *offset;
int origin;
{
int retour;
off_t big_offset;
char erreur[DPcSzErr];
unsigned offset_low,offset_high;

IFE(OgBnumToHighLow(offset,&offset_high,&offset_low,1));
if (sizeof(off_t) == 4) {
  /** File size is limited to 2Gb and fseek works for 0x8000000 **/
  if (offset_high || offset_low>=0x80000000) {
    char value[128];
    OgBigFormatThousand(offset,value);
    sprintf(erreur, "OgFseek: offset too big: %s bytes",value);
    PcErr(-1,erreur); DPcErr;
    }
  big_offset = offset_low; 
  retour = fseeko(stream,big_offset,origin);
  }
else if (sizeof(off_t) == 8) {
  big_offset = offset_high; big_offset <<= 32;
  big_offset |= offset_low; 
  retour = fseeko(stream,big_offset,origin);
  }
else {
  sprintf(erreur, "OgFseek: unhandled sizeof(off_t) = %d", sizeof(off_t));
  PcErr(-1,erreur); DPcErr;
  }
if (retour==0) DONE;
DPcErr;
}



PUBLIC(int) OgFtell(stream,offset)
void *stream; struct og_bnum *offset;
{
off_t big_offset;
char erreur[DPcSzErr];

big_offset = ftello(stream);
if (big_offset==(off_t)(-1)) {
  sprintf(erreur, "OgFtell: impossible to ftello");
  PcErr(-1,erreur); DPcErr;
  }

if (sizeof(off_t) == 4) {
  IFE(OgSetBnum(offset,big_offset,0,0));
  }
else if (sizeof(off_t) == 8) {
  unsigned high,low;
  high = (unsigned)(big_offset>>32);
  low = (unsigned)(big_offset & 0xffffffff);
  OgHighLowToBnum(high,low,offset);
  }
else {
  sprintf(erreur, "OgFtell: unhandled sizeof(off_t) = %d", sizeof(off_t));
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}


#else
#if (DPcArch == DPcArchtru64)

PUBLIC(int) OgFseek(stream,offset,origin)
void *stream; struct og_bnum *offset;
int origin;
{
int retour;
long int long_offset;
IFE(OgBnumToLong64(offset,&long_offset,0));
retour = fseek(stream,long_offset,origin);
if (retour==0) DONE;
DPcErr;
}


#else
#if (DPcArch == DPcArchaix)


PUBLIC(int) OgFseek(stream,offset,origin)
void *stream; struct og_bnum *offset;
int origin;
{
int retour;
off_t big_offset;
char erreur[DPcSzErr];
unsigned offset_low,offset_high;

IFE(OgBnumToHighLow(offset,&offset_high,&offset_low,1));
if (sizeof(off_t) == 4) {
  /** File size is limited to 2Gb and fseek works for 0x8000000 **/
  if (offset_high || offset_low>=0x80000000) {
    char value[128];
    OgBigFormatThousand(offset,value);
    sprintf(erreur, "OgFseek: offset too big: %s bytes",value);
    PcErr(-1,erreur); DPcErr;
    }
  big_offset = offset_low; 
  retour = fseeko(stream,big_offset,origin);
  }
else if (sizeof(off_t) == 8) {
  big_offset = offset_high; big_offset <<= 32;
  big_offset |= offset_low; 
  retour = fseeko(stream,big_offset,origin);
  }
else {
  sprintf(erreur, "OgFseek: unhandled sizeof(off_t) = %d", sizeof(off_t));
  PcErr(-1,erreur); DPcErr;
  }
if (retour==0) DONE;
DPcErr;
}



PUBLIC(int) OgFtell(stream,offset)
void *stream; struct og_bnum *offset;
{
off_t big_offset;
char erreur[DPcSzErr];

big_offset = ftello(stream);
if (big_offset==(off_t)(-1)) {
  sprintf(erreur, "OgFtell: impossible to ftello");
  PcErr(-1,erreur); DPcErr;
  }

if (sizeof(off_t) == 4) {
  IFE(OgSetBnum(offset,big_offset,0,0));
  }
else if (sizeof(off_t) == 8) {
  unsigned high,low;
  high = (unsigned)(big_offset>>32);
  low = (unsigned)(big_offset & 0xffffffff);
  OgHighLowToBnum(high,low,offset);
  }
else {
  sprintf(erreur, "OgFtell: unhandled sizeof(off_t) = %d", sizeof(off_t));
  PcErr(-1,erreur); DPcErr;
  }
DONE;
}


#else
DPcArch must be defined
#if 0
/** This version is limited to 2 giga-bytes for file size **/
PUBLIC(int) OgFseek(stream,offset,origin)
void *stream; struct og_bnum *offset;
int origin;
{
int retour;
int small_offset;
IFE(OgBnumToInt(offset,&small_offset,0));
retour = fseek(stream,small_offset,origin);
if (retour==0) DONE;
DPcErr;
}
#endif
#endif
#endif
#endif
#endif




PUBLIC(int) OgFwrite(void *stream, void *buffer, unsigned int size)
{
int retour;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];

retour=fwrite(buffer,1,size,(FILE *)stream);
IFn(retour) {
  OgSysErrMes(OgSysErr(),DPcSzErr,sys_erreur);
  sprintf(erreur, "OgFwrite: %s",sys_erreur);	
  PcErr(-1,erreur);
  }
return(retour);
}




PUBLIC(int) OgFread(void *stream, void *buffer, unsigned int size)
{
return(fread(buffer,1,size,(FILE *)stream));
}



PUBLIC(int) OgFflush(void *stream)
{
int retour;
retour = fflush((FILE *)stream);
if (retour==0) DONE;
DPcErr;
}




PUBLIC(int) OgFsync(void *stream)
{
int retour;
int sys_err;
char erreur[DPcSzErr*2];
char sys_erreur[DPcSzErr];
int handle = fileno((FILE *)stream);

retour = fsync(handle);
if (retour==0) DONE;

sys_err=OgSysErr();
OgSysErrMes(sys_err,DPcSzErr,sys_erreur);
sprintf(erreur, "OgFseek: %s",sys_erreur);	
PcErr(-1,erreur); DPcErr;
}




PUBLIC(int) OgFclose(void *stream)
{
int retour;
retour = fclose((FILE *)stream);
if (retour==0) DONE;
DPcErr;
}

