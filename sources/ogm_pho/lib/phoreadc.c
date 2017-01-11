/*
 *  Reading conf program for Phonet function
 *  Copyright (c) 2006 Pertimm by P.Constant,G.Logerot and L.Rigouste
 *  Dev : November 2006
 *  Version 1.0
*/
#include "ogm_pho.h"




#define DOgPhoReadConfTagNil                               0
#define DOgPhoReadConfTag_phonet_configuration             1
#define DOgPhoReadConfTag_rules                            2
#define DOgPhoReadConfTag_rule                             3
#define DOgPhoReadConfTag_classes                          4
#define DOgPhoReadConfTag_class                            5
#define DOgPhoReadConfTag_c                                6
#define DOgPhoReadConfTag_option                           7

#define DOgPhoReadConfOptionNil                  0
#define DOgPhoReadConfOptionAppendingCharacter   1
#define DOgPhoReadConfOptionNonAlphaToSpace      2
#define DOgPhoReadConfOptionSpaceCharacter       3
#define DOgPhoReadConfKeepDigit                  4


static int PhoReadConf1(void *ptr, int type, int ib, unsigned char *b);
STATICF(int) PhoReadConf1TagAttribute(pr_(void *) pr(struct og_read_tag *));
STATICF(int) PhoReadConf1RuleReset(pr(struct og_xml_info *));

static int PhoReadConf(struct lang_context *lang_context, char *conf_file, int filelang);
static og_status PhoReadConfFile(void *ptr, int is_dir, int filename_length, char *filename);
static og_bool PhoConfFileValidNameAndGetLang(struct og_ctrl_pho *ctrl_pho, unsigned char *filename, int *pfilelang);
static og_bool PhoConfFileValidXsd(struct og_ctrl_pho *ctrl_pho, unsigned char *filename);


struct og_tree_xml_tag PhoReadConfTag[] =  {
                                                  /* phonet_configuration (20)*/
  { DOgPhoReadConfTag_phonet_configuration,    40, "\0p\0h\0o\0n\0e\0t\0_\0c\0o\0n\0f\0i\0g\0u\0r\0a\0t\0i\0o\0n" },
                                                  /* rules (5) */
  { DOgPhoReadConfTag_rules,                   10, "\0r\0u\0l\0e\0s" },
                                                  /* rule (4) */
  { DOgPhoReadConfTag_rule,                     8, "\0r\0u\0l\0e" },
                                                  /* classes (7) */
  { DOgPhoReadConfTag_classes,                 14, "\0c\0l\0a\0s\0s\0e\0s" },
                                                  /* class (5) */
  { DOgPhoReadConfTag_class,                   10, "\0c\0l\0a\0s\0s" },
                                                  /* c (1) */
  { DOgPhoReadConfTag_c,                        2, "\0c" },
                                                  /* option (6) */
  { DOgPhoReadConfTag_option,                  12, "\0o\0p\0t\0i\0o\0n" },
  { 0, 0, "" }
  };


og_status PhoReadConfFiles(struct og_ctrl_pho *ctrl_pho, unsigned char *conf_directory)
{
  unsigned char old_configuration_file[DPcPathSize];

  sprintf(old_configuration_file, "%s.xml", conf_directory);
  if (OgFileExists(old_configuration_file))
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog,
        "PhoReadConfFiles: old configuration file '%s' detected and deprecated : it will not be processed, use new configuration",
        old_configuration_file);
  }
  else
  {
    sprintf(ctrl_pho->conf_directory, "%s", conf_directory);
    IFE(OgScanDir(conf_directory, PhoReadConfFile, (void * ) ctrl_pho, ctrl_pho->loginfo->where));
  }

  DONE;
}

static og_status PhoReadConfFile(void *ptr, int is_dir, int filename_length, char *filename)
{
  struct og_ctrl_pho *ctrl_pho = (struct og_ctrl_pho *) ptr;

  if (is_dir) DONE;
  if (strstr(filename, "/xsd/")) DONE;

  if (ctrl_pho->loginfo->trace & DOgPhoTraceMinimal)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConfFile: processing configuration file '%s'", filename);
  }

  int filelang = 0;
  og_bool valid = FALSE;
  IFE(valid = PhoConfFileValidNameAndGetLang(ctrl_pho, filename, &filelang));
  if (!valid) DONE;
  IFE(PhoConfFileValidXsd(ctrl_pho, filename));

  size_t Ilang_context_heap = 0;
  struct lang_context *lang_context = (struct lang_context *) OgHeapNewCell(ctrl_pho->lang_context_heap, &Ilang_context_heap);

  g_hash_table_insert(ctrl_pho->lang_context_map, GINT_TO_POINTER(filelang), lang_context);

  IFE(PhoInitLangContext(ctrl_pho,lang_context));

  IFE(PhoReadConf(lang_context, filename, filelang));

  DONE;
}


static og_bool PhoConfFileValidNameAndGetLang(struct og_ctrl_pho *ctrl_pho, unsigned char *filename, int *pfilelang)
{
  int configuration_directory_length = strlen(ctrl_pho->conf_directory);
  unsigned char *reduced_filename = filename + configuration_directory_length + 1;
  int reduced_filename_length = strlen(reduced_filename);


  int start = 0;
  unsigned char lang[DPcPathSize];
  int i = 0;
  // en/phonet_conf.xml
  for(i=0; i < reduced_filename_length; i++)
  {
    int c = reduced_filename[i];
    if(c == '/')
    {
      memcpy(lang,reduced_filename+start,i-start);
      lang[i-start]=0;
      start = i+1;
    }
  }

  unsigned char name[DPcPathSize];
  memcpy(name,reduced_filename+start,i-start);
  name[i-start]=0;


  int filelang = OgCodeToIso639(lang);
  if(filelang == 0)
  {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoConfFileValidNameAndGetLang: filename='%s' is not valid, language code '%s' is not correct", filename, lang);
    return FALSE;
  }
  *pfilelang = filelang;


  if (Ogstricmp(ctrl_pho->conf_filename,name))
  {
    OgMsg(ctrl_pho->hmsg,"",DOgMsgDestInLog,"PhoConfFileValidNameAndGetLang: filename='%s' is not valid : wrong name '%s' should be phonet_conf.xml", filename, name);
    return FALSE;
  }

  return TRUE;
}


static og_bool PhoConfFileValidXsd(struct og_ctrl_pho *ctrl_pho, unsigned char *filename)
{
  if (!OgFileExists(filename)) DONE;

  unsigned char xsd_dir[DPcPathSize];
  sprintf(xsd_dir, "%s/xsd", ctrl_pho->conf_directory);

  unsigned char xsd_filename[DPcPathSize];
  sprintf(xsd_filename, "%s/phonet_conf.xsd", xsd_dir);

  og_status status = OgXmlXsdValidateFile(ctrl_pho->hmsg, ctrl_pho->herr, NULL, filename, xsd_filename);
  IFE(status);
  DONE;
}




static int PhoReadConf(struct lang_context *lang_context, char *conf_file, int filelang)
{

  struct og_ctrl_pho *ctrl_pho = lang_context->ctrl_pho;

  unsigned char *B, *Btemp;
  struct og_xml_info cinfo, *info = &cinfo;
  struct stat filestat;
  void *hgetcp;
  FILE *fd, *fbuffer_test;

  if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "Start scanning '%s'", conf_file);
  }

  IFn(fd=fopen(conf_file,"rb"))
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf: No file or impossible to fopen '%s'", conf_file);
    DONE;
  }

  IF(fstat(fileno(fd),&filestat))
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf: impossible to fstat '%s'", conf_file);
    DONE;
  }

  IFn(Btemp=(unsigned char *)malloc(filestat.st_size+9))
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf: impossible to allocate %d bytes for '%s'",
        filestat.st_size, conf_file);
    fclose(fd);
    DONE;
  }

  int iBtemp=fread(Btemp,1,filestat.st_size,fd);
  IFn(iBtemp)
  {
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf: impossible to fread '%s', removing file", conf_file);
    DPcFree(Btemp);
    fclose(fd);
    DONE;
  }
  fclose(fd);

  /* we get the codepage */
  struct og_getcp_param cpparam[1];
  memset(cpparam, 0, sizeof(struct og_getcp_param));
  cpparam->herr = ctrl_pho->herr;
  cpparam->hmutex = ctrl_pho->hmutex;
  memcpy(&cpparam->loginfo, ctrl_pho->loginfo, sizeof(struct og_loginfo));
  cpparam->loginfo.trace = DOgGetcpTraceMinimal + DOgGetcpTraceMemory;

  IFn(hgetcp=OgGetCpInit(cpparam)) return (0);
  int codepage = 0;
  IFE(OgGetCp(hgetcp, iBtemp, Btemp, &codepage));
  IFE(OgGetCpFlush(hgetcp));

  /* we convert to a Unicode buffer */
  B = (unsigned char *) malloc(2 * iBtemp);
  int iB = 0;
  IFE(OgCpToUni(iBtemp, Btemp, 2 * iBtemp, &iB, B, OgNewCpToOld(codepage), 0, 0));
  DPcFree(Btemp);

  if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
  {
    fbuffer_test = fopen("log\\conf_buffer.uni", "wb");
    fwrite("\xFE\xFF", sizeof(unsigned char), 2, fbuffer_test);
    fwrite(B, sizeof(unsigned char), iB, fbuffer_test);
    fclose(fbuffer_test);
  }

  memset(info, 0, sizeof(struct og_xml_info));
  info->ctrl_pho = ctrl_pho;
  info->lang_context = lang_context;
  info->iB = iB;
  info->B = B;

  IFE(OgParseXmlUni(ctrl_pho->herr, 0, iB, B, PhoReadConf1, (void * )info));

  /* we minimize and fasten the automaton if read from xml */
  IFE(OgAum(lang_context->ha_rules));
  IFE(OgAuf(lang_context->ha_rules, 0));
  IFE(OgAutFree(lang_context->ha_rules));

  DPcFree(B);

  DONE;
}




STATICF(int) PhoReadConf1RuleReset(struct og_xml_info *info)
{

  info->ileft = 0;
  info->ikey = 0;
  info->iright = 0;
  info->iphonetic = 0;
  info->replace = 0;

  DONE;
}


static int PhoReadConf1(void *ptr, int type, int ib, unsigned char *b)
{
  struct og_xml_info *info = (struct og_xml_info *) ptr;
  struct og_ctrl_pho *ctrl_pho = info->ctrl_pho;
  struct lang_context *lang_context = info->lang_context;

  if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
  {
    char B[DPcPathSize];
    int iB;
    IFE(OgUniToCp(ib,b,DPcPathSize,&iB,B,DOgCodePageANSI,0,0));
    char buffer[DPcPathSize];
    buffer[0] = 0;
    for (int i = 0; i < info->ixml_path; i++)
    {
      sprintf(buffer + strlen(buffer), "%d ", info->xml_path[i]);
    }

    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf1: xml_path: %s type = %d [[%.*s]]", buffer, type, iB, B);
  }

  if (type == DOgParseXmlTag)
  {
    int value = DOgPhoReadConfTagNil;
    if (b[ib - 2] == 0 && b[ib - 1] == '/')
    {
      info->auto_tag = 1;
      info->end_tag = 0;
    }
    else
    {
      info->auto_tag = 0;
      if (b[0] == 0 && b[1] == '/') info->end_tag = 1;
      else info->end_tag = 0;
    }
    int i = 0;
    for (i = 0; i < ib; i += 2)
    {
      if (OgUniIsspace((b[i] << 8) + b[i + 1])) break;
    }
    int iname_tag = i;
    OgUniStrlwr(iname_tag, b, b);
    for (i = 0; PhoReadConfTag[i].value; i++)
    {
      if (PhoReadConfTag[i].iname == iname_tag - 2 * info->end_tag
          && !memcmp(PhoReadConfTag[i].name, b + 2 * info->end_tag, iname_tag - 2 * info->end_tag))
      {
        value = PhoReadConfTag[i].value;
        break;
      }
    }
    if (value != DOgPhoReadConfTagNil)
    {
      if (!info->end_tag)
      {
        if (info->ixml_path < DOgMaxXmlPath - 1) info->xml_path[info->ixml_path++] = value;
      }
    }

    if (value == DOgPhoReadConfTag_rules)
    {
      IFE(OgReadTagUni(ib, b, b - info->B, PhoReadConf1TagAttribute, info));
    }
    if (value == DOgPhoReadConfTag_rule)
    {
      IFE(OgReadTagUni(ib, b, b - info->B, PhoReadConf1TagAttribute, info));
      IFE(PhoRulesRuleAdd(info));
      IFE(PhoReadConf1RuleReset(info));
    }

    if (value == DOgPhoReadConfTag_class && !info->end_tag)
    {
      IFE(OgReadTagUni(ib, b, b - info->B, PhoReadConf1TagAttribute, info));
    }

    if (value == DOgPhoReadConfTag_option)
    {
      IFE(OgReadTagUni(ib, b, b - info->B, PhoReadConf1TagAttribute, info));
    }

    if (value != DOgPhoReadConfTagNil)
    {
      if (info->auto_tag || info->end_tag)
      {
        if (info->ixml_path > 0) info->ixml_path--;
      }
    }

  }
  else if (type == DOgParseXmlContent)
  {
    int value = info->xml_path[info->ixml_path - 1];

    if (value == DOgPhoReadConfTag_c)
    {
      IFE(ClassAddC(lang_context, info->Ichar_class, b));
    }

    if (info->option_type)
    {
      switch (info->option_type)
      {
        case DOgPhoReadConfOptionAppendingCharacter:
          IFE(PhoFormatAppendingCharAdd(lang_context, ib, b));
          break;
        case DOgPhoReadConfOptionNonAlphaToSpace:
          if (ib > 1 && !memcmp(b + 1, "1", 1))
          {
            lang_context->non_alpha_to_space = 1;
          }
          break;
        case DOgPhoReadConfOptionSpaceCharacter:
          memcpy(lang_context->space_character, b, 2 * sizeof(unsigned char));
          break;
        case DOgPhoReadConfKeepDigit:
          if (ib > 1 && !memcmp(b + 1, "1", 1))
          {
            lang_context->keep_digit = TRUE;
          }
          break;
      }
      info->option_type = DOgPhoReadConfOptionNil;
    }

  }

  return (0);
}



static int  PhoReadConf1TagAttribute(void *ptr, struct og_read_tag *rt)
{
  struct og_xml_info *info = (struct og_xml_info *) ptr;
  struct og_ctrl_pho *ctrl_pho = info->ctrl_pho;
  struct lang_context *lang_context = info->lang_context;

  if (rt->iattr < 2) DONE;
  if (rt->ivalue < 2) DONE;

  OgUniStrlwr(rt->iattr, rt->attr, rt->attr);

  if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
  {
    char B1[DPcPathSize];
    char B2[DPcPathSize];
    int iB1 = 0, iB2 = 0;
    IFE(OgUniToCp(rt->iattr,rt->attr,DPcPathSize,&iB1,B1,DOgCodePageANSI,0,0));
    IFE(OgUniToCp(rt->ivalue,rt->value,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
    OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf1TagAttribute: Reading Tag attr=%.*s value=%.*s", iB1, B1,
        iB2, B2);
  }

  if (rt->closing_tag) DONE;

  if (info->xml_path[info->ixml_path - 1] == DOgPhoReadConfTag_option)
  {
    if (rt->iattr == 8 && !Ogmemicmp("\0n\0a\0m\0e", rt->attr, 8))
    { /* name (4) */
      /* appending character (19) */
      if (rt->ivalue == 38 && !Ogmemicmp("\0a\0p\0p\0e\0n\0d\0i\0n\0g\0 \0c\0h\0a\0r\0a\0c\0t\0e\0r", rt->value, 38))
      {
        info->option_type = DOgPhoReadConfOptionAppendingCharacter;
        if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
        {
          OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf1TagAttribute: Appending Character detected");
        }
      }
      /* non-alpha character to space (28) */
      if (rt->ivalue == 56
          && !Ogmemicmp("\0n\0o\0n\0-\0a\0l\0p\0h\0a\0 \0c\0h\0a\0r\0a\0c\0t\0e\0r\0 \0t\0o\0 \0s\0p\0a\0c\0e",
              rt->value, 56))
      {
        info->option_type = DOgPhoReadConfOptionNonAlphaToSpace;
        if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
        {
          OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf1TagAttribute: Non Alpha To Space Parameter detected");
        }
      }
      /* keep_digit (10) */
      if (rt->ivalue == 20 && !Ogmemicmp("\0k\0e\0e\0p\0_\0d\0i\0g\0i\0t", rt->value, 20))
      {
        info->option_type = DOgPhoReadConfKeepDigit;
        if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
        {
          OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf1TagAttribute: keep_digit Parameter detected");
        }
      }
      /* space character (15) */
      if (rt->ivalue == 30 && !Ogmemicmp("\0s\0p\0a\0c\0e\0 \0c\0h\0a\0r\0a\0c\0t\0e\0r", rt->value, 30))
      {
        info->option_type = DOgPhoReadConfOptionSpaceCharacter;
        if (ctrl_pho->loginfo->trace & DOgPhoTracePhoReadConfDetail)
        {
          OgMsg(ctrl_pho->hmsg, "", DOgMsgDestInLog, "PhoReadConf1TagAttribute: Space Character detected");
        }
      }
    }
  }

  if (info->xml_path[info->ixml_path - 1] == DOgPhoReadConfTag_rules)
  {
    if (rt->iattr == 8 && !Ogmemicmp("\0s\0t\0e\0p", rt->attr, 8))
    { /* step (4) */
      char B2[DPcPathSize];
      int iB2 = 0;
      IFE(OgUniToCp(rt->ivalue,rt->value,DPcPathSize,&iB2,B2,DOgCodePageANSI,0,0));
      B2[iB2] = 0;
      info->step = atoi(B2);
      if (info->step > lang_context->max_steps) lang_context->max_steps = info->step;
    }
  }
  if (info->xml_path[info->ixml_path - 1] == DOgPhoReadConfTag_rule)
  {
    if (rt->iattr == 8 && !Ogmemicmp("\0l\0e\0f\0t", rt->attr, 8))
    {/* left (4) */
      memcpy(info->left, rt->value, rt->ivalue);
      info->ileft = rt->ivalue;
    }
    if (rt->iattr == 6 && !Ogmemicmp("\0k\0e\0y", rt->attr, 6))
    {/* key (3) */
      memcpy(info->key, rt->value, rt->ivalue);
      info->ikey = rt->ivalue;
      info->replace = rt->ivalue;
    }
    if (rt->iattr == 10 && !Ogmemicmp("\0r\0i\0g\0h\0t", rt->attr, 10))
    {/* right (5) */
      memcpy(info->right, rt->value, rt->ivalue);
      info->iright = rt->ivalue;
    }
    if (rt->iattr == 16 && !Ogmemicmp("\0p\0h\0o\0n\0e\0t\0i\0c", rt->attr, 16))
    {/* phonetic (8) */
      memcpy(info->phonetic, rt->value, rt->ivalue);
      info->iphonetic = rt->ivalue;
    }
  }
  if (info->xml_path[info->ixml_path - 1] == DOgPhoReadConfTag_class)
  {
    if (rt->iattr == 18 && !Ogmemicmp("\0c\0h\0a\0r\0a\0c\0t\0e\0r", rt->attr, 18))
    { /* character (9) */
      IFE(info->Ichar_class = ClassCreate(lang_context, rt->value));
    }
  }

  DONE;
}

