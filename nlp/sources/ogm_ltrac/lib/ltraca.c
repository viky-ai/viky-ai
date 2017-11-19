/*
 *  Main function for Ltrac compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
 */
#include "ogm_ltrac.h"

PUBLIC(int) OgLtracAddWord(void *handle, struct og_ltrac_word_input *word_input)
{
  struct og_ctrl_ltrac *ctrl_ltrac = (struct og_ctrl_ltrac *) handle;
  struct ltrac_dic_input dic_input[1];

  memset(dic_input, 0, sizeof(struct ltrac_dic_input));
  dic_input->value_length = word_input->value_length;
  dic_input->value = word_input->value;
  dic_input->language_code = word_input->language_code;
  dic_input->is_expression = 0;

  dic_input->frequency = word_input->frequency;

  IFE(LtracDicAdd(ctrl_ltrac, dic_input));

  DONE;
}

