/*
 *  handling packages word automaton
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

og_status NlpInputPartAliasInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_nlp_th->herr;
  aut_param->hmutex = ctrl_nlp_th->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
  aut_param->loginfo.where = ctrl_nlp_th->loginfo->where;
  aut_param->state_number = 0x10;
  sprintf(aut_param->name, "package_ha_interpretation_id");
  package->ha_interpretation_id = OgAutInit(aut_param);
  IFn(package->ha_interpretation_id) DPcErr;
  DONE;
}


og_status NlpInputPartAliasFlush(package_t package)
{
  IFE(OgAutFlush(package->ha_interpretation_id));
  DONE;
}


og_status NlpInputPartAliasAdd(og_nlp_th ctrl_nlp_th, package_t package, og_string interpretation_id,
    size_t Iinput_part)
{
  unsigned char buffer[DPcAutMaxBufferSize];
  int length_interpretation_id = strlen(interpretation_id);
  memcpy(buffer, interpretation_id, length_interpretation_id);
  buffer[length_interpretation_id] = 1;

  unsigned char *p = buffer + length_interpretation_id + 1;
  OggNout(Iinput_part,&p);
  int length = p - buffer;
  IFE(OgAutAdd(package->ha_interpretation_id, length, buffer));
  DONE;
}


og_status NlpInputPartAliasDigitAdd(og_nlp_th ctrl_nlp_th, package_t package, size_t Iinput_part)
{
  size_t Idigit_input_part;
  struct digit_input_part *digit_input_part = OgHeapNewCell(package->hdigit_input_part, &Idigit_input_part);
  IFN(digit_input_part) DPcErr;
  digit_input_part->Iinput_part = Iinput_part;
  DONE;
}


og_status NlpInputPartAliasLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Interpretation ids for package '%s' '%s':", package->slug, package->id);

  if ((retour = OgAufScanf(package->ha_interpretation_id, 0, "", &iout, out, &nstate0, &nstate1, states)))
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
        NlpThrowErrorTh(ctrl_nlp_th, "NlpInputPartAliasLog: error in ha_word");
        DPcErr;
      }

      int Iinput_part;
      unsigned char *p = out+sep+1;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %.*s : %d",sep, out, Iinput_part);
    }
    while ((retour = OgAufScann(package->ha_interpretation_id, &iout, out, nstate0, &nstate1, states)));
  }


  DONE;
}

