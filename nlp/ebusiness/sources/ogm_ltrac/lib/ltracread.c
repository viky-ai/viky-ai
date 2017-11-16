/*
 * ltracread.c *
 *  Created on: 22 févr. 2017
 *      Author: caroline
 */

#include "ogm_ltrac.h"
#include <logis639_3166.h>

struct og_ltrac_entry
{
  int iword;
  char *word;
  int language_code;
  int frequency;
};

struct og_ltrac_update_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int language_code;
  int frequency;
};

struct og_ltrac_found_context
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  int language_code;
  og_bool found;
};

struct og_ltrac_lip_ctx
{
  struct og_ctrl_ltrac *ctrl_ltrac;
  struct read_ltraf_context *ltraf_ctx;

  og_bool is_expression;
  og_bool missing_expression_part;

  int inormalize_expression;
  unsigned char normalize_expression[DPcPathSize];
};

static og_bool LtracGetLtraf(struct og_ctrl_ltrac *ctrl_ltrac, int ientry, unsigned char *entry, int *pIltraf);
static og_status LtracAllocLtraf(struct og_ctrl_ltrac *ctrl_ltrac, struct ltraf **pltraf);
static int LtracAddExpressionLipCallback(void *context, int Ilip_word);
static og_status LtracAddEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, og_bool is_ltraf);
static og_status LtracScanWord(struct og_ctrl_ltrac *ctrl_ltrac, int iword, og_string word,
    int (*func)(void *context, struct og_ltrac_scan *scan), void *context);
static og_status LtracIsFound(void *context, struct og_ltrac_scan *scan);

og_status LtracReadLtrafs(struct og_ctrl_ltrac *ctrl_ltrac, og_string filename, int min_frequency,
    int (*func)(struct og_ctrl_ltrac *ctrl_ltrac, struct read_ltraf_context *ctx))
{
  char ltrac_dir[DPcPathSize];
  sprintf(ltrac_dir, "%s/%s", ctrl_ltrac->data_directory, "ltra");

  char ltraf_file[DPcPathSize];
  sprintf(ltraf_file, "%s/%s", ltrac_dir, filename);
  if (!OgFileExists(ltraf_file))
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtracReadLtrafs: file '%s' does not exist.", ltraf_file);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  FILE *fd = fopen(ltraf_file, "rb");
  IFn(fd)
  {
    char erreur[DOgErrorSize];
    sprintf(erreur, "LtracReadLtrafs: Impossible to open '%s'", ltraf_file);
    OgErr(ctrl_ltrac->herr, erreur);
    DPcErr;
  }

  unsigned char utf8[DOgLtracMaxWordsSize];

  int frequency = 0;
  int language_code = DOgLangNil;
  unsigned char buffer[DOgLtracMaxWordsSize];

  OgTrimString(utf8, utf8);
  int iutf8 = strlen(utf8);

  int i, n, c, start, end, start_word;
  while (fgets(utf8, DOgLtracMaxWordsSize, fd))
  {
    OgTrimString(utf8, utf8);
    iutf8 = strlen(utf8);
    for (n = 0, start = 0, end = 0, start_word = (-1), i = 0; !end; i++)
    {
      if (i >= iutf8)
      {
        end = 1;
        c = ':';
      }
      else c = utf8[i];
      if (c == ':')
      {
        sprintf(buffer, "%.*s", i - start, utf8 + start);
        switch (n)
        {
          case 0:
            frequency = atoi(buffer);
            break;
          case 1:
            language_code = OgCodeToIso639_3166(ctrl_ltrac->herr, buffer);
            start_word = i + 1;
            break;
        }
        if (start_word >= 0) break;
        start = i + 1;
        n++;
      }
    }
    if (start_word < 0) continue;
    if (frequency < min_frequency) continue;

    unsigned char entry[DOgLtracMaxWordsSize];
    int ientry;
    IFE(OgCpToUni(iutf8-start_word,utf8+start_word,DOgLtracMaxWordsSize,&ientry,entry,DOgCodePageUTF8,0,0));

    struct read_ltraf_context ctx[1];
    memset(ctx, 0, sizeof(struct read_ltraf_context));
    ctx->ientry = ientry;
    ctx->entry = entry;
    ctx->language_code = language_code;
    ctx->frequency = frequency;
    ctx->min_frequency = min_frequency;

    IFE(func(ctrl_ltrac, ctx));
  }

  fclose(fd);

  DONE;
}

og_status LtracAddLtrafEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct read_ltraf_context *ctx)
{
  struct ltrac_dic_input dic_input[1];
  memset(dic_input, 0, sizeof(struct ltrac_dic_input));
  dic_input->value_length = ctx->ientry;
  dic_input->value = ctx->entry;
  dic_input->language_code = ctx->language_code;
  dic_input->is_expression = FALSE;

  if (ctrl_ltrac->has_ltraf_requests)
  {
    dic_input->frequency = ctx->min_frequency;
  }
  else
  {
    dic_input->frequency = ctx->frequency;
  }

  IFE(LtracAddEntry(ctrl_ltrac, dic_input, TRUE));

  DONE;
}

og_status LtracAddLtrafRequestEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct read_ltraf_context *ctx)
{
  struct og_ltrac_lip_ctx lip_ctx[1];
  memset(lip_ctx, 0, sizeof(struct og_ltrac_lip_ctx));
  lip_ctx->ctrl_ltrac = ctrl_ltrac;
  lip_ctx->ltraf_ctx = ctx;

  struct og_lip_input lip_input[1];
  memset(lip_input, 0, sizeof(struct og_lip_input));
  lip_input->content_length = ctx->ientry;
  lip_input->content = (unsigned char *) ctx->entry;
  lip_input->word_func = LtracAddExpressionLipCallback;
  lip_input->context = lip_ctx;

  IFE(OgLip(ctrl_ltrac->hlip, lip_input));

  if (!lip_ctx->missing_expression_part)
  {
    // use normalized value instead
    struct ltrac_dic_input dic_input[1];
    memset(dic_input, 0, sizeof(struct ltrac_dic_input));
    dic_input->language_code = ctx->language_code;
    dic_input->frequency = ctx->frequency;
    dic_input->value = lip_ctx->normalize_expression;
    dic_input->value_length = lip_ctx->inormalize_expression;
    dic_input->is_expression = lip_ctx->is_expression;

    IFE(LtracAddEntry(ctrl_ltrac, dic_input, FALSE));
  }
  else
  {
    if (ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd)
    {
      int ibuffer = 0;
      unsigned char buffer[DPcPathSize];

      int inormalize_buffer = 0;
      unsigned char normalize_buffer[DPcPathSize];
      unsigned char lang[3];

      OgIso639_3166ToCode(ctx->language_code, lang);

      IFE(OgUniToCp(ctx->ientry, ctx->entry, DPcPathSize, &ibuffer, buffer, DOgCodePageUTF8, 0, 0));
      IFE(
          OgUniToCp(lip_ctx->inormalize_expression, lip_ctx->normalize_expression, DPcPathSize, &inormalize_buffer, normalize_buffer, DOgCodePageUTF8, 0, 0));

      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog,
          "LtracAddExpression: expression \"%s\" in %s normalize in \"%s\" discarded", buffer, lang, normalize_buffer);
    }
  }

  DONE;

}

static og_status LtracAddEntry(struct og_ctrl_ltrac *ctrl_ltrac, struct ltrac_dic_input *dic_input, og_bool is_ltraf)
{

  unsigned char entry[DOgLtracMaxWordsSize];
  memcpy(entry, dic_input->value, dic_input->value_length);
  unsigned char *p = entry + dic_input->value_length;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;

  OggNout(dic_input->language_code, &p);

  int ientry = p - entry;

  int Iltraf;
  og_bool found = LtracGetLtraf(ctrl_ltrac, ientry, entry, &Iltraf);
  IFE(found);

  struct ltraf *ltraf = NULL;
  if (found)
  {
    ltraf = ctrl_ltrac->Ltraf + Iltraf;
  }
  else
  {
    Iltraf = LtracAllocLtraf(ctrl_ltrac, &ltraf);
    IFE(Iltraf);

    OggNout(Iltraf, &p);
    OggNout(dic_input->is_expression, &p);

    ientry = p - entry;
    IFE(OgAutAdd(ctrl_ltrac->ha_ltrac, ientry, entry));
  }

  if (!is_ltraf && ltraf->original_frequency)
  {
    // ltraf_request.txt override freq
    ltraf->frequency = dic_input->frequency;
    ltraf->original_frequency = FALSE;
  }
  else
  {
    ltraf->frequency += dic_input->frequency;
  }

  DONE;
}

static og_bool LtracGetLtraf(struct og_ctrl_ltrac *ctrl_ltrac, int ientry, unsigned char *entry, int *pIltraf)
{
  unsigned char *p, out[DOgLtracMaxWordsSize];
  oindex states[DOgLtracMaxWordsSize];
  int retour, nstate0, nstate1, iout;

  *pIltraf = (-1);

  if ((retour = OgAutScanf(ctrl_ltrac->ha_ltrac, ientry, entry, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      p = out;
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,pIltraf));
      return TRUE;
    }
    while ((retour = OgAutScann(ctrl_ltrac->ha_ltrac, &iout, out, nstate0, &nstate1, states)));
  }

  return FALSE;
}

static og_status LtracAllocLtraf(struct og_ctrl_ltrac *ctrl_ltrac, struct ltraf **pltraf)
{
  char erreur[DOgErrorSize];
  struct ltraf *ltraf = 0;
  int i = ctrl_ltrac->LtrafNumber;

  beginAllocLtraf:

  if (ctrl_ltrac->LtrafUsed < ctrl_ltrac->LtrafNumber)
  {
    i = ctrl_ltrac->LtrafUsed++;
  }

  if (i == ctrl_ltrac->LtrafNumber)
  {
    unsigned a, b;
    struct ltraf *og_l;

    if (ctrl_ltrac->loginfo->trace & DOgSidxTraceMemory)
    {
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracAllocLtraf: max Ltraf number (%d) reached",
          ctrl_ltrac->LtrafNumber);
    }
    a = ctrl_ltrac->LtrafNumber;
    b = a + (a >> 2) + 1;
    IFn(og_l=(struct ltraf *)malloc(b*sizeof(struct ltraf)))
    {
      sprintf(erreur, "LtracAllocLtraf: malloc error on Ltraf");
      OgErr(ctrl_ltrac->herr, erreur);
      DPcErr;
    }

    memcpy(og_l, ctrl_ltrac->Ltraf, a * sizeof(struct ltraf));
    DPcFree(ctrl_ltrac->Ltraf);
    ctrl_ltrac->Ltraf = og_l;
    ctrl_ltrac->LtrafNumber = b;

    if (ctrl_ltrac->loginfo->trace & DOgSidxTraceMemory)
    {
      OgMsg(ctrl_ltrac->hmsg, "", DOgMsgDestInLog, "LtracAllocLtraf: new Ltraf number is %d\n",
          ctrl_ltrac->LtrafNumber);
    }

    goto beginAllocLtraf;
  }

  ltraf = ctrl_ltrac->Ltraf + i;
  memset(ltraf, 0, sizeof(struct ltraf));

  ltraf->original_frequency = TRUE;

  if (pltraf) *pltraf = ltraf;
  return (i);
}

static og_status LtracScanWord(struct og_ctrl_ltrac *ctrl_ltrac, int iword, og_string word,
    int (*func)(void *context, struct og_ltrac_scan *scan), void *context)
{
  oindex states[DOgLtracMaxWordsSize];
  int retour, nstate0, nstate1, iout;
  unsigned char out[DOgLtracMaxWordsSize];

  unsigned char entry[DOgLtracMaxWordsSize];
  memcpy(entry, word, iword);
  unsigned char *p = entry + iword;
  *p++ = 0;
  *p++ = DOgLtracExtStringSeparator;

  int ientry = p - entry;

  if ((retour = OgAutScanf(ctrl_ltrac->ha_ltrac, ientry, entry, &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      p = out;

      struct og_ltrac_scan scan[1];
      memset(scan, 0, sizeof(struct og_ltrac_scan));

      scan->iword = iword;
      scan->word = word;

      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->language_code));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->Iltraf));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->is_expression));

      IFE(func(context, scan));
    }
    while ((retour = OgAutScann(ctrl_ltrac->ha_ltrac, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

og_status LtracScan(struct og_ctrl_ltrac *ctrl_ltrac, int (*func)(void *context, struct og_ltrac_scan *scan),
    void *context)
{
  oindex states[DOgLtracMaxWordsSize];
  int retour, nstate0, nstate1, iout;
  unsigned char out[DOgLtracMaxWordsSize];
  if ((retour = OgAutScanf(ctrl_ltrac->ha_ltrac, (-1), "", &iout, out, &nstate0, &nstate1, states)))
  {
    do
    {
      IFE(retour);
      int sep = (-1);
      for (int i = 0; i < iout; i += 2)
      {
        int c = (out[i] << 8) + out[i + 1];
        if (c == DOgLtracExtStringSeparator)
        {
          sep = i;
          break;
        }
      }
      if (sep < 0) continue;
      unsigned char *p = out + sep + 2;

      struct og_ltrac_scan scan[1];
      memset(scan, 0, sizeof(struct og_ltrac_scan));

      scan->iword = sep;
      scan->word = out;

      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->language_code));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->Iltraf));
      IFE(DOgPnin4(ctrl_ltrac->herr,&p,&scan->is_expression));

      IFE(func(context, scan));
    }
    while ((retour = OgAutScann(ctrl_ltrac->ha_ltrac, &iout, out, nstate0, &nstate1, states)));
  }

  DONE;
}

static int LtracAddExpressionLipCallback(void *context, int Ilip_word)
{
  struct og_ltrac_lip_ctx *lip_ctx = (struct og_ltrac_lip_ctx *) context;
  struct og_ctrl_ltrac *ctrl_ltrac = lip_ctx->ctrl_ltrac;

  struct og_lip_word lip_word[1];

  IFE(OgLipGetWord(ctrl_ltrac->hlip, Ilip_word, lip_word));

  unsigned char *word = lip_word->input->content + lip_word->start;
  int iword = lip_word->length;

  OgUniStrlwr(iword, word, word);

  // check if all word are in ltraf.txt
  struct og_ltrac_found_context found_ctx[1];
  memset(found_ctx, 0, sizeof(struct og_ltrac_found_context));
  found_ctx->ctrl_ltrac = ctrl_ltrac;
  found_ctx->language_code = lip_ctx->ltraf_ctx->language_code;
  found_ctx->found = FALSE;

  IFE(LtracScanWord(ctrl_ltrac, iword, word, LtracIsFound, found_ctx));
  if (!found_ctx->found)
  {
    lip_ctx->missing_expression_part = TRUE;

    // continue to build expression with trace enable to log it later
    if (!(ctrl_ltrac->loginfo->trace & DOgLtracTraceAdd))
    {
      return TRUE;
    }
  }

  // cat each lip word to build a normalize expresssion
  unsigned char *p = lip_ctx->normalize_expression + lip_ctx->inormalize_expression;
  if (lip_ctx->inormalize_expression > 0)
  {
    *p++ = '\0';
    *p++ = ' ';
    lip_ctx->is_expression = TRUE;
  }
  memcpy(p, word, iword);
  p = p + iword;
  lip_ctx->inormalize_expression = p - lip_ctx->normalize_expression;

  DONE;

}

static og_status LtracIsFound(void *context, struct og_ltrac_scan *scan)
{
  struct og_ltrac_found_context *ctx = (struct og_ltrac_found_context *) context;

  if ((ctx->language_code == scan->language_code) || (scan->language_code == DOgLangNil))
  {
    ctx->found = TRUE;
  }

  DONE;
}
