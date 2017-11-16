/*
 *  Main function for Linguistic Transformation compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
*/
#include "ogm_ltrac.h"




PUBLIC(int) OgLtrac(void *handle, struct og_ltrac_input *input)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  IFn(handle) DONE;

  IFE(LtracDicInit(ctrl_ltrac, input));

  IFE(LtracAdd(ctrl_ltrac, input));

  IFE(LtracDicWrite(ctrl_ltrac, input));
  IFE(LtracDicFlush(ctrl_ltrac, input));

  DONE;
}

