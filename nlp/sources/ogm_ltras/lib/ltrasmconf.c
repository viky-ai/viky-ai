/*
 *  Reads modules configuration file
 *  Copyright (c) 2010 Pertimm by Lois Rigouste
 *  Dev : January, March 2010
 *  Version 1.1
 */
#include "ogm_ltras.h"

#define DOgLtrasModuleReadConfTagNil                      0
#define DOgLtrasModuleReadConfTag_ltras_modules           1
#define DOgLtrasModuleReadConfTag_ltras_module            2
#define DOgLtrasModuleReadConfTag_name                    3
#define DOgLtrasModuleReadConfTag_function_name           4
#define DOgLtrasModuleReadConfTag_library_name            5
#define DOgLtrasModuleReadConfTag_minimum_score           6
#define DOgLtrasModuleReadConfTag_minimum_final_score     7
#define DOgLtrasModuleReadConfTag_maximum_transformation  8
#define DOgLtrasModuleReadConfTag_parameters              9
#define DOgLtrasModuleReadConfTag_parameter               10
#define DOgLtrasModuleReadConfTag_equivalent_letter       11
#define DOgLtrasModuleReadConfTag_letter                  12
#define DOgLtrasModuleReadConfTag_space_insertion_costs   13
#define DOgLtrasModuleReadConfTag_space_insertion_cost    14
#define DOgLtrasModuleReadConfTag_space_deletion_costs    15
#define DOgLtrasModuleReadConfTag_space_deletion_cost     16

#define DOgMaxXmlPath                            10
#define DOgLtrasMaxEquivalentLetterSize           5
#define DOgLtrasMinEquivalentLetterSize           2

struct og_ltras_xml_info
{
  struct og_ctrl_ltras *ctrl_ltras;
  int iB;
  unsigned char *B;
  int end_tag, auto_tag, ixml_path, xml_path[DOgMaxXmlPath];
  unsigned char name[DPcPathSize];
  unsigned char function_name[DPcPathSize];
  unsigned char library_name[DPcPathSize];
  int minimum_score_length;
  double minimum_score_score;
  int max_trans_nb_request_words, max_trans_number;
  int some_maximum_transformation;
  unsigned char parameter_name[DPcPathSize];
  unsigned char parameter_value[DPcPathSize];

  unsigned char equivalent_letter_score_string[DPcPathSize];
  double equivalent_letter_score;

  int equivalent_letters_tab[DOgLtrasMaxEquivalentLetterSize];
  int equivalent_letters_tab_used;

  long int space_cost_occurence;
  double space_cost_cost;
};

struct og_ltras_xml_tag
{
  int value, iname;
  char *name;
};

struct og_ltras_xml_tag LtrasXmlReadTag[] = { { DOgLtrasModuleReadConfTag_ltras_modules, 13, "ltras_modules" }, {
DOgLtrasModuleReadConfTag_ltras_module, 12, "ltras_module" }, { DOgLtrasModuleReadConfTag_name, 4, "name" }, {
DOgLtrasModuleReadConfTag_function_name, 13, "function_name" }, { DOgLtrasModuleReadConfTag_library_name, 12,
    "library_name" }, { DOgLtrasModuleReadConfTag_minimum_score, 13, "minimum_score" }, {
DOgLtrasModuleReadConfTag_minimum_final_score, 19, "minimum_final_score" }, {
DOgLtrasModuleReadConfTag_maximum_transformation, 22, "maximum_transformation" }, {
DOgLtrasModuleReadConfTag_parameters, 10, "parameters" }, { DOgLtrasModuleReadConfTag_parameter, 9, "parameter" }, {
DOgLtrasModuleReadConfTag_equivalent_letter, 17, "equivalent_letter" },
    { DOgLtrasModuleReadConfTag_letter, 6, "letter" }, { DOgLtrasModuleReadConfTag_space_insertion_costs, 21,
        "space_insertion_costs" }, { DOgLtrasModuleReadConfTag_space_insertion_cost, 20, "space_insertion_cost" }, {
        DOgLtrasModuleReadConfTag_space_deletion_costs, 20, "space_deletion_costs" }, {
        DOgLtrasModuleReadConfTag_space_deletion_cost, 19, "space_deletion_cost" }, { 0, 0, "" } };

static int OgLtrasModuleReadConf1(void *, int, int, unsigned char *);
static int ReadScore(void *ptr, struct og_read_tag *rt);
static int ReadMaxTrans(void *ptr, struct og_read_tag *rt);
static int ReadParameter(void *ptr, struct og_read_tag *rt);
static int InterpolateScores(struct og_ctrl_ltras *ctrl_ltras, double *ms);
static int InterpolateMaxTrans(struct og_ctrl_ltras *ctrl_ltras, int *mt, int default_maximum_transformations);
static og_status initTransformationAndScores(struct og_ctrl_ltras *ctrl_ltras, int some_maximum_transformation,
    int default_maximum_transformations);
static og_status ReadEquivalentLetterScore(void *ptr, struct og_read_tag *rt);
static og_status CreateAndStoreEquivalentLetterCombinations(struct og_ctrl_ltras *ctrl_ltras,
    struct og_ltras_xml_info *info);
static og_status ComputeEquivalentLetterTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned
char *buffer, int ibuffer);
static og_status ComputeSpaceCostTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b,
    int ib, og_bool is_insertion);

static og_status ComputeLetterTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer);
static og_status ComputeLtrasModuleTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value);
static og_status ComputeParametersTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value);
static og_status ComputeEquivalentLetterTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value);
static og_status ComputeSpaceCostTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value);

static og_status readTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b, int ib,
    int *value);
static og_status ComputeLtrasModuleTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info);
static og_status ComputeMinimumScoreTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *b, int ib);
static og_status ComputeMinimumFinalScoreTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *b, int ib);
static og_status ComputeMaximumTransformationTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *b, int ib);
static og_status ComputeParameterTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b,
    int ib);
static og_status parseXmlContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b,
    int ib);
static og_status parseXmlTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b, int ib);
static og_status ReadSpaceCostsOccurence(void *ptr, struct og_read_tag *rt);

/*
 * Read an XML configuration file, whose format is as follows :
 *   <?xml version="1.0" encoding="UTF-8" ?>
 *   <ltras>
 *   <parameters>
 *     <parameter name="toto">value</parameter>
 *   </parameters>
 *   <minimum_scores>
 *     <minimum_score length="3" score="0.90" />
 *     <minimum_score length="4" score="0.85" />
 *   </minimum_scores>
 *   <minimum_final_scores>
 *     <minimum_final_score length="3" score="0.90" />
 *     <minimum_final_score length="4" score="0.85" />
 *   </minimum_final_scores>
 *   <maximum_transformations>
 *     <maximum_transformation nb_request_words="1" number="100"/>
 *     <maximum_transformation nb_request_words="2" number="8"/>
 *   </maximum_transformations>
 *   <ltras_modules>
 *    <ltras_module>
 *     <name>xxxx</name>
 *     <function_name>OgLtrasModuleXxxx</function_name>
 *     <library_name>my_library</library_name>
 *    </ltras_module>
 *   </ltras_modules>
 *   </ltras>
 * Encoding is supposed to be UTF-8, but in any case,
 * there will not be too many accents, as we are
 * handle library names and function names
 */
PUBLIC(og_status) OgLtrasModuleReadConf(void *handle, char *filename)
{
  struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *) handle;

  int default_maximum_transformations = 0;
  if (OgFileExists(filename))
  {
    FILE *fd = NULL;
    struct stat filestat;

    IFn(fd=fopen(filename,"rb"))
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasModuleReadConf: No file or impossible to fopen '%s'",
          filename);
      DPcErr;
    }
    IF(fstat(fileno(fd),&filestat))
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasModuleReadConf: impossible to fstat '%s'", filename);
      fclose(fd);
      DPcErr;
    }

    int iB = 0;
    unsigned char *B = NULL;
    IFn(B=(unsigned char *)malloc(filestat.st_size+9))
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasModuleReadConf: impossible to allocate %d bytes for '%s'",
          filestat.st_size, filename);
      fclose(fd);
      DPcErr;
    }
    IFn(iB=fread(B,1,filestat.st_size,fd))
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasModuleReadConf: impossible to fread '%s'", filename);
      DPcFree(B);
      fclose(fd);
      DPcErr;
    }

    // close conf file
    fclose(fd);

    struct og_ltras_xml_info info[1];
    memset(info, 0, sizeof(struct og_ltras_xml_info));
    info->ctrl_ltras = ctrl_ltras;
    info->iB = iB;
    info->B = B;

    IF(OgParseXml(ctrl_ltras->herr,0,iB,B,OgLtrasModuleReadConf1,info))
    {
      DPcFree(B);
      DPcErr;
    }

    IF(initTransformationAndScores(ctrl_ltras, info->some_maximum_transformation, default_maximum_transformations))
    {
      DPcFree(B);
      DPcErr;
    }

  }
  else
  {
    IFE(initTransformationAndScores(ctrl_ltras, FALSE, default_maximum_transformations));

    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasModuleReadConf: since file '%s' does not exist,"
        " activating modules lem1, lem2, tra and term, and continuing", filename);
    IFE(LtrasAddPluginModule(ctrl_ltras, "lem1", "OgLtrasModuleLem1", "libogltras"));
    IFE(LtrasAddPluginModule(ctrl_ltras, "lem2", "OgLtrasModuleLem2", "libogltras"));
    IFE(LtrasAddPluginModule(ctrl_ltras, "tra", "OgLtrasModuleTra", "libogltras"));
    IFE(LtrasAddPluginModule(ctrl_ltras, "term", "OgLtrasModuleTerm", "libogltras"));
  }

  DONE;
}

static og_status initTransformationAndScores(struct og_ctrl_ltras *ctrl_ltras, int some_maximum_transformation,
    int default_maximum_transformations)
{
  /* default  value of zero for scores is perfect
   * however, it is not good at all for transformations
   * thus we set a good default values for transformations
   * when ltras_conf.xml does not exist or when the
   * <maximum_transformations> section does not exist */
  if (!some_maximum_transformation)
  {
    ctrl_ltras->maximum_transformation[1] = 100;
    ctrl_ltras->maximum_transformation[2] = 8;
    ctrl_ltras->maximum_transformation[3] = 5;
    ctrl_ltras->maximum_transformation[4] = 4;
    ctrl_ltras->maximum_transformation[5] = 3;
    ctrl_ltras->maximum_transformation[6] = 3;
    ctrl_ltras->maximum_transformation[7] = 2;
    ctrl_ltras->maximum_transformation[10] = 1;
    default_maximum_transformations = 1;
  }

  IFE(InterpolateScores(ctrl_ltras, ctrl_ltras->minimum_score));
  IFE(InterpolateScores(ctrl_ltras, ctrl_ltras->minimum_final_score));
  IFE(InterpolateMaxTrans(ctrl_ltras, ctrl_ltras->maximum_transformation, default_maximum_transformations));

  DONE;
}

static int OgLtrasModuleReadConf1(void *ptr, int type, int ib, unsigned char *b)
{
  struct og_ltras_xml_info *info = (struct og_ltras_xml_info *) ptr;
  struct og_ctrl_ltras *ctrl_ltras = info->ctrl_ltras;

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleConf)
  {
    char buffer[DPcPathSize * 2];
    buffer[0] = 0;
    for (int i = 0; i < info->ixml_path; i++)
    {
      sprintf(buffer + strlen(buffer), "%d ", info->xml_path[i]);
    }
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "OgLtrasModuleReadConf1: xml_path (length %d): %s type = %d [[%.*s]]",
        info->ixml_path, buffer, type, ib, b);
  }

  if (type == DOgParseXmlTag)
  {
    IFE(parseXmlTag(ctrl_ltras, info, b, ib));
  }
  else if (type == DOgParseXmlContent)
  {
    IFE(parseXmlContent(ctrl_ltras, info, b, ib));
  }

  return (0);
}

/******************************************** PARSE XML **********************************************************/

static og_status parseXmlTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b, int ib)
{

  int value = DOgLtrasModuleReadConfTagNil;
  IFE(readTag(ctrl_ltras, info, b, ib, &value));

  if (value != DOgLtrasModuleReadConfTagNil)
  {
    if (!info->end_tag)
    {
      if (info->ixml_path < DOgMaxXmlPath - 1)
      {
        info->xml_path[info->ixml_path++] = value;
      }
    }
    if (info->auto_tag || info->end_tag)
    {
      if (info->ixml_path > 0)
      {
        info->ixml_path--;
      }
    }
  }
  if (value == DOgLtrasModuleReadConfTag_ltras_module)
  {
    IFE(ComputeLtrasModuleTag(ctrl_ltras, info));
  }
  else if (value == DOgLtrasModuleReadConfTag_minimum_score)
  {
    IFE(ComputeMinimumScoreTag(ctrl_ltras, info, b, ib));
  }
  else if (value == DOgLtrasModuleReadConfTag_minimum_final_score)
  {
    IFE(ComputeMinimumFinalScoreTag(ctrl_ltras, info, b, ib));
  }
  else if (value == DOgLtrasModuleReadConfTag_maximum_transformation)
  {
    IFE(ComputeMaximumTransformationTag(ctrl_ltras, info, b, ib));
  }
  else if (value == DOgLtrasModuleReadConfTag_parameter)
  {
    IFE(ComputeParameterTag(ctrl_ltras, info, b, ib));
  }
  else if (value == DOgLtrasModuleReadConfTag_equivalent_letter)
  {
    IFE(ComputeEquivalentLetterTag(ctrl_ltras, info, b, ib));
  }
  else if (value == DOgLtrasModuleReadConfTag_space_insertion_cost)
  {
    IFE(ComputeSpaceCostTag(ctrl_ltras, info, b, ib, TRUE));
  }
  else if (value == DOgLtrasModuleReadConfTag_space_deletion_cost)
  {
    IFE(ComputeSpaceCostTag(ctrl_ltras, info, b, ib, FALSE));
  }

  DONE;
}

static og_status parseXmlContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b,
    int ib)
{
  if (info->ixml_path >= 2)
  {
    int ibuffer = ib;
    if (ibuffer > DPcPathSize - 1) ibuffer = DPcPathSize - 1;
    char buffer[DPcPathSize * 2];
    memcpy(buffer, b, ibuffer);
    buffer[ibuffer] = 0;
    ibuffer = strlen(buffer);

    int value = info->xml_path[info->ixml_path - 1];
    int valuep = info->xml_path[info->ixml_path - 2];

    if (valuep == DOgLtrasModuleReadConfTag_ltras_module)
    {
      IFE(ComputeLtrasModuleTagContent(ctrl_ltras, info, buffer, ibuffer, value));
    }
    else if (valuep == DOgLtrasModuleReadConfTag_parameters)
    {
      IFE(ComputeParametersTagContent(ctrl_ltras, info, buffer, ibuffer, value));
    }
    else if (valuep == DOgLtrasModuleReadConfTag_equivalent_letter)
    {
      IFE(ComputeEquivalentLetterTagContent(ctrl_ltras, info, buffer, ibuffer, value));
    }
    else if ((valuep == DOgLtrasModuleReadConfTag_space_insertion_costs)
        || (valuep == DOgLtrasModuleReadConfTag_space_deletion_costs))
    {
      IFE(ComputeSpaceCostTagContent(ctrl_ltras, info, buffer, ibuffer, value));
    }
  }
  DONE;
}

/******************************************** PARSE TAGS **********************************************************/

static og_status readTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b, int ib,
    int *value)
{
  *value = DOgLtrasModuleReadConfTagNil;
  for (int i = 0; i < ib; i++)
  {
    b[i] = tolower(b[i]);
  }

  // fin auto_tag
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

  int i;
  for (i = 0; i < ib; i++)
  {
    if (PcIsspace(b[i])) break;
  }
  int iname_tag = i;
  for (i = 0; LtrasXmlReadTag[i].value; i++)
  {
    if (LtrasXmlReadTag[i].iname == iname_tag - info->end_tag
        && !memcmp(LtrasXmlReadTag[i].name, b + info->end_tag, iname_tag - info->end_tag))
    {
      *value = LtrasXmlReadTag[i].value;
      break;
    }
  }
  DONE;
}

static og_status ComputeLtrasModuleTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info)
{
  if (info->end_tag)
  {
    IFE(LtrasAddPluginModule(ctrl_ltras, info->name, info->function_name, info->library_name));
    info->name[0] = 0;
    info->function_name[0] = 0;
    info->library_name[0] = 0;
  }
  DONE;
}

static og_status ComputeMinimumScoreTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *b, int ib)
{
  info->minimum_score_length = -1;
  info->minimum_score_score = 0;
  IFE(OgReadTag(ib, b, b - info->B, ReadScore, info));
  if (0 <= info->minimum_score_length && info->minimum_score_length < DOgLtrasMinimumScoreLength)
  {
    ctrl_ltras->minimum_score[info->minimum_score_length] = info->minimum_score_score;
  }
  DONE;
}

static og_status ComputeMinimumFinalScoreTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *b, int ib)
{
  info->minimum_score_length = -1;
  info->minimum_score_score = 0;
  IFE(OgReadTag(ib, b, b - info->B, ReadScore, info));
  if (0 <= info->minimum_score_length && info->minimum_score_length < DOgLtrasMinimumScoreLength)
  {
    ctrl_ltras->minimum_final_score[info->minimum_score_length] = info->minimum_score_score;
  }
  DONE;
}

static og_status ComputeMaximumTransformationTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *b, int ib)
{
  info->max_trans_nb_request_words = -1;
  info->max_trans_number = 0;
  IFE(OgReadTag(ib, b, b - info->B, ReadMaxTrans, info));
  if (0 <= info->max_trans_nb_request_words && info->max_trans_nb_request_words < DOgLtrasMaximumTransformationLength)
  {
    ctrl_ltras->maximum_transformation[info->max_trans_nb_request_words] = info->max_trans_number;
    info->some_maximum_transformation = 1;
  }
  DONE;
}

static og_status ComputeParameterTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b,
    int ib)
{
  if (info->end_tag)
  {
    IFE(LtrasAddParameter(ctrl_ltras, info->parameter_name, info->parameter_value));
    info->parameter_name[0] = 0;
    info->parameter_value[0] = 0;
  }
  else
  {
    IFE(OgReadTag(ib, b, b - info->B, ReadParameter, info));
  }
  DONE;
}

static og_status ComputeEquivalentLetterTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer)
{
  if (!info->end_tag)
  {
    // We get the equivalent_letter_cost, and set it to default value if it is not set
    IFE(OgReadTag(ibuffer, buffer, buffer - info->B, ReadEquivalentLetterScore, info));
  }
  else
  {
    if ((info->equivalent_letters_tab_used < 2))
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "OgLtrasModuleReadConf1: <equivalent_letter> tag must contain a number of letters between "
          "%d and %d. %d is not inside the range", DOgLtrasMinEquivalentLetterSize, DOgLtrasMaxEquivalentLetterSize,
          info->equivalent_letters_tab_used);
      OgErr(ctrl_ltras->herr, erreur);
      DPcErr;
    }

    IFE(CreateAndStoreEquivalentLetterCombinations(ctrl_ltras, info));
    info->equivalent_letters_tab_used = 0;
  }
  DONE;
}

static og_status ComputeSpaceCostTag(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info, unsigned char *b,
    int ib, og_bool is_insertion)
{
  if (info->end_tag)
  {
    IFE(OgStmAddSpaceCost(ctrl_ltras->hstm, info->space_cost_occurence - 1, info->space_cost_cost, is_insertion));
  }
  else
  {
    info->space_cost_cost = 0;
    info->space_cost_occurence = -1;

    IFE(OgReadTag(ib, b, b - info->B, ReadSpaceCostsOccurence, info));
  }
  DONE;
}

/******************************************** PARSE CONTENT **********************************************************/

static og_status ComputeLtrasModuleTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value)
{
  if (value == DOgLtrasModuleReadConfTag_name)
  {
    strcpy(info->name, buffer);
  }
  else if (value == DOgLtrasModuleReadConfTag_function_name)
  {
    strcpy(info->function_name, buffer);
  }
  else if (value == DOgLtrasModuleReadConfTag_library_name)
  {
    strcpy(info->library_name, buffer);
  }
  DONE;
}

static og_status ComputeParametersTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value)
{
  if (value == DOgLtrasModuleReadConfTag_parameter)
  {
    strcpy(info->parameter_value, buffer);
  }
  DONE;
}

static og_status ComputeEquivalentLetterTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value)
{
  // <letter>a<letter>
  if (value == DOgLtrasModuleReadConfTag_letter)
  {
    IFE(ComputeLetterTagContent(ctrl_ltras, info, buffer, ibuffer));
  }
  DONE;
}

static og_status ComputeLetterTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer)
{
  // Xml decode character
  char xml_decoded_buffer[DPcPathSize];
  int xml_decoded_buffer_length;
  IFE(OgXmlDecode(ibuffer, buffer, DPcPathSize, &xml_decoded_buffer_length, xml_decoded_buffer, DOgCodePageUTF8, 0, 0));

  // Transformation of utf-8 xml decoded character to unicode
  unsigned char unibuffer[DPcPathSize];
  int unibuffer_length;
  IFE(
      OgCpToUni(xml_decoded_buffer_length, xml_decoded_buffer, DPcPathSize, &unibuffer_length, unibuffer, DOgCodePageUTF8, 0, 0));

  int length;
  if (OgLipIsPunctuationWord(&ctrl_ltras->lip_conf, unibuffer_length, unibuffer, &length))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasModuleReadConf1: letter %s is a punctuation word. Punctuation words are forbidden as "
        "equivalent letters", buffer);
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }

  if (unibuffer_length == 0)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasModuleReadConf1: one letter tag is empty");
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }

  // if not a letter, we go on error
  if (unibuffer_length != 2)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasModuleReadConf1: %s is not a letter", buffer);
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }

  // Transformation in integer of the unicode character
  int letter = (unibuffer[0] << 8) + unibuffer[1];

  if (OgLipIsPunctuation(&ctrl_ltras->lip_conf, letter))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasModuleReadConf1: letter %s is a punctuation. Punctuations are forbidden as "
        "equivalent letters", buffer);
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }

  if (info->equivalent_letters_tab_used == DOgLtrasMaxEquivalentLetterSize)
  {
    info->equivalent_letters_tab_used++;
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasModuleReadConf1: <equivalent_letter> tag must contain a number of letters between "
        "%d and %d. %d is not inside the range", DOgLtrasMinEquivalentLetterSize, DOgLtrasMaxEquivalentLetterSize,
        info->equivalent_letters_tab_used);
    OgErr(ctrl_ltras->herr, erreur);
    DPcErr;
  }

  info->equivalent_letters_tab[info->equivalent_letters_tab_used] = letter;
  info->equivalent_letters_tab_used++;

  DONE;
}

static og_status ComputeSpaceCostTagContent(struct og_ctrl_ltras *ctrl_ltras, struct og_ltras_xml_info *info,
    unsigned char *buffer, int ibuffer, int value)
{
  if ((value == DOgLtrasModuleReadConfTag_space_deletion_cost)
      || (value == DOgLtrasModuleReadConfTag_space_insertion_cost))
  {
    char *errors;
    info->space_cost_cost = strtod(buffer, &errors);
    if (*errors)
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "OgLtrasModuleReadConf1: a space_cost %s must be a decimal value", buffer);
      OgErr(ctrl_ltras->herr, erreur);
      DPcErr;
    }
  }

  DONE;
}

/******************************************** TREATMENTS **********************************************************/

static og_status CreateAndStoreEquivalentLetterCombinations(struct og_ctrl_ltras *ctrl_ltras,
    struct og_ltras_xml_info *info)
{
  for (int i = 0; i < (info->equivalent_letters_tab_used - 1); i++)
  {
    int letter1 = info->equivalent_letters_tab[i];
    int letter2 = info->equivalent_letters_tab[i + 1];

    // We create the entry in the order and reverse order to cover all the possibilities
    if (letter1 != letter2)
    {
      IFE(OgStmInitCreateEquivalentLetterEntry(ctrl_ltras->hstm, letter1, letter2, info->equivalent_letter_score));
      IFE(OgStmInitCreateEquivalentLetterEntry(ctrl_ltras->hstm, letter2, letter1, info->equivalent_letter_score));
    }
    else
    {
      // we get the letter as a string to display an error
      unsigned char unibuffer[DPcPathSize];
      int unibuffer_length = 2;

      unibuffer[0] = (letter1 & 0xff00) >> 8;
      unibuffer[1] = letter1 & 0x00ff;

      unsigned char utf8_buffer[DPcPathSize];
      int utf8_buffer_length;
      IFE(OgUniToCp(unibuffer_length,unibuffer,DPcPathSize,&utf8_buffer_length,utf8_buffer,DOgCodePageUTF8,0,0));

      unsigned char xml_encoded[DPcPathSize];
      int xml_encoded_length;
      IFE(OgXmlEncode(utf8_buffer_length,utf8_buffer,DPcPathSize,&xml_encoded_length,xml_encoded,DOgCodePageUTF8,0));

      char erreur[DOgErrorSize];
      sprintf(erreur, "OgLtrasModuleReadConf1: letter %s is specified twice ", xml_encoded);
      OgErr(ctrl_ltras->herr, erreur);
      DPcErr;
    }
  }
  DONE;
}

static og_status ReadEquivalentLetterScore(void *ptr, struct og_read_tag *rt)
{
  struct og_ltras_xml_info *info = (struct og_ltras_xml_info *) ptr;

  if (rt->closing_tag) DONE;

  if (!Ogstricmp("cost", rt->attr))
  {
    info->equivalent_letter_score = atof(rt->value);
  }
  else
  {
    info->equivalent_letter_score = DOgLtrasDefaultEquivalentLetterScore;
  }

  DONE;
}

static int ReadScore(void *ptr, struct og_read_tag *rt)
{
  struct og_ltras_xml_info *info = (struct og_ltras_xml_info *) ptr;

  if (rt->closing_tag) DONE;

  if (!Ogstricmp("length", rt->attr))
  {
    info->minimum_score_length = atoi(rt->value);
  }
  else if (!Ogstricmp("score", rt->attr))
  {
    info->minimum_score_score = atof(rt->value);
    info->minimum_score_score -= DOgLtrasTinyScore;
  }

  DONE;
}

static og_status ReadSpaceCostsOccurence(void *ptr, struct og_read_tag *rt)
{
  struct og_ltras_xml_info *info = (struct og_ltras_xml_info *) ptr;

  if (rt->closing_tag) DONE;

  if (!Ogstricmp("occurence", rt->attr) && (rt->ivalue > 0))
  {
    char *errors;
    info->space_cost_occurence = strtol(rt->value, &errors, 10);
    if (*errors || (info->space_cost_occurence < 0))
    {
      char erreur[DOgErrorSize];
      sprintf(erreur, "OgLtrasModuleReadConf1: occurence of space_cost %s is not a positive number", rt->value);
      OgErr(info->ctrl_ltras->herr, erreur);
      DPcErr;
    }
  }

  DONE;
}

static int ReadMaxTrans(void *ptr, struct og_read_tag *rt)
{
  struct og_ltras_xml_info *info = (struct og_ltras_xml_info *) ptr;

  if (rt->closing_tag) DONE;

  if (!Ogstricmp("nb_request_words", rt->attr))
  {
    info->max_trans_nb_request_words = atoi(rt->value);
  }
  else if (!Ogstricmp("number", rt->attr))
  {
    info->max_trans_number = atoi(rt->value);
  }

  DONE;
}

static int ReadParameter(void *ptr, struct og_read_tag *rt)
{
  struct og_ltras_xml_info *info = (struct og_ltras_xml_info *) ptr;

  if (rt->closing_tag) DONE;

  if (!Ogstricmp("name", rt->attr))
  {
    strcpy(info->parameter_name, rt->value);
  }

  DONE;
}

static int InterpolateScores(struct og_ctrl_ltras *ctrl_ltras, double *ms)
{
  int i, j, imin, imax, ilast, inext, first;
  double a, b;

  for (imin = (-1), imax = (-1), first = 1, i = 0; i < DOgLtrasMinimumScoreLength; i++)
  {
    IFn(ms[i]) continue;
    if (first)
    {
      first = 0;
      imin = i;
    }
    imax = i;
  }
  /** No score, thus all scores at zero **/
  if (imin < 0 || imax < 0) DONE;

  for (i = 0; i < imin; i++)
  {
    ms[i] = ms[imin];
  }

  ilast = imin;
  for (i = imin + 1; i <= imax; i++)
  {
    IFn(ms[i])
    {
      for (j = i + 1; j <= imax; j++)
      {
        IFx(ms[j])
        {
          inext = j;
          break;
        }
      }
      a = ms[inext] - ms[ilast];
      a /= inext - ilast;
      b = ms[ilast] - a * ilast;
      ms[i] = a * i + b;
    }
    else ilast = i;
  }

  for (i = imax; i < DOgLtrasMinimumScoreLength; i++)
  {
    ms[i] = ms[imax];
  }

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleConf)
  {
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "InterpolateScores: list of scores:");
    for (i = imin; i <= imax; i++)
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, " %2d: %2f", i, ms[i]);
    }
  }

  DONE;
}

static int InterpolateMaxTrans(struct og_ctrl_ltras *ctrl_ltras, int *mt, int default_maximum_transformations)
{
  int i, j, imin, imax, ilast, inext, first;
  int a, b;

  for (imin = (-1), imax = (-1), first = 1, i = 0; i < DOgLtrasMaximumTransformationLength; i++)
  {
    IFn(mt[i]) continue;
    if (first)
    {
      first = 0;
      imin = i;
    }
    imax = i;
  }
  /** No score, thus all scores at zero **/
  if (imin < 0 || imax < 0) DONE;

  for (i = 0; i < imin; i++)
  {
    mt[i] = mt[imin];
  }

  ilast = imin;
  for (i = imin + 1; i <= imax; i++)
  {
    IFn(mt[i])
    {
      for (j = i + 1; j <= imax; j++)
      {
        IFx(mt[j])
        {
          inext = j;
          break;
        }
      }
      a = mt[inext] - mt[ilast];
      a /= inext - ilast;
      b = mt[ilast] - a * ilast;
      mt[i] = a * i + b;
    }
    else ilast = i;
  }

  for (i = imax; i < DOgLtrasMaximumTransformationLength; i++)
  {
    mt[i] = mt[imax];
  }

  if (ctrl_ltras->loginfo->trace & DOgLtrasTraceModuleConf)
  {
    OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, "InterpolateMaxTrans: list of maximum transformation numbers%s:",
        default_maximum_transformations ? " (default numbers used)" : "");
    for (i = imin; i <= imax; i++)
    {
      OgMsg(ctrl_ltras->hmsg, "", DOgMsgDestInLog, " %2d: %2d", i, mt[i]);
    }
  }

  DONE;
}

