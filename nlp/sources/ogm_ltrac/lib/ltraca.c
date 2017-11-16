/*
 *  Main function for Ltrac compile library
 *  Copyright (c) 2009 Pertimm, Inc. by Patrick Constant
 *  Dev : November 2009
 *  Version 1.0
 */
#include "ogm_ltrac.h"

#define DOgLtracMaxValueLength  0x800

static int LtracAddInDictionary(void *context, struct og_ltrac_scan *scan);

struct merge_frequency_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int min_frequency;
};

struct add_expressions_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
};

struct best_frequency_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int language_code;
  int best_frequency;
};

struct is_found_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int language_code;
  og_bool found;
};

/*
 *  Reads the items.ory, but also the atva in matrix.
 *  So we need to read that as well.
 */

struct ltrac_add_ctx
{
    struct og_ctrl_ltrac *ctrl_ltrac;
    struct og_ltrac_input *input;
};

int LtracAdd(struct og_ctrl_ltrac *ctrl_ltrac, struct og_ltrac_input *input)
{
  struct ltrac_add_ctx ctx[1];
  memset(ctx, 0, sizeof(struct ltrac_add_ctx));
  ctx->ctrl_ltrac = ctrl_ltrac;
  ctx->input = input;

  IFE(LtracReadLtrafs(ctrl_ltrac, "ltraf.txt", input->min_frequency, LtracAddLtrafEntry));
  if (ctrl_ltrac->has_ltraf_requests)
  {
    IFE(LtracReadLtrafs(ctrl_ltrac, "ltraf_requests.txt", input->min_frequency, LtracAddLtrafRequestEntry));
  }
  IFE(LtracScan(ctrl_ltrac, LtracAddInDictionary, ctx));

  DONE;
}

static int LtracAddInDictionary(void *context, struct og_ltrac_scan *scan)
{
  struct ltrac_dic_input cdic_input, *dic_input = &cdic_input;
  struct ltrac_add_ctx *ctx = (struct ltrac_add_ctx *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = ctx->ctrl_ltrac;

  memset(dic_input, 0, sizeof(struct ltrac_dic_input));
  dic_input->value_length = scan->iword;
  dic_input->value = scan->word;
  dic_input->language_code = scan->language_code;
  dic_input->is_expression = scan->is_expression;

  struct ltraf *ltraf = ctrl_ltrac->Ltraf + scan->Iltraf;
  dic_input->frequency = ltraf->frequency;

  if (dic_input->value_length <= 0) DONE;
  if (dic_input->frequency < ctx->input->min_frequency) DONE;

  IFE(LtracDicAdd(ctrl_ltrac, ctx->input, dic_input));

  DONE;
}
