/*
 *  Initialization for ogm_lip functions
 *  Copyright (c) 2006-2007 Pertimm by Patrick Constant
 *  Dev : October 2006, July 2007
 *  Version 1.1
*/
#include "ogm_lip.h"

PUBLIC(void *)
OgLipInit(struct og_lip_param *param)
{
  struct og_entity_param centity_param, *entity_param = &centity_param;
  struct og_msg_param cmsg_param, *msg_param = &cmsg_param;
  struct og_aut_param caut_param, *aut_param = &caut_param;
  struct og_ctrl_lip *ctrl_lip;
  char erreur[DOgErrorSize];
  int size;

  IFn(ctrl_lip = (struct og_ctrl_lip *) malloc(sizeof(struct og_ctrl_lip)))
  {
    sprintf(erreur, "OgLipInit: malloc error on ctrl_lip");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_lip, 0, sizeof(struct og_ctrl_lip));

  ctrl_lip->herr = param->herr;
  ctrl_lip->hmutex = param->hmutex;
  ctrl_lip->cloginfo = param->loginfo;
  ctrl_lip->loginfo = &ctrl_lip->cloginfo;

  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = param->herr;
  msg_param->hmutex = param->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = param->loginfo.where;
  msg_param->module_name = "ogm_lip";
  IFn(ctrl_lip->hmsg = OgMsgInit(msg_param))
  return (0);
  IF(OgMsgTuneInherit(ctrl_lip->hmsg, param->hmsg))
  return (0);

  ctrl_lip->PawoNumber = DOgPawoNumber;
  size = ctrl_lip->PawoNumber * sizeof(struct pawo);
  IFn(ctrl_lip->Pawo = (struct pawo *) malloc(size))
  {
    sprintf(erreur, "OgLipInit: malloc error on Pawo (%d bytes)", size);
    OgErr(ctrl_lip->herr, erreur);
    return (0);
  }

  memset(entity_param, 0, sizeof(struct og_entity_param));
  entity_param->herr = ctrl_lip->herr;
  entity_param->hmutex = ctrl_lip->hmutex;
  entity_param->loginfo.trace = DOgAutaTraceMinimal + DOgAutaTraceMemory;
  entity_param->loginfo.where = ctrl_lip->loginfo->where;
  IFn(ctrl_lip->hentity = OgEntityInit(entity_param))
  return (0);

  // if a lip_conf pointer is used outside in another library, it is specified. Otherwise, we use default lip conf
  if (param->conf)
  {
    ctrl_lip->conf = param->conf;
  }
  else
  {
    ctrl_lip->conf = &ctrl_lip->default_conf;
  }

  // If a punctuation file is specified, we get the conf from this file. Otherwise, we get default conf
  if (param->filename[0] && OgFileExists(param->filename))
  {

    if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
    {
      OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipInit: reading configuration file '%s'", param->filename);
    }

    // xsd validation
    og_status status = OgXmlXsdValidateFile(ctrl_lip->hmsg, ctrl_lip->herr, NULL,
        param->filename, "conf/xsd/punct.xsd");
    IF(status) return (0);

    // xml parse
    IF(OgLipReadPunctConf((void *) ctrl_lip, param->filename, ctrl_lip->conf))
    return (0);

  }
  else
  {
    // Init lip_conf default value
    OgLipInitConfWithDefault(ctrl_lip->hmsg, ctrl_lip->conf);
  }

  if (param->language_dictionary[0])
  {
    if (OgFileExists(param->language_dictionary))
    {
      memset(aut_param, 0, sizeof(struct og_aut_param));
      aut_param->herr = ctrl_lip->herr;
      aut_param->hmutex = ctrl_lip->hmutex;
      aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
      aut_param->loginfo.where = ctrl_lip->loginfo->where;
      aut_param->state_number = 0;
      sprintf(aut_param->name, "lip languages");
      IFn(ctrl_lip->ha_lang = OgAutInit(aut_param))
      return (0);
      IF(OgAufRead(ctrl_lip->ha_lang, param->language_dictionary))
      return (0);
    }
    else
    {
      if (ctrl_lip->loginfo->trace & DOgLipTraceMinimal)
      {
        OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog + DOgMsgSeverityWarning,
            "OgLipInit: language file '%s' does not exist, language recognition disabled", param->language_dictionary);
      }
    }
  }

  return ((void *) ctrl_lip);
}






PUBLIC(int) OgLipFlush(void *handle)
{
struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *)handle;

IFE(OgEntityFlush(ctrl_lip->hentity));
IFE(OgAutFlush(ctrl_lip->ha_lang));
IFE(OgMsgFlush(ctrl_lip->hmsg));

DPcFree(ctrl_lip->Pawo);
DPcFree(ctrl_lip);
DONE;
}





PUBLIC(int) OgLipResize(void *handle,size_t max_size)
{
struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *)handle;
size_t size,new_size,max_number;
char erreur[DOgErrorSize];

ctrl_lip->PawoUsed = 0;
size = ctrl_lip->PawoNumber*sizeof(struct pawo);
if (size > max_size) {
  DPcFree(ctrl_lip->Pawo);
  max_number = max_size/sizeof(struct pawo);
  ctrl_lip->PawoNumber = max_number;
  new_size = ctrl_lip->PawoNumber*sizeof(struct pawo);
  IFn(ctrl_lip->Pawo=(struct pawo *)malloc(new_size)) {
    sprintf(erreur,"OgLipResize: malloc error on Pawo (%lu bytes)",size);
    OgErr(ctrl_lip->herr,erreur); DPcErr;
    }
  if (ctrl_lip->loginfo->trace & DOgLipTraceMemory) {
    OgMsg(ctrl_lip->hmsg,"",DOgMsgDestInLog
      , "OgLipResize: reducing Pawo size from %ld to %ld bytes"
      , size,new_size);
    }
  }
DONE;
}


/**
 * Initialize lip_conf with default values
 *
 * @param hmsg handle for log
 * @param conf lip conf pointer
 * @return function status
 */
PUBLIC(int) OgLipInitConfWithDefault(void *hmsg, struct og_lip_conf *conf)
{
  // reset lip_conf
  memset(conf, 0, sizeof(struct og_lip_conf));

  // /!\ WARNNING /!\ It must be in sync with :
  // ogmios/products/ebusiness-default/instances/default/conf/punctuation.xml

  // Init default punctuation
  IFE(OgLipConfAddPunctuation(hmsg, conf, '-', FALSE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '_', FALSE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0x2013, FALSE));

  IFE(OgLipConfAddPunctuation(hmsg, conf, '\'', FALSE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0x2019, FALSE));

  IFE(OgLipConfAddPunctuation(hmsg, conf, ',', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, ';', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, ':', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '!', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0xA1, TRUE)); // INVERTED EXCLAMATION MARK
  IFE(OgLipConfAddPunctuation(hmsg, conf, '?', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0xBF, TRUE)); // INVERTED QUESTION MARK
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0x2026, TRUE));

  IFE(OgLipConfAddPunctuation(hmsg, conf, '|', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '{', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '}', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '(', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, ')', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '[', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, ']', TRUE));

  IFE(OgLipConfAddPunctuation(hmsg, conf, '/', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '\\', TRUE));

  IFE(OgLipConfAddPunctuation(hmsg, conf, '<', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '>', TRUE));
  IFE(OgLipConfAddPunctuation(hmsg, conf, '=', TRUE));

  IFE(OgLipConfAddPunctuation(hmsg, conf, 0x060C, TRUE)); // ARABIC COMMA
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0x061B, TRUE)); // ARABIC SEMICOLON
  IFE(OgLipConfAddPunctuation(hmsg, conf, 0x061F, TRUE)); // ARABIC QUESTION MARK

  // Init default indivisible punctuation
  IFE(OgLipConfAddPunctuationIndivisible(hmsg, conf, '"', '"'));
  IFE(OgLipConfAddPunctuationIndivisible(hmsg, conf, 0x201C, 0x201D));
  IFE(OgLipConfAddPunctuationIndivisible(hmsg, conf, 0x00AB, 0x00BB));

  // Init default punctuation word
  IFE(OgLipConfAddPunctuationWord(hmsg, conf, "&"));

  DONE;
}

