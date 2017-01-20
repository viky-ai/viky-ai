/*
 *  The Cut module for linguistic transformations
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: January-June 2010
 *  Version 1.1
*/
#include "ltrasmcut.h"

static int LtrasModuleCut1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf, int lexicon_type);
static int LtrasModuleCut1Token(void *context, struct og_ltrap_token *token);
static og_status getStartAndLengthPositions(struct og_ctrl_cut *ctrl_cut, struct og_ltra_trfs *trfs, struct og_ltra_trf *trf,
    struct og_ltra_add_trf_word *word, int start);

void *OgLtrasModuleCutInit(struct og_ltra_module_param *param)
{
  struct og_ctrl_cut *ctrl_cut = (struct og_ctrl_cut *) malloc(sizeof(struct og_ctrl_cut));
  IFn(ctrl_cut)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "OgLtrasInit: malloc error on ctrl_cut");
    OgErr(param->herr, erreur);
    return (0);
  }
  memset(ctrl_cut, 0, sizeof(struct og_ctrl_cut));

  ctrl_cut->herr = param->herr;
  ctrl_cut->hltras = param->hltras;
  ctrl_cut->hmutex = param->hmutex;
  ctrl_cut->cloginfo = param->loginfo;
  ctrl_cut->loginfo = &ctrl_cut->cloginfo;
  ctrl_cut->tinput = &ctrl_cut->ctinput;

  struct og_msg_param msg_param[1];
  memset(msg_param, 0, sizeof(struct og_msg_param));
  msg_param->herr = ctrl_cut->herr;
  msg_param->hmutex = ctrl_cut->hmutex;
  msg_param->loginfo.trace = DOgMsgTraceMinimal + DOgMsgTraceMemory;
  msg_param->loginfo.where = ctrl_cut->loginfo->where;
  msg_param->module_name = "ltra_module_cut";
  IFn(ctrl_cut->hmsg = OgMsgInit(msg_param))
  return (0);
  IF(OgMsgTuneInherit(ctrl_cut->hmsg, param->hmsg))
  return (0);

  if (param->hmodule_to_inherit)
  {
    struct og_ctrl_cut *inheriting_cut = (struct og_ctrl_cut *) param->hmodule_to_inherit;
    ctrl_cut->is_inherited = 1;
    ctrl_cut->ha_cut = inheriting_cut->ha_cut;
    ctrl_cut->max_small_word_length = inheriting_cut->max_small_word_length;
  }
  else
  {
    struct og_aut_param aut_param[1];
    memset(aut_param, 0, sizeof(struct og_aut_param));
    aut_param->herr = ctrl_cut->herr;
    aut_param->hmutex = ctrl_cut->hmutex;
    aut_param->loginfo.trace = DOgAutTraceMinimal + DOgAutTraceMemory;
    aut_param->loginfo.where = ctrl_cut->loginfo->where;
    aut_param->state_number = 0x1000;
    sprintf(aut_param->name, "ltras_module_cut");
    IFn(ctrl_cut->ha_cut = OgAutInit(aut_param))
    return (0);
    /* Setting default value to 3 because ltrap creates too many solutions.
     * In this case, ltrap uses too much memory and is too slow,
     * thus beeing close to a bug */
    ctrl_cut->max_small_word_length = 3;
    char *WorkingDirectory = OgLtrasWorkingDirectory(ctrl_cut->hltras);

    char ltras_cut[DPcPathSize];
    if (WorkingDirectory[0]) sprintf(ltras_cut, "%s/ling/ltras_cut.xml", WorkingDirectory);
    else strcpy(ltras_cut, "ling/ltras_cut.xml");

    og_status status = OgXmlXsdValidateFile(ctrl_cut->hmsg, ctrl_cut->herr, WorkingDirectory,
        ltras_cut, "ling/xsd/ltras_cut.xsd");
    IF(status) return (0);

    IF (LtrasModuleCutReadConf( ctrl_cut, ltras_cut))
    return (0);
  }

  struct og_ltrap_param ltrap_param[1];
  memset(ltrap_param, 0, sizeof(struct og_ltrap_param));
  ltrap_param->herr = ctrl_cut->herr;
  ltrap_param->hmsg = ctrl_cut->hmsg;
  ltrap_param->hmutex = ctrl_cut->hmutex;
  ltrap_param->loginfo.trace = DOgLtrapTraceMinimal + DOgLtrapTraceMemory;
  if (ctrl_cut->loginfo->trace & DOgLtrasTraceModuleCut)
  {
    ltrap_param->loginfo.trace |= DOgLtrapTraceAdd + DOgLtrapTraceBuild + DOgLtrapTraceSort + DOgLtrapTraceSend;
  }
  ltrap_param->loginfo.where = ctrl_cut->loginfo->where;
  ltrap_param->max_word_frequency = OgLtrasMaxWordFrequency(ctrl_cut->hltras);
  IFn(ltrap_param->ha_base = OgLtrasHaBase(ctrl_cut->hltras))
  return (0);
  IFn(ltrap_param->ha_swap = OgLtrasHaSwap(ctrl_cut->hltras))
  return (0);
  IFn(ltrap_param->ha_phon = OgLtrasHaPhon(ctrl_cut->hltras))
  return (0);
  ltrap_param->max_small_word_length = ctrl_cut->max_small_word_length;
  ltrap_param->ha_small_words = ctrl_cut->ha_cut;
  ltrap_param->ha_false = OgLtrasHaFalse(ctrl_cut->hltras);
  IFn(ctrl_cut->hltrap = OgLtrapInit(ltrap_param))
  return (0);

  ctrl_cut->hstm = OgLtrasHstm(ctrl_cut->hltras);
  IF(OgLtrasGetLevenshteinCosts(ctrl_cut->hltras, ctrl_cut->levenshtein_costs))
  return (0);

  // needed for ltrap but not used after for levenshtein*

  IF(OgStmGetSpaceCost(ctrl_cut->hstm, 0, TRUE, &ctrl_cut->cut_cost))
  return (0);

  return ctrl_cut;
}



int OgLtrasModuleCutFlush(void *handle)
{
struct og_ctrl_cut *ctrl_cut = (struct og_ctrl_cut *)handle;
IFE(OgLtrapFlush(ctrl_cut->hltrap));
if (!ctrl_cut->is_inherited) {
  IFE(OgAutFlush(ctrl_cut->ha_cut));
  }
IFE(OgMsgFlush(ctrl_cut->hmsg));
DPcFree(ctrl_cut);
DONE;
}



/*
 * the parameter max_nb_letters can only be zero or 1
 */
int OgLtrasModuleCut(struct og_ltra_module_input *module_input
    , struct og_ltra_trfs *input, struct og_ltra_trfs **output, ogint64_t *elapsed)
{
  struct og_ctrl_cut *ctrl_cut = (struct og_ctrl_cut *) module_input->handle;
  void *hltras = ctrl_cut->hltras;
  struct og_ltra_trfs *trfs;
  char buffer[DPcPathSize];
  int i, found, TrfUsed;
  char *value;

  ogint64_t micro_clock_start = OgMicroClock();

  /** Default is use basic automaton **/
  ctrl_cut->use_swap_automaton = 0;
  IFE(found=OgLtrasGetParameterValue(ctrl_cut->hltras,"cut_use_swap_automaton",DPcPathSize,buffer));
  if (found)
  {
    if (!Ogstricmp(buffer, "yes")) ctrl_cut->use_swap_automaton = 1;
    else if (!Ogstricmp(buffer, "no")) ctrl_cut->use_swap_automaton = 0;
  }
  if (module_input->argc > 1)
  {
    value = module_input->argv[1];
    if (!Ogstricmp(value, "yes")) ctrl_cut->use_swap_automaton = 1;
    else if (!Ogstricmp(value, "no")) ctrl_cut->use_swap_automaton = 0;
  }

  ctrl_cut->max_nb_solutions = 1;
  IFE(found=OgLtrasGetParameterValue(ctrl_cut->hltras,"cut_max_nb_solutions",DPcPathSize,buffer));
  if (found)
  {
    ctrl_cut->max_nb_solutions = atoi(buffer);
  }
  if (module_input->argc > 2)
  {
    ctrl_cut->max_nb_solutions = atoi(module_input->argv[2]);
  }

  /** This parameter is necessary to avoid combinatory explosions **/
  ctrl_cut->max_nb_candidates = 1000;
  IFE(found=OgLtrasGetParameterValue(ctrl_cut->hltras,"cut_max_nb_candidates",DPcPathSize,buffer));
  if (found)
  {
    ctrl_cut->max_nb_candidates = atoi(buffer);
  }
  if (module_input->argc > 3)
  {
    ctrl_cut->max_nb_candidates = atoi(module_input->argv[3]);
  }

  IFE(OgLtrasTrfsDuplicate(hltras, input, &trfs));
  TrfUsed = trfs->TrfUsed;
  ctrl_cut->trfs = trfs;
  *output = trfs;

  /** Cut with the basic automaton **/
  if (ctrl_cut->use_swap_automaton == 0)
  {
    for (i = 0; i < TrfUsed; i++)
    {
      IFE(LtrasModuleCut1(module_input,trfs,i,DOgLtrapLexiconTypeBase));
    }
  }
  else
  {
    /** Cut with the swap automaton **/
    for (i = 0; i < TrfUsed; i++)
    {
      IFE(LtrasModuleCut1(module_input,trfs,i,DOgLtrapLexiconTypeSwap));
    }
  }

  *elapsed = OgMicroClock() - micro_clock_start;

  DONE;

}




static int LtrasModuleCut1(struct og_ltra_module_input *module_input
  , struct og_ltra_trfs *trfs, int Itrf, int lexicon_type)
{
struct og_ctrl_cut *ctrl_cut = (struct og_ctrl_cut *)module_input->handle;
int ibuffer; unsigned char buffer[DPcPathSize];
struct og_ltrap_input cinput,*input=&cinput;
struct og_ltra_trf *trf = trfs->Trf+Itrf;
struct og_ltra_word *word;

/** we do not cut a transformation that is already cut **/
if (trf->nb_words > 1) DONE;
word = trfs->Word+trf->start_word;

if (ctrl_cut->loginfo->trace & DOgLtrasTraceModuleCut) {
  IFE(OgUniToCp(word->length,trfs->Ba+word->start,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  OgMsg(ctrl_cut->hmsg,"",DOgMsgDestInLog,"LtrasModuleCut1: working on '%s' with Itrf=%d",buffer,Itrf);
  }

memset(input,0,sizeof(struct og_ltrap_input));
input->lexicon_type = lexicon_type;
input->language = DOgLangNil;
input->iuni = word->length; input->uni = trfs->Ba+word->start;
input->send_token = LtrasModuleCut1Token; input->context = (void *)module_input;
input->synchronization_length = DOgLtrapSynchronizationLength;
input->max_candidates = ctrl_cut->max_nb_candidates;
input->max_solutions = ctrl_cut->max_nb_solutions;
/** Do we need to control that parameter at a higher level? **/
input->min_swap_word_length = DOgLtrapMinSwapWordLength;
input->score_factor = OgLtrasScoreFactor(ctrl_cut->hltras);
input->cut_cost = ctrl_cut->cut_cost;

ctrl_cut->iuni = input->iuni;
ctrl_cut->uni = input->uni;
ctrl_cut->Itrf = Itrf;

IFE(OgLtrap(ctrl_cut->hltrap,input));

DONE;
}




static int LtrasModuleCut1Token(void *context, struct og_ltrap_token *token)
{
struct og_ltra_module_input *module_input = (struct og_ltra_module_input *)context;
struct og_ctrl_cut *ctrl_cut = (struct og_ctrl_cut *)module_input->handle;
struct og_ltra_trf *trf = ctrl_cut->trfs->Trf+ctrl_cut->Itrf;
struct og_ltra_add_trf_input *tinput=ctrl_cut->tinput;
int to_length; unsigned char to[DPcPathSize];
struct og_ltra_trfs *trfs = ctrl_cut->trfs;
struct og_ltra_add_trf_word *new_word;
void *hltras = ctrl_cut->hltras;
struct og_ltra_word *word;
int i,is_false;

if (!token->solution_complete) DONE;

if (token->starting) {
  memset(tinput,0,sizeof(struct og_ltra_add_trf_input));
  ctrl_cut->new_words_length=0;
  ctrl_cut->removed_score=0;
  }

// Check if can add more words
if ((tinput->nb_words + 1) >= DOgLtrasAddTrfMaxNbWords) DONE;

//Keep original string of trfs to compare with transmormation string and
//to calculate a levenchtein score
unsigned char origin[DPcPathSize];
int origin_length = 0;
IFE(OgLtrasTrfGetOriginalString(hltras, ctrl_cut->trfs, ctrl_cut->Itrf, origin, DPcPathSize, &origin_length));

word = ctrl_cut->trfs->Word+trf->start_word;

memcpy(ctrl_cut->new_words+ctrl_cut->new_words_length,token->string,token->length_string);
new_word = tinput->word+tinput->nb_words;
new_word->string = ctrl_cut->new_words+ctrl_cut->new_words_length;
new_word->string_length = token->length_string;
new_word->frequency = token->frequency;
new_word->base_frequency = word->base_frequency;
new_word->start_position = token->position;
new_word->length_position = token->length;
new_word->language = word->language;

IFE(getStartAndLengthPositions(ctrl_cut, trfs, trf, new_word, word->start_position));

ctrl_cut->new_words_length+=token->length_string;
tinput->nb_words++;

if (token->modified) ctrl_cut->removed_score+=0.05;
else ctrl_cut->removed_score+=0.025;

to_length=0;
if (token->ending) {
  //IFE(LtrasModuleCut1Adjust(module_input));
  tinput->start = trf->start;
  tinput->length = trf->length;
  tinput->module_id = module_input->id;
  tinput->from_trf = ctrl_cut->Itrf;
  tinput->final = 1;
  for (i=0; i<tinput->nb_words; i++) {
    new_word = tinput->word + i;
    if (i>0) {
      memcpy(to+to_length,"\0 ",2); to_length+=2;
      }
    memcpy(to+to_length,new_word->string,new_word->string_length); to_length+=new_word->string_length;
    }
  IFE(is_false=OgLtrasIsFalseTransformation(hltras, word->length_position, trfs->Ba+word->start_position, to_length, to));
  if (!is_false) {
    double dlevenshtein_distance;
    IFE(dlevenshtein_distance = OgStmLevenshteinFast(ctrl_cut->hstm, origin_length, origin, to_length,
        to, ctrl_cut->levenshtein_costs));
    tinput->score = 1.0 - dlevenshtein_distance;
    IFE(OgLtrasTrfAdd(hltras,ctrl_cut->trfs,tinput,0));
    }
  }

DONE;
}

// Fill start and length positions. If previous trf is a paste transformation, we use the transposition heap to
//compute start_position (start in the original string) and length_position (length in the original string)
static og_status getStartAndLengthPositions(struct og_ctrl_cut *ctrl_cut, struct og_ltra_trfs *trfs, struct og_ltra_trf *trf,
    struct og_ltra_add_trf_word *word, int start)
{
  int transposition_used = OgHeapGetCellsUsed(trfs->htransposition);
  int *transposition = OgHeapGetCell(trfs->htransposition, trf->start_transposition);
  IFn(transposition) DPcErr;

  int end_position = word->start_position + word->length_position - 2;
  if (trf->length_transposition > 0)
  {
    if((end_position / 2) > transposition_used)
    {
      char erreur[DOgErrorSize];
       sprintf(erreur,"getStartAndLengthPositions: end_position %d is greater than transposition_used %d",
           end_position, transposition_used);
       OgErr(ctrl_cut->herr,erreur);
       DPcErr;
    }
    word->start_position = transposition[word->start_position / 2] * 2;
    int end_transposition = transposition[end_position / 2] * 2 + 2;
    word->length_position = end_transposition - word->start_position;
  }
  else
  {
    word->start_position += start;
  }

  DONE;
}
