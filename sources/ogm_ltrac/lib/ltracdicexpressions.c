/*
 * ltracdicexpressions.c
 *
 *  Created on: 27 févr. 2017
 *      Author: caroline
 */

#include "ogm_ltrac.h"



og_status LtracDicExpressionAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input)
{
  if(!dic_input->is_expression) DONE;

  int ientry;
  unsigned char *p, entry[DPcPathSize];

  /* word | attribute_number language_code frequency **/
  memcpy(entry, dic_input->value, dic_input->value_length);
  p = entry + dic_input->value_length;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;
  OggNout(dic_input->attribute_number, &p);
  OggNout(dic_input->language_code, &p);
  OggNout(dic_input->frequency, &p);
  ientry = p - entry;
  IFE(OgAutAdd(ctrl_ltrac->ha_expressions, ientry, entry));

  DONE;
}


PUBLIC(int) OgLtracDicExpressionsLog(void *handle)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;

  IFn(handle) DONE;

  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_ltrac->herr;
  aut_param->hmutex = ctrl_ltrac->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_ltrac->loginfo->where;
  aut_param->state_number = 0;
  sprintf(aut_param->name, "ltrac expressions");
  void *ha_expressions = OgAutInit(aut_param);
  IFn(ha_expressions) DPcErr;
  IFE(OgAufRead(ha_expressions, ctrl_ltrac->name_expressions));

  FILE *fd = fopen(ctrl_ltrac->log_expressions, "w");
  IFn(fd)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtracDicExpressionsLog: impossible to open '%s' for writing", ctrl_ltrac->log_expressions);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  int iout;
  unsigned char *p, out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1;
  if ((retour = OgAufScanf(ha_expressions, 0, "", &iout, out, &nstate0, &nstate1, states)))
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

      int attribute_number = -1;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&attribute_number));

      int language_code = DOgLangNil;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&language_code));

      int frequency = -1;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&frequency));

      int ibuffer;
      unsigned char buffer[DPcPathSize];
      IFE(OgUniToCp(sep,out,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));

      og_string attribute_name = "0";
      struct og_attribute_info ai[1];
      og_bool found = OgAttributeGetInfoFromAttributeNumber(ctrl_ltrac->hattribute, ctrl_ltrac->herr, attribute_number,
          ai);
      IFE(found);
      if (found)
      {
        attribute_name = ai->attribute_name;
      }

      fprintf(fd, "%s | %d=%s %d %d\n", buffer, attribute_number, attribute_name, language_code, frequency);

    }
    while ((retour = OgAufScann(ha_expressions, &iout, out, nstate0, &nstate1, states)));
  }

  fclose(fd);
  IFE(OgAutFlush(ha_expressions));

  DONE;
}
