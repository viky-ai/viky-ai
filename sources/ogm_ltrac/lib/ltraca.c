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


static int LtracAddInDictionary(void *context, struct og_ltrac_scan *scan);
static int LtracAttributesAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input, struct ltrac_dic_input *dic_input);
static int LtracAddAttributeNumber(struct og_ctrl_ltrac *ctrl_ltrac);


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

  IFE(LtracReadLtraf(ctrl_ltrac, input->min_frequency));
  if (ctrl_ltrac->has_ltraf_requests)
  {
    IFE(LtracReadLtrafRequest(ctrl_ltrac, input->min_frequency));
    if (ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd)
    {
      IFE(LtracLogLtrac(ctrl_ltrac));
      IFE(LtracLogLtracRequests(ctrl_ltrac));
    }
  }
  IFE(LtracScan(ctrl_ltrac, LtracAddInDictionary, info));

  DONE;
}



static int LtracAddInDictionary(void *context, struct og_ltrac_scan *scan)
{
struct ltrac_dic_input cdic_input,*dic_input=&cdic_input;
struct og_ltraca_info *info = (struct og_ltraca_info *)context;
struct og_ctrl_ltrac *ctrl_ltrac = info->ctrl_ltrac;


memset(dic_input,0,sizeof(struct ltrac_dic_input));
dic_input->value_length = scan->iword;
dic_input->value = scan->word;
dic_input->attribute_number = scan->attribute_number;
dic_input->language_code = scan->language_code;

struct ltraf *ltraf = ctrl_ltrac->Ltraf + scan->Iltraf;
dic_input->frequency = ltraf->frequency;


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

