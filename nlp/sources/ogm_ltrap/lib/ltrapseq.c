/*
 *	Handling sequence heap.
 *	Copyright (c) 2009 Pertimm by Patrick Constant
 *	Dev : October 2009
 *	Version 1.0
*/
#include "ogm_ltrap.h"


static int LtrapAllocSequence(struct og_ctrl_ltrap *,struct sequence **);



int LtrapTestReallocSequence(struct og_ctrl_ltrap *ctrl_ltrap,int Isequence)
{
while (1) {
  if (Isequence < ctrl_ltrap->SequenceUsed) DONE;
  IFE(LtrapAllocSequence(ctrl_ltrap,0));
  }
DONE;
}



static int LtrapAllocSequence(ctrl_ltrap,psequence)
struct og_ctrl_ltrap *ctrl_ltrap;
struct sequence **psequence;
{
char erreur[DOgErrorSize];
struct sequence *sequence = 0;
int i=ctrl_ltrap->SequenceNumber;

beginLtrapAllocSequence:

if (ctrl_ltrap->SequenceUsed < ctrl_ltrap->SequenceNumber) {
  i = ctrl_ltrap->SequenceUsed++; 
  }

if (i == ctrl_ltrap->SequenceNumber) {
  unsigned a, b; struct sequence *og_l;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSequence: max Sequence number (%d) reached"
      , ctrl_ltrap->SequenceNumber);
    }
  a = ctrl_ltrap->SequenceNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct sequence *)malloc(b*sizeof(struct sequence))) {
    sprintf(erreur,"LtrapAllocSequence: malloc error on Sequence");
    OgErr(ctrl_ltrap->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_ltrap->Sequence, a*sizeof(struct sequence));
  DPcFree(ctrl_ltrap->Sequence); ctrl_ltrap->Sequence = og_l;
  ctrl_ltrap->SequenceNumber = b;

  if (ctrl_ltrap->loginfo->trace & DOgLtrapTraceMemory) {
    OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
      , "LtrapAllocSequence: new Sequence number is %d\n", ctrl_ltrap->SequenceNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"LtrapAllocSequence: SequenceNumber reached (%d)",ctrl_ltrap->SequenceNumber);
  OgErr(ctrl_ltrap->herr,erreur); DPcErr;
#endif

  goto beginLtrapAllocSequence;
  }

sequence = ctrl_ltrap->Sequence + i;
memset(sequence,0,sizeof(struct sequence));

if (psequence) *psequence = sequence;

return(i);
}


