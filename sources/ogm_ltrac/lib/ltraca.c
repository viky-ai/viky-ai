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
  struct og_ltrac_input *input;
  int iovalue; unsigned char ovalue[DOgLtracMaxValueLength];
  int first_attval,frequency;
  int oattribute_number;
  };


static int SidxLtracScan(void *context, struct og_sidx_ltrac_scan *scan);
static int LtracAttributeExists(struct og_ctrl_ltrac *ctrl_ltrac, int attribute_number);
static int LtracAttributesScan(void *context,int ibstring, int rupture, unsigned char *bstring);
static int LtracScanAtvamat(void *context,struct og_attval *attval, int frequency);
static int LtracAttributesAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input, struct ltrac_dic_input *dic_input);
static int LtracAddAttributeNumber(struct og_ctrl_ltrac *ctrl_ltrac);

static og_status SidxProcessLtracRequests(struct og_ctrl_ltrac *ctrl_ltrac, int min_frequency);
static og_status SidxLtracAddExpressions(void *context, struct og_sidx_ltrac_scan *scan);
static og_status SidxLtracIsFound(void *context, struct og_sidx_ltrac_scan *scan);
static og_status SidxLtracMergeFrequency(void *context, struct og_sidx_ltrac_scan *scan);
static og_status SidxLtracGetBestFrequencyForWord(void *context, struct og_sidx_ltrac_scan *scan);


struct merge_frequency_context
{
    struct og_ctrl_ltrac *ctrl_ltrac;
    int min_frequency;
};

struct add_expressions_context
{
    struct og_ctrl_ltrac *ctrl_ltrac;
};


struct best_frequency_context
{
    struct og_ctrl_ltrac *ctrl_ltrac;
    int language_code;
    int best_frequency;
};

struct is_found_context
{
    struct og_ctrl_ltrac *ctrl_ltrac;
    int language_code;
    og_bool found;
};

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

if ((retour=OgAutScanf(ctrl_ltrac->ha_attrstr,-1,"",&iout,out,&nstate0,&nstate1,states))) {
  do {
      IFE(retour);

      og_attribute attr = OgAttributeGetFromName(ctrl_ltrac->hattribute, out);
      IFE(attr);
      if (!attr) continue;

      // if attribute is a virtual, add real children attributes instead
      int ira = 0;
      int sra = 50;
      og_attribute ra[sra];

      IFE(OgAttributeGetRealsFromVirtual(ctrl_ltrac->hattribute, attr, sra, &ira, ra));
      if (ira == 0)
      {
        struct og_attribute_info ai[1];
        IFE(OgAttributeGetInfo(ctrl_ltrac->hattribute, attr, ai));

        sprintf(buffer, "%d", ai->attribute_number);
        ibuffer = strlen(buffer);
        IFE(OgAutAdd(ctrl_ltrac->ha_attrnum, ibuffer, buffer));
        ctrl_ltrac->nb_attrnum++;
      }
      else
      {
        for (int i = 0; i < ira; i++)
        {
          og_attribute real_attr = ra[i];

          struct og_attribute_info ai[1];
          IFE(OgAttributeGetInfo(ctrl_ltrac->hattribute, real_attr, ai));

          sprintf(buffer, "%d", ai->attribute_number);
          ibuffer = strlen(buffer);
          IFE(OgAutAdd(ctrl_ltrac->ha_attrnum, ibuffer, buffer));
          ctrl_ltrac->nb_attrnum++;
        }
      }

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

int LtracAttributes(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input)
{

  IFE(OgSidxConfSetMinFrequency(ctrl_ltrac->hsidx, input->min_frequency));

  ctrl_ltrac->nb_attrnum = 0;
  IFE(OgAutReset(ctrl_ltrac->ha_attrnum));

  IFE(LtracAddAttributeNumber(ctrl_ltrac));

  struct og_ltraca_info info[1];
  memset(info, 0, sizeof(struct og_ltraca_info));
  info->ctrl_ltrac = ctrl_ltrac;
  info->first_attval = 1;
  info->input = input;

  if (ctrl_ltrac->must_ltrac)
  {
    if(ctrl_ltrac->has_ltraf_requests)
    {
      IFE(SidxProcessLtracRequests(ctrl_ltrac, input->min_frequency));
    }
    IFE(OgSidxLtracScan(ctrl_ltrac->hsidx, SidxLtracScan, info));
  }
  else
  {
    struct og_sidx_scan_input scaninput[1];
    memset(scaninput, 0, sizeof(struct og_sidx_scan_input));
    scaninput->type = DOgSidxScanTypeItemAttribute;
    //scaninput->filename="log/ltrac_sidxscan.log";
    scaninput->func = LtracAttributesScan;
    scaninput->context = info;
    scaninput->silent = 1;

    IFE(OgSidxScan(ctrl_ltrac->hsidx, scaninput));
    if (!info->first_attval)
    {
      struct ltrac_dic_input dic_input[1];
      memset(dic_input, 0, sizeof(struct ltrac_dic_input));
      dic_input->value_length = info->iovalue;
      dic_input->value = info->ovalue;
      dic_input->attribute_number = info->oattribute_number;
      dic_input->frequency = info->frequency;
      IFE(LtracAttributesAdd(ctrl_ltrac, input, dic_input));
    }

    IFE(OgSidxScanAtvamat(ctrl_ltrac->hsidx, LtracScanAtvamat, info));

  }

  DONE;
}



static og_status SidxProcessLtracRequests(struct og_ctrl_ltrac *ctrl_ltrac, int min_frequency)
{

  struct merge_frequency_context merge_context[1];
  memset(merge_context, 0, sizeof(struct merge_frequency_context));
  merge_context->ctrl_ltrac = ctrl_ltrac;
  merge_context->min_frequency = min_frequency;

  IFE(OgSidxLtracScan(ctrl_ltrac->hsidx, SidxLtracMergeFrequency, merge_context));


  struct add_expressions_context expr_context[1];
  memset(expr_context, 0, sizeof(struct add_expressions_context));
  expr_context->ctrl_ltrac = ctrl_ltrac;

  IFE(OgSidxLtracRequestsScan(ctrl_ltrac->hsidx, SidxLtracAddExpressions, expr_context));

  // Apply changes stored in a temp automaton in ha_ltrac
  IFE(OgSidxLtracApply(ctrl_ltrac->hsidx));

  DONE;
}


static og_status SidxLtracAddExpressions(void *context, struct og_sidx_ltrac_scan *scan)
{
  struct add_expressions_context *expr_context = (struct add_expressions_context *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = expr_context->ctrl_ltrac;

  if (ctrl_ltrac->nb_attrnum > 0)
  {
    og_bool exists = LtracAttributeExists(ctrl_ltrac, scan->attribute_number);
    IFE(exists);
    if (ctrl_ltrac->is_attrnum_positive && !exists) CONT;
    if (!ctrl_ltrac->is_attrnum_positive && exists) CONT;
  }

  og_bool found = TRUE;
  int start = 0;
  for (int i = 0; i < scan->iword; i += 2)
  {
    int c = (scan->word[i] << 8) + scan->word[i + 1];
    if (c == ' ')
    {

      start = i;
      int length = i-start;

      char buffer[DPcAutMaxBufferSize];
      memcpy(buffer, scan->word + start, length);
      int ibuffer = length;
      buffer[ibuffer++] = 0;


      struct is_found_context found_ctx[1];
      memset(found_ctx, 0, sizeof(struct is_found_context));
      found_ctx->ctrl_ltrac = ctrl_ltrac;
      found_ctx->language_code = scan->language_code;

      IFE(OgSidxLtracScanWord(ctrl_ltrac->hsidx, SidxLtracIsFound, ibuffer, buffer, found_ctx));

      if (!found_ctx->found)
      {
        found = FALSE;
        break;
      }
    }
  }

  //TODO nettoyer la chaine

  if (found)
  {

    IFE(OgSidxLtracAddEntry(ctrl_ltrac->hsidx, scan));

  }

  DONE;
}


static og_status SidxLtracIsFound(void *context, struct og_sidx_ltrac_scan *scan)
{
  struct is_found_context *found_context = (struct is_found_context *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = found_context->ctrl_ltrac;

  if (ctrl_ltrac->nb_attrnum > 0)
  {
    og_bool exists = LtracAttributeExists(ctrl_ltrac, scan->attribute_number);
    IFE(exists);
    if (ctrl_ltrac->is_attrnum_positive && !exists) CONT;
    if (!ctrl_ltrac->is_attrnum_positive && exists) CONT;
  }

  if(found_context->language_code != scan->language_code) CONT;

  found_context->found = TRUE;

  DONE;
}


static og_status SidxLtracMergeFrequency(void *context, struct og_sidx_ltrac_scan *scan)
{
  struct merge_frequency_context *merge_context = (struct merge_frequency_context *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = merge_context->ctrl_ltrac;

  if (ctrl_ltrac->nb_attrnum > 0)
  {
    og_bool exists = LtracAttributeExists(ctrl_ltrac, scan->attribute_number);
    IFE(exists);
    if (ctrl_ltrac->is_attrnum_positive && !exists) CONT;
    if (!ctrl_ltrac->is_attrnum_positive && exists) CONT;
  }


  struct best_frequency_context ctx[1];
  memset(ctx, 0, sizeof(struct best_frequency_context));
  ctx->ctrl_ltrac = ctrl_ltrac;
  ctx->language_code = scan->language_code;


  IFE(OgSidxLtracRequestsScanWord(ctrl_ltrac->hsidx, SidxLtracGetBestFrequencyForWord, scan->iword, scan->word, ctx));

  if(ctx->best_frequency != 0)
  {
    scan->frequency = ctx->best_frequency;
  }
  else
  {
    scan->frequency = merge_context->min_frequency;
  }

  IFE(OgSidxLtracAddEntryForceFrequency(ctrl_ltrac->hsidx, scan));


  DONE;
}



static og_status SidxLtracGetBestFrequencyForWord(void *context, struct og_sidx_ltrac_scan *scan)
{
  struct best_frequency_context *ctx = (struct best_frequency_context *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = ctx->ctrl_ltrac;

  if (ctrl_ltrac->nb_attrnum > 0)
  {
    og_bool exists = LtracAttributeExists(ctrl_ltrac, scan->attribute_number);
    IFE(exists);
    if (ctrl_ltrac->is_attrnum_positive && !exists) CONT;
    if (!ctrl_ltrac->is_attrnum_positive && exists) CONT;
  }

  if (ctx->language_code != scan->language_code) CONT;

  if (scan->frequency > ctx->best_frequency)
  {
    ctx->best_frequency = scan->frequency;
  }

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
IFE(LtracAttributesAdd(ctrl_ltrac, info->input, dic_input));

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
IFE(is_stopped=OgSidxItemIsStopped(ctrl_ltrac->hsidx,item_number));
if (is_stopped) DONE;

p=bstring+4;
IFE(DOgPnin4(ctrl_ltrac->herr,&p,&type_attr));
if (type_attr != DOgSidxTypeAVNumber) DONE;

IFE(DOgPnin4(ctrl_ltrac->herr,&p,&attribute_number));

struct og_attribute_info ai[1];
og_bool found = OgAttributeGetInfoFromAttributeNumber(ctrl_ltrac->hattribute, ctrl_ltrac->herr, attribute_number, ai);
IFE(found);
if (!found) DONE;

// Do not fill in dictionary if ling ort is not used in configuration file
if (!(ai->ling & DOgSsiScanFieldLingOrt)) DONE;

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
  IFE(OgUniToCp(ivalue,value,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_ltrac->hmsg,"",DOgMsgDestInLog, "LtracAttributesScan: %d=%s '%s'",attribute_number,ai->attribute_name,buffer);
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
  struct ltrac_dic_input dic_input[1];
  memset(dic_input,0,sizeof(struct ltrac_dic_input));
  dic_input->value_length = info->iovalue;
  dic_input->value = info->ovalue;
  dic_input->attribute_number = info->oattribute_number;
  dic_input->frequency = info->frequency;
  IFE(LtracAttributesAdd(ctrl_ltrac, info->input, dic_input));
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
IFE(LtracAttributesAdd(ctrl_ltrac, info->input, dic_input));

DONE;
}




static int LtracAttributesAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input, struct ltrac_dic_input *dic_input)
{
if (dic_input->value_length<=0) DONE;
if (dic_input->frequency < input->min_frequency) DONE;

IFE(LtracDicAdd(ctrl_ltrac, input, dic_input));

DONE;
}

