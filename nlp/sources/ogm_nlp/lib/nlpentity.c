/*
 *  handling packages word automaton
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : October 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

#define DOgNlpMinEntityNumber 1000

og_status NlpEntityInit(og_nlp_th ctrl_nlp_th, package_t package)
{
  struct og_aut_param aut_param[1];
  memset(aut_param, 0, sizeof(struct og_aut_param));
  aut_param->herr = ctrl_nlp_th->herr;
  aut_param->hmutex = ctrl_nlp_th->hmutex;
  aut_param->loginfo.trace = DOgAutTraceMinimal;
  aut_param->loginfo.where = ctrl_nlp_th->loginfo->where;
  aut_param->state_number = 0x10;
  sprintf(aut_param->name, "package_ha_entity");
  package->ha_entity = OgAutInit(aut_param);
  IFn(package->ha_entity) DPcErr;
  package->nb_entities = 0;
  package->max_nb_words_per_entity = 0;
  DONE;
}

og_status NlpEntityFlush(package_t package)
{
  IFN(package->ha_entity) DONE;
  IFE(OgAutFlush(package->ha_entity));
  package->ha_entity = NULL;
  package->nb_entities = 0;
  package->max_nb_words_per_entity = 0;
  DONE;
}

og_status NlpEntityAdd(og_nlp_th ctrl_nlp_th, package_t package, int nb_words, og_string string_word,
    int length_string_word, struct expression *expression)
{
  unsigned char buffer[DPcAutMaxBufferSize];
  unsigned char *p;
  int ibuffer = 0;

  p = buffer + ibuffer;
  OggNout(nb_words, &p);

  ibuffer = p - buffer;
  memcpy(buffer + ibuffer, string_word, length_string_word);
  ibuffer += length_string_word;
  buffer[ibuffer++] = '\1';

  p = buffer + ibuffer;
  OggNout64((ogint64_t) expression, &p);

  int length = p - buffer;
  IFE(OgAutAdd(package->ha_entity, length, buffer));

  package->nb_entities++;
  if (package->max_nb_words_per_entity < nb_words) package->max_nb_words_per_entity = nb_words;

  DONE;
}


og_status NlpReduceEntities(og_nlp_th ctrl_nlp_th, package_t package)
{
  // we remove all entities and put them back from ha_entity to ha_word
  if (package->nb_entities >= DOgNlpMinEntityNumber) DONE;

  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  if ((retour = OgAutScanf(package->ha_entity, 0, "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int nb_words;
      unsigned char *entity = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&entity,&nb_words));
      int offset_nb_words = entity - out;

      int sep = -1;
      for (int i = offset_nb_words; i < iout; i++)
      {
        if (out[i] == '\1')
        {
          sep = i;
          break;
        }
      }
      if (sep < 0)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpEntityLog: error in ha_entity");
        DPcErr;
      }

      long expression_ptr;
      struct expression *expression;
      unsigned char *p = out + sep + 1;
      IFE(DOgPnin8(ctrl_nlp_th->herr,&p,&expression_ptr));
      expression = (struct expression *) expression_ptr;
      IFE(NlpConsolidateExpressionWord(ctrl_nlp_th, package, expression));
    }
    while ((retour = OgAutScann(package->ha_entity, &iout, out, nstate0, &nstate1, states)));
  }

  IFE(NlpEntityFlush(package));

  DONE;
}

og_status NlpEntityLog(og_nlp_th ctrl_nlp_th, package_t package)
{
  unsigned char out[DPcAutMaxBufferSize + 9];
  oindex states[DPcAutMaxBufferSize + 9];
  int retour, nstate0, nstate1, iout;

  OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "Entities for package '%s' '%s':", package->slug, package->id);

  if ((retour = OgAufScanf(package->ha_entity, 0, "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int nb_words;
      unsigned char *entity = out;
      IFE(DOgPnin4(ctrl_nlp_th->herr,&entity,&nb_words));
      int offset_nb_words = entity - out;

      int sep = -1;
      for (int i = offset_nb_words; i < iout; i++)
      {
        if (out[i] == '\1')
        {
          sep = i;
          break;
        }
      }
      if (sep < 0)
      {
        NlpThrowErrorTh(ctrl_nlp_th, "NlpEntityLog: error in ha_entity");
        DPcErr;
      }

      long expression_ptr;
      struct expression *expression;
      unsigned char *p = out + sep + 1;
      IFE(DOgPnin8(ctrl_nlp_th->herr,&p,&expression_ptr));
      expression = (struct expression *) expression_ptr;
      OgMsg(ctrl_nlp_th->hmsg, "", DOgMsgDestInLog, "  %.*s : '%s'", sep-offset_nb_words, entity, expression->text);
    }
    while ((retour = OgAufScann(package->ha_entity, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

