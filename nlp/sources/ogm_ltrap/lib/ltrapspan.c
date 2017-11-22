/*
 *	Handling span heap.
 *	Copyright (c) 2009-2010 Pertimm by Patrick Constant
 *	Dev : October 2009, March 2010
 *	Version 1.1
*/
#include "ogm_ltrap.h"


static int LtrapAllocSpan(struct og_ctrl_ltrap *,struct span **);
static int LtrapSpanGet(struct og_ctrl_ltrap *ctrl_ltrap,int position,int length,int iword,unsigned char *word,int frequency, int *pIspan);




int LtrapSpanAdd(struct og_ctrl_ltrap *ctrl_ltrap,int position,int length,int iword,unsigned char *word,int frequency)
{
int Ispan,found,is_valid,is_false;
int iw; unsigned char *w;
struct span *span;

if (iword > 0) { iw=iword; w=word; }
else { iw=length; w=ctrl_ltrap->input->uni+position; }
IFE(is_false=LtrapIsFalseWord(ctrl_ltrap,iw,w));
if (is_false) DONE;
IFE(is_valid=LtrapIsValidAsSmallWord(ctrl_ltrap,iw,w));
if (!is_valid) DONE;
/** if the span spans the whole word the span is discarded **/
if (position==0 && iw==ctrl_ltrap->input->iuni && !memcmp(w,ctrl_ltrap->input->uni+position,length)) DONE;

IFE(found=LtrapSpanGet(ctrl_ltrap,position,length,iword,word,frequency,&Ispan));
if (found) {
  span = ctrl_ltrap->Span + Ispan;
  span->frequency += frequency;
  DONE;
  }

if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceAdd) {
  int ib1,ib2; unsigned char b1[DPcPathSize],b2[DPcPathSize],b3[DPcPathSize];
  IFE(OgUniToCp(length, ctrl_ltrap->input->uni+position, DPcPathSize, &ib1, b1, DOgCodePageUTF8, 0, 0));
  if (iword > 0) {
    IFE(OgUniToCp(iword, word, DPcPathSize, &ib2, b2, DOgCodePageUTF8, 0, 0));
    sprintf(b3," with string '%s'",b2);
    }
  else b3[0]=0;
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "LtrapSpanAdd: adding span '%s' (%d,%d) freq=%d%s"
    ,b1, position, length, frequency, b3);    
  }
  
IFE(Ispan=LtrapAllocSpan(ctrl_ltrap,&span));
span->position = position;
span->length = length;
span->frequency = frequency;

if (iword > 0) {
  span->length_word = iword;
  span->start_word = ctrl_ltrap->BaUsed;
  IFE(LtrapAppendBa(ctrl_ltrap,iword,word));
  }
  
DONE;
}




static int LtrapSpanGet(struct og_ctrl_ltrap *ctrl_ltrap,int position,int length,int iword,unsigned char *word,int frequency, int *pIspan)
{
struct span *span;
int i;

*pIspan=(-1);

if (ctrl_ltrap->SpanUsed<=0) return(0);
for (i=ctrl_ltrap->SpanUsed-1; i>=0; i--) {
  span = ctrl_ltrap->Span + i;
  if (span->position == position && span->length == length && span->length_word == iword) {
    int same=0;
    IFn(iword) same=1;
    else if (!memcmp(ctrl_ltrap->Ba+span->start_word,word,iword)) same=1;
    if (same) {
      *pIspan=i;
      return(1);
      }
    }
  }

return(0);
}





static int LtrapAllocSpan(ctrl_ltrap,pspan)
struct og_ctrl_ltrap *ctrl_ltrap;
struct span **pspan;
{
char erreur[DOgErrorSize];
struct span *span = 0;
int i=ctrl_ltrap->SpanNumber;

beginLtrapAllocSpan:

if (ctrl_ltrap->SpanUsed < ctrl_ltrap->SpanNumber) {
  i = ctrl_ltrap->SpanUsed++; 
  }

if (i == ctrl_ltrap->SpanNumber) {
  unsigned a, b; struct span *og_l;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSpan: max Span number (%d) reached"
      , ctrl_ltrap->SpanNumber);
    }
  a = ctrl_ltrap->SpanNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct span *)malloc(b*sizeof(struct span))) {
    sprintf(erreur,"LtrapAllocSpan: malloc error on Span");
    OgErr(ctrl_ltrap->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_ltrap->Span, a*sizeof(struct span));
  DPcFree(ctrl_ltrap->Span); ctrl_ltrap->Span = og_l;
  ctrl_ltrap->SpanNumber = b;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSpan: new Span number is %d\n", ctrl_ltrap->SpanNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"LtrapAllocSpan: SpanNumber reached (%d)",ctrl_ltrap->SpanNumber);
  OgErr(ctrl_ltrap->herr,erreur); DPcErr;
#endif

  goto beginLtrapAllocSpan;
  }

span = ctrl_ltrap->Span + i;
memset(span,0,sizeof(struct span));
span->start_word=(-1);

if (pspan) *pspan = span;

return(i);
}


