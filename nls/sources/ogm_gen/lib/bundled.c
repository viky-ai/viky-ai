/*
 *	Calculates some information on a bundled filename.
 *	Copyright (c) 2000-2005	Pertimm by Patrick Constant
 *	Dev : January 2000, January 2001, April 2005
 *	Version 1.3
*/
#include <loggen.h>
#include <logfix.h>
#include <logstat.h>





/*
 *  Gets bundled file information from a filename
 *  returns 1 if this is a bundled file, 0 otherwise
 *  'info' can be set to zero, if we just need the 
 *  information that is is a bundled file.
 *  example of a double bundle: 
 *    g:\test\eml\zip1.eml eml 2/PR_EPO_SDS_10.zip zip PR_EPO_SDS_10.doc
 *  example of a single bundle with a 'tail':
 *    d:\cserve\download\zlib122dll.zip zip readme.txt 2800 1061 104:10:7:12:52:52
 *  space are 0x1 ('\1') chars in those examples.
*/

PUBLIC(int) OgGetBundledFileInfo(int ifilename, char *filename, struct og_bundled_file_info *info)
{
int i,j,d,c,end=0,state=1,len,is_bundle=0;
int start_bundle=0,start_tail_element=0;
int is=ifilename; char *s=filename;
struct og_bundle_file_info_type *bt;
char erreur[DOgErrorSize];
char buffer[DPcPathSize];

IFn(info) {
  for (i=0; i<is; i++) {
    if (s[i] == DOgSepReference) return(1);
    }
  return(0);
  }
memset(info,0,sizeof(struct og_bundled_file_info));
info->ifilename = is;

for (i=0; !end; i++) {
  if (i>=is) { end=1; c=DOgSepReference; }
  else c=s[i];

  switch(state) {
    /** in file before bundle type or tail **/
    case 1:
      if (c==DOgSepReference) {
        if (i+1<is) {
          if (!isdigit(s[i+1])) { start_bundle=i+1; state=2; }
          else { info->ifilename=i; info->is_tail=1; start_tail_element=i+1; state=3; }
          }
        }
      break;
    /** in bundle **/
    case 2:
      if (c==DOgSepReference) {
        if (info->nb_type < DOgMaxBundleType) {
          bt = info->type + info->nb_type;
          bt->start=start_bundle; bt->length=len=i-start_bundle;
          if (len==3 && !Ogmemicmp(s+bt->start,"zip",len)) bt->type=DOgBundledFileTypeZip;
          else if (len==3 && !Ogmemicmp(s+bt->start,"eml",len)) bt->type=DOgBundledFileTypeEml;
          else if (len==3 && !Ogmemicmp(s+bt->start,"nsf",len)) bt->type=DOgBundledFileTypeNsf;
          else if (len==2 && !Ogmemicmp(s+bt->start,"gz",len)) bt->type=DOgBundledFileTypeGz;
          else if (len==3 && !Ogmemicmp(s+bt->start,"tgz",len)) bt->type=DOgBundledFileTypeTgz;
          else bt->type=DOgBundledFileTypeNil;
          info->nb_type++; is_bundle=1;
          }
        state=1;
        }
      break;
    /** in tail, size **/
    case 3:
      if (c==DOgSepReference) {
        memcpy(buffer,s+start_tail_element,i-start_tail_element); buffer[i-start_tail_element]=0;
        info->stat.size_low=atoi(buffer); bt = info->type + (info->nb_type-1);
        if (bt->type == DOgBundledFileTypeZip) state=4; else state=5;
        start_tail_element = i+1;
        }
      break;
    /** in tail, zipped_size **/
    case 4:
      if (c==DOgSepReference) {
        memcpy(buffer,s+start_tail_element,i-start_tail_element); buffer[i-start_tail_element]=0;
        info->zipped_size=atoi(buffer); state=5;
        }
      break;
    /** first char of tail **/
    case 5:
      /** date is supposed to be the last element **/
      len = ifilename-i; memcpy(buffer,s+i,len); buffer[len]=0;
      for (j=0; j<len; j++) {
          if (buffer[j]==':') buffer[j]=0;
          }
      info->date.iyear = 1900+atoi(buffer); d=1;
      for (j=0; j<len; j++) {
        if (buffer[j]==0) {
          switch (d) {
            case 1: info->date.month = atoi(buffer+j+1); break;
            case 2: info->date.mday = atoi(buffer+j+1); break;
            case 3: info->date.hour = atoi(buffer+j+1); break;
            case 4: info->date.minute = atoi(buffer+j+1); break;
            case 5: info->date.isecond = atoi(buffer+j+1); break;
            }
          d++;
          }
        }
      state=1;
      break;
    }
  }

if (info->is_tail) {
  IF(OgDateToTime_t(&info->date,&info->stat.mtime)) {
    OgErrLast(0,erreur,0);
    memset(&info->date,0,sizeof(struct og_date));
    info->date.iyear=2035; info->date.month=1; info->date.mday=1;
    IF(OgDateToTime_t(&info->date,&info->stat.mtime));
    }
  //OgMessageLog(DOgMlogInLog,"ogm_ocea",0,"Y=%d M=%d D=%d h=%d m=%d s=%d date = %.24s"
  // ,info->date.iyear ,info->date.month ,info->date.mday 
  // ,info->date.hour ,info->date.minute ,info->date.isecond,OgGmtime(&info->stat.mtime));
  }
return(is_bundle);
}


