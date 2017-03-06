/*
 *  Handling solutions
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : October 2009
 *  Version 1.0
*/
#include "ogm_ltrap.h"


static int LtrapCompleteSolution(struct og_ctrl_ltrap *ctrl_ltrap,int Isol);
static int LtrapWeightSolution(struct og_ctrl_ltrap *ctrl_ltrap,int Isol);
static int LtrapLimitSolution(struct og_ctrl_ltrap *ctrl_ltrap,int limit,int Isol);
static int LtrapSendSolution(struct og_ctrl_ltrap *ctrl_ltrap, int iuni, unsigned char *uni, int Isol);
static int LtrapMarkSpan(struct og_ctrl_ltrap *ctrl_ltrap, int Isol);
static int sol_cmp(const void *ptr1, const void *ptr2);



/*
 * In order to handle combinatory explosion, the max_candidates is used.
 * When a given number of solutions is created, we return 1.
*/

int OgLtrapBuildSolution(struct og_ctrl_ltrap *ctrl_ltrap)
{
struct sequence *sequence;
struct span *span;
int Isol;
int i;

IFn(ctrl_ltrap->SequenceUsed) return(0);

if (ctrl_ltrap->max_candidates && ctrl_ltrap->SolUsed >= ctrl_ltrap->max_candidates) {
  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceBuild) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "OgLtrapBuildSolution: SolUsed (%d) >= max_candidates (%d)", ctrl_ltrap->SolUsed, ctrl_ltrap->max_candidates);
    }
  return(1);
  }

IFE(Isol=LtrapSolAdd(ctrl_ltrap));

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceBuild) {
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "OgLtrapBuildSolution: adding spans for Isol=%d:",Isol);
  }

for (i=0; i<ctrl_ltrap->SequenceUsed; i++) {
  sequence = ctrl_ltrap->Sequence + i;
  span = ctrl_ltrap->Span + sequence->Ispan;
  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceBuild) {
    int ib1,ib2; unsigned char b1[DPcPathSize],b2[DPcPathSize],b3[DPcPathSize];
    IFE(OgUniToCp(span->length, ctrl_ltrap->input->uni+span->position, DPcPathSize, &ib1, b1, DOgCodePageUTF8, 0, 0));
    if (span->length_word > 0) {
      IFE(OgUniToCp(span->length_word, ctrl_ltrap->Ba+span->start_word, DPcPathSize, &ib2, b2, DOgCodePageUTF8, 0, 0));
      sprintf(b3," '%s'",b2);
      }
    else b3[0]=0;
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "%3d: '%s' Ispan=%d pos=%d length=%d freq=%d%s"
      , i, b1, sequence->Ispan, span->position
      , span->length, span->frequency, b3);
    }
  IFE(LtrapSpansolAdd(ctrl_ltrap,Isol,sequence->Ispan,span->position,span->length,span->frequency));
  }

return(0);
}




int OgLtrapSortSolutions(struct og_ctrl_ltrap *ctrl_ltrap,int is_end,int *pstart)
{
struct spansol *spansol;
struct sol *sol;
int i,start;

IFn(ctrl_ltrap->SolUsed) DONE;

for (i=0; i<ctrl_ltrap->SolUsed; i++) {
  IFE(LtrapCompleteSolution(ctrl_ltrap,i));
  IFE(LtrapWeightSolution(ctrl_ltrap,i));
  }

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceSort) {
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "OgLtrapSortSolutions: sorting %d solutions",ctrl_ltrap->SolUsed);
  }

qsort(ctrl_ltrap->Sol,ctrl_ltrap->SolUsed,sizeof(struct sol),sol_cmp);

sol = ctrl_ltrap->Sol + 0;
//kept_words = ctrl_ltrap->kept_words;
//if (kept_words > sol->length_spansol) kept_words = sol->length_spansol;
spansol = ctrl_ltrap->Spansol + sol->start_spansol + sol->length_spansol - 1;

start = spansol->position + spansol->length;

if (!is_end) {
  for (i=0; i<ctrl_ltrap->SolUsed; i++) {
    IFE(LtrapLimitSolution(ctrl_ltrap,start,i));
    }
  }

*pstart = start;

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceSort) {
  IFE(LtrapSolsLog(ctrl_ltrap));
  }

DONE;
}





static int LtrapCompleteSolution(struct og_ctrl_ltrap *ctrl_ltrap,int Isol)
{
struct sol *sol = ctrl_ltrap->Sol + Isol;
struct spansol *spansol;
int start,end;

spansol = ctrl_ltrap->Spansol + sol->start_spansol + 0;
start = spansol->position;

spansol = ctrl_ltrap->Spansol + sol->start_spansol + sol->length_spansol - 1;
end = spansol->position + spansol->length;

if (start==0 &&  end==ctrl_ltrap->input->iuni) sol->complete=1;
else sol->complete=0;

DONE;
}





static int LtrapWeightSolution(struct og_ctrl_ltrap *ctrl_ltrap,int Isol)
{
double score_factor = ctrl_ltrap->score_factor;
struct sol *sol = ctrl_ltrap->Sol + Isol;
double global_frequency_log10;
double normalized_frequency;
struct spansol *spansol;
struct span *span;
int i;

/** Calculating the frequency of the whole solution **/
sol->global_frequency=0;
for (i=0; i<sol->length_spansol; i++) {
  spansol = ctrl_ltrap->Spansol + sol->start_spansol + i;
  if (i==0) sol->global_frequency = spansol->frequency;
  else if (sol->global_frequency > spansol->frequency) sol->global_frequency = spansol->frequency;
  }

/* Calculating the Levenshtein distance as a score.
 * This score can be easily calculated because
 * when there is a different word there is a difference of one letter */
sol->global_score=1.0;
for (i=0; i<sol->length_spansol; i++) {
  spansol = ctrl_ltrap->Spansol + sol->start_spansol + i;
  span = ctrl_ltrap->Span + spansol->Ispan;
  if (span->length_word > 0) sol->global_score -= 0.1;
  /* A space counts for two letters (default value for cut_price 0.2)
   * it is better than half a letter (0.05)
   * or one letter (0.1) */
  if (i) sol->global_score -= ctrl_ltrap->cut_cost;
  }
if (sol->global_score<0) sol->global_score=0;

/** Same formula as in ogm_ltras **/
global_frequency_log10 = log10(sol->global_frequency);
normalized_frequency = global_frequency_log10/ctrl_ltrap->max_word_frequency_log10;
sol->weight = score_factor*sol->global_score
            + (1-score_factor)*normalized_frequency;

DONE;
}




static int LtrapLimitSolution(struct og_ctrl_ltrap *ctrl_ltrap,int limit,int Isol)
{
struct sol *sol = ctrl_ltrap->Sol + Isol;
struct spansol *spansol;
int i;

for (i=0; i<sol->length_spansol; i++) {
  spansol = ctrl_ltrap->Spansol + sol->start_spansol + i;
  if (spansol->position + spansol->length > limit) {
    sol->length_spansol = i; break;
    }
  }

DONE;
}





int LtrapSendSolutions(struct og_ctrl_ltrap *ctrl_ltrap, int iuni, unsigned char *uni)
{
struct og_ltrap_token cltrap_token,*ltrap_token=&cltrap_token;
int ibuffer; unsigned char buffer[DPcPathSize];
struct span *span;
int max_solutions;
int i;

IFn(ctrl_ltrap->SolUsed) DONE;

max_solutions = ctrl_ltrap->max_solutions;
if (max_solutions > ctrl_ltrap->SolUsed) max_solutions = ctrl_ltrap->SolUsed;

for (i=0; i<ctrl_ltrap->SpanUsed; i++) {
  span = ctrl_ltrap->Span + i;
  span->selected=0;
  }

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceSend) {
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "LtrapSendSolutions: sending following spans:");
  }

if (ctrl_ltrap->send_spans_by_solution) {
  for (i=0; i<max_solutions; i++) {
    IFE(LtrapSendSolution(ctrl_ltrap,iuni,uni,i));
    }
  }
else {
  for (i=0; i<max_solutions; i++) {
    IFE(LtrapMarkSpan(ctrl_ltrap,i));
    }

  for (i=0; i<ctrl_ltrap->SpanUsed; i++) {
    span = ctrl_ltrap->Span + i;
    if (!span->selected) continue;
    if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceSend) {
      IFE(OgUniToCp(span->length, ctrl_ltrap->input->uni+span->position, DPcPathSize
          , &ibuffer, buffer, DOgCodePageUTF8, 0, 0));
      OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
        , "%3d: '%s' Isol=%d pos=%d length=%d freq=%d"
        , i, buffer, span->Isol, span->position, span->length, span->frequency);
      }
    ltrap_token->solution_number = span->Isol;
    ltrap_token->global_frequency = ctrl_ltrap->Sol[span->Isol].global_frequency;
    ltrap_token->global_score = ctrl_ltrap->Sol[span->Isol].global_score;
    ltrap_token->solution_complete = ctrl_ltrap->Sol[span->Isol].complete;
    ltrap_token->position = span->position;
    ltrap_token->length = span->length;
    ltrap_token->string = buffer;
    ltrap_token->starting=0; /* Not applicable */
    ltrap_token->ending=0; /* Not applicable */
    ltrap_token->modified=0;
    ltrap_token->frequency=span->frequency;
    memcpy(buffer,uni+ltrap_token->position,ltrap_token->length); buffer[ltrap_token->length]=0;
    IFE(ctrl_ltrap->send_token(ctrl_ltrap->context,ltrap_token));
    }
  }

DONE;
}




static int LtrapSendSolution(struct og_ctrl_ltrap *ctrl_ltrap, int iuni, unsigned char *uni, int Isol)
{
struct og_ltrap_token cltrap_token,*ltrap_token=&cltrap_token;
int ib1,ib2; unsigned char b1[DPcPathSize],b2[DPcPathSize];
struct sol *sol = ctrl_ltrap->Sol + Isol;
unsigned char buffer[DPcPathSize];
struct spansol *spansol;
struct span *span;
int i;

for (i=0; i<sol->length_spansol; i++) {
  spansol = ctrl_ltrap->Spansol + sol->start_spansol + i;
  span = ctrl_ltrap->Span + spansol->Ispan;
  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceSend) {
    IFE(OgUniToCp(spansol->length, ctrl_ltrap->input->uni+spansol->position, DPcPathSize, &ib1, b1, DOgCodePageUTF8, 0, 0));
    if (span->start_word >= 0) {
      sprintf(b2," '");
      IFE(OgUniToCp(span->length_word, ctrl_ltrap->Ba+span->start_word, DPcPathSize, &ib2, b2+2, DOgCodePageUTF8, 0, 0));
      sprintf(b2+strlen(b2),"'");
      }
    else b2[0]=0;
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "%2d: '%s' pos=%d length=%d freq=%d%s"
      , Isol, b1, spansol->position, spansol->length, spansol->frequency,b2);
    }
  ltrap_token->solution_number = Isol;
  ltrap_token->global_frequency = ctrl_ltrap->Sol[Isol].global_frequency;
  ltrap_token->global_score = ctrl_ltrap->Sol[Isol].global_score;
  ltrap_token->solution_complete = ctrl_ltrap->Sol[Isol].complete;
  ltrap_token->position = spansol->position;
  ltrap_token->length = spansol->length;
  ltrap_token->string = buffer;
  if (i==0) ltrap_token->starting=1; else ltrap_token->starting=0;
  if (i==sol->length_spansol-1) ltrap_token->ending=1; else ltrap_token->ending=0;
  if (span->start_word >= 0) {
    memcpy(buffer,ctrl_ltrap->Ba+span->start_word,span->length_word); buffer[ltrap_token->length]=0;
    ltrap_token->length_string = span->length_word;
    ltrap_token->modified=1;
    }
  else {
    memcpy(buffer,uni+ltrap_token->position,ltrap_token->length); buffer[ltrap_token->length]=0;
    ltrap_token->length_string = ltrap_token->length;
    ltrap_token->modified=0;
    }
  ltrap_token->frequency=spansol->frequency;
  IFE(ctrl_ltrap->send_token(ctrl_ltrap->context,ltrap_token));
  }
DONE;

}





static int LtrapMarkSpan(struct og_ctrl_ltrap *ctrl_ltrap, int Isol)
{

struct sol *sol = ctrl_ltrap->Sol + Isol;
struct spansol *spansol;
struct span *span;
int i;

for (i=0; i<sol->length_spansol; i++) {
  spansol = ctrl_ltrap->Spansol + sol->start_spansol + i;
  span = ctrl_ltrap->Span + spansol->Ispan;
  span->selected = 1;
  span->Isol = Isol;
  }

DONE;
}




static int sol_cmp(const void *ptr1, const void *ptr2)
{
struct sol *sol1 = (struct sol *)ptr1;
struct sol *sol2 = (struct sol *)ptr2;
double dcmp;
int cmp;
dcmp = sol2->weight - sol1->weight;
if (dcmp > 0) return(1);
else if (dcmp < 0) return(-1);
/** Making sure the solutions are always ordered the same way **/
cmp = sol1->start_spansol - sol2->start_spansol;
return(cmp);
}



