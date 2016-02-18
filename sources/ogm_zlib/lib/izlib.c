/*
 *  Initialization for ogm_zlib functions
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : April 2005
 *  Version 1.0
 */
#include "ogm_zlib.h"

PUBLIC(og_zip_lib) OgZlibInit(struct og_zlib_param *param)
{
  struct og_ctrl_zlib *ctrl_zlib;
  char erreur[DOgErrorSize];
  int size;

  IFn(ctrl_zlib=(struct og_ctrl_zlib *)malloc(sizeof(struct og_ctrl_zlib)))
  {
    sprintf(erreur,"OgZlibInit: malloc error on ctrl_zlib");
    OgErr(param->herr,erreur); return(0);
  }
  memset(ctrl_zlib,0,sizeof(struct og_ctrl_zlib));

  ctrl_zlib->herr = param->herr;
  ctrl_zlib->hmutex = param->hmutex;
  ctrl_zlib->cloginfo = param->loginfo;
  ctrl_zlib->loginfo = &ctrl_zlib->cloginfo;

  ctrl_zlib->BaSize = DOgBaSize;
  size = ctrl_zlib->BaSize*sizeof(unsigned char);
  IFn(ctrl_zlib->Ba=(unsigned char *)malloc(size))
  {
    sprintf(erreur,"OgZlibInit: malloc error on Ba (%d bytes)",size);
    OgErr(ctrl_zlib->herr,erreur); return(0);
  }

  ctrl_zlib->ZfileNumber = DOgZfileNumber;
  IFn(ctrl_zlib->Zfile=(struct og_zlib_file *)malloc(ctrl_zlib->ZfileNumber*sizeof(struct og_zlib_file)))
  {
    sprintf(erreur,"OgZlibInit: malloc error on Zfile");
    OgErr(ctrl_zlib->herr,erreur); return(0);
  }

  return((og_zip_lib)ctrl_zlib);
}

PUBLIC(int) OgZlibFlush(og_zip_lib handle)
{
  struct og_ctrl_zlib *ctrl_zlib = (struct og_ctrl_zlib *) handle;

  DPcFree(ctrl_zlib->Ba);
  DPcFree(ctrl_zlib->Zfile);
  DPcFree(ctrl_zlib);
  DONE;
}

