/*
 *  The heart function for handling Gz files.
 *  This is fully based upon the zlib122/contrib/miniunz.c program.
 *  Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *  Dev : April 2005, July 2007
 *  Version 1.1
 */
#include "ogm_zlib.h"
#include "zlib.h"

STATICF(int) OgGzScan1(pr_(struct og_ctrl_zlib *) pr_(gzFile) pr(char *));

/*
 * In fact, this does not work, when the file is not a zip file
 * it is simply copied. However this imformation is somewhere
 * but may not be accessible easily through the interface.
 */

PUBLIC(int) OgZlibIsGz(char *filename)
{
  gzFile gzfile;
  IFn(gzfile = gzopen(filename, "rb"))
  return (0);
  gzclose(gzfile);
  return (1);
}

PUBLIC(int) OgGzScan(og_zip handle, struct og_zlib_scan_param *zlib_scan_param)
{
  struct og_ctrl_zlib *ctrl_zlib = (struct og_ctrl_zlib *) handle;
  char filename_ingz[DPcPathSize];
  char erreur[DOgErrorSize];
  int i, retour, is;
  char *s;
  gzFile in;

  ctrl_zlib->scan_param = &ctrl_zlib->cscan_param;
  memcpy(ctrl_zlib->scan_param, zlib_scan_param, sizeof(struct og_zlib_scan_param));

  strcpy(ctrl_zlib->zlib_name, ctrl_zlib->scan_param->zlib_name);
  ctrl_zlib->zlib_part[0] = 0;

  s = ctrl_zlib->zlib_name;
  is = strlen(s);
  for (i = 0; i + 5 < is; i++)
  {
    if (memcmp(s + i, "\1gzi\1", 5)) continue;
    s[i] = 0;
    strcpy(ctrl_zlib->zlib_part, s + i + 5);
    break;
  }

  ctrl_zlib->BaUsed = 0;
  ctrl_zlib->ZfileUsed = 0;

  IFn(in = gzopen(ctrl_zlib->zlib_name, "rb"))
  {
    sprintf(erreur, "OgGzScan: impossible to gzopen '%s'", ctrl_zlib->zlib_name);
    OgErr(ctrl_zlib->herr, erreur);
    DPcErr;
  }

  IFE(OgGzScan1(ctrl_zlib, in, filename_ingz));

  retour = gzclose(in);
  if (retour != Z_OK)
  {
    sprintf(erreur, "OgGzScan: gzclose: error %d on '%s'", retour, ctrl_zlib->zlib_name);
    OgErr(ctrl_zlib->herr, erreur);
    DPcErr;
  }

  IFE(OgZfileListing(ctrl_zlib));
  DONE;
}

STATICF(int) OgGzScan1(ctrl_zlib, in, filename_ingz)
  struct og_ctrl_zlib *ctrl_zlib;
  gzFile in;char *filename_ingz;
{
  char erreur[DOgErrorSize];
  int len, sbuf;
  char *buf;
  FILE *out;

  IFn(out = fopen(filename_ingz, "wb"))
  {
    sprintf(erreur, "OgGzScan1: impossible to fopen '%s' for extraction from '%s'"
        , filename_ingz, ctrl_zlib->zlib_name);
    OgErr(ctrl_zlib->herr, erreur);
    DPcErr;
  }

  /** making sure we have some room left **/
  IFE(OgZlibTestReallocBa(ctrl_zlib, 8192));
  buf = ctrl_zlib->Ba + ctrl_zlib->BaUsed;
  sbuf = ctrl_zlib->BaSize - ctrl_zlib->BaUsed;

  while (1)
  {
    len = gzread(in, buf, sbuf);
    if (len < 0)
    {
      sprintf(erreur, "OgZipScan1: gzread: error %d for '%s' in '%s'"
          , len, filename_ingz, ctrl_zlib->zlib_name);
      OgErr(ctrl_zlib->herr, erreur);
      DPcErr;
    }
    if (len == 0) break;
    if ((int) fwrite(buf, 1, (unsigned) len, out) != len)
    {
      sprintf(erreur, "OgZipScan1: fwrite error for '%s' in '%s'"
          , filename_ingz, ctrl_zlib->zlib_name);
      OgErr(ctrl_zlib->herr, erreur);
      DPcErr;
    }
  }

  if (fclose(out))
  {
    sprintf(erreur, "OgZipScan1: fclose error for '%s' in '%s'"
        , filename_ingz, ctrl_zlib->zlib_name);
    OgErr(ctrl_zlib->herr, erreur);
    DPcErr;
  }
  DONE;
}

