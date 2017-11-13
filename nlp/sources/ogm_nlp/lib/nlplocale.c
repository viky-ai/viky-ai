/*
 *  Handling locale which is the user langage preferences
 *  Copyright (c) 2017 Pertimm, by Patrick Constant
 *  Dev : November 2017
 *  Version 1.0
 */
#include "ogm_nlp.h"

static og_status NlpAddAcceptLanguage(og_nlp_th ctrl_nlp_th, og_string s, int is);
static og_status NlpAcceptLanguageGetQualityFactor(og_nlp_th ctrl_nlp_th, unsigned char *string_quality_factor,
    float *pquality_factor);

/*
 * handles string such as : "fr-FR, fr;q=0.9, en;q=0.8, de;q=0.7, *;q=0.5"
 * which is the value of the "Accept-Language" key
 */
og_status NlpInterpretRequestBuildAcceptLanguage(og_nlp_th ctrl_nlp_th, json_t *json_accept_language)
{
  og_string string_accept_language = NULL;

  // the Accept-Language request is not mandatory
  IFN(json_accept_language) DONE;

  if (json_is_string(json_accept_language))
  {
    string_accept_language = json_string_value(json_accept_language);
  }
  else
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpInterpretRequestBuildAcceptLanguage: json_accept_language is not a string");
    DPcErr;
  }

  og_string s = string_accept_language;
  int is = strlen(string_accept_language);

  for (int i = 0, end = 0, c = 0, start = 0; !end; i++)
  {
    if (i >= is)
    {
      end = 1;
      c = ',';
    }
    else
    {
      c = s[i];
    }
    if (c == ',')
    {
      IFE(NlpAddAcceptLanguage(ctrl_nlp_th, s + start, i-start));
      start = i + 1;
    }
  }

  if (ctrl_nlp_th->loginfo->trace & DOgNlpTraceInterpret)
  {
    unsigned char found_accept_language[DPcPathSize];
    NlpAcceptLanguageString(ctrl_nlp_th, DPcPathSize, found_accept_language);
    NlpLog(DOgNlpTraceInterpret, "NlpInterpretRequestBuildAcceptLanguage: found Accept-Language='%s'",
        found_accept_language)
  }

  DONE;
}

static og_status NlpAddAcceptLanguage(og_nlp_th ctrl_nlp_th, og_string s, int is)
{
  if (is >= DPcPathSize)
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpAddAcceptLanguage: string '%.*s' too long, length=%d", DPcPathSize, s, is);
    DPcErr;
  }

  // s has format: " fr-FR;q=0.9"
  int sep = (-1);
  for (int i = 0; i < is; i++)
  {
    if (s[i] == ';')
    {
      sep = i;
      break;
    }
  }
  unsigned char string_locale[DPcPathSize];
  unsigned char string_quality_factor[DPcPathSize];

  struct accept_language accept_language[1];

  if (sep < 0)
  {
    // no quality factor
    memcpy(string_locale, s, is);
    string_locale[is] = 0;
    OgTrimString(string_locale, string_locale);
    if (!strcmp(string_locale, "*"))
    {
      accept_language->locale = DOgLangNil;
    }
    else
    {
      IFE(accept_language->locale = OgCodeToIso639_3166(ctrl_nlp_th->herr, (char * )string_locale));
    }
    accept_language->quality_factor = 1;
  }
  else
  {
    memcpy(string_locale, s, sep);
    string_locale[sep] = 0;
    OgTrimString(string_locale, string_locale);
    if (!strcmp(string_locale, "*"))
    {
      accept_language->locale = DOgLangNil;
    }
    else
    {
      IFE(accept_language->locale = OgCodeToIso639_3166(ctrl_nlp_th->herr, (char * )string_locale));
    }
    memcpy(string_quality_factor, s + sep + 1, is - sep - 1);
    string_quality_factor[is - sep - 1] = 0;
    IFE(NlpAcceptLanguageGetQualityFactor(ctrl_nlp_th, string_quality_factor, &accept_language->quality_factor));
  }

  IFE(OgHeapAppend(ctrl_nlp_th->haccept_language, 1, accept_language));

  DONE;
}

static og_status NlpAcceptLanguageGetQualityFactor(og_nlp_th ctrl_nlp_th, unsigned char *string_quality_factor,
    float *pquality_factor)
{
  *pquality_factor = 0;
  unsigned char *s = string_quality_factor;
  int is = strlen(s);
  int sep = (-1);
  for (int i = 0; i < is; i++)
  {
    if (s[i] == '=')
    {
      sep = i;
      break;
    }
  }
  if (sep < 0)
  {
    NlpLog(DOgNlpTraceInterpret, "NlpAcceptLanguageGetQualityFactor: string '%s' is not a quality factor declaration",
        string_quality_factor);
    DPcErr;
  }
  unsigned char string_q[DPcPathSize];
  unsigned char string_float[DPcPathSize];

  memcpy(string_q, s, sep);
  string_q[sep] = 0;
  OgTrimString(string_q, string_q);
  if (strcmp(string_q, "q"))
  {
    NlpThrowErrorTh(ctrl_nlp_th, "NlpAcceptLanguageGetQualityFactor: string '%s' is not a quality factor declaration",
        string_quality_factor);
    DPcErr;
  }

  memcpy(string_float, s + sep + 1, is - sep - 1);
  string_float[is - sep - 1] = 0;
  *pquality_factor = atof(string_float);

  DONE;
}

int NlpAcceptLanguageString(og_nlp_th ctrl_nlp_th, int size, char *string)
{
  int accept_language_used = OgHeapGetCellsUsed(ctrl_nlp_th->haccept_language);
  struct accept_language *accept_language_all = OgHeapGetCell(ctrl_nlp_th->haccept_language, 0);
  IFN(accept_language_all) DPcErr;
  int length = 0;
  string[length] = 0;
  for (int i = 0; i < accept_language_used; i++)
  {
    struct accept_language *accept_language = accept_language_all + i ;

    unsigned char string_locale[DPcPathSize];
    OgIso639_3166ToCode(accept_language->locale, string_locale);

    length = strlen(string);
    snprintf(string + length, size - length, "%s%s;q=%.2f", (i ? ", " : ""), string_locale,
        accept_language->quality_factor);
  }
  DONE;

}

