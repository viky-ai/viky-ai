/*
 *  Functions for the ltra_base dictionary
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
 */
#include "ogm_ltrac.h"
#include <logis639_3166.h>

int LtracDicBaseAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{

  int ientry;
  unsigned char *p, entry[DPcPathSize];

  /* word | language_code frequency **/
  memcpy(entry, dic_input->value, dic_input->value_length);
  p = entry + dic_input->value_length;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->language_code, &p);
  OggNout(dic_input->frequency, &p);
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_base, ientry, entry));

  DONE;
}

PUBLIC(int) OgLtracDicBaseLog(void *handle, void *ha_base)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  int ibuffer;
  unsigned char buffer[DPcPathSize];
  int language_code, frequency;
  oindex states[DPcAutMaxBufferSize + 9];
  int iout;
  unsigned char *p, out[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1;

  IFn(handle) DONE;

  if ((retour = OgAufScanf(ha_base, 0, "", &iout, out, &nstate0, &nstate1, states)))
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
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));
      IFE(OgUniToCp(sep,out,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));

      char slang_country[DPcPathSize];

      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "%s | %s %d", buffer,
          OgIso639_3166ToCode(language_code, slang_country), frequency);

    }
    while ((retour = OgAufScann(ha_base, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

