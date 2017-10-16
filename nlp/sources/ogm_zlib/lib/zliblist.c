/*
 *  Handling listing for content parts of EML files.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : April 2005
 *  Version 1.0
*/
#include "ogm_zlib.h"



/*
 *  Simply updating the 'listing' structure.
*/

int OgZfileListing(ctrl_zlib)
struct og_ctrl_zlib *ctrl_zlib;
{
struct og_zlib_listing *listing = ctrl_zlib->scan_param->listing;

IFn(listing) DONE;

memset(listing,0,sizeof(struct og_zlib_listing));
listing->nb_files = ctrl_zlib->ZfileUsed;
listing->file = ctrl_zlib->Zfile;

DONE;
}





int GetZfile(ctrl_zlib,pzfile)
struct og_ctrl_zlib *ctrl_zlib;
struct og_zlib_file **pzfile;
{
int i = ctrl_zlib->ZfileNumber;
struct og_zlib_file *zfile = 0;

beginGetZfile:

if (ctrl_zlib->ZfileUsed < ctrl_zlib->ZfileNumber) {
  i = ctrl_zlib->ZfileUsed++;
  zfile = ctrl_zlib->Zfile + i;
  }

if (i == ctrl_zlib->ZfileNumber) {
  char erreur[DOgErrorSize];
  unsigned a, b; struct og_zlib_file *og_zfile;
  if (ctrl_zlib->loginfo->trace & DOgZlibTraceMemory) {
    OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
                , "GetZfile: max zfile number (%d) reached"
                , ctrl_zlib->ZfileNumber);
    }
  a = ctrl_zlib->ZfileNumber; b = a + (a>>2) + 1;
  IFn(og_zfile=(struct og_zlib_file *)malloc(b*sizeof(struct og_zlib_file))) {
    sprintf(erreur,"GetZfile: malloc error on Zfile for %d structures and %lu bytes", b, b*sizeof(struct og_zlib_file));
    OgErr(ctrl_zlib->herr,erreur); DPcErr;
    }
  memcpy( og_zfile, ctrl_zlib->Zfile, a*sizeof(struct og_zlib_file));

  DPcFree(ctrl_zlib->Zfile); ctrl_zlib->Zfile = og_zfile;
  ctrl_zlib->ZfileNumber = b;

  if (ctrl_zlib->loginfo->trace & DOgZlibTraceMemory) {
    OgThrMessageLog(ctrl_zlib->hmutex, DOgMlogInLog, ctrl_zlib->loginfo->where, 0
                , "GetZfile: new zfile number is %d", ctrl_zlib->ZfileNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur, "GetZfile: no more zfile max is %d", ctrl_zlib->ZfileNumber);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
#endif

  goto beginGetZfile;
  }

memset(zfile,0,sizeof(struct og_zlib_file));

if (pzfile) *pzfile = zfile;
return(i);
}



