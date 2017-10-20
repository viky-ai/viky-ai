/*
 *  handling packages word automaton
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

og_status NlpInputPartWordInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_nlp_th->herr;
  aut_param->hmutex = ctrl_nlp_th->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_nlp_th->loginfo->where;
  aut_param->state_number = 0x1000;
  sprintf(aut_param->name, "package_ha_word");
  package->ha_word = OgAutInit(aut_param);
  IFn(package->ha_word) DPcErr;
  DONE;
}

og_status NlpInputPartWordFlush(og_nlp_th ctrl_nlp_th, package_t package)
{
  IFE(OgAutFlush(package->ha_word));
  DONE;
}


og_status NlpInputPartWordAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string string_word, int length_string_word,
    int Iinput_part)
{
  unsigned char buffer[DPcAutMaxBufferSize];
  memcpy(buffer, string_word, length_string_word);
  buffer[length_string_word] = 1;

  unsigned char *p = buffer + length_string_word + 1;
  OggNout(Iinput_part,&p);
  int length = p - buffer;
  IFE(OgAutAdd(package->ha_word, length, buffer));

  DONE;
}

og_status NlpInputPartWordLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  og_string package_id = OgHeapGetCell(package->hba, package->id_start);
  IFN(package_id) DPcErr;
  og_string package_slug = OgHeapGetCell(package->hba, package->slug_start);
  IFN(package_slug) DPcErr;
  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Words for package '%s' '%s':", package_slug, package_id);

  if ((retour = OgAutScanf(package->ha_word, 0, "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int sep = -1;
      for (int i = 0; i < iout; i++)
      {
        if (out[i] == 1)
        {
          sep = i;
          break;
        }
      }
      if (sep < 0)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInputPartWordLog: error in ha_word");
        DPcErr;
      }

      int Iinput_part;
      unsigned char *p = out+sep+1;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "%.*s : %d",sep, out, Iinput_part);
    }
    while ((retour = OgAutScann(package->ha_word, &iout, out, nstate0, &nstate1, states)));
  }

  return (0);
}

