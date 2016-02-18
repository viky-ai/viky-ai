/*
 *	This function get codepage, using charset and encoding instructions.
 *	Copyright (c) 2007 Pertimm by Patrick Constant
 *	Dev : November 2007
 *	Version 1.0
*/
#include <loguni.h>
#include <logauta.h>


struct og_ctrl_getcp {
  void *herr; ogmutex_t *hmutex;
  struct og_loginfo cloginfo;
  struct og_loginfo *loginfo;
  int hcharset_initialized;
  void *hcharset;
  };


struct og_getcp_info {
  struct og_ctrl_getcp *ctrl_getcp;
  int icontent; char content[DPcPathSize];
  int is_content_type;
  int found_html_tag;
  int codepage;
  int found;
  };



STATICF(int) OgGetCp1(pr_(void *) pr(struct og_read_tag *));
STATICF(int) OgGetCharset(pr(struct og_getcp_info *));




PUBLIC(void *) OgGetCpInit(param)
struct og_getcp_param *param;
{
char erreur[DOgErrorSize];
struct og_ctrl_getcp *ctrl_getcp;

IFn(ctrl_getcp=(struct og_ctrl_getcp *)malloc(sizeof(struct og_ctrl_getcp))) {
  sprintf(erreur,"OgGetCpInit: malloc error on ctrl_getcp");
  OgErr(param->herr,erreur); return(0);
  }
memset(ctrl_getcp,0,sizeof(struct og_ctrl_getcp));

ctrl_getcp->herr = param->herr;
ctrl_getcp->hmutex = param->hmutex;
ctrl_getcp->cloginfo = param->loginfo;
ctrl_getcp->loginfo = &ctrl_getcp->cloginfo;
ctrl_getcp->hcharset = param->hcharset;

IFn(ctrl_getcp->hcharset) {
  struct og_charset_param ccharset_param, *charset_param=&ccharset_param;
  memset(charset_param,0,sizeof(struct og_charset_param));
  charset_param->herr=ctrl_getcp->herr; 
  charset_param->hmutex=ctrl_getcp->hmutex;
  charset_param->loginfo.trace = DOgAutaTraceMinimal+DOgAutaTraceMemory; 
  charset_param->loginfo.where = ctrl_getcp->loginfo->where;
  IFn(ctrl_getcp->hcharset=OgCharsetInit(charset_param)) return(0);
  ctrl_getcp->hcharset_initialized=1;
  }

return((void *)ctrl_getcp);
}




PUBLIC(int) OgGetCpFlush(handle)
void *handle;
{
struct og_ctrl_getcp *ctrl_getcp = (struct og_ctrl_getcp *)handle;

if (ctrl_getcp->hcharset_initialized) {
  IFE(OgCharsetFlush(ctrl_getcp->hcharset));
  }

DPcFree(ctrl_getcp);
DONE;
}



/* 
 * The tag we are looking for is typically:
 *   <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-2">
 * but we can also have this information in reverse order  
 *   <meta content="text/html; charset=UTF-8" http-equiv="Content-Type">
 * We also accept XML tags such as:
 *   <?xml version="1.0" encoding="UTF-8"?>
 *  Returns 1 if codepage is found.
*/

PUBLIC(int) OgGetCp(handle,icode,code,pcodepage)
void *handle; int icode; unsigned char *code;
int *pcodepage;
{
struct og_ctrl_getcp *ctrl_getcp = (struct og_ctrl_getcp *)handle;
struct og_getcp_info cinfo,*info=&cinfo;
int i,retour,tag_length;

*pcodepage=0;

memset(info,0,sizeof(struct og_getcp_info));
info->ctrl_getcp = ctrl_getcp;

for (i=0; i<icode; i++) {
  if (code[i]!='<') continue;
  retour = OgIsTag(0,icode-i,code+i,&tag_length);
  /** Normal tag **/
  if (retour == 1) {
    info->is_content_type=0; info->icontent=0; info->found=0;
    IFE(OgReadTag(tag_length,code+i+1,0,OgGetCp1,info));
    if (info->found_html_tag && info->is_content_type==2) {
      if (ctrl_getcp->loginfo->trace & DOgGetcpTraceNormal) {
        OgMessageLog(DOgMlogInLog,ctrl_getcp->loginfo->where,0
          ,"OgGetCp: discarding real codepage from '%.*s' because in HTML", tag_length, code+i);
        }
      info->is_content_type=0;
      }
    if (info->is_content_type && info->icontent>0) {
      IFE(OgGetCharset(info));
      if (info->found) {
        if (ctrl_getcp->loginfo->trace & DOgGetcpTraceNormal) {
          OgMessageLog(DOgMlogInLog,ctrl_getcp->loginfo->where,0
            ,"OgGetCp: found real codepage from '%.*s'", tag_length, code+i);
          }
        break;
        }
      }
    }
  }

if (info->found) {
  *pcodepage=info->codepage;
  return(1);
  }
  
return(0);
}





STATICF(int) OgGetCp1(ptr,rt)
void *ptr; struct og_read_tag *rt;
{
struct og_getcp_info *info = (struct og_getcp_info *)ptr;

if (rt->closing_tag) DONE;
if (!Ogstricmp(rt->name,"?xml")) {
  if (!Ogstricmp(rt->attr,"encoding")) {
    int length=strlen(rt->value);
    info->is_content_type = 2;
    if (length > DPcPathSize-1) length=DPcPathSize-1;
    memcpy(info->content,rt->value,length);
    info->content[length]=0;
    info->icontent=length;
    }
  }
else if (!Ogstricmp(rt->name,"meta")) {
  if (!Ogstricmp(rt->attr,"http-equiv")) {
    /* We need to check to value, because we can have tags such as:
     * <meta content="fr" http-equiv="Content-Language"> */
    if (!Ogstricmp(rt->value,"content-type")) {
      info->is_content_type = 1;
      }
    }
  else if (!Ogstricmp(rt->attr,"content")) {
    int length=strlen(rt->value);
    if (length > DPcPathSize-1) length=DPcPathSize-1;
    memcpy(info->content,rt->value,length);
    info->content[length]=0;
    info->icontent=length;
    }
  }
else if (!Ogstricmp(rt->name,"html")) {
  info->found_html_tag=1;
  }

DONE;
}





STATICF(int) OgGetCharset(info)
struct og_getcp_info *info;
{
char *s=info->content;
char *charset="charset=";
int icharset=strlen(charset);
int i,start,found=0,is=info->icontent;
struct og_ctrl_getcp *ctrl_getcp = info->ctrl_getcp;

if (info->is_content_type == 1) {
  for (i=0; i<is; i++) {
    if (Ogmemicmp(charset,s+i,icharset)) continue;
    found=1; break;
    }

  if (!found) DONE;

  start=i+icharset;
  for (i=start; i<is; i++) {
    if (s[i]==';' || PcIsspace(s[i])) { s[i]=0; break; }
    }
  }
else if (info->is_content_type == 2) {
  start=0;
  }
else DONE;

IFE(found=OgCharsetGetCode(ctrl_getcp->hcharset,s+start,&info->codepage));
if (found) {
  info->found=1;
  if (ctrl_getcp->loginfo->trace & DOgGetcpTraceNormal) {
    char buffer[DPcPathSize];
    IFE(OgCharsetGetString(ctrl_getcp->hcharset,info->codepage,buffer));
    OgMessageLog(DOgMlogInLog,ctrl_getcp->loginfo->where,0
      ,"Found real codepage %d from charset '%s' original '%s'"
      , info->codepage, buffer, s+start);
    }
  }

DONE;
}


