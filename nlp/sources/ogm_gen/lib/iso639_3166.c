/*
 *  Generic functions for language-country, based on ISO 639 and ISO 3166
 *  Copyright (c) 2014 Pertimm by Patrick Constant
 *  Dev : September 2014
 *  Version 1.0
 */
#include <loggen.h>
#include <logis639.h>
#include <logis3166.h>
#include <logis639_3166.h>

static int validateLangCountry(unsigned char *slang_country);

PUBLIC(unsigned char *) OgIso639_3166ToCode(int lang_country, unsigned char *slang_country)
{
  int lang,country;
  lang=lang_country%DOgLangMax;
  country=lang_country/DOgLangMax;
  if (country==DOgCountryNil)
  {
    sprintf(slang_country,"%s",OgIso639ToCode(lang));
  }
  else
  {
    sprintf(slang_country,"%s-%s",OgIso639ToCode(lang),OgIso3166ToCode(country));
  }
  return(slang_country);
}

PUBLIC(int) OgCodeToIso639_3166(void *herr, unsigned char *slang_country)
{
  int i, sep = (-1), lang = DOgLangNil, country = DOgCountryNil;
  char slang[3], scountry[3];

  IFN(validateLangCountry(slang_country))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgCodeToIso639_3166: Language_country code '%s' is not well formed", slang_country);
    OgErr(herr, erreur);
    DPcErr;
  }

  for (i = 0; slang_country[i]; i++)
  {
    if (slang_country[i] == '-')
    {
      sep = i;
      break;
    }
  }
  memcpy(slang, slang_country, 2);
  slang[2] = 0;
  PcStrlwr(slang);
  lang = OgCodeToIso639(slang);
  if (lang == 0 && strcmp(slang, "--") != 0)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgCodeToIso639_3166: Language '%s' does not exist", slang);
    OgErr(herr, erreur);
    DPcErr;
  }
  if (sep >= 0)
  {
    memcpy(scountry, slang_country + sep + 1, 2);
    scountry[2] = 0;
    PcStrupr(scountry);
    country = OgCodeToIso3166(scountry);
    if (country == 0 && strcmp(slang, "--") != 0)
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "OgCodeToIso639_3166: Country '%s' does not exist", scountry);
      OgErr(herr, erreur);
      DPcErr;
    }
  }
  return (lang + DOgLangMax * country);
}


static int validateLangCountry(unsigned char *slang_country)
{
  //Must be xx or -- or xx-yy or xx-YY or xx-yY or xx-Yy
  if (strlen(slang_country) != 2 && strlen(slang_country) != 5)
  {
    return 0;
  }

  int i;
  for (i = 0; slang_country[i]; i++)
  {
    if (i == 2 && slang_country[i] != '-')
    {
      return 0;
    }
    else if (i < 2)
    {
      //Not -- but (x- or -x)
      if (i > 0 && ((slang_country[i - 1] != '-' && slang_country[i] == '-')
          || (slang_country[i - 1] == '-' && slang_country[i] != '-')))
      {
        return 0;
      }
      // Not lowercase alphabetic letter or not -
      if (slang_country[i] != '-' && (slang_country[i] < 97 || slang_country[i] > 122))
      {
        return 0;
      }
    }
    else if (i > 2)
    {
      //Not -- but (x- or -x)
      if (i > 3 && ((slang_country[i - 1] != '-' && slang_country[i] == '-')
          || (slang_country[i - 1] == '-' && slang_country[i] != '-')))
      {
        return 0;
      }
      // Not uppercase or lowercase alphabetic letter or not -
      if (slang_country[i] != '-' && (slang_country[i] < 65 || (slang_country[i] > 90 && slang_country[i] < 97)
          || slang_country[i] > 122))
      {
        return 0;
      }
    }
  }
  return 1;
}

PUBLIC(int) OgIso639_3166ToIso639_3166(int lang, int country)
{
return (lang + DOgLangMax * country);
}

PUBLIC(int) OgIso639_3166ToLang(int lang_country)
{
return (lang_country%DOgLangMax);
}

PUBLIC(int) OgIso639_3166ToCountry(int lang_country)
{
return (lang_country/DOgLangMax);
}

