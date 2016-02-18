/*
 *  Header for library ogm_zlib.dll
 *  Copyright (c) 2005-2007 Pertimm by Patrick Constant
 *  Dev : April,December 2005, January,March 2006, July 2007
 *  Version 1.5
*/
#ifndef _LOGZLIBALIVE_
#include <loggen.h>
#include <logthr.h>
#include <logstat.h>


#define DOgZlibBanner  "ogm_zlib V1.21, Copyright (c) 2005-2007 Pertimm, Inc."
#define DOgZlibVersion 121


/** Trace levels **/
#define DOgZlibTraceMinimal          0x1
#define DOgZlibTraceMemory           0x2
#define DOgZlibTraceListing          0x4
#define DOgZlibTraceWriting          0x8

typedef struct og_ctrl_zlib *og_zip_lib;
typedef struct og_ctrl_gz *og_zip;

struct og_zlib_param {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo loginfo;
  };

/** Zlib type **/
#define DOgZlibTypeNil      0
#define DOgZlibTypeZip      1
#define DOgZlibTypeGz       2
#define DOgZlibTypeTgz      3

struct og_zlib_info {
  struct og_stat stat;
  int type;
  };

struct og_zlib_file {
  int is_dir;
  int compressed_size;
  int uncompressed_size;
  struct og_date date;
  unsigned char *original_name;
  unsigned char *written_name;
  };

struct og_zlib_listing {
  int nb_files;
  struct og_zlib_file *file;
  };


/* Values for the control_flags field:
 *   - DOgZlibScanFlagExtractFlat: extracts without internal path.
*/
#define DOgZlibScanFlagExtractFlat  0x1

struct og_zlib_scan_param {
  int control_flags;
  unsigned char *zlib_name, *directory;
  struct og_zlib_listing *listing;
  char *password;
  };


DEFPUBLIC(og_zip_lib) OgZlibInit(struct og_zlib_param *param);
DEFPUBLIC(int) OgZlibScan(og_zip_lib handle, struct og_zlib_scan_param *zlib_scan_param);
DEFPUBLIC(int) OgZlibInfo(og_zip_lib handle, char *filename, struct og_zlib_info *info);
DEFPUBLIC(char *) OgZlibTypeString(int type);
DEFPUBLIC(int) OgZlibFlush(og_zip_lib handle);

DEFPUBLIC(int) OgZipScan(og_zip_lib handle, struct og_zlib_scan_param *zlib_scan_param);
DEFPUBLIC(int) OgZipExtractPart(og_zip_lib handle, char *file_ref, char *file_name, char *directory);
DEFPUBLIC(int) OgZlibIsZip(og_zip_lib handle, char *filename);
DEFPUBLIC(int) OgZlibFileList(og_zip_lib handle, char *zlib_name, char *output_zlib_name, char *filename);

DEFPUBLIC(int) OgZlibIsGz(char *filename);

DEFPUBLIC(int) OgGzScan(og_zip handle, struct og_zlib_scan_param *zlib_scan_param);
DEFPUBLIC(int) OgGzTest(char *path);
DEFPUBLIC(og_zip) OgGzOpen(void *herr, char *path, char *mode);
DEFPUBLIC(int) OgGzRead(og_zip handle, char *buf,unsigned len);
DEFPUBLIC(int) OgGzWrite(og_zip handle, char *buf, unsigned len);
DEFPUBLIC(char *) OgGzGets(og_zip handle, char *buf, int len);
DEFPUBLIC(int) OgGzGetc(og_zip handle);
DEFPUBLIC(long) OgGzSeek(og_zip handle, long offset, int whence);
DEFPUBLIC(int) OgGzRewind(og_zip handle);
DEFPUBLIC(int) OgGzEof(og_zip handle);
DEFPUBLIC(int) OgGzClose(og_zip handle);


#define _LOGZLIBALIVE_
#endif


