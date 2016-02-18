/*
 *  Functions for the ltra_swap dictionary
 *  Copyright (c) 2010 Pertimm, by Patrick Constant
 *  Dev : January 2010
 *  Version 1.0
 */
#include "ogm_ltrac.h"

static og_status LtracDicSwapAddOneLetter(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, int index);
static og_status LtracDicSwapAddTwoLetters(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, int index);
static og_status LtracDicSwapAddTwoSameLetters(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input,
    int index, int *index_double1, int index_double2);
static og_status LtracDicSwapAddOrigin(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input);

int LtracDicSwapAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{
  int is = dic_input->value_length;

  if (is <= 2) DONE;

  IFE(LtracDicSwapAddOrigin(ctrl_ltrac, dic_input));

  int index_double1 = -1;
  int index_double2 = -1;

  /* Adding the form with one letter deleted */
  for (int i = 0; i < dic_input->value_length; i += 2)
  {
    IFE(LtracDicSwapAddOneLetter(ctrl_ltrac, dic_input, i));
    if(i < dic_input->value_length - 2)
    {
      IFE(LtracDicSwapAddTwoLetters(ctrl_ltrac, dic_input, i));
    }
    IFE(LtracDicSwapAddTwoSameLetters(ctrl_ltrac, dic_input, i, &index_double1, index_double2));
  }

  DONE;
}

static og_status LtracDicSwapAddOneLetter(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, int index)
{
  int ientry;
  unsigned char *p, entry[DPcPathSize];
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  unsigned char *s = dic_input->value;
  int is = dic_input->value_length;

  memcpy(buffer, s, index);
  memcpy(buffer + index, s + index + 2, is - index - 2);
  ibuffer = is - 2;

  p = entry;
  memcpy(p, buffer, ibuffer);
  p += ibuffer;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->attribute_number, &p);
  OggNout(dic_input->language_code, &p);
  OggNout(index, &p);
  OggNout(dic_input->frequency, &p);
  memcpy(p, s, is);
  p += is;
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_swap, ientry, entry));

  DONE;
}

// carfour => carrefour, rhumatogue => rhumatologue
static og_status LtracDicSwapAddTwoLetters(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, int index)
{
  int ientry;
  unsigned char *p, entry[DPcPathSize];
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  unsigned char *s = dic_input->value;
  int is = dic_input->value_length;

  memcpy(buffer, s, index);
  memcpy(buffer + index, s + index + 4, is - index - 4);
  ibuffer = is - 4;

  p = entry;
  memcpy(p, buffer, ibuffer);
  p += ibuffer;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->attribute_number, &p);
  OggNout(dic_input->language_code, &p);
  OggNout(index, &p);
  OggNout(dic_input->frequency, &p);
  memcpy(p, s, is);
  p += is;
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_swap, ientry, entry));

  DONE;
}

// comisariat => commissariat, apele => appelle
static og_status LtracDicSwapAddTwoSameLetters(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input,
    int index, int *index_double1, int index_double2)
{
  int ientry;
  unsigned char *p, entry[DPcPathSize];
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  unsigned char *s = dic_input->value;
  int is = dic_input->value_length;

  if(index > 0)
  {
    int c1 = (s[index] << 8) + s[index + 1];
    int c2 = (s[index - 2] << 8) + s[index - 1];
    if(c1 == c2)
    {
      if(*index_double1 == -1)
      {
        *index_double1 = index;
      }
      else
      {
        // We avoid to suppress once gain when we have 3 identical successive letters
        if((index > 2) && (*index_double1 != (index - 2)))
        {
          index_double2 = index;
          memcpy(buffer, s, *index_double1);
          memcpy(buffer + *index_double1, s + *index_double1 + 2, index_double2 - *index_double1 - 2);
          memcpy(buffer + index_double2 - 2, s + index_double2 + 2, is - index_double2 - 2);
          ibuffer = is - 4;
          /* word_less_1_letter | attribute_number language_code position frequency word */
          p = entry;
          memcpy(p, buffer, ibuffer);
          p += ibuffer;
          *p++ = 0;
          *p++ = DOgLtracExtStringSeparator;
          OggNout(dic_input->attribute_number, &p);
          OggNout(dic_input->language_code, &p);
          OggNout(index, &p);
          OggNout(dic_input->frequency, &p);
          memcpy(p, s, is);
          p += is;
          ientry = p - entry;
          IFE(OgAutAdd(ctrl_ltrac->ha_swap, ientry, entry));
        }
      }
    }
  }

  DONE;
}

static og_status LtracDicSwapAddOrigin(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{
  int ientry;
  unsigned char *p, entry[DPcPathSize];

  p = entry;
  memcpy(entry, dic_input->value, dic_input->value_length);
  p += dic_input->value_length;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->attribute_number, &p);
  OggNout(dic_input->language_code, &p);
  OggNout(0, &p);
  OggNout(dic_input->frequency, &p);
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_swap, ientry, entry));

  DONE;
}


PUBLIC(int) OgLtracDicSwapLog(void *handle)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  struct og_sidx_param csidx_param, *sidx_param = &csidx_param;
  struct og_aut_param caut_param, *aut_param = &caut_param;
  int attribute_number, language_code, position, frequency;
  int iout;
  unsigned char *p, out[DPcAutMaxBufferSize + 9];
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  int iword;
  unsigned char word[DPcPathSize];
  oindex states[DPcAutMaxBufferSize + 9];
  char attribute_string[DPcPathSize];
  int retour, nstate0, nstate1;
  char erreur[DOgErrorSize];
  void *ha_swap;
  FILE *fd;

  IFn(handle) DONE;

  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_ltrac->herr;
  aut_param->hmutex = ctrl_ltrac->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
  aut_param->state_number = 0;
  sprintf(aut_param->name, "ltrac swap");
  IFn(ha_swap=OgAutInit(aut_param)) DPcErr;
  IFE(OgAufRead(ha_swap, ctrl_ltrac->name_swap));

  IFn(fd=fopen(ctrl_ltrac->log_swap,"w"))
  {
    sprintf(erreur, "OgLtracDicBaseLog: impossible to open '%s' for writing", ctrl_ltrac->log_swap);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  IFE(LtracAttributesPlugInit(ctrl_ltrac));

  memset(sidx_param, 0, sizeof(struct og_sidx_param));
  sidx_param->herr = ctrl_ltrac->herr;
  sidx_param->hmsg = ctrl_ltrac->hmsg;
  sidx_param->hmutex = ctrl_ltrac->hmutex;
  sidx_param->loginfo.trace = DOgSidxTraceMinimal + DOgSidxTraceMemory;
  sidx_param->loginfo.where = ctrl_ltrac->loginfo->where;
  strcpy(sidx_param->WorkingDirectory, ctrl_ltrac->WorkingDirectory);
  strcpy(sidx_param->configuration_file, ctrl_ltrac->configuration_file);
  strcpy(sidx_param->data_directory, ctrl_ltrac->data_directory);
  strcpy(sidx_param->import_directory, "");
  IFn(ctrl_ltrac->OgSidxInit(sidx_param,&ctrl_ltrac->hsidx,&ctrl_ltrac->authorized)) DPcErr;

  if ((retour = OgAufScanf(ha_swap, 0, "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      int i, c, sep = (-1);
      IFE(retour);
      for (i = 0; i < iout; i += 2)
      {
        c = (out[i] << 8) + out[i + 1];
        if (c == DOgLtracExtStringSeparator)
        {
          sep = i;
          break;
        }
      }
      if (sep < 0) continue;
      p = out + sep + 2;
      IFE(OgUniToCp(sep,out,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&attribute_number));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&position));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));
      word[0] = 0;
      if (p - out < iout)
      {
        IFE(OgUniToCp(iout-(p-out),p,DPcPathSize,&iword,word,DOgCodePageUTF8,0,0));
      }
      IFE(ctrl_ltrac->OgSidxAttributeNumberToString(ctrl_ltrac->hsidx, attribute_number, attribute_string));
      fprintf(fd, "%s | %d=%s %d %d %d %s\n", buffer, attribute_number, attribute_string, language_code, position,
          frequency, word);

    }
    while ((retour = OgAufScann(ha_swap, &iout, out, nstate0, &nstate1, states)));
  }

  fclose(fd);
  IFE(OgAutFlush(ha_swap));
  IFE(ctrl_ltrac->OgSidxFlush(ctrl_ltrac->hsidx));

  DONE;
}

