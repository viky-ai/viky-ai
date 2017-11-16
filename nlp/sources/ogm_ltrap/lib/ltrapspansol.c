/*
 *	Handling spansol heap.
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : October 2009
 *	Version 1.0
*/
#include "ogm_ltrap.h"


static int LtrapAllocSpansol(struct og_ctrl_ltrap *,struct spansol **);




int LtrapSpansolAdd(struct og_ctrl_ltrap *ctrl_ltrap,int Isol,int Ispan,int position,int length,int frequency)
{
int Ispansol;
struct spansol *spansol;
struct sol *sol = ctrl_ltrap->Sol + Isol;

IFE(Ispansol=LtrapAllocSpansol(ctrl_ltrap,&spansol));
spansol->position = position;
spansol->length = length;
spansol->frequency = frequency;
spansol->Ispan = Ispan;

if (sol->start_spansol < 0) {
  sol->start_spansol = Ispansol;
  sol->length_spansol = 1;
  }
else {
  sol->length_spansol++;
  }

return(Ispansol);
}




static int LtrapAllocSpansol(ctrl_ltrap,pspansol)
struct og_ctrl_ltrap *ctrl_ltrap;
struct spansol **pspansol;
{
char erreur[DOgErrorSize];
struct spansol *spansol = 0;
int i=ctrl_ltrap->SpansolNumber;

beginLtrapAllocSpansol:

if (ctrl_ltrap->SpansolUsed < ctrl_ltrap->SpansolNumber) {
  i = ctrl_ltrap->SpansolUsed++; 
  }

if (i == ctrl_ltrap->SpansolNumber) {
  unsigned a, b; struct spansol *og_l;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSpansol: max Spansol number (%d) reached"
      , ctrl_ltrap->SpansolNumber);
    }
  a = ctrl_ltrap->SpansolNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct spansol *)malloc(b*sizeof(struct spansol))) {
    sprintf(erreur,"LtrapAllocSpansol: malloc error on Spansol");
    OgErr(ctrl_ltrap->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_ltrap->Spansol, a*sizeof(struct spansol));
  DPcFree(ctrl_ltrap->Spansol); ctrl_ltrap->Spansol = og_l;
  ctrl_ltrap->SpansolNumber = b;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSpansol: new Spansol number is %d\n", ctrl_ltrap->SpansolNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"LtrapAllocSpansol: SpansolNumber reached (%d)",ctrl_ltrap->SpansolNumber);
  OgErr(ctrl_ltrap->herr,erreur); DPcErr;
#endif

  goto beginLtrapAllocSpansol;
  }

spansol = ctrl_ltrap->Spansol + i;
memset(spansol,0,sizeof(struct spansol));

if (pspansol) *pspansol = spansol;

return(i);
}

