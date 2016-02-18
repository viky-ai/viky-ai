/*
 *	Common sources for ogm_fltr.exe and ogm_ocea.exe
 *	Copyright (c) 1999 Ogmios by Patrick Constant
 *	Dev : December 1999
 *	Version 1.2
*/
#include <logpath.h>
#include <loggen.h>
#include <string.h>


/** The minimum sort buffer size is 1 Mb **/
#define DOgMinSortBufferSize  (1 * 0x100000)
#define DOgMinDiskRamSortRatio      1
#define DOgDefaultDiskRamSortRatio 20
#define DOgMaxDiskRamSortRatio     70




PUBLIC(int) OgGetMaxSortBufferSize(char *where)
{
int retour;
char value[256];
int MaxSortBufferSize;
char *conf = DOgFileOgmConf_Txt;

IFE(retour=OgConfGetVar(conf,"MaxSortBufferSize",value,256,0));
if (retour) {
  MessageInfoLog(0,where, 0,"MaxSortBufferSize is: '%s'",value);
  MaxSortBufferSize = OgArgSize(value);
  }
else {
  MessageInfoLog(0,where, 0
    ,"MaxSortBufferSize not defined in '%s', using '5M'",conf);
  MaxSortBufferSize = DOgMinSortBufferSize;
  }
if (MaxSortBufferSize < DOgMinSortBufferSize) {
  MaxSortBufferSize = DOgMinSortBufferSize;
  }
OgFormatThousand(MaxSortBufferSize,value);
MessageInfoLog(0,where, 0
    ,"MaxSortBufferSize chosen: %s bytes",value);
return(MaxSortBufferSize);
}






PUBLIC(int) OgGetDiskRamSortRatio(char *where)
{
int retour;
char value[256];
int DiskRamSortRatio;
char *conf = DOgFileOgmConf_Txt;

IFE(retour=OgConfGetVar(conf,"DiskRamSortRatio",value,256,0));
if (retour) {
  MessageInfoLog(0,where, 0,"DiskRamSortRatio is: '%s'",value);
  DiskRamSortRatio = OgArgSize(value);
  }
else {
  MessageInfoLog(0,where, 0
    ,"DiskRamSortRatio not defined in '%s', using '%d'"
    ,conf,DOgDefaultDiskRamSortRatio);
  DiskRamSortRatio = DOgDefaultDiskRamSortRatio;
  }
if (DiskRamSortRatio < DOgMinDiskRamSortRatio) {
  DiskRamSortRatio = DOgMinDiskRamSortRatio;
  }
if (DiskRamSortRatio > DOgMaxDiskRamSortRatio) {
  DiskRamSortRatio = DOgMaxDiskRamSortRatio;
  }
MessageInfoLog(0,where, 0
    ,"DiskRamSortRatio chosen: %d",DiskRamSortRatio);

return(DiskRamSortRatio);
} 


