/*
 *  Handling XML results file
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev: February 2010
 *  Version 1.0
*/
#include "ogm_ltras.h"


static int OgLtrasTrfsAddResult1(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf, int send_result);
static int OgLtrasTrfsNbSuggestions(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf);





PUBLIC(og_status) OgLtrasTrfsAddResult(void *handle, struct og_ltra_trfs *trfs, int send_result)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
int ibuffer; unsigned char buffer[DPcPathSize];
unsigned char snb_nb_suggestions[DPcPathSize];
char output_file_tmp[DPcPathSize];
int i,iuni; unsigned char *uni;
int found,nb_suggestions;

if (ctrl_ltras->first_added_result) {
  ctrl_ltras->first_added_result=0;
  sprintf(output_file_tmp,"%s.tmp",ctrl_ltras->output_file);
  IFn(ctrl_ltras->fdout=fopen(output_file_tmp,"w")) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      ,"OgLtrasTrfsAddResult: impossible to fopen '%s', results will not be built"
      ,output_file_tmp);
    }
  }
IFn(ctrl_ltras->fdout) DONE;

if (send_result) {
  snb_nb_suggestions[0]=0; nb_suggestions=0;
  if (ctrl_ltras->nb_suggestions>0) {
    for (i=0; i<trfs->TrfUsed; i++) {
      IFE(found=OgLtrasTrfsNbSuggestions(ctrl_ltras,trfs,i));
      if (found) nb_suggestions++;
      }
    sprintf(snb_nb_suggestions," nb_suggestions=\"%d\"",nb_suggestions);
    }
  fprintf(ctrl_ltras->fdout,
    "  <linguistic_transformation%s>\n",snb_nb_suggestions);
  uni=trfs->Ba+trfs->start_text; iuni=trfs->length_text;
  IFE(OgUniToCp(iuni,uni,DPcPathSize,&ibuffer,buffer,DOgCodePageUTF8,0,0));
  fprintf(ctrl_ltras->fdout,
    "    <request>%s</request>\n"
    ,buffer);
  }

ctrl_ltras->current_trf=0;
ctrl_ltras->suggestion_found=0;
for (i=0; i<trfs->TrfUsed; i++) {
  IFE(OgLtrasTrfsAddResult1(ctrl_ltras,trfs,i,send_result));
  }

if (send_result) {
  fprintf(ctrl_ltras->fdout,
    "  </linguistic_transformation>\n");
  }

ctrl_ltras->statistics.nb_requests++;

DONE;
}





static int OgLtrasTrfsAddResult1(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf, int send_result)
{
  int iunitext;
  unsigned char unitext[DPcPathSize];
  int isword;
  unsigned char sword[DPcPathSize];
  struct og_ltra_trf *trf = trfs->Trf + Itrf;
  unsigned char ssuggestion[DPcPathSize];
  unsigned char slanguage[DPcPathSize];
  unsigned char text[DPcPathSize];
  struct og_ltra_module *module;
  struct og_ltra_word *word;
  int i, iglobal_score, found;
  int suggestion_number;
  double dglobal_score;

  if (!trf->final) DONE;
  if (!trf->total) DONE;

  ctrl_ltras->statistics.mean_range.final_score += trf->final_score;
  ctrl_ltras->statistics.mean_range.score += trf->global_score;
  ctrl_ltras->statistics.mean_range.word_frequency += trf->word_frequency;
  ctrl_ltras->statistics.mean_range.expression_frequency += trf->expression_frequency;
  ctrl_ltras->statistics.nb_trfs++;

  int max = DOgLtrasScorePrecision;
  dglobal_score = trf->global_score * max;
  iglobal_score = (int) round(dglobal_score);
  if (0 <= iglobal_score && iglobal_score <= max)
  {
    ctrl_ltras->statistics.range[iglobal_score].nb_trfs += 1;
    ctrl_ltras->statistics.range[iglobal_score].word_frequency += trf->word_frequency;
    ctrl_ltras->statistics.range[iglobal_score].expression_frequency += trf->expression_frequency;
  }

  iunitext = 0;
  for (i = 0; i < trf->nb_words; i++)
  {
    word = trfs->Word + trf->start_word + i;
    if (i > 0)
    {
      unitext[iunitext++] = 0;
      unitext[iunitext++] = ' ';
    }
    memcpy(unitext + iunitext, trfs->Ba + word->start, word->length);
    iunitext += word->length;
  }
  text[0] = 0;
  for (i = 0; i < trf->nb_words; i++)
  {
    word = trfs->Word + trf->start_word + i;
    IFE(LtrasWordString(ctrl_ltras, trfs, trf->start_word + i, &isword, sword));
    sprintf(text + strlen(text), "%s%s", (i ? " " : ""), sword);
  }
  ssuggestion[0] = 0;
  IFE(found = LtrasSuggestionGet(ctrl_ltras, iunitext, unitext, &suggestion_number));
  if (found)
  {
    sprintf(ssuggestion, " suggestion=\"%d\" position=\"%d\"", suggestion_number, ctrl_ltras->current_trf);
    if (0 <= ctrl_ltras->current_trf && ctrl_ltras->current_trf < DOgLtrasSuggestionSize)
    {
      /** Registering only first suggestion for statistics **/
      if (!ctrl_ltras->suggestion_found)
      {
        ctrl_ltras->suggestion[ctrl_ltras->current_trf].nb_propositions++;
        ctrl_ltras->suggestion[ctrl_ltras->current_trf].mean_final_score += trf->final_score;
        ctrl_ltras->suggestion[ctrl_ltras->current_trf].mean_score += trf->global_score;
        ctrl_ltras->suggestion[ctrl_ltras->current_trf].mean_word_frequency += trf->word_frequency;
        ctrl_ltras->suggestion[ctrl_ltras->current_trf].mean_expression_frequency += trf->expression_frequency;
        ctrl_ltras->suggestion[ctrl_ltras->current_trf].operations[ctrl_ltras->current_op]++;
        ctrl_ltras->nb_suggestion_levels++;
        ctrl_ltras->suggestion_found = 1;
      }
    }
  }
  slanguage[0] = 0;
  if (trf->language)
  {
    sprintf(slanguage, "language=\"%s\" ", OgIso639ToCode(trf->language));
  }

  ctrl_ltras->current_trf++;

  if (!send_result) DONE;

  fprintf(ctrl_ltras->fdout,
      "    <proposition %sfinal_score=\"%.4f\" score=\"%.4f\" expression_frequency=\"%d\" word_frequency=\"%d\"%s>\n"
      , slanguage, trf->final_score, trf->global_score, trf->expression_frequency, trf->word_frequency, ssuggestion);

  fprintf(ctrl_ltras->fdout,
      "      <text>%s</text>\n", text);

  fprintf(ctrl_ltras->fdout,
      "      <words>\n");

  unsigned char spositions[DPcPathSize];
  for (i = 0; i < trf->nb_words; i++)
  {
    word = trfs->Word + trf->start_word + i;
    IFE(LtrasWordString(ctrl_ltras, trfs, trf->start_word + i, &isword, sword));
    slanguage[0] = 0;
    if (word->language)
    {
      sprintf(slanguage, "language=\"%s\" ", OgIso639ToCode(word->language));
    }
    spositions[0] = 0;
    if(OgLtrasScoreFactorIsLogPosActivated(ctrl_ltras))
    {
      sprintf(spositions, " startpos=\"%d\" lengthpos=\"%d\"", word->start_position, word->length_position);
    }
    fprintf(ctrl_ltras->fdout,
        "        <word %sfrequency=\"%d\"%s>%s</word>\n"
        , slanguage, word->frequency, spositions, sword);
  }
  fprintf(ctrl_ltras->fdout,
      "      </words>\n");

  if (trf->nb_modules > 0)
  {
    fprintf(ctrl_ltras->fdout,
        "      <modules>\n");
    for (i = 0; i < trf->nb_modules; i++)
    {
      module = trfs->Module + trf->start_module + i;
      fprintf(ctrl_ltras->fdout,
          "        <module>%s</module>\n"
          , OgLtrasModuleName(ctrl_ltras, module->module_id));
    }
    fprintf(ctrl_ltras->fdout,
        "      </modules>\n");
  }

  fprintf(ctrl_ltras->fdout,
      "    </proposition>\n");

  DONE;
}





static int OgLtrasTrfsNbSuggestions(struct og_ctrl_ltras *ctrl_ltras, struct og_ltra_trfs *trfs, int Itrf)
{
int iunitext; unsigned char unitext[DPcPathSize];
int isword; unsigned char sword[DPcPathSize];
struct og_ltra_trf *trf = trfs->Trf + Itrf;
unsigned char text[DPcPathSize];
struct og_ltra_word *word;
int suggestion_number;
int i,found;

if (!trf->final) DONE;

iunitext=0;
for (i=0; i<trf->nb_words; i++) {
  word = trfs->Word + trf->start_word + i;
  if (i>0) { unitext[iunitext++]=0; unitext[iunitext++]=' '; }
  memcpy(unitext+iunitext,trfs->Ba+word->start,word->length);
  iunitext+=word->length;
  }
text[0]=0;
for (i=0; i<trf->nb_words; i++) {
  word = trfs->Word + trf->start_word + i;
  IFE(LtrasWordString(ctrl_ltras,trfs,trf->start_word+i,&isword,sword));
  sprintf(text+strlen(text),"%s%s",(i?" ":""),sword);
  }

IFE(found=LtrasSuggestionGet(ctrl_ltras,iunitext,unitext,&suggestion_number));
return(found);
}






PUBLIC(og_status) OgLtrasTrfsConsolidateResults(void *handle, int send_result)
{
struct og_ctrl_ltras *ctrl_ltras = (struct og_ctrl_ltras *)handle;
double score,mean_score,mean_final_score;
char output_file_tmp[DPcPathSize];
unsigned char line[DPcPathSize];
int i;
ogint64_t mean;
FILE *fd;
unsigned char *c;
unsigned char out[DPcAutMaxBufferSize+9],buffer[DPcAutMaxBufferSize+9];
oindex states[DPcAutMaxBufferSize+9];
int nstate0,nstate1,iout,ibuffer;
int operation_number;

IFn(ctrl_ltras->fdout) DONE;
fclose(ctrl_ltras->fdout);

IFn(fd=fopen(ctrl_ltras->output_file,"w")) {
  OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
    ,"OgLtrasTrfsConsolidateResults: impossible to fopen '%s', results will not be built"
    ,ctrl_ltras->output_file);
  }

fprintf(fd,
  "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
  "<ogltras>\n");

fprintf(fd,
  "<statistics flow_chart=\"%s\" score_factor=\"%.2f\" frequency_ratio=\"%.2f\">\n"
  , ctrl_ltras->input->flow_chart, ctrl_ltras->input->score_factor, ctrl_ltras->input->frequency_ratio);

fprintf(fd,
  "  <elapsed>\n");
ctrl_ltras->statistics.global_elapsed = OgMicroClock()-ctrl_ltras->statistics.global_start;
mean = ctrl_ltras->statistics.global_elapsed / ctrl_ltras->statistics.nb_requests;

fprintf(fd,
  "    <global elapsed=\"%lld\" nb_requests=\"%d\" mean=\"%lld\"/>\n"
  , ctrl_ltras->statistics.global_elapsed
  , ctrl_ltras->statistics.nb_requests
  , mean);

mean = ctrl_ltras->statistics.total_elapsed / ctrl_ltras->statistics.nb_requests;

fprintf(fd,
  "    <local total_elapsed=\"%lld\" min=\"%lld\" mean=\"%lld\" max=\"%lld\"/>\n"
  , ctrl_ltras->statistics.total_elapsed
  , ctrl_ltras->statistics.min_elapsed, mean
  , ctrl_ltras->statistics.max_elapsed);

for (int j = 0; j < ctrl_ltras->ModuleUsed; j++)
{
  struct module *module = ctrl_ltras->Module + j;
  if (module->elapsed > 0)
  {
    ogint64_t mean_elapsed = module->elapsed / module->nb_requests;
    fprintf(fd, "    <module module_id=\"%s\" total_elapsed=\"%lld\" mean=\"%lld\"/>\n"
        , OgLtrasModuleName(ctrl_ltras, j), module->elapsed, mean_elapsed);
  }
}

fprintf(fd,
  "  </elapsed>\n");

IFx(ctrl_ltras->statistics.nb_trfs) {
  mean_final_score = ctrl_ltras->statistics.mean_range.final_score / ctrl_ltras->statistics.nb_trfs;
  mean_score = ctrl_ltras->statistics.mean_range.score / ctrl_ltras->statistics.nb_trfs;
  int mean_word_frequency = ctrl_ltras->statistics.mean_range.word_frequency / ctrl_ltras->statistics.nb_trfs;
  int mean_expression_frequency = ctrl_ltras->statistics.mean_range.expression_frequency / ctrl_ltras->statistics.nb_trfs;

  fprintf(fd,
    "  <ranges mean_final_score=\"%.4f\" mean_score=\"%.4f\" mean_expression_frequency=\"%d\" mean_word_frequency=\"%d\" nb_trfs=\"%d\">\n"
    , mean_final_score, mean_score, mean_expression_frequency, mean_word_frequency, ctrl_ltras->statistics.nb_trfs);

  int max = DOgLtrasScorePrecision;
  for (i=max; i>=0; i--) {
    if (!ctrl_ltras->statistics.range[i].word_frequency) continue;
    int nb_trfs = (int)ctrl_ltras->statistics.range[i].nb_trfs;
    mean_word_frequency = ctrl_ltras->statistics.range[i].word_frequency / nb_trfs;
    mean_expression_frequency = ctrl_ltras->statistics.range[i].expression_frequency / nb_trfs;
    score = i; score /= max;
    fprintf(fd,
      "    <range total_trfs=\"%d\" score=\"%.4f\" mean_expression_frequency=\"%d\" mean_word_frequency=\"%d\"/>\n"
      , nb_trfs, score, mean_expression_frequency, mean_word_frequency);
    }
  fprintf(fd,
    "  </ranges>\n");
  }

if (ctrl_ltras->nb_suggestion_levels>0) {
  int cumul_nb_propositions=0;
  double benchmark;
  fprintf(fd,
    "  <suggestions>\n");
  for (i=0; i<DOgLtrasSuggestionSize; i++) {
    if (ctrl_ltras->suggestion[i].nb_propositions<=0) continue;
    cumul_nb_propositions += ctrl_ltras->suggestion[i].nb_propositions;
    benchmark = cumul_nb_propositions*100; benchmark /= ctrl_ltras->statistics.nb_requests;
    fprintf(fd,
      "    <suggestion position=\"%d\" benchmark=\"%.2f\" nb_propositions=\"%d\" mean_final_score=\"%.4f\" mean_score=\"%.4f\" mean_expression_frequency=\"%.2f\" mean_word_frequency=\"%.2f\">\n"
        , i, benchmark, ctrl_ltras->suggestion[i].nb_propositions
        , ctrl_ltras->suggestion[i].mean_final_score / ctrl_ltras->suggestion[i].nb_propositions
        , ctrl_ltras->suggestion[i].mean_score / ctrl_ltras->suggestion[i].nb_propositions
        , ctrl_ltras->suggestion[i].mean_expression_frequency / ctrl_ltras->suggestion[i].nb_propositions
        , ctrl_ltras->suggestion[i].mean_word_frequency / ctrl_ltras->suggestion[i].nb_propositions);

    memset(buffer,0,DPcPathSize);
    ibuffer=0;
    if (OgAutScanf(ctrl_ltras->ha_oper,ibuffer,buffer,&iout,out,&nstate0,&nstate1,states)) {
      do {
        c=strchr(out,'|');
      if(!c) {
        OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
            ,"OgLtrasTrfsConsolidateResults: not a proper line in automaton '%.*s'"
        ,iout,out);
        }
      else{
      c[0]=0;  // remove '|' for pretty print
      c++;
        IFE(DOgPnin4(ctrl_ltras->herr,&c,&operation_number));
          if(ctrl_ltras->suggestion[i].operations[operation_number]>0){
            int length = c-out-1;
            fprintf(fd,
            "      <operation id=\"%.*s\" nb=\"%d\"/>\n",length,out,ctrl_ltras->suggestion[i].operations[operation_number]);
          }
      }
      }
    while(OgAutScann(ctrl_ltras->ha_oper,&iout,out,nstate0,&nstate1,states));
    }
    fprintf(fd,"    </suggestion>\n");
    }
  fprintf(fd,"  </suggestions>\n");
  }

fprintf(fd,
  "</statistics>\n");

if (send_result) {

  fprintf(fd,
    "<linguistic_transformations flow_chart=\"%s\" score_factor=\"%.2f\">\n"
    , ctrl_ltras->input->flow_chart, ctrl_ltras->input->score_factor);

  sprintf(output_file_tmp,"%s.tmp",ctrl_ltras->output_file);
  IFn(ctrl_ltras->fdout=fopen(output_file_tmp,"r")) {
    OgMsg(ctrl_ltras->hmsg,"",DOgMsgDestInLog
      ,"OgLtrasTrfsConsolidateResults: impossible to fopen '%s' for reading"
      ,output_file_tmp);
    }
  else {
    while (fgets(line,DPcPathSize,ctrl_ltras->fdout)) {
      fprintf(fd,"%s",line);
      }
    fclose(ctrl_ltras->fdout);
    }
  remove(output_file_tmp);
  fprintf(fd,
    "</linguistic_transformations>\n");
  }

fprintf(fd,
  "</ogltras>\n");
fclose(fd);

DONE;
}





int LtrasWordString(struct og_ctrl_ltras *ctrl_ltras,struct og_ltra_trfs *trfs,int Iword,int *piword,unsigned char *sword)
{
struct og_ltra_word *word = trfs->Word + Iword;
IFE(OgUniToCp(word->length,trfs->Ba+word->start,DPcPathSize,piword,sword,DOgCodePageUTF8,0,0));
DONE;
}


