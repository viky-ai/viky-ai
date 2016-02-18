/*
 *  Read punctuatio file
 *  Copyright (c) 2008 Pertimm by Lois Rigouste
 *  Dev : October 2008
 *  Version 1.0
*/
#include "ogm_lip.h"


#define DOgLipReadPunctConfTagNil                      0
#define DOgLipReadPunctConfTag_lip_conf                1
#define DOgLipReadPunctConfTag_punctuation             2
#define DOgLipReadPunctConfTag_punctuation_word        3
#define DOgLipReadPunctConfTag_indivisible             4
#define DOgLipReadPunctConfTag_char                    5
#define DOgLipReadPunctConfTag_code                    6
#define DOgLipReadPunctConfTag_word                    7

#define DOgMaxXmlPath                            10

/**
 * List iterator types.
 *
 * @ingroup list
 */
typedef enum
{
  DOgLipSepFalse = 0, DOgLipSepTrue = 1, DOgLipSepOpen = 2, DOgLipSepClose = 3
} og_lip_sep_attribute;

static int OgLipReadPunctConf1(void *, int, int, unsigned char *);
static int OgLipConfReadSepAttribute(void *ptr, struct og_read_tag *rt);


struct og_lip_xml_info {
  struct og_ctrl_lip *ctrl_lip;
  int iB; unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];
  og_lip_sep_attribute last_sep_attribute_value;
  int indivisible_code_open;
  int indivisible_code_close;
  struct og_lip_conf *conf;
  };



struct og_conf_xml_info {
  struct og_ctrl_lip *ctrl_lip;
  int iB; unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];
  };

struct og_tree_xml_tag {
  int value, iname; char *name;
  };


struct og_tree_xml_tag LipXmlReadTag[] =  {
  { DOgLipReadPunctConfTag_lip_conf,          8, "lip_conf" },
  { DOgLipReadPunctConfTag_punctuation,      11, "punctuation" },
  { DOgLipReadPunctConfTag_punctuation_word, 16, "punctuation_word" },
  { DOgLipReadPunctConfTag_indivisible,      11, "indivisible" },
  { DOgLipReadPunctConfTag_char,              4, "char" },
  { DOgLipReadPunctConfTag_code,              4, "code" },
  { DOgLipReadPunctConfTag_word,              4, "word" },
  { 0, 0, "" }
  };


/**
* Validate conf again XSD.
*
* @param handle LIP handle
* @param xsd_dir xsd_dir
* @param xml_file xml_file
* @return function status
*
**/
PUBLIC(int) OgLipXsdValidatePunctFile(void *handle, unsigned char *xml_file, unsigned char *working_dir)
{
  struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *) handle;
  unsigned char xsd_file[DPcPathSize];

  if (working_dir != NULL && working_dir[0])
  {
    sprintf(xsd_file, "%s/conf/xsd/punct.xsd", working_dir);
  }
  else
  {
    sprintf(xsd_file, "conf/xsd/punct.xsd");
  }

  if (!OgFileExists(xml_file))
  {
    // no such file do nothing
  }
  else if (OgFileExists(xsd_file))
  {
    int good_format;
    IFE(good_format = OgXmlCheckXsd(ctrl_lip->hmsg, xsd_file, xml_file));
    if (!good_format)
    {
      unsigned char erreur[DPcPathSize];
      sprintf(erreur, "OgLipValidatePunctFile: file '%s' is not validated by '%s'", xml_file, xsd_file);
      OgErr(ctrl_lip->herr, erreur);
      DPcErr;
    }
  }
  else
  {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog,
        "OgLipValidatePunctFile: warning: file '%s' does not exist, could not check '%s'", xsd_file, xml_file);
  }

  DONE;
}

/**
* Reading XML configuration file (conf/lip_conf.xml). Can be a file or a buffer.
*
* @param handle LIP handle
* @param filename configuration file
* @param conf og_lip_conf
* @return function success
*
**/
PUBLIC(int) OgLipReadPunctConf(void *handle, char *filename, struct og_lip_conf *conf)
{
  struct og_ctrl_lip *ctrl_lip = (struct og_ctrl_lip *) handle;
  struct og_getcp_param ccpparam, *cpparam = &ccpparam;
  int iB, iBtemp, codepage;
  unsigned char *B, *Btemp;
  struct og_lip_xml_info cinfo, *info = &cinfo;
  struct stat filestat;
  void *hgetcp;
  FILE *fd;

  if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
  {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf: Start scanning '%s'", filename);
  }
  IFn(fd=fopen(filename,"rb"))
  {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf: No file or impossible to fopen '%s'", filename);
    DONE;
  }
  IF(fstat(fileno(fd),&filestat))
  {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf: impossible to fstat '%s'", filename);
    DONE;
  }
  IFn( (B = (unsigned char *) malloc(filestat.st_size + 9)) )
  {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf: impossible to allocate %d bytes for '%s'",
        filestat.st_size, filename);
    fclose(fd);
    DONE;
  }
  IFn( (iB = fread(B, 1, filestat.st_size, fd)) )
  {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf: impossible to fread '%s'", filename);
    DPcFree(B);
    fclose(fd);
    DONE;
  }
  fclose(fd);

  /* we get the codepage */
  memset(cpparam, 0, sizeof(struct og_getcp_param));
  cpparam->herr = ctrl_lip->herr;
  cpparam->hmutex = ctrl_lip->hmutex;
  cpparam->loginfo.where = ctrl_lip->loginfo->where;
  cpparam->loginfo.trace = DOgGetcpTraceMinimal + DOgGetcpTraceMemory;
  IFn(hgetcp = OgGetCpInit(cpparam)) DPcErr;
  IFE(OgGetCp(hgetcp, iB, B, &codepage));
  IFE(OgGetCpFlush(hgetcp));
  IFn(codepage) DONE;

  if (codepage != DOgCharset_UTF_8)
  {
    /* we convert to a UTF-8 buffer */
    IFn(Btemp=(unsigned char *)malloc(2*iB))
    {
      OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog,
          "OgLipReadPunctConf: impossible to allocate %d bytes for utf8 conversion", 2 * iB);
      DONE;
    }
    IFE(OgCpToUni(iB,B,2*iB,&iBtemp,Btemp,OgNewCpToOld(codepage),0,0));
    DPcFree(B);
    IFn(B=(unsigned char *)malloc(2*iB))
    {
      OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog,
          "OgLipReadPunctConf: impossible to allocate %d bytes for Lip buffering", 2 * iB);
      DONE;
    }
    IFE(OgUniToCp(iBtemp,Btemp,2*iB,&iB,B,OgNewCpToOld(DOgCharset_UTF_8),0,0));
    DPcFree(Btemp);
  }

  memset(info, 0, sizeof(struct og_lip_xml_info));
  info->ctrl_lip = ctrl_lip;
  info->iB = iB;
  info->B = B;
  info->conf = conf;
  info->last_sep_attribute_value = DOgLipSepFalse;
  info->indivisible_code_open = -1;
  info->indivisible_code_close = -1;

  IFE(OgParseXml(ctrl_lip->herr, 0, iB, B, OgLipReadPunctConf1, (void *)info));

  conf->conf_in_file = TRUE;

  if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
  {
    OgLipConfLog(ctrl_lip->hmsg, "OgLipReadPunctConf", conf);
  }

  DPcFree(B);
  DONE;
}




PUBLIC(int) OgLipConfAddPunctuationWord(void *hmsg, struct og_lip_conf *conf, unsigned char *word)
{
  int const max_length = DOgLipConfMaxPunctuationWordSize;
  struct og_lip_conf_word *punc_word = NULL;
  int word_length = strlen(word);

  // check overflow
  if (conf->punctuation_word_length >= max_length)
  {
    OgMsg(hmsg, "", DOgMsgDestInLog, "OgLipConfAddPunctuationWord: On '%s' : Maximum number punctuation word"
        " reach. Cannot be more than %d.", word, max_length);
    DPcErr;
  }
  if (word_length >= (max_length / 2))
  {
    OgMsg(hmsg, "", DOgMsgDestInLog, "OgLipConfAddPunctuationWord: On '%s' :Maximum punctuation word length"
        " reach. Cannot be more than %d", word, max_length / 2);
    DPcErr;
  }

  punc_word = conf->punctuation_word + conf->punctuation_word_length;

  IFE(OgCpToUni(word_length, word, max_length, &punc_word->length, punc_word->string, DOgCharset_UTF_8, 0, 0));
  conf->punctuation_word_length++;

  DONE;
}

PUBLIC(int) OgLipConfAddPunctuation(void *hmsg, struct og_lip_conf *conf, int unicode_char, int is_separator)
{
  // check overflow
  if (conf->punctuation_length >= DOgLipConfMaxPunctuationSize)
  {
    OgMsg(hmsg, "", DOgMsgDestInLog,
        "OgLipConfAddPunctuation: On (%4d, 0x%04x) : Maximum number of punctuation"
            " reach. Cannot be more than %d.", unicode_char, unicode_char, DOgLipConfMaxIndivisibleSize);
    DPcErr;
  }

  if (is_separator && conf->separator_length >= DOgLipConfMaxPunctuationSize)
  {
    OgMsg(hmsg, "", DOgMsgDestInLog,
        "OgLipConfAddPunctuation: On (%4d, 0x%04x) : Maximum number of punctuation separator"
            " reach. Cannot be more than %d.", unicode_char, unicode_char, DOgLipConfMaxPunctuationSize);
    DPcErr;
  }

  if (!OgLipIsPunctuation(conf, unicode_char))
  {
    conf->punctuation[conf->punctuation_length++] = unicode_char;
  }
  if (is_separator && !OgLipIsSingleSeparator(conf, unicode_char))
  {
    conf->separator[conf->separator_length++] = unicode_char;
  }

  DONE;
}

PUBLIC(int) OgLipConfAddPunctuationIndivisible(void *hmsg, struct og_lip_conf *conf, int unicode_char_open,
    int unicode_char_close)
{
  // check overflow
  if (conf->indivisible_length >= DOgLipConfMaxIndivisibleSize)
  {
    OgMsg(hmsg, "", DOgMsgDestInLog,
        "OgLipConfAddPunctuationIndivisible: On (%4d, 0x%04x)/(%4d, 0x%04x) : Maximum number of indivisible punctuation"
            " reach. Cannot be more than %d.", unicode_char_open, unicode_char_open, unicode_char_close,
        unicode_char_close, DOgLipConfMaxIndivisibleSize);
    DPcErr;
  }

  // add open char to punctuation
  IFE(OgLipConfAddPunctuation(hmsg, conf, unicode_char_open, FALSE));

  // add close char to punctuation
  IFE(OgLipConfAddPunctuation(hmsg, conf, unicode_char_close, FALSE));

  conf->indivisible[conf->indivisible_length].open = unicode_char_open;
  conf->indivisible[conf->indivisible_length].close = unicode_char_close;

  conf->indivisible_length++;

  DONE;
}



/**
 * Log lip_conf punct unitary
 *
 * @param hmsg msg handle
 * @param message message header in log file
 * @param type punct type
 * @param c unicode punct char
 **/
static void OgLipConfLog1(void *hmsg, const unsigned char *message, const unsigned char *type, int c)
{
  int ibuffer = 0;
  unsigned char buffer[8];

  memset(buffer, 0, sizeof(unsigned char) * 8);

  OgUnisToUTF8(1, (unsigned short *) &c, 8, &ibuffer, buffer, NULL, NULL);

  OgMsg(hmsg, "", DOgMsgDestInLog, "%s: OgLipConfLog:  '%s' (%4d, 0x%04x, type='%s')", message, buffer, c, c, type);
}


static void OgLipConfLog2(void *hmsg, const unsigned char *message, const unsigned char *type, struct og_lip_conf_word *word)
{
  int iutf8; unsigned char utf8[DPcPathSize];
  OgUniToCp(word->length,word->string, DPcPathSize, &iutf8, utf8, DOgCodePageUTF8, 0, 0);
  OgMsg(hmsg, "", DOgMsgDestInLog, "%s: OgLipConfLog:  '%s' (type='%s')", message, utf8, type);
}




/**
 * Log lip_conf punct.
 *
 * @param hmsg msg handle
 * @param message message header in log file
 * @param conf lip_conf handle
 * @return function success
 **/
PUBLIC(int) OgLipConfLog(void *hmsg, const unsigned char *message, struct og_lip_conf *conf)
{
  int i;

  OgMsg(hmsg, "", DOgMsgDestInLog, "%s: OgLipConfLog: Lip conf in use %s: ", message,
      conf->conf_in_file ? "from conf file " : "");
  for (i = 0; i < conf->punctuation_length; i++)
  {
    unsigned char *type = "punctuation";
    if(OgLipIsSeparator(conf, conf->punctuation[i]))
    {
      type = "punctuation,separator";
    }

    OgLipConfLog1(hmsg, message, type, conf->punctuation[i]);
  }

  for (i = 0; i < conf->punctuation_word_length; i++)
  {
    OgLipConfLog2(hmsg, message, "punctuation_word", conf->punctuation_word+i);
  }

  for (i = 0; i < conf->indivisible_length; i++)
  {
    int ibuffer_open = 0;
    unsigned char buffer_open[8];
    int ibuffer_close = 0;
    unsigned char buffer_close[8];

    int open_char = conf->indivisible[i].open;
    int close_char = conf->indivisible[i].close;

    memset(buffer_open, 0, sizeof(unsigned char) * 8);
    memset(buffer_close, 0, sizeof(unsigned char) * 8);

    OgUnisToUTF8(1, (unsigned short *) &open_char, 8, &ibuffer_open, buffer_open, NULL, NULL );
    OgUnisToUTF8(1, (unsigned short *) &close_char, 8, &ibuffer_close, buffer_close, NULL, NULL );

    OgMsg(hmsg, "", DOgMsgDestInLog, "%s: OgLipConfLog:   indivisible separator [open : '%s', %4d, 0x%04x ;"
        " close : '%s', %4d, 0x%04x]", message, buffer_open, open_char, open_char, buffer_close, close_char, close_char);

  }

  DONE;
}


static int OgLipReadPunctConf1(void *ptr, int type, int ib, unsigned char *b)
{
  int suni = DPcPathSize * 2, iuni;
  unsigned char uni[DPcPathSize * 2];
  struct og_lip_xml_info *info = (struct og_lip_xml_info *) ptr;
  int i = 0, value = 0, valuep = 0, valuepp = 0, iname_tag, ibuffer, found, code;
  struct og_ctrl_lip *ctrl_lip = info->ctrl_lip;
  char buffer[DPcPathSize * 2];
  char *after;

  if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
  {
    buffer[0] = 0;
    for (i = 0; i < info->ixml_path; i++)
    {
      sprintf(buffer + strlen(buffer), "%d ", info->xml_path[i]);
    }
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf1: xml_path (length %d): %s type = %d [[%.*s]]",
        info->ixml_path, buffer, type, ib, b);
  }

  if (type == DOgParseXmlTag)
  {
    value = DOgLipReadPunctConfTagNil;
    for (i = 0; i < ib; i++)
      b[i] = tolower(b[i]);
    if (b[ib - 1] == '/')
    {
      info->auto_tag = 1;
      info->end_tag = 0;
    }
    else
    {
      info->auto_tag = 0;
      if (b[0] == '/') info->end_tag = 1;
      else info->end_tag = 0;
    }
    for (i = 0; i < ib; i++)
    {
      if (OgUniIsspace(b[i])) break;
    }
    iname_tag = i;
    for (i = 0; LipXmlReadTag[i].value; i++)
    {
      if (LipXmlReadTag[i].iname == iname_tag - info->end_tag
          && !memcmp(LipXmlReadTag[i].name, b + info->end_tag, iname_tag - info->end_tag))
      {
        value = LipXmlReadTag[i].value;
        break;
      }
    }
    if (value != DOgLipReadPunctConfTagNil)
    {
      if (!info->end_tag)
      {
        if (info->ixml_path < DOgMaxXmlPath - 1) info->xml_path[info->ixml_path++] = value;
      }
      if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
      {
        OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf1: Recognized tag_value=%d", value);
      }
      if (info->auto_tag || info->end_tag)
      {
        if (info->ixml_path > 0) info->ixml_path--;
      }
    }

    // reset when opening a tag
    if (!info->end_tag)
    {
      switch (value)
      {
        case DOgLipReadPunctConfTag_punctuation:
          info->conf->punctuation_length = 0;
          memset(info->conf->punctuation, 0, sizeof(int) * DOgLipConfMaxPunctuationSize);

          info->conf->separator_length = 0;
          memset(info->conf->separator, 0, sizeof(int) * DOgLipConfMaxPunctuationSize);

          info->conf->indivisible_length = 0;
          memset(info->conf->indivisible, 0, sizeof(struct og_lip_conf_separator) * DOgLipConfMaxIndivisibleSize);

        break;
        case DOgLipReadPunctConfTag_punctuation_word:
          info->conf->punctuation_word_length = 0;
          memset(info->conf->punctuation_word, 0, sizeof(int) * DOgLipConfMaxPunctuationWordSize);
        break;

        case DOgLipReadPunctConfTag_char:
        case DOgLipReadPunctConfTag_code:
          // read attribute
          IFE(OgReadTag(ib, b, 0, OgLipConfReadSepAttribute, &info->last_sep_attribute_value));
        break;

        case DOgLipReadPunctConfTag_indivisible:
          info->indivisible_code_open = -1;
          info->indivisible_code_open = -1;
          break;

        default:
        break;
      }
    }
    else
    {
      // on closing tag : indivisible prepare for next one
      if(value == DOgLipReadPunctConfTag_indivisible)
      {
        if (info->indivisible_code_open > 0 && info->indivisible_code_close > 0)
        {
          // add punctuation
          IFE(OgLipConfAddPunctuationIndivisible(ctrl_lip->hmsg, info->conf, info->indivisible_code_open,
                  info->indivisible_code_close));
        }

        // reset
        info->indivisible_code_open = -1;
        info->indivisible_code_close = -1;
      }
    }

  }
  else if (type == DOgParseXmlContent)
  {
    if (info->ixml_path < 2) return (0);
    value = info->xml_path[info->ixml_path - 1];
    valuep = info->xml_path[info->ixml_path - 2];
    if(info->ixml_path >= 3) valuepp  = info->xml_path[info->ixml_path - 3];
    ibuffer = ib;
    if (ibuffer > DPcPathSize - 1) ibuffer = DPcPathSize - 1;
    memcpy(buffer, b, ibuffer);
    buffer[ibuffer] = 0;
    OgTrimString(buffer, buffer);
    ibuffer = strlen(buffer);

    if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
    {
      OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog,
          "OgLipReadPunctConf1: Analyzing content %s ixml_path %d valuep %d value %d", buffer, info->ixml_path, valuep,
          value);
    }

    if (info->ixml_path >= 2 && (value == DOgLipReadPunctConfTag_char || value == DOgLipReadPunctConfTag_code || value == DOgLipReadPunctConfTag_word))
    {
      if ((valuep == DOgLipReadPunctConfTag_punctuation && valuepp == DOgLipReadPunctConfTag_lip_conf) ||
          (valuep == DOgLipReadPunctConfTag_punctuation_word && valuepp == DOgLipReadPunctConfTag_lip_conf) ||
          (valuep == DOgLipReadPunctConfTag_indivisible && valuepp == DOgLipReadPunctConfTag_punctuation))
      {
       // continue
      }
      else
      {
        // unknow tag
        DONE;
      }

      if (ibuffer <= 0)
      {
        OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf1: empty buffer");
        return (0);
      }
      else
      {
        if (value == DOgLipReadPunctConfTag_char)
        {
          found = 0;
          if (ibuffer > 2 && buffer[0] == '&' && buffer[ibuffer - 1] == ';')
          {
            buffer[--ibuffer] = 0;
            IFE(found = OgEntityGetCode(ctrl_lip->hentity, buffer + 1, &code));
          }
          if (!found)
          {
            IFE(OgCpToUni(ibuffer, buffer, suni, &iuni, uni, DOgCharset_UTF_8, 0, 0));
            if (iuni == 2)
            {
              code = (uni[0] << 8) + uni[1];
              found = 1;
            }
          }
        }
        else if (value == DOgLipReadPunctConfTag_code)
        {
          code = strtol(buffer, &after, 0);
          found = 1;
        }
        else if (value == DOgLipReadPunctConfTag_word)
        {
          found = 1;
        }
        else
        {
          found = 0;
        }

        if (found)
        {
          if (valuep == DOgLipReadPunctConfTag_punctuation)
          {
            // add punctuation
            IFE(OgLipConfAddPunctuation(ctrl_lip->hmsg, info->conf, code, info->last_sep_attribute_value));

            // reset
            info->last_sep_attribute_value = DOgLipSepFalse;
          }
          else if (valuep == DOgLipReadPunctConfTag_punctuation_word)
          {
            int idecoded; unsigned char decoded[DPcPathSize*2];
            IFE(OgXmlDecode(strlen(buffer), buffer, DPcPathSize, &idecoded, decoded, DOgCodePageUTF8, 0, 0));
            IFE(OgLipConfAddPunctuationWord(ctrl_lip->hmsg, info->conf, decoded));
          }
          else if (valuep == DOgLipReadPunctConfTag_indivisible && valuepp == DOgLipReadPunctConfTag_punctuation)
          {
            if (info->last_sep_attribute_value == DOgLipSepOpen)
            {
              info->indivisible_code_open = code;
            }
            else if (info->last_sep_attribute_value == DOgLipSepClose)
            {
              info->indivisible_code_close = code;
            }
            else
            {
              OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf1: invalid sep attribute.");
            }

            // reset
            info->last_sep_attribute_value = DOgLipSepFalse;
          }

          if (ctrl_lip->loginfo->trace & DOgLipTraceXMLScan)
          {
            OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog, "OgLipReadPunctConf1: lip_conf char  %d (hex 0x%x) type=%d",
                code, code, valuep);
          }
        }
      }
    }
  }
  return (0);
}


/**
 * Read Sep attribute
 *
 * @param ptr ctx
 * @param rt og_read_tag
 * @return status
 */
static int OgLipConfReadSepAttribute(void *ptr, struct og_read_tag *rt)
{
  og_lip_sep_attribute *result = (og_lip_sep_attribute *) ptr;

  if (!Ogstricmp("sep", rt->attr))
  {
    PcStrlwr(rt->value);
    if (!Ogstricmp(rt->value, "true"))
    {
      *result = DOgLipSepTrue;
    }
    else if (!Ogstricmp(rt->value, "false"))
    {
      *result = DOgLipSepFalse;
    }
    else if (!Ogstricmp(rt->value, "open"))
    {
      *result = DOgLipSepOpen;
    }
    else if (!Ogstricmp(rt->value, "close"))
    {
      *result = DOgLipSepClose;
    }
  }

  DONE;
}


