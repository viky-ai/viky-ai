/*
 *  Initialisation functions for Linguistic transformation parse library
 *  Copyright (c) 2010 Pertimm by Patrick Constant
 *  Dev : January 2010
 *  Version 1.0
*/
#include "ogm_ltrap.h"



static int OgLtrapAddSpan(struct og_ctrl_ltrap *ctrl_ltrap, struct og_ltrap_input *input, int iuni, unsigned char *uni,
    int icurrent);
static int OgLtrapSendSequence(struct og_ctrl_ltrap *ctrl_ltrap,int iuni,unsigned char *uni,int start,int length);
static int OgLtrapBuildSequence(struct og_ctrl_ltrap *ctrl_ltrap,int iuni,unsigned char *uni,int start,int length);
static int OgLtrapBuildSeq(struct og_ctrl_ltrap *ctrl_ltrap,int iuni,unsigned char *uni
  ,int start,int length,int icurrent,int Isequence);




int OgLtrap(void *handle, struct og_ltrap_input *input)
{
struct og_ctrl_ltrap *ctrl_ltrap = (struct og_ctrl_ltrap *)handle;
int iuni=input->iuni; unsigned char *uni=input->uni;
int i,c,start,overlapped;

IFE(LtrapReset(ctrl_ltrap));

ctrl_ltrap->input = input;
ctrl_ltrap->send_spans_by_solution = 1;

ctrl_ltrap->context = input->context;
ctrl_ltrap->send_token = input->send_token;
ctrl_ltrap->score_factor = input->score_factor;
ctrl_ltrap->cut_cost = input->cut_cost;

if (input->synchronization_length <= 0) ctrl_ltrap->synchronization_length = DOgLtrapSynchronizationLength;
else ctrl_ltrap->synchronization_length = input->synchronization_length;

if (input->max_candidates <= 0) ctrl_ltrap->max_candidates = DOgLtrapMaxCandidates;
else ctrl_ltrap->max_candidates = input->max_candidates;

if (input->max_solutions <= 0) ctrl_ltrap->max_solutions = DOgLtrapMaxSolutions;
else ctrl_ltrap->max_solutions = input->max_solutions;

if (input->max_header_length <= 0) ctrl_ltrap->max_header_length = DOgLtrapHeaderLength;
else ctrl_ltrap->max_header_length = input->max_header_length;

if (input->min_swap_word_length <= 0) ctrl_ltrap->min_swap_word_length = DOgLtrapMinSwapWordLength;
else ctrl_ltrap->min_swap_word_length = input->min_swap_word_length;

/** Main loop is to get the possible sequences **/
start=0; ctrl_ltrap->span_overlap_length = 0;
for (i=start; i<iuni; i+=2) {
  c = (uni[i]<<8) + uni[i+1];
  if (OgUniIspunct(c)) {
    IFE(OgLtrapSendSequence(ctrl_ltrap,iuni,uni,start,i-start));
    ctrl_ltrap->SpanUsed=0;
    start=i+2;
    }
  else {
    IFE(overlapped=OgLtrapAddSpan(ctrl_ltrap,input,iuni,uni,i));
    if (!overlapped) {
      IFE(OgLtrapSendSequence(ctrl_ltrap,iuni,uni,start,i+2-start));
      ctrl_ltrap->SpanUsed=0;
      start=i+2;
      }
    }
  }

if (i > start) {
  IFE(OgLtrapSendSequence(ctrl_ltrap,iuni,uni,start,i-start));
  }

DONE;
}




static int OgLtrapAddSpan(struct og_ctrl_ltrap *ctrl_ltrap, struct og_ltrap_input *input, int iuni, unsigned char *uni,
    int icurrent)
{

  int language = 0, position = 0, frequency = 1;
  int overlapped = 0, end;

  ctrl_ltrap->offset_small_word = 0;

  int lexicon_type = input->lexicon_type;
  void *ha_lexicon = NULL;
  switch (lexicon_type)
  {
    case DOgLtrapLexiconTypeBase:
      ha_lexicon = ctrl_ltrap->ha_base;
      break;
    case DOgLtrapLexiconTypeSwap:
      ha_lexicon = ctrl_ltrap->ha_swap;
      ctrl_ltrap->offset_small_word = 1;
      break;
    case DOgLtrapLexiconTypePhon:
      ha_lexicon = ctrl_ltrap->ha_phon;
      break;
    case DOgLtrapLexiconTypeCompound:
      language = input->language;
      ha_lexicon = input->haut_external_dictionary;
      break;
  }

  int min_header_length = 1;
  if (lexicon_type == DOgLtrapLexiconTypeSwap) min_header_length = ctrl_ltrap->min_swap_word_length;
  int max_header_length = ctrl_ltrap->max_header_length;
  if (max_header_length < min_header_length) max_header_length = min_header_length;

  /** Looking directly for word of min_header_length to max_header_length **/
  for (int header_length = min_header_length; header_length < max_header_length; header_length++)
  {
    int unicode_header_length = header_length * 2;
    if (icurrent + unicode_header_length > iuni) break;
    int ibuffer = 0;
    unsigned char buffer[DPcPathSize];
    for (int i = 0; i < unicode_header_length; i++)
    {
      buffer[ibuffer++] = uni[icurrent + i];
    }

    buffer[ibuffer++] = 0;
    buffer[ibuffer++] = 1;

    unsigned char out[DPcAutMaxBufferSize + 9];
    oindex states[DPcAutMaxBufferSize + 9];
    int retour, nstate0, nstate1, iout;
    if ((retour = OgAufScanf(ha_lexicon, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
    {
      do
      {
        IFE(retour);
        unsigned char *p = out;
        if (lexicon_type != DOgLtrapLexiconTypeCompound)
        {
          IFE(DOgPnin4(ctrl_ltrap->herr,&p,&language));
          if (lexicon_type == DOgLtrapLexiconTypeSwap)
          {
            IFE(DOgPnin4(ctrl_ltrap->herr,&p,&position));
          }
        }
        IFE(DOgPnin4(ctrl_ltrap->herr,&p,&frequency));
        int length = ibuffer - 2;

        if (lexicon_type == DOgLtrapLexiconTypeSwap)
        {
          int iword = iout - (p - out);
          unsigned char *word = p;
          IFE(LtrapSpanAdd(ctrl_ltrap, icurrent, length, iword, word, frequency));
        }
        else
        {
          // iword = ibuffer - 2;
          // word = uni + icurrent;
          IFE(LtrapSpanAdd(ctrl_ltrap, icurrent, length, 0, 0, frequency));
        }

        if (length > 2)
        {
          if (ctrl_ltrap->span_overlap_length < icurrent + length) ctrl_ltrap->span_overlap_length = icurrent + length;
        }
      }
      while ((retour = OgAufScann(ha_lexicon, &iout, out, nstate0, &nstate1, states)));
    }
  }

  /** Then looking for all words whose length is bigger than max_header_length **/
  int unicode_header_length = max_header_length * 2;
  if (icurrent + max_header_length > iuni) return (0);
  if (icurrent + unicode_header_length <= iuni)
  {
    int ibuffer = 0;
    unsigned char buffer[DPcPathSize];
    for (int i = 0; i < unicode_header_length; i++)
    {
      buffer[ibuffer++] = uni[icurrent + i];
    }

    unsigned char out[DPcAutMaxBufferSize + 9];
    oindex states[DPcAutMaxBufferSize + 9];
    int retour, nstate0, nstate1, iout;
    if ((retour = OgAufScanf(ha_lexicon, ibuffer, buffer, &iout, out, &nstate0, &nstate1, states)))
    {
      do
      {
        IFE(retour);
        for (int i = 0; i < iout; i += 2)
        {
          int cuni = 0;
          if (icurrent + unicode_header_length + i == iuni)
          {
            end = 1;
          }
          else
          {
            end = 0;
            cuni = (uni[icurrent + unicode_header_length + i] << 8) + uni[icurrent + unicode_header_length + i + 1];
          }

          int cout = (out[i] << 8) + out[i + 1];
          if (cout == 0x1)
          {
            /** we found a span solution **/
            unsigned char *p = out + i + 2;
            if (lexicon_type != DOgLtrapLexiconTypeCompound)
            {
              IFE(DOgPnin4(ctrl_ltrap->herr,&p,&language));
              if (lexicon_type == DOgLtrapLexiconTypeSwap)
              {
                IFE(DOgPnin4(ctrl_ltrap->herr,&p,&position));
              }
            }
            IFE(DOgPnin4(ctrl_ltrap->herr,&p,&frequency));
            int length = i + unicode_header_length;

            //if (lexicon_type == DOgLtrapLexiconTypeSwap) {
            if (iout > p - out)
            {
              int iword = iout - (p - out);
              unsigned char *word = p;
              IFE(LtrapSpanAdd(ctrl_ltrap, icurrent, length, iword, word, frequency));
            }
            else
            {
              // iword = ibuffer - 2;
              // word = uni + icurrent;
              IFE(LtrapSpanAdd(ctrl_ltrap, icurrent, length, 0, 0, frequency));
            }
            if (length > 2)
            {
              if (ctrl_ltrap->span_overlap_length < icurrent + length) ctrl_ltrap->span_overlap_length = icurrent
                  + length;
            }
            break;
          }
          /** no span solution **/
          if (cout != cuni) break;
          if (end) break;
        }
      }
      while ((retour = OgAufScann(ha_lexicon, &iout, out, nstate0, &nstate1, states)));
    }
  }

  if (icurrent < ctrl_ltrap->span_overlap_length) overlapped = 1;

  return (overlapped);
}



/*
 *  we need to avoid combinatory explosion, so we analyse
 *  for a zone of maximum size synchronization_length
 *  and we keep only kept_words from the best solution
 *  we have chosen.
*/

static int OgLtrapSendSequence(struct og_ctrl_ltrap *ctrl_ltrap,int iuni,unsigned char *uni,int start0,int length0)
{
int start=start0,length;
int new_start,is_end;

while (1) {
  length=ctrl_ltrap->synchronization_length;
  if (start+length > start0+length0) length=length0+start0-start;
  /** this can only happen with = **/
  if (start+length >= start0+length0) is_end=1; else is_end=0;
  IFE(OgLtrapBuildSequence(ctrl_ltrap,iuni,uni,start,length));
  /* We need to check that otherwise OgLtrapSortSolutions
   * does not change new_start, leading to an infinite loop */
  IFn(ctrl_ltrap->SolUsed) break;
  IFE(OgLtrapSortSolutions(ctrl_ltrap,is_end,&new_start));
  IFE(LtrapSendSolutions(ctrl_ltrap,iuni,uni));
  if (is_end) break;
  start=new_start;
  }

DONE;
}




static int OgLtrapBuildSequence(struct og_ctrl_ltrap *ctrl_ltrap,int iuni,unsigned char *uni,int start,int length)
{
ctrl_ltrap->SequenceUsed = 0;
ctrl_ltrap->SpansolUsed = 0;
ctrl_ltrap->SolUsed = 0;
IFE(OgLtrapBuildSeq(ctrl_ltrap,iuni,uni,start,length,start,0));
DONE;
}



static int OgLtrapBuildSeq(struct og_ctrl_ltrap *ctrl_ltrap,int iuni,unsigned char *uni
  ,int start,int length,int icurrent,int Isequence)
{
struct span *span;
int i,must_stop;

if (icurrent > start+length) DONE;
if (icurrent == start+length) {
  #if 0
  struct sequence *sequence;
  /** Ajouter la solution qui est la liste des Ispan dans Sequence **/
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog, "OgLtrapBuildSeq: sequence of spans:");
  for (i=0; i<ctrl_ltrap->SequenceUsed; i++) {
    sequence = ctrl_ltrap->Sequence + i;
    span = ctrl_ltrap->Span + sequence->Ispan;
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "%3d: Ispan=%d pos=%d length=%d freq=%d"
      , i, sequence->Ispan, span->position
      , span->length, span->frequency);
    }
  #endif
  IFE(must_stop=OgLtrapBuildSolution(ctrl_ltrap));
  return(must_stop);
  }

for (i=0; i<ctrl_ltrap->SpanUsed; i++) {
  span = ctrl_ltrap->Span + i;
  if (span->position != icurrent) continue;
  IFE(LtrapTestReallocSequence(ctrl_ltrap,Isequence));
  ctrl_ltrap->Sequence[Isequence].Ispan = i;
  ctrl_ltrap->SequenceUsed = Isequence + 1;
  IFE(must_stop=OgLtrapBuildSeq(ctrl_ltrap,iuni,uni,start,length,icurrent+span->length,Isequence+1));
  if (must_stop) return(must_stop);
  }

DONE;
}



