/*
 *  Handling of pawo structure (attribut-value) for statistics
 *  Copyright (c) 2006 Pertimm by Patrick Constant
 *  Dev : October 2006
 *  Version 1.0
*/
#include "ogm_lip.h"



static int AllocPawo(struct og_ctrl_lip *, struct pawo **);





int LipAddPawo(struct og_ctrl_lip *ctrl_lip, int start, int length, int next_char, int punctuation_before_word,
    int Iexpression_group, int is_indivisible_expression)
{
struct og_lip_rpos *rpos = ctrl_lip->input->rpos;
int Ipawo,end,real_end;
struct pawo *pawo;

IFE(Ipawo=AllocPawo(ctrl_lip,&pawo));
pawo->length=length;
pawo->start=start;

IFn(rpos) {
  pawo->real_start=start;
  pawo->real_length=length;
  }
else {
  pawo->real_start=rpos[start>>1].real_position;
  end = start+length; real_end=rpos[end>>1].real_position;
  pawo->real_length=real_end-pawo->real_start;
  }

if (next_char=='\'') pawo->has_quote=1;
else if (next_char==0x2019) pawo->has_quote=1; /* RIGHT SINGLE QUOTATION MARK */
else if (next_char=='.') pawo->has_dot_separation=1;

//if (punctuation_before_word) {
//  if (Ipawo > 0) {
//    struct pawo *pawo_before = ctrl_lip->Pawo + Ipawo - 1;
//    if (pawo_before->has_quote) {
//      if (punctuation_before_word=='\'' || punctuation_before_word==0x2019) punctuation_before_word=0;
//      }
//    }
//  }
pawo->punctuation_before_word=punctuation_before_word;

pawo->Iexpression_group = Iexpression_group;
pawo->is_indivisible_expression = is_indivisible_expression;

return(Ipawo);
}





static int AllocPawo(struct og_ctrl_lip *ctrl_lip, struct pawo **ppawo)
{
char erreur[DOgErrorSize];
int i=ctrl_lip->PawoNumber;
struct pawo *pawo=0;

beginAllocPawo:

if (ctrl_lip->PawoUsed < ctrl_lip->PawoNumber) {
  i = ctrl_lip->PawoUsed++; 
  }

if (i == ctrl_lip->PawoNumber) {
  unsigned a, b; struct pawo *og_l;

  if (ctrl_lip->loginfo->trace & DOgLipTraceMemory) {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog
                , "AllocPawo: max Pawo number (%d) reached"
                , ctrl_lip->PawoNumber);
    }
  a = ctrl_lip->PawoNumber; b = a + (a>>2) + 1;
  IFn(og_l=(struct pawo *)malloc(b*sizeof(struct pawo))) {
    sprintf(erreur,"AllocPawo: malloc error on Pawo");
    OgErr(ctrl_lip->herr,erreur); DPcErr;
    }

  memcpy( og_l, ctrl_lip->Pawo, a*sizeof(struct pawo));
  DPcFree(ctrl_lip->Pawo); ctrl_lip->Pawo = og_l;
  ctrl_lip->PawoNumber = b;

  if (ctrl_lip->loginfo->trace & DOgLipTraceMemory) {
    OgMsg(ctrl_lip->hmsg, "", DOgMsgDestInLog
                , "AllocPawo: new Pawo number is %d\n", ctrl_lip->PawoNumber);
    }

#ifdef DOgNoMainBufferReallocation
  sprintf(erreur,"AllocPawo: PawoNumber reached (%d)",ctrl_lip->PawoNumber);
  OgErr(ctrl_lip->herr,erreur); DPcErr;
#endif

  goto beginAllocPawo;
  }

pawo = ctrl_lip->Pawo + i;
memset(pawo,0,sizeof(struct pawo));

if (ppawo) *ppawo = pawo;
return(i);
}


