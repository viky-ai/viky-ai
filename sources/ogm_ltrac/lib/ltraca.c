/*
 *  Main function for Ltrac compile library for attributes
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#include "ogm_ltrac.h"

#define DOgLtracMaxValueLength  0x800

struct og_ltraca_info {
  struct og_ctrl_ltrac *ctrl_ltrac;
  int iovalue; unsigned char ovalue[DOgLtracMaxValueLength];
  int first_attval,frequency;
  int oattribute_number;
  };


static int SidxLtracScan(void *context, struct og_sidx_ltrac_scan *scan);
static int LtracAttributeExists(struct og_ctrl_ltrac *ctrl_ltrac, int attribute_number);
static int LtracAttributesScan(void *context,int ibstring, int rupture, unsigned char *bstring);
static int LtracScanAtvamat(void *context,struct og_attval *attval, int frequency);
static int LtracAttributesAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input);
static int LtracAddAttributeNumber(struct og_ctrl_ltrac *ctrl_ltrac);




PUBLIC(int) OgLtracAddAttributeInit(void *handle, int positive)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
ctrl_ltrac->is_attrnum_positive = positive;
IFE(OgAutReset(ctrl_ltrac->ha_attrstr));
DONE;
}




PUBLIC(int) OgLtracAddAttribute(void *handle, char *attribute_string)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
int ibuffer; char buffer[DPcPathSize];

sprintf(buffer,"%s",attribute_string); ibuffer=strlen(buffer);
IFE(OgAutAdd(ctrl_ltrac->ha_attrstr,ibuffer,buffer));

DONE;
}




static int LtracAddAttributeNumber(struct og_ctrl_ltrac *ctrl_ltrac)
{
unsigned char out[DPcAutMaxBufferSize+9];
int ibuffer; char buffer[DPcPathSize];
oindex states[DPcAutMaxBufferSize+9];
int retour,nstate0,nstate1,iout;
int found,attribute_number;

if ((retour=OgAutScanf(ctrl_ltrac->ha_attrstr,-1,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
    IFE(retour);
    IFE(found=ctrl_ltrac->OgSidxAttributeStringToNumber(ctrl_ltrac->hsidx,out,&attribute_number));
    if (!found) continue;
    sprintf(buffer,"%d",attribute_number); ibuffer=strlen(buffer);
    IFE(OgAutAdd(ctrl_ltrac->ha_attrnum,ibuffer,buffer));
    ctrl_ltrac->nb_attrnum++;
    }
  while((retour=OgAutScann(ctrl_ltrac->ha_attrstr,&iout,out,nstate0,&nstate1,states)));
  }

DONE;
}



static int LtracAttributeExists(struct og_ctrl_ltrac *ctrl_ltrac, int attribute_number)
{
int retour;
int ibuffer; char buffer[DPcPathSize];
sprintf(buffer,"%d",attribute_number); ibuffer=strlen(buffer);
IFE(retour=OgAutTry(ctrl_ltrac->ha_attrnum,ibuffer,buffer));
if (retour==2) return(1);
return(0);
}




/*
 *  Reads the items.ory, but also the atva in matrix.
 *  So we need to read that as well.
*/

int LtracAttributes(struct og_ctrl_ltrac *ctrl_ltrac)
{
struct og_sidx_param csidx_param,*sidx_param=&csidx_param;
struct og_sidx_scan_input cscaninput,*scaninput=&cscaninput;
struct og_ltrac_input *input = ctrl_ltrac->input;
struct og_ltraca_info cinfo,*info=&cinfo;

ctrl_ltrac->nb_attrnum=0;
IFE(OgAutReset(ctrl_ltrac->ha_attrnum));

IFE(LtracAttributesPlugInit(ctrl_ltrac));

memset(sidx_param,0,sizeof(struct og_sidx_param));
sidx_param->herr=ctrl_ltrac->herr;
sidx_param->hmsg=ctrl_ltrac->hmsg;
sidx_param->hmutex=ctrl_ltrac->hmutex;
sidx_param->loginfo.trace = DOgSidxTraceMinimal+DOgSidxTraceMemory;
sidx_param->loginfo.where = ctrl_ltrac->loginfo->where;
strcpy(sidx_param->WorkingDirectory,ctrl_ltrac->WorkingDirectory);
strcpy(sidx_param->configuration_file,ctrl_ltrac->configuration_file);
strcpy(sidx_param->data_directory,ctrl_ltrac->data_directory);
strcpy(sidx_param->import_directory,"");
if (ctrl_ltrac->OgSidxHasLtraDirectory(sidx_param->data_directory)) {
  sidx_param->must_ltrac=1;
  }
/** Avoid asking for an import directory which is not necessary **/
sidx_param->scan_only=1;
sidx_param->ltrac_min_frequency=input->min_frequency;

// avoid matrix autoloading which use a lot of memory
sidx_param->dont_load_matrix_on_feed_init = 1;

IFn(ctrl_ltrac->OgSidxInit(sidx_param,&ctrl_ltrac->hsidx,&ctrl_ltrac->authorized)) DPcErr;
if (!ctrl_ltrac->authorized) {
  char *message=ctrl_ltrac->OgSidxAuthorizationMessage(ctrl_ltrac->hsidx);
  OgErr(ctrl_ltrac->herr,message);
  DPcErr;
  }
IFE(ctrl_ltrac->OgSidxFeedInit(ctrl_ltrac->hsidx,0));

IFE(LtracAddAttributeNumber(ctrl_ltrac));

memset(info,0,sizeof(struct og_ltraca_info));
info->ctrl_ltrac=ctrl_ltrac;
info->first_attval=1;

if (sidx_param->must_ltrac) {
  IFE(ctrl_ltrac->OgSidxLtracScan(ctrl_ltrac->hsidx,SidxLtracScan, info));
  }
else {
  memset(scaninput,0,sizeof(struct og_sidx_scan_input));
  scaninput->type=DOgSidxScanTypeItemAttribute;
  //scaninput->filename="log/ltrac_sidxscan.log";
  scaninput->func=LtracAttributesScan;
  scaninput->context=info;
  scaninput->silent=1;

  IFE(ctrl_ltrac->OgSidxScan(ctrl_ltrac->hsidx,scaninput));
  if (!info->first_attval) {
    struct ltrac_dic_input cdic_input,*dic_input=&cdic_input;
    memset(dic_input,0,sizeof(struct ltrac_dic_input));
    dic_input->value_length = info->iovalue;
    dic_input->value = info->ovalue;
    dic_input->attribute_number = info->oattribute_number;
    dic_input->frequency = info->frequency;
    IFE(LtracAttributesAdd(ctrl_ltrac,dic_input));
    }

  IFE(ctrl_ltrac->OgSidxScanAtvamat(ctrl_ltrac->hsidx,LtracScanAtvamat,info));
  }

IFE(ctrl_ltrac->OgSidxFlush(ctrl_ltrac->hsidx));

DONE;
}





static int SidxLtracScan(void *context, struct og_sidx_ltrac_scan *scan)
{
struct ltrac_dic_input cdic_input,*dic_input=&cdic_input;
struct og_ltraca_info *info = (struct og_ltraca_info *)context;
struct og_ctrl_ltrac *ctrl_ltrac = info->ctrl_ltrac;
int exists;

if (ctrl_ltrac->nb_attrnum>0) {
  IFE(exists=LtracAttributeExists(ctrl_ltrac,scan->attribute_number));
  if (ctrl_ltrac->is_attrnum_positive && !exists) DONE;
  if (!ctrl_ltrac->is_attrnum_positive && exists) DONE;
  }

memset(dic_input,0,sizeof(struct ltrac_dic_input));
dic_input->value_length = scan->iword;
dic_input->value = scan->word;
dic_input->attribute_number = scan->attribute_number;
dic_input->language_code = scan->language_code;
dic_input->frequency = scan->frequency;
IFE(LtracAttributesAdd(ctrl_ltrac,dic_input));

DONE;
}






static int LtracAttributesScan(void *context,int ibstring, int rupture, unsigned char *bstring)
{
struct og_ltraca_info *info = (struct og_ltraca_info *)context;
struct og_ctrl_ltrac *ctrl_ltrac = info->ctrl_ltrac;
int ibuffer; unsigned char buffer[DPcPathSize];
int c,exists,is_stopped,same_attval;
int type_attr,type_value,lang_value;
int ivalue; unsigned char *value;
ogrefnum_t item_number;
int attribute_number;
unsigned char *p;

if (bstring[3] != DOgItemAttribute) DONE;

p=bstring+rupture; IFE(DOgPninLong(ctrl_ltrac->herr,&p,&item_number));
IFE(is_stopped=ctrl_ltrac->OgSidxItemIsStopped(ctrl_ltrac->hsidx,item_number));
if (is_stopped) DONE;

p=bstring+4;
IFE(DOgPnin4(ctrl_ltrac->herr,&p,&type_attr));
if (type_attr != DOgSidxTypeAVNumber) DONE;

IFE(DOgPnin4(ctrl_ltrac->herr,&p,&attribute_number));

if (ctrl_ltrac->nb_attrnum>0) {
  IFE(exists=LtracAttributeExists(ctrl_ltrac,attribute_number));
  if (ctrl_ltrac->is_attrnum_positive && !exists) DONE;
  if (!ctrl_ltrac->is_attrnum_positive && exists) DONE;
  }

p+=2; /** jumping DOgItemAttValSep **/
IFE(DOgPnin4(ctrl_ltrac->herr,&p,&type_value));
IFE(DOgPnin4(ctrl_ltrac->herr,&p,&lang_value));
if (type_value != DOgSidxTypeAVString) DONE;

ivalue = rupture - (p-bstring);
value=p;

if (ivalue<=0) DONE;
if (ivalue >= DOgLtracMaxValueLength) DONE;

/** Removing phonetics **/
c=(value[0]<<8)+value[1];
if (c=='{') DONE;

if (ctrl_ltrac->loginfo->trace & 0) {
  char attribute_string[DPcPathSize];
  IFE(ctrl_ltrac->OgSidxAttributeNumberToString(ctrl_ltrac->hsidx,attribute_number,attribute_string));
  IFE(OgUniToCp(ivalue,value,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_ltrac->hmsg,"",DOgMsgDestInLog, "LtracAttributesScan: %d=%s '%s'",attribute_number,attribute_string,buffer);
  }

if (info->first_attval) { same_attval=1; info->first_attval=0; }
else if (info->oattribute_number == attribute_number
  && info->iovalue == ivalue
  && !memcmp(info->ovalue,value,ivalue)) same_attval=1;
else same_attval=0;

if (same_attval) {
  info->frequency++;
  }
else {
  struct ltrac_dic_input cdic_input,*dic_input=&cdic_input;
  memset(dic_input,0,sizeof(struct ltrac_dic_input));
  dic_input->value_length = info->iovalue;
  dic_input->value = info->ovalue;
  dic_input->attribute_number = info->oattribute_number;
  dic_input->frequency = info->frequency;
  IFE(LtracAttributesAdd(ctrl_ltrac,dic_input));
  info->oattribute_number = attribute_number; info->iovalue = ivalue;
  memcpy(info->ovalue,value,ivalue);
  info->frequency=1;
  }

DONE;
}




static int LtracScanAtvamat(void *context,struct og_attval *attval, int frequency)
{
struct og_ltraca_info *info = (struct og_ltraca_info *)context;
struct ltrac_dic_input cdic_input,*dic_input=&cdic_input;
struct og_ctrl_ltrac *ctrl_ltrac = info->ctrl_ltrac;
int c,attribute_number,exists;
unsigned char *p;

if (attval->type_attr != DOgSidxTypeAVNumber) DONE;
if (attval->type_value != DOgSidxTypeAVString) DONE;
p=attval->attr; IFE(attribute_number=OggNin4(&p));

if (ctrl_ltrac->nb_attrnum>0) {
  IFE(exists=LtracAttributeExists(ctrl_ltrac,attribute_number));
  if (ctrl_ltrac->is_attrnum_positive && !exists) DONE;
  if (!ctrl_ltrac->is_attrnum_positive && exists) DONE;
  }
/** Removing phonetics **/
c=(attval->value[0]<<8)+attval->value[1];
if (c=='{') DONE;

memset(dic_input,0,sizeof(struct ltrac_dic_input));
dic_input->value_length = attval->ivalue;
dic_input->value = attval->value;
dic_input->attribute_number = attribute_number;
dic_input->frequency = frequency;
IFE(LtracAttributesAdd(ctrl_ltrac,dic_input));

DONE;
}




static int LtracAttributesAdd(struct og_ctrl_ltrac *ctrl_ltrac,struct ltrac_dic_input *dic_input)
{
struct og_ltrac_input *input = ctrl_ltrac->input;

if (dic_input->value_length<=0) DONE;
if (dic_input->frequency < input->min_frequency) DONE;

IFE(LtracDicAdd(ctrl_ltrac,dic_input));

DONE;
}






/*
 * We mainly load the sidx necessary functions
 * and start the OgSidxInit function
*/

#if (DPcSystem == DPcSystemUnix)

int LtracAttributesPlugInit(struct og_ctrl_ltrac *ctrl_ltrac)
{
char *libogsidx="libogsidx.so";
char libname[DPcPathSize];
char erreur[DOgErrorSize];

IFn(ctrl_ltrac->hlibogsidx = dlopen(libogsidx, RTLD_LAZY)) {
  char *sys_erreur = dlerror();
  IFn(sys_erreur) sys_erreur="no error found";
  sprintf(erreur,"LtracAttributesPlugInit: Impossible to load library '%s':\n%s", libname, sys_erreur);
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxInit = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxInit");
IFn(ctrl_ltrac->OgSidxInit) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxInit");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxAuthorizationMessage = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxAuthorizationMessage");
IFn(ctrl_ltrac->OgSidxAuthorizationMessage) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxAuthorizationMessage");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxScan = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxScan");
IFn(ctrl_ltrac->OgSidxScan) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxScan");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxAttributeNumberToString = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxAttributeNumberToString");
IFn(ctrl_ltrac->OgSidxAttributeNumberToString) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxAttributeNumberToString");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxAttributeStringToNumber = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxAttributeStringToNumber");
IFn(ctrl_ltrac->OgSidxAttributeStringToNumber) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxAttributeStringToNumber");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxScanAtvamat = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxScanAtvamat");
IFn(ctrl_ltrac->OgSidxScanAtvamat) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxScanAtvamat");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxLtracScan = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxLtracScan");
IFn(ctrl_ltrac->OgSidxLtracScan) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxLtracScan");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxFeedInit = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxFeedInit");
IFn(ctrl_ltrac->OgSidxFeedInit) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxFeedInit");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxItemIsStopped = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxItemIsStopped");
IFn(ctrl_ltrac->OgSidxItemIsStopped) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxItemIsStopped");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxHasLtraDirectory = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxHasLtraDirectory");
IFn(ctrl_ltrac->OgSidxHasLtraDirectory) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxHasLtraDirectory");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

ctrl_ltrac->OgSidxFlush = dlsym(ctrl_ltrac->hlibogsidx, "OgSidxFlush");
IFn(ctrl_ltrac->OgSidxFlush) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxFlush");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

DONE;
}



#else
#if (DPcSystem == DPcSystemWin32)


int LtracAttributesPlugInit(struct og_ctrl_ltrac *ctrl_ltrac)
{
char *libogsidx="libogsidx.dll",*DOgMAIN;
char libname[DPcPathSize];
char erreur[DOgErrorSize];

if( DOgMAIN=getenv("DOgMAIN") ) sprintf(libname,"%s/private/bin/%s",DOgMAIN,libogsidx);
else strcpy(libname,libogsidx);
IFn(ctrl_ltrac->hlibogsidx=LoadLibrary(libname)) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load library '%s'", libname);
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxInit) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxInit");
IFn(ctrl_ltrac->OgSidxInit) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxInit");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxAuthorizationMessage) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxAuthorizationMessage");
IFn(ctrl_ltrac->OgSidxAuthorizationMessage) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxAuthorizationMessage");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxScan) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxScan");
IFn(ctrl_ltrac->OgSidxScan) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxScan");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxAttributeNumberToString) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxAttributeNumberToString");
IFn(ctrl_ltrac->OgSidxAttributeNumberToString) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxAttributeNumberToString");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxAttributeStringToNumber) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxAttributeStringToNumber");
IFn(ctrl_ltrac->OgSidxAttributeStringToNumber) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxAttributeStringToNumber");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxScanAtvamat) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxScanAtvamat");
IFn(ctrl_ltrac->OgSidxScanAtvamat) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxScanAtvamat");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxLtracScan) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxLtracScan");
IFn(ctrl_ltrac->OgSidxLtracScan) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxLtracScan");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxFeedInit) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxFeedInit");
IFn(ctrl_ltrac->OgSidxFeedInit) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxFeedInit");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxItemIsStopped) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxItemIsStopped");
IFn(ctrl_ltrac->OgSidxItemIsStopped) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxItemIsStopped");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxHasLtraDirectory) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxHasLtraDirectory");
IFn(ctrl_ltrac->OgSidxHasLtraDirectory) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxHasLtraDirectory");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

(FARPROC)(ctrl_ltrac->OgSidxFlush) = GetProcAddress(ctrl_ltrac->hlibogsidx,"OgSidxFlush");
IFn(ctrl_ltrac->OgSidxFlush) {
  sprintf(erreur, "LtracAttributesPlugInit: Impossible to load OgSidxFlush");
  OgErr(ctrl_ltrac->herr,erreur); DPcErr;
  }

DONE;
}

#endif
#endif


