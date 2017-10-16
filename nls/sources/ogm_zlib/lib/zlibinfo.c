/*
 *  Getting information about a file and checks which type it is: zip, gz, tgz
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : April 2005
 *  Version 1.0
 */
#include "ogm_zlib.h"

PUBLIC(int) OgZlibInfo(og_zip_lib handle, char *filename, struct og_zlib_info *info)
{
  struct og_ctrl_zlib *ctrl_zlib = (struct og_ctrl_zlib *)handle;
  int retour;

  memset(info,0,sizeof(struct og_zlib_info));

  IFx(OgStat(filename,0xf,&info->stat))
  {
    OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
        , "OgZlibInfoDate: could not stat '%s'", filename);
  }

  IFE(retour=OgZlibIsZip(handle,filename));
  if (retour)
  { info->type=DOgZlibTypeZip; return(1);}

  return(0);
}

PUBLIC(char *) OgZlibTypeString(type)
  int type;
{
  switch (type)
  {
    case DOgZlibTypeZip:
      return ("zip");
    case DOgZlibTypeGz:
      return ("gzi");
    case DOgZlibTypeTgz:
      return ("tgz");
  }
  return ("nil");
}

