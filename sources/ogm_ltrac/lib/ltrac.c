/*
 *	Main function for Linguistic Transformation compile library
 *	Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *	Dev : November 2009
 *	Version 1.0
*/
#include "ogm_ltrac.h"




PUBLIC(int) OgLtrac(void *handle,struct og_ltrac_input *input)
{
struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *)handle;
IFn(handle) DONE;

ctrl_ltrac->input = input;

IFE(LtracDicInit(ctrl_ltrac));

if (input->input_type & DOgLtracInputTypeWords) {
  //IFE(LtracWords(ctrl_ltrac));
  }
if (input->input_type & DOgLtracInputTypeAttributes) {
  IFE(LtracAttributes(ctrl_ltrac));
  }
if (input->input_type & DOgLtracInputTypeTextuals) {
  //IFE(LtracTextuals(ctrl_ltrac));
  }

if(input->add_filter_words && input->filter_dict[0]){
  IFE(LtracDicAddFilterWords(ctrl_ltrac));
  }

IFE(LtracDicWrite(ctrl_ltrac));
IFE(LtracDicFlush(ctrl_ltrac));

DONE;
}

