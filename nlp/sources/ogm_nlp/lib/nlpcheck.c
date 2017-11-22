/*
 *  Checking some consistancy between packages for a given request
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpCheckPackage(og_nlp_th ctrl_nlp_th, package_t package);
static og_status NlpCheckMissingInterpretation(og_nlp_th ctrl_nlp_th, package_t package, struct input_part *input_part,
    og_string interpretation_id);
static og_bool NlpCheckMissingInterpretationPackage(og_nlp_th ctrl_nlp_th, og_string interpretation_id,
    package_t package);

og_status NlpCheckPackages(og_nlp_th ctrl_nlp_th)
{
  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_package_all = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  for (int i = 0; i < package_used; i++)
  {
    IFE(NlpCheckPackage(ctrl_nlp_th, interpret_package_all[i].package));
  }

  DONE;
}

static og_status NlpCheckPackage(og_nlp_th ctrl_nlp_th, package_t package)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "NlpCheckPackage checking interpretation ids for package '%s' '%s':",
      package->slug, package->id);

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
        NlpThrowErrorTh(ctrl_nlp_th, "NlpCheckPackage: error in ha_interpretation_id");
        DPcErr;
      }

      int Iinput_part;
      unsigned char *p = out + sep + 1;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&p,&Iinput_part));
      struct input_part *input_part = OgHeapGetCell(package->hinput_part, Iinput_part);

      unsigned char interpretation_id[DPcAutMaxBufferSize + 9];
      memcpy(interpretation_id, out, sep);
      interpretation_id[sep] = 0;

      IFE(NlpCheckMissingInterpretation(ctrl_nlp_th, package, input_part, interpretation_id));
    }
    while ((retour = OgAufScann(package->ha_interpretation_id, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static og_status NlpCheckMissingInterpretation(og_nlp_th ctrl_nlp_th, package_t package, struct input_part *input_part,
    og_string interpretation_id)
{
  int package_used = OgHeapGetCellsUsed(ctrl_nlp_th->hinterpret_package);
  struct interpret_package *interpret_package_all = OgHeapGetCell(ctrl_nlp_th->hinterpret_package, 0);
  for (int i = 0; i < package_used; i++)
  {
    int found_interpretation;
    found_interpretation = NlpCheckMissingInterpretationPackage(ctrl_nlp_th, interpretation_id,
        interpret_package_all[i].package);
    IFE(found_interpretation);
    if (found_interpretation) DONE;
  }
  NlpThrowErrorTh(ctrl_nlp_th,
      "NlpCheckMissingInterpretation: missing interpretation '%s' declared in alias '%s' in expression '%s' in package '%s' '%s'",
      interpretation_id, input_part->alias->alias, input_part->expression->text, package->slug, package->id);
  DPcErr;
}

static og_bool NlpCheckMissingInterpretationPackage(og_nlp_th ctrl_nlp_th, og_string interpretation_id,
    package_t package)
{
  int interpretation_used = OgHeapGetCellsUsed(package->hinterpretation);
  struct interpretation *interpretation_all = OgHeapGetCell(package->hinterpretation, 0);
  IFN(interpretation_all) DPcErr;
  for (int i = 0; i < interpretation_used; i++)
  {
    struct interpretation *interpretation = interpretation_all + i;
    if (!strcmp(interpretation->id, interpretation_id)) return TRUE;
  }
  return (FALSE);
}

