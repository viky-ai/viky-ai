/*
 *  Making a listing of the zip file in a file, format is "history" format
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : December 2005
 *  Version 1.0
*/
#include "ogm_zlib.h"
#include <logfix.h>



/*
 *  Makes a history file listing from a zip,gzi,tgz file.
 *  returns 0 if this is not a zip file, otherwise
 *  returns the type of the file as defined in logzlib.h
*/

PUBLIC(int) OgZlibFileList(og_zip_lib handle, char *zlib_name, char *output_zlib_name, char *filename)
{
char date_string[DPcPathSize], unzipped_filename[DPcPathSize*2];
struct og_ctrl_zlib *ctrl_zlib = (struct og_ctrl_zlib *)handle;
struct og_zlib_scan_param cscan_param,*scan_param=&cscan_param;
struct og_zlib_info czlib_info,*zlib_info=&czlib_info;
struct og_zlib_listing clisting;
char erreur[DOgErrorSize];
int i,c=DOgSepReference;
FILE *fd;

IFE(OgZlibInfo(handle,zlib_name,zlib_info));
if (zlib_info->type <= 0) return(0);

memset(scan_param,0,sizeof(struct og_zlib_scan_param));
scan_param->zlib_name=zlib_name;
scan_param->listing=&clisting;

IFE(OgZipScan(handle,scan_param));

IFn(fd=fopen(filename,"w")) {
  sprintf(erreur,"OgZlibFileList: impossible to open '%s' for writing",filename);
  OgErr(ctrl_zlib->herr,erreur); DPcErr;
  }

for (i=0; i<scan_param->listing->nb_files; i++) {
  struct og_zlib_file *file = scan_param->listing->file + i;
  if (file->is_dir) continue;

  sprintf(date_string, "%d:%d:%d:%d:%d:%d"
    , file->date.iyear-1900, file->date.month, file->date.mday
    , file->date.hour, file->date.minute, file->date.isecond);

  sprintf(unzipped_filename,"%s%czip%c%s%c%d%c%d%c%s"
    ,output_zlib_name, c, c, file->written_name, c
    ,file->uncompressed_size, c, file->compressed_size, c, date_string);

  #if (DPcSystem == DPcSystemWin32)
  PcStrlwr(unzipped_filename);
  #endif

  fprintf(fd,"%s\n",unzipped_filename);
  }

fclose(fd);

return(zlib_info->type);
}





