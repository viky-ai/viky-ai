/*
 *  Handling Gz files components.
 *  Copyright (c) 2007 Pertimm by Guillaume Logerot and Patrick Constant
 *  Dev : July 2007
 *  Version 1.0
 */
#include "ogm_zlib.h"
#include "zlib.h"

struct og_ctrl_gz
{
  gzFile gzfile;
};

int OgGzTest(char *path)
{
  int nbread, len = DPcPathSize;
  char buf[DPcPathSize];
  gzFile gzfile;

  IFn(gzfile = gzopen(path, "rb"))
  DONE;

  while ((nbread = gzread(gzfile, buf, len)))
  {
  }

  gzclose(gzfile);

  DONE;
}

/* encapsulated functions from the zlib library :

 gzFile     gzopen  OF((const char *path, const char *mode));
 int        gzread  OF((gzFile file, voidp buf, unsigned len));
 char *     gzgets OF((gzFile file, char *buf, int len));
 int        gzgetc OF((gzFile file));
 z_off_t    gzseek OF((gzFile file,z_off_t offset, int whence));
 int        gzrewind OF((gzFile file));
 int        gzeof OF((gzFile file));
 int        gzclose OF((gzFile file));
 */

PUBLIC(og_zip) OgGzOpen(void *herr, char *path, char *mode)
{
  struct og_ctrl_gz *ctrl_gz;
  char erreur[DPcPathSize];

  IFn(ctrl_gz=malloc(sizeof(struct og_ctrl_gz)))
  {
    sprintf(erreur,"OgGzOpen: malloc error on ctrl_gz");
    OgErr(herr,erreur); return(0);
  }

  IFn(ctrl_gz->gzfile = gzopen(path,mode)) return(0);

  return((og_zip)ctrl_gz);
}

PUBLIC(int) OgGzRead(og_zip handle, char *buf, unsigned len)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  int nbread;

  IFE(nbread = gzread(ctrl_gz->gzfile, buf, len));

  return (nbread);
}

PUBLIC(int) OgGzWrite(og_zip handle, char *buf, unsigned len)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  int nbread;

  IFn (len) return (0);

  IFn(nbread = gzwrite(ctrl_gz->gzfile, buf, len))
  DPcErr;

  return (nbread);
}

PUBLIC(char *) OgGzGets(og_zip handle, char *buf, int len)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  char *stringp;

  IFn(stringp = gzgets(ctrl_gz->gzfile, buf, len))
  return (0);

  return (stringp);
}

PUBLIC(int) OgGzGetc(og_zip handle)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  int character;

  IFE(character = gzgetc(ctrl_gz->gzfile));

  return (character);
}

PUBLIC(long) OgGzSeek(og_zip handle, long offset, int whence)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  long offst;

  IFE(offst = (long) gzseek(ctrl_gz->gzfile, (z_off_t) offset, whence));

  return (offst);
}

PUBLIC(int) OgGzRewind(og_zip handle)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  IFE(gzrewind(ctrl_gz->gzfile));
  DONE;
}

PUBLIC(int) OgGzEof(og_zip handle)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  int eof;
  eof = gzeof(ctrl_gz->gzfile);
  return (eof);
}

PUBLIC(int) OgGzClose(og_zip handle)
{
  struct og_ctrl_gz *ctrl_gz = (struct og_ctrl_gz *) handle;
  int error;
  error = gzclose(ctrl_gz->gzfile);
  DPcFree(ctrl_gz);
  return (error);
}

