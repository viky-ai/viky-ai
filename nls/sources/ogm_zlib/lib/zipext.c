/*
 *  Extracting content parts of a zip file.
 *  Copyright (c) 2005 Pertimm by Patrick Constant
 *  Dev : April 2005
 *  Version 1.0
*/
#include "ogm_zlib.h"




/*
 *  Extracts a part of an EML file into 'directory'.
 *  file_ref has following format:
 *    <zlib_filename>\1zlib\1<zlib_partname>
*/
PUBLIC(int) OgZipExtractPart(og_zip_lib handle, char *file_ref, char *file_name, char *directory)
{
struct og_zlib_scan_param cscan_param,*scan_param=&cscan_param;
struct og_zlib_listing clisting;
int i;

file_name[0]=0;

memset(scan_param,0,sizeof(struct og_zlib_scan_param));
scan_param->zlib_name=file_ref;
scan_param->listing=&clisting;
scan_param->directory=directory;
scan_param->control_flags |= DOgZlibScanFlagExtractFlat;

IFE(OgZipScan(handle,scan_param));

if (scan_param->listing->nb_files<=0) return(0);

for (i=0; i<scan_param->listing->nb_files; i++) {
  struct og_zlib_file *file = scan_param->listing->file + i;
  if (file->is_dir) continue;
  sprintf(file_name,"%s/%s",directory,file->written_name);
  break;
  }

return(1);
}



