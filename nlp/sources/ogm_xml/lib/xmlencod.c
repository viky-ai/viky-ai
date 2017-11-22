/*
 *  Xml encoding an XML content.
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev: July 2006
 *  Version 1.0
 */
#include "ogm_xml.h"

static int OgXmlEncodeAscii(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout,
    int *truncated, int is_attribute);
static int OgXmlEncodeUnicode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout,
    int *truncated, int is_attribute);

/*
 *  Making a content XML-compatible. Codepage is now used to specify
 *  DOgCodePageUnicode when the buffer is Unicode. Please note
 *  that most encoding use Ascii encoding as base encoding, and
 *  since we handle only < > and &, specifying DOgCodePageUTF8
 *  will work for almost any codepage.
 */

PUBLIC(int) OgXmlEncode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout, int codepage,
    int *truncated)
{
  if (codepage == DOgCodePageUnicode)
  {
    IFE(OgXmlEncodeUnicode(ibin, bin, sbout, ibout, bout, truncated,FALSE));
  }
  else
  {
    IFE(OgXmlEncodeAscii(ibin, bin, sbout, ibout, bout, truncated,FALSE));
  }
  DONE;
}

PUBLIC(int) OgXmlAttributeEncode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout,
    int codepage, int *truncated)
{
  if (codepage == DOgCodePageUnicode)
  {
    IFE(OgXmlEncodeUnicode(ibin, bin, sbout, ibout, bout, truncated,TRUE));
  }
  else
  {
    IFE(OgXmlEncodeAscii(ibin, bin, sbout, ibout, bout, truncated,TRUE));
  }
  DONE;
}

static int OgXmlEncodeAscii(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout,
    int *truncated, int is_attribute)
{
  int i, j, c, ientity;
  unsigned char *entity;

  if (truncated) *truncated = 0;

  for (i = j = 0; i < ibin; i++)
  {
    c = bin[i];
    entity = 0;
    switch (c)
    {
      case '<':
        entity = "&lt;";
        ientity = 4;
        break;
      case '>':
        entity = "&gt;";
        ientity = 4;
        break;
      case '&':
        entity = "&amp;";
        ientity = 5;
        break;
      case '"':
        if(is_attribute)
        {
          entity = "&quot;";
          ientity = 6;
        }
        break;
      case '\'':
        if(is_attribute)
        {
          entity = "&apos;";
          ientity = 6;
        }
        break;
    }
    IFx(entity)
    {
      if (j + ientity >= sbout)
      {
        if (truncated) *truncated = 1;
        break;
      }
      memcpy(bout + j, entity, ientity);
      j += ientity;
    }
    else
    {
      if (j + 1 >= sbout)
      {
        if (truncated) *truncated = 1;
        break;
      }
      bout[j++] = c;
    }
  }
  *ibout = j;
  bout[j++] = 0;
  DONE;
}

static int OgXmlEncodeUnicode(int ibin, unsigned char const *bin, int sbout, int *ibout, unsigned char *bout,
    int *truncated, int is_attribute)
{
  int i, j, c, ientity;
  unsigned char *entity;

  if (truncated) *truncated = 0;

  for (i = j = 0; i < ibin; i += 2)
  {
    c = (bin[i] << 8) + bin[i + 1];
    entity = 0;
    switch (c)
    {
      case '<':
        entity = "\0&\0l\0t\0;";
        ientity = 8;
        break;
      case '>':
        entity = "\0&\0g\0t\0;";
        ientity = 8;
        break;
      case '&':
        entity = "\0&\0a\0m\0p\0;";
        ientity = 10;
        break;
      case '"':
        if(is_attribute)
        {
          entity = "\0&\0q\0u\0o\0t\0;";
          ientity = 12;
        }
        break;
      case '\'':
        if(is_attribute)
        {
          entity = "\0&\0a\0p\0o\0s\0;";
          ientity = 12;
        }
        break;
    }
    IFx(entity)
    {
      if (j + ientity > sbout)
      {
        if (truncated) *truncated = 1;
        break;
      }
      memcpy(bout + j, entity, ientity);
      j += ientity;
    }
    else
    {
      if (j + 2 > sbout)
      {
        if (truncated) *truncated = 1;
        break;
      }
      bout[j++] = bin[i];
      bout[j++] = bin[i + 1];
    }
  }
  *ibout = j;
  DONE;
}

