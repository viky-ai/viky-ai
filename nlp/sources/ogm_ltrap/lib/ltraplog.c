/*
 *  Handling log functions.
 *  Copyright (c) 2009 Pertimm by Patrick Constant
 *  Dev : October 2009
 *  Version 1.0
*/
#include "ogm_ltrap.h"



int LtrapSolsLog(struct og_ctrl_ltrap *ctrl_ltrap)
{
int i;
for (i=0; i<ctrl_ltrap->SolUsed; i++) {
  IFE(LtrapSolLog(ctrl_ltrap,i));
  }
DONE;
}



int LtrapSolLog(struct og_ctrl_ltrap *ctrl_ltrap, int Isol)
{
int ib1,ib2; unsigned char b1[DPcPathSize],b2[DPcPathSize],b3[DPcPathSize];
struct sol *sol = ctrl_ltrap->Sol + Isol;
struct spansol *spansol;
struct span *span;
int i;

sprintf(b1,"%.2f",sol->weight);
sprintf(b2,"%.2f",sol->global_score);

OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
  , "Sol %d (weight=%s global_score=%s global_frequency=%d complete=%d):"
  , Isol, b1, b2, sol->global_frequency, sol->complete);

for (i=0; i<sol->length_spansol; i++) {
  spansol = ctrl_ltrap->Spansol + sol->start_spansol + i;
  span = ctrl_ltrap->Span + spansol->Ispan;
  IFE(OgUniToCp(spansol->length, ctrl_ltrap->input->uni+spansol->position, DPcPathSize
      , &ib1, b1, DOgCodePageUTF8, 0, 0));
  if (span->length_word > 0) {
    IFE(OgUniToCp(span->length_word, ctrl_ltrap->Ba+span->start_word, DPcPathSize, &ib2, b2, DOgCodePageUTF8, 0, 0));
    sprintf(b3," '%s'",b2);
    }
    else b3[0]=0;
  OgMsg(ctrl_ltrap->hmsg,"",DOgMsgDestInLog
    , "  %3d: '%s' pos=%d length=%d freq=%d%s"
    , i, b1, spansol->position, spansol->length, spansol->frequency,b3);
  }

DONE;
}
